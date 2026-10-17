//! Named pipe between the unelevated process and the elevated worker, and starting the worker
//! (NOTES.md 6.4). Both ends verify each other:
//! - the worker only lets the *user* of the client process connect (pipe ACL), only local clients,
//!   and checks the connected client's PID;
//! - the client checks that the pipe server is the worker process it started (no pipe squatting).

use std::ffi::{OsStr, c_void};
use std::fs::File;
use std::io;
use std::os::windows::ffi::OsStrExt;
use std::os::windows::io::{AsRawHandle, FromRawHandle, OwnedHandle};
use std::path::Path;
use std::ptr::{null, null_mut};
use std::time::{Duration, Instant};

use windows_sys::Win32::Foundation::{
    ERROR_FILE_NOT_FOUND, ERROR_PIPE_BUSY, ERROR_PIPE_CONNECTED, GENERIC_READ, GENERIC_WRITE,
    HANDLE, INVALID_HANDLE_VALUE, LocalFree, STILL_ACTIVE,
};
use windows_sys::Win32::Security::Authorization::{
    ConvertSidToStringSidW, ConvertStringSecurityDescriptorToSecurityDescriptorW, SDDL_REVISION_1,
};
use windows_sys::Win32::Security::Cryptography::{
    BCRYPT_USE_SYSTEM_PREFERRED_RNG, BCryptGenRandom,
};
use windows_sys::Win32::Security::{
    GetTokenInformation, PSECURITY_DESCRIPTOR, SECURITY_ATTRIBUTES, TOKEN_QUERY, TOKEN_USER,
    TokenUser,
};
use windows_sys::Win32::Storage::FileSystem::{
    CreateFileW, FILE_FLAG_FIRST_PIPE_INSTANCE, OPEN_EXISTING, PIPE_ACCESS_DUPLEX,
    SECURITY_IDENTIFICATION, SECURITY_SQOS_PRESENT,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, GetNamedPipeClientProcessId, GetNamedPipeServerProcessId,
    PIPE_READMODE_BYTE, PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{
    GetExitCodeProcess, GetProcessId, INFINITE, OpenProcess, OpenProcessToken,
    PROCESS_QUERY_LIMITED_INFORMATION, WaitForSingleObject,
};
use windows_sys::Win32::UI::Shell::{
    SEE_MASK_FLAG_NO_UI, SEE_MASK_NOASYNC, SEE_MASK_NOCLOSEPROCESS, SHELLEXECUTEINFOW,
    ShellExecuteExW,
};
use windows_sys::Win32::UI::WindowsAndMessaging::SW_HIDE;

pub const PIPE_PREFIX: &str = r"\\.\pipe\hlae-setup-";
const BUFFER_SIZE: u32 = 64 * 1024;

/// How long the client waits for the worker's pipe, and the worker for its client. Generous on
/// purpose (slow / loaded machines, AV scanning the freshly started exe); `connect` returns as soon
/// as the pipe is there, so a long timeout costs nothing in the normal case.
pub const CONNECT_TIMEOUT: Duration = Duration::from_secs(90);

fn wide(s: &OsStr) -> Vec<u16> {
    s.encode_wide().chain(Some(0)).collect()
}

/// `\\.\pipe\hlae-setup-<128 random bits as hex>`
pub fn random_pipe_name() -> io::Result<String> {
    let mut bytes = [0u8; 16];
    // SAFETY: valid buffer of the given length; NULL algorithm handle with the system RNG flag.
    let status = unsafe {
        BCryptGenRandom(
            null_mut(),
            bytes.as_mut_ptr(),
            bytes.len() as u32,
            BCRYPT_USE_SYSTEM_PREFERRED_RNG,
        )
    };
    if status != 0 {
        return Err(io::Error::other(format!(
            "BCryptGenRandom failed: {status:#x}"
        )));
    }
    Ok(format!("{PIPE_PREFIX}{}", crate::hash::to_hex(&bytes)))
}

/// Whether `name` looks like one of ours (the worker refuses anything else).
pub fn is_valid_pipe_name(name: &str) -> bool {
    name.strip_prefix(PIPE_PREFIX)
        .is_some_and(|id| id.len() == 32 && id.bytes().all(|b| b.is_ascii_hexdigit()))
}

/// SID (`S-1-5-21-…`) of the user a process runs as.
pub fn process_user_sid(pid: u32) -> io::Result<String> {
    // SAFETY: handles are closed, buffers are sized as the API asks, strings freed with LocalFree.
    unsafe {
        let process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
        if process.is_null() {
            return Err(io::Error::last_os_error());
        }
        let process = OwnedHandle::from_raw_handle(process);
        let mut token: HANDLE = null_mut();
        if OpenProcessToken(process.as_raw_handle(), TOKEN_QUERY, &mut token) == 0 {
            return Err(io::Error::last_os_error());
        }
        let token = OwnedHandle::from_raw_handle(token);
        let mut len = 0u32;
        GetTokenInformation(token.as_raw_handle(), TokenUser, null_mut(), 0, &mut len);
        let mut buffer = vec![0u64; (len as usize).div_ceil(8)];
        if GetTokenInformation(
            token.as_raw_handle(),
            TokenUser,
            buffer.as_mut_ptr() as *mut c_void,
            len,
            &mut len,
        ) == 0
        {
            return Err(io::Error::last_os_error());
        }
        let user = &*(buffer.as_ptr() as *const TOKEN_USER);
        let mut sid_string = null_mut();
        if ConvertSidToStringSidW(user.User.Sid, &mut sid_string) == 0 {
            return Err(io::Error::last_os_error());
        }
        let len = (0..).take_while(|&i| *sid_string.add(i) != 0).count();
        let sid = String::from_utf16_lossy(std::slice::from_raw_parts(sid_string, len));
        LocalFree(sid_string as *mut c_void);
        Ok(sid)
    }
}

/// Worker side: creates pipe `name` that only the user of `client_pid` may open, waits (blocking)
/// for the client and checks that it is `client_pid`.
pub fn serve(name: &str, client_pid: u32) -> io::Result<File> {
    if !is_valid_pipe_name(name) {
        return Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            "invalid pipe name",
        ));
    }
    let sid = process_user_sid(client_pid)?;
    // Read + write for the client's user only (the worker uses its own handle), not inherited.
    let sddl = wide(OsStr::new(&format!("D:P(A;;GRGW;;;{sid})")));
    let name_wide = wide(OsStr::new(name));
    // SAFETY: descriptor freed with LocalFree; pipe handle owned by the returned File.
    unsafe {
        let mut descriptor: PSECURITY_DESCRIPTOR = null_mut();
        if ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl.as_ptr(),
            SDDL_REVISION_1,
            &mut descriptor,
            null_mut(),
        ) == 0
        {
            return Err(io::Error::last_os_error());
        }
        let attributes = SECURITY_ATTRIBUTES {
            nLength: size_of::<SECURITY_ATTRIBUTES>() as u32,
            lpSecurityDescriptor: descriptor,
            bInheritHandle: 0,
        };
        let pipe = CreateNamedPipeW(
            name_wide.as_ptr(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            1,
            BUFFER_SIZE,
            BUFFER_SIZE,
            0,
            &attributes,
        );
        let error = io::Error::last_os_error();
        LocalFree(descriptor);
        if pipe == INVALID_HANDLE_VALUE {
            return Err(error);
        }
        let file = File::from_raw_handle(pipe);
        if ConnectNamedPipe(pipe, null_mut()) == 0 {
            let error = io::Error::last_os_error();
            if error.raw_os_error() != Some(ERROR_PIPE_CONNECTED as i32) {
                return Err(error);
            }
        }
        let mut pid = 0u32;
        if GetNamedPipeClientProcessId(pipe, &mut pid) == 0 {
            return Err(io::Error::last_os_error());
        }
        if pid != client_pid {
            return Err(io::Error::new(
                io::ErrorKind::PermissionDenied,
                format!("unexpected pipe client process {pid}, expected {client_pid}"),
            ));
        }
        Ok(file)
    }
}

