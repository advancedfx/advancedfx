//! HTTPS download via WinHTTP with the system proxy settings (NOTES.md 6.8a).
//!
//! - `WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY`: system + per-user proxy settings incl. PAC / WPAD;
//!   fallback to the IE proxy configuration where that isn't supported (e.g. Wine).
//! - Proxy authentication with the logged-in user's Windows credentials (Negotiate / NTLM),
//!   only ever sent to the proxy (`WINHTTP_AUTOLOGON_SECURITY_LEVEL_PROXY_ONLY`).
//! - HTTPS only, TLS 1.2 or newer, Windows certificate store. Redirects are followed (not to HTTP).
//! - Synchronous, streamed to a file in chunks; cancel is checked between chunks.
//!
//! Integrity doesn't depend on this: callers verify the SHA-256 of what was downloaded.

use std::ffi::{OsStr, c_void};
use std::fs::File;
use std::io::{BufWriter, Write};
use std::os::windows::ffi::OsStrExt;
use std::path::Path;
use std::ptr::{null, null_mut};
use std::sync::atomic::{AtomicBool, Ordering};

use windows_sys::Win32::Foundation::{ERROR_INVALID_PARAMETER, GlobalFree};
use windows_sys::Win32::Networking::WinHttp::*;

use crate::error::{Error, IoContext, Result};

/// Read chunk size; also the cancel granularity.
const CHUNK: usize = 256 * 1024;
/// Connect / send / receive timeouts.
const TIMEOUT_MS: i32 = 60_000;

fn wide(s: &str) -> Vec<u16> {
    OsStr::new(s).encode_wide().chain(Some(0)).collect()
}

/// Closes a WinHTTP handle on drop.
struct Handle(*mut c_void);

impl Drop for Handle {
    fn drop(&mut self) {
        if !self.0.is_null() {
            // SAFETY: owned WinHTTP handle.
            unsafe { WinHttpCloseHandle(self.0) };
        }
    }
}

fn last_error(url: &str, op: &str) -> Error {
    let code = std::io::Error::last_os_error().raw_os_error().unwrap_or(0) as u32;
    let reason = match code {
        ERROR_WINHTTP_NAME_NOT_RESOLVED => {
            "server name could not be resolved (no internet / DNS?)".to_owned()
        }
        ERROR_WINHTTP_CANNOT_CONNECT => "could not connect to the server".to_owned(),
        ERROR_WINHTTP_TIMEOUT => "the request timed out".to_owned(),
        ERROR_WINHTTP_CONNECTION_ERROR => "the connection was reset".to_owned(),
        ERROR_WINHTTP_SECURE_FAILURE => "TLS / certificate error".to_owned(),
        ERROR_WINHTTP_SECURE_FAILURE_PROXY => "TLS / certificate error with the proxy".to_owned(),
        _ => std::io::Error::from_raw_os_error(code as i32).to_string(),
    };
    Error::Download {
        url: url.to_owned(),
        reason: format!("{op}: {reason} (error {code})"),
    }
}

fn download_error(url: &str, reason: impl Into<String>) -> Error {
    Error::Download {
        url: url.to_owned(),
        reason: reason.into(),
    }
}

/// `https://host[:port]/path?query` → (host, port, path with query).
fn split_https_url(url: &str) -> Option<(String, u16, String)> {
    let rest = url.strip_prefix("https://")?;
    let (authority, path) = match rest.find('/') {
        Some(i) => (&rest[..i], &rest[i..]),
        None => (rest, "/"),
    };
    if authority.is_empty() || authority.contains('@') {
        return None;
    }
    let (host, port) = match authority.rsplit_once(':') {
        Some((host, port)) => (host, port.parse().ok()?),
        None => (authority, 443),
    };
    Some((host.to_owned(), port, path.to_owned()))
}