/// Client side: connects to pipe `name` and checks that its server is `server_pid`.
/// Retries until `timeout` while `alive()` (e.g. "the worker process still runs").
pub fn connect(
    name: &str,
    server_pid: u32,
    timeout: Duration,
    alive: impl Fn() -> bool,
) -> io::Result<File> {
    let name_wide = wide(OsStr::new(name));
    let deadline = Instant::now() + timeout;
    loop {
        // SAFETY: valid NUL-terminated name; the handle is owned by the returned File.
        // SECURITY_IDENTIFICATION: the server may identify, but not impersonate us.
        let pipe = unsafe {
            CreateFileW(
                name_wide.as_ptr(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                null(),
                OPEN_EXISTING,
                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                null_mut(),
            )
        };
        if pipe != INVALID_HANDLE_VALUE {
            // SAFETY: valid, owned pipe handle.
            let file = unsafe { File::from_raw_handle(pipe) };
            let mut pid = 0u32;
            // SAFETY: valid pipe handle and out pointer.
            if unsafe { GetNamedPipeServerProcessId(pipe, &mut pid) } == 0 {
                return Err(io::Error::last_os_error());
            }
            if pid != server_pid {
                return Err(io::Error::new(
                    io::ErrorKind::PermissionDenied,
                    format!("unexpected pipe server process {pid}, expected {server_pid}"),
                ));
            }
            return Ok(file);
        }
        let error = io::Error::last_os_error();
        let retry = matches!(
            error.raw_os_error().map(|e| e as u32),
            Some(ERROR_FILE_NOT_FOUND | ERROR_PIPE_BUSY)
        );
        if !retry || !alive() || Instant::now() >= deadline {
            return Err(error);
        }
        std::thread::sleep(Duration::from_millis(50));
    }
}

/// Starts `exe` with `args` elevated (UAC prompt), hidden. Declined prompt = error 1223 (`ERROR_CANCELLED`).
pub fn spawn_elevated(exe: &Path, args: &str) -> io::Result<OwnedHandle> {
    let verb = wide(OsStr::new("runas"));
    let file = wide(exe.as_os_str());
    let params = wide(OsStr::new(args));
    // SAFETY: SHELLEXECUTEINFOW is plain data; strings outlive the call; hProcess is owned afterwards.
    unsafe {
        let mut info: SHELLEXECUTEINFOW = std::mem::zeroed();
        info.cbSize = size_of::<SHELLEXECUTEINFOW>() as u32;
        info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
        info.lpVerb = verb.as_ptr();
        info.lpFile = file.as_ptr();
        info.lpParameters = params.as_ptr();
        info.nShow = SW_HIDE;
        if ShellExecuteExW(&mut info) == 0 {
            return Err(io::Error::last_os_error());
        }
        if info.hProcess.is_null() {
            return Err(io::Error::other("ShellExecuteExW returned no process"));
        }
        Ok(OwnedHandle::from_raw_handle(info.hProcess))
    }
}

pub fn process_id(process: &OwnedHandle) -> u32 {
    // SAFETY: valid process handle.
    unsafe { GetProcessId(process.as_raw_handle()) }
}

/// Exit code if the process has ended.
pub fn exit_code(process: &OwnedHandle) -> Option<u32> {
    let mut code = 0u32;
    // SAFETY: valid process handle and out pointer.
    let ok = unsafe { GetExitCodeProcess(process.as_raw_handle(), &mut code) };
    (ok != 0 && code != STILL_ACTIVE as u32).then_some(code)
}

/// Waits for the process to end and returns its exit code.
pub fn wait(process: &OwnedHandle) -> Option<u32> {
    // SAFETY: valid process handle.
    unsafe { WaitForSingleObject(process.as_raw_handle(), INFINITE) };
    exit_code(process)
}

#[cfg(test)]
mod tests {
    use std::io::{BufRead, BufReader, Write};

    use super::*;

    #[test]
    fn pipe_names() {
        let name = random_pipe_name().unwrap();
        assert!(is_valid_pipe_name(&name), "{name}");
        assert_ne!(name, random_pipe_name().unwrap());
        assert!(!is_valid_pipe_name(r"\\.\pipe\other"));
        assert!(!is_valid_pipe_name(&format!("{PIPE_PREFIX}xyz")));
    }

    #[test]
    fn own_user_sid() {
        assert!(
            process_user_sid(std::process::id())
                .unwrap()
                .starts_with("S-1-5-")
        );
    }

    #[test]
    fn serve_and_connect_verify_each_other() {
        let pid = std::process::id();
        let name = random_pipe_name().unwrap();
        let server_name = name.clone();
        let server = std::thread::spawn(move || {
            let pipe = serve(&server_name, pid).unwrap();
            let mut line = String::new();
            BufReader::new(&pipe).read_line(&mut line).unwrap();
            (&pipe)
                .write_all(format!("echo {line}").as_bytes())
                .unwrap();
        });
        let pipe = connect(&name, pid, CONNECT_TIMEOUT, || true).unwrap();
        (&pipe).write_all(b"hello\n").unwrap();
        let mut line = String::new();
        BufReader::new(&pipe).read_line(&mut line).unwrap();
        assert_eq!(line, "echo hello\n");
        server.join().unwrap();
    }

    #[test]
    fn client_rejects_unexpected_server() {
        let name = random_pipe_name().unwrap();
        let server_name = name.clone();
        let server = std::thread::spawn(move || {
            let _ = serve(&server_name, std::process::id());
        });
        let result = connect(&name, std::process::id() + 1, CONNECT_TIMEOUT, || true);
        assert_eq!(result.unwrap_err().kind(), io::ErrorKind::PermissionDenied);
        server.join().unwrap();
    }

    #[test]
    fn server_refuses_unknown_client_process() {
        // No such process → no user to allow → no pipe is created at all.
        let name = random_pipe_name().unwrap();
        assert!(serve(&name, 0xFFFF_FFF0).is_err());
        assert!(serve(r"\\.\pipe\not-ours", std::process::id()).is_err());
    }
}