/// Downloads `url` (HTTPS) to `dest` (created / replaced). `progress(done, total)` is called per chunk;
/// setting `cancel` aborts with [`Error::Cancelled`]. Returns the number of bytes written.
/// On error `dest` is removed again.
pub fn download(
    url: &str,
    dest: &Path,
    cancel: &AtomicBool,
    mut progress: impl FnMut(u64, Option<u64>),
) -> Result<u64> {
    let result = download_inner(url, dest, cancel, &mut progress);
    if result.is_err() {
        let _ = std::fs::remove_file(dest);
    }
    result
}

fn download_inner(
    url: &str,
    dest: &Path,
    cancel: &AtomicBool,
    progress: &mut dyn FnMut(u64, Option<u64>),
) -> Result<u64> {
    let (host, port, path) = split_https_url(url)
        .ok_or_else(|| download_error(url, "only plain https:// URLs are supported"))?;
    let agent = wide(&format!("HLAE_Setup/{}", env!("CARGO_PKG_VERSION")));

    // SAFETY (whole block): WinHTTP calls with valid NUL-terminated strings and buffers that
    // outlive the calls; all handles are owned by `Handle`s and closed in reverse order.
    unsafe {
        let mut session = Handle(WinHttpOpen(
            agent.as_ptr(),
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
            null(),
            null(),
            0,
        ));
        let mut manual_proxy = false;
        if session.0.is_null() {
            if std::io::Error::last_os_error().raw_os_error()
                != Some(ERROR_INVALID_PARAMETER as i32)
            {
                return Err(last_error(url, "WinHttpOpen"));
            }
            // AUTOMATIC_PROXY unknown (e.g. Wine): resolve the IE proxy settings per request.
            log::info!("automatic proxy configuration not supported, using the IE proxy settings");
            session = Handle(WinHttpOpen(
                agent.as_ptr(),
                WINHTTP_ACCESS_TYPE_NO_PROXY,
                null(),
                null(),
                0,
            ));
            if session.0.is_null() {
                return Err(last_error(url, "WinHttpOpen"));
            }
            manual_proxy = true;
        }

        set_tls_protocols(&session).map_err(|_| last_error(url, "set TLS protocols"))?;
        WinHttpSetTimeouts(session.0, TIMEOUT_MS, TIMEOUT_MS, TIMEOUT_MS, TIMEOUT_MS);

        let host_wide = wide(&host);
        let connection = Handle(WinHttpConnect(session.0, host_wide.as_ptr(), port, 0));
        if connection.0.is_null() {
            return Err(last_error(url, "WinHttpConnect"));
        }
        let verb = wide("GET");
        let path_wide = wide(&path);
        let request = Handle(WinHttpOpenRequest(
            connection.0,
            verb.as_ptr(),
            path_wide.as_ptr(),
            null(),
            null(),
            null(),
            WINHTTP_FLAG_SECURE,
        ));
        if request.0.is_null() {
            return Err(last_error(url, "WinHttpOpenRequest"));
        }

        // Windows credentials only for the proxy, never for the download server.
        let policy = WINHTTP_AUTOLOGON_SECURITY_LEVEL_PROXY_ONLY;
        WinHttpSetOption(
            request.0,
            WINHTTP_OPTION_AUTOLOGON_POLICY,
            &policy as *const u32 as *const c_void,
            size_of::<u32>() as u32,
        );
        if manual_proxy {
            apply_ie_proxy(&session, &request, url);
        }

        // Send; answer a proxy authentication challenge with the Windows credentials (max. twice).
        let mut status = 0u32;
        for attempt in 0..3 {
            if WinHttpSendRequest(request.0, null(), 0, null(), 0, 0, 0) == 0 {
                return Err(last_error(url, "send request"));
            }
            if WinHttpReceiveResponse(request.0, null_mut()) == 0 {
                return Err(last_error(url, "receive response"));
            }
            status = query_status(&request).unwrap_or(0);
            if status != 407 || attempt == 2 || !set_proxy_credentials(&request) {
                break;
            }
            log::info!("proxy authentication required, using the Windows credentials");
        }
        match status {
            200 => {}
            407 => {
                return Err(download_error(
                    url,
                    "the proxy requires authentication (HTTP 407)",
                ));
            }
            _ => return Err(download_error(url, format!("HTTP status {status}"))),
        }
        let total = query_content_length(&request);

        let file = File::create(dest).ctx("create", dest)?;
        let mut out = BufWriter::new(file);
        let mut buffer = vec![0u8; CHUNK];
        let mut done = 0u64;
        progress(0, total);
        loop {
            if cancel.load(Ordering::SeqCst) {
                return Err(Error::Cancelled);
            }
            let mut read = 0u32;
            if WinHttpReadData(
                request.0,
                buffer.as_mut_ptr() as *mut c_void,
                CHUNK as u32,
                &mut read,
            ) == 0
            {
                return Err(last_error(url, "read"));
            }
            if read == 0 {
                break;
            }
            out.write_all(&buffer[..read as usize]).ctx("write", dest)?;
            done += u64::from(read);
            progress(done, total);
        }
        out.flush().ctx("write", dest)?;
        if let Some(total) = total.filter(|&total| total != done) {
            return Err(download_error(
                url,
                format!("incomplete: got {done} of {total} bytes"),
            ));
        }
        Ok(done)
    }
}

/// TLS 1.2 + 1.3; older Windows 10 builds don't know the 1.3 flag → TLS 1.2 only.
unsafe fn set_tls_protocols(session: &Handle) -> std::result::Result<(), ()> {
    for protocols in [
        WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3,
        WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2,
    ] {
        // SAFETY: valid session handle and u32 buffer.
        let ok = unsafe {
            WinHttpSetOption(
                session.0,
                WINHTTP_OPTION_SECURE_PROTOCOLS,
                &protocols as *const u32 as *const c_void,
                size_of::<u32>() as u32,
            )
        };
        if ok != 0 {
            return Ok(());
        }
    }
    Err(())
}

/// HTTP status code (32-bit query; `WINHTTP_QUERY_FLAG_NUMBER64` doesn't work for it).
unsafe fn query_status(request: &Handle) -> Option<u32> {
    let mut value = 0u32;
    let mut len = size_of::<u32>() as u32;
    // SAFETY: valid request handle; u32 buffer with its size.
    let ok = unsafe {
        WinHttpQueryHeaders(
            request.0,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            null(),
            &mut value as *mut u32 as *mut c_void,
            &mut len,
            null_mut(),
        )
    };
    (ok != 0).then_some(value)
}

/// `Content-Length` (64-bit), `None` if absent.
unsafe fn query_content_length(request: &Handle) -> Option<u64> {
    let mut value = 0u64;
    let mut len = size_of::<u64>() as u32;
    // SAFETY: valid request handle; u64 buffer with its size.
    let ok = unsafe {
        WinHttpQueryHeaders(
            request.0,
            WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER64,
            null(),
            &mut value as *mut u64 as *mut c_void,
            &mut len,
            null_mut(),
        )
    };
    (ok != 0).then_some(value)
}

/// After a 407: select Negotiate or NTLM with the default (= logged-in user's) credentials.
unsafe fn set_proxy_credentials(request: &Handle) -> bool {
    let (mut supported, mut first, mut target) = (0u32, 0u32, 0u32);
    // SAFETY: valid request handle and out pointers.
    if unsafe { WinHttpQueryAuthSchemes(request.0, &mut supported, &mut first, &mut target) } == 0
        || target != WINHTTP_AUTH_TARGET_PROXY
    {
        return false;
    }
    let scheme = if supported & WINHTTP_AUTH_SCHEME_NEGOTIATE != 0 {
        WINHTTP_AUTH_SCHEME_NEGOTIATE
    } else if supported & WINHTTP_AUTH_SCHEME_NTLM != 0 {
        WINHTTP_AUTH_SCHEME_NTLM
    } else {
        return false; // Basic etc. would need a user name + password
    };
    // SAFETY: NULL user / password = default credentials of the logged-in user.
    unsafe {
        WinHttpSetCredentials(
            request.0,
            WINHTTP_AUTH_TARGET_PROXY,
            scheme,
            null(),
            null(),
            null_mut(),
        ) != 0
    }
}

/// Fallback without `AUTOMATIC_PROXY`: IE proxy settings of the current user, incl. PAC / auto detect.
unsafe fn apply_ie_proxy(session: &Handle, request: &Handle, url: &str) {
    // SAFETY (whole fn): plain WinHTTP calls; strings returned by WinHTTP are freed with GlobalFree.
    unsafe {
        let mut ie: WINHTTP_CURRENT_USER_IE_PROXY_CONFIG = std::mem::zeroed();
        if WinHttpGetIEProxyConfigForCurrentUser(&mut ie) == 0 {
            return; // no settings → direct
        }
        let mut proxy: WINHTTP_PROXY_INFO = std::mem::zeroed();
        let mut have_proxy = false;
        if ie.fAutoDetect != 0 || !ie.lpszAutoConfigUrl.is_null() {
            let mut options: WINHTTP_AUTOPROXY_OPTIONS = std::mem::zeroed();
            if ie.lpszAutoConfigUrl.is_null() {
                options.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
                options.dwAutoDetectFlags =
                    WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
            } else {
                options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
                options.lpszAutoConfigUrl = ie.lpszAutoConfigUrl;
            }
            options.fAutoLogonIfChallenged = 1;
            let url_wide = wide(url);
            have_proxy =
                WinHttpGetProxyForUrl(session.0, url_wide.as_ptr(), &mut options, &mut proxy) != 0;
        }
        if have_proxy {
            WinHttpSetOption(
                request.0,
                WINHTTP_OPTION_PROXY,
                &proxy as *const WINHTTP_PROXY_INFO as *const c_void,
                size_of::<WINHTTP_PROXY_INFO>() as u32,
            );
        } else if !ie.lpszProxy.is_null() {
            let named = WINHTTP_PROXY_INFO {
                dwAccessType: WINHTTP_ACCESS_TYPE_NAMED_PROXY,
                lpszProxy: ie.lpszProxy,
                lpszProxyBypass: ie.lpszProxyBypass,
            };
            WinHttpSetOption(
                request.0,
                WINHTTP_OPTION_PROXY,
                &named as *const WINHTTP_PROXY_INFO as *const c_void,
                size_of::<WINHTTP_PROXY_INFO>() as u32,
            );
        }
        for ptr in [
            ie.lpszAutoConfigUrl,
            ie.lpszProxy,
            ie.lpszProxyBypass,
            proxy.lpszProxy,
            proxy.lpszProxyBypass,
        ] {
            if !ptr.is_null() {
                GlobalFree(ptr as *mut c_void);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn splits_urls() {
        assert_eq!(
            split_https_url("https://github.com/a/b.zip?x=1"),
            Some(("github.com".into(), 443, "/a/b.zip?x=1".into()))
        );
        assert_eq!(
            split_https_url("https://h:8443"),
            Some(("h".into(), 8443, "/".into()))
        );
        assert_eq!(split_https_url("http://github.com/a"), None);
        assert_eq!(split_https_url("https://user@h/a"), None);
        assert_eq!(split_https_url("https:///a"), None);
    }

    /// Needs internet: `cargo test -p hlae-installer-core download -- --ignored --nocapture`
    #[test]
    #[ignore]
    fn downloads_from_github_with_redirect() {
        let dir = tempfile::tempdir().unwrap();
        let dest = dir.path().join("checksums.sha256");
        let url = "https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-09-30-13-08/checksums.sha256";
        let cancel = AtomicBool::new(false);
        let size = download(url, &dest, &cancel, |done, total| {
            println!("{done} / {total:?}")
        })
        .unwrap();
        assert!(size > 0);
        println!("{}", std::fs::read_to_string(&dest).unwrap());
    }

    #[test]
    #[ignore]
    fn reports_http_errors_and_cleans_up() {
        let dir = tempfile::tempdir().unwrap();
        let dest = dir.path().join("x");
        let cancel = AtomicBool::new(false);
        let error = download(
            "https://github.com/advancedfx/does-not-exist-404",
            &dest,
            &cancel,
            |_, _| {},
        )
        .unwrap_err();
        assert!(error.to_string().contains("HTTP status 404"), "{error}");
        assert!(!dest.exists());
    }
}
