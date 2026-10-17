//! Files-in-use check with the Windows Restart Manager (NOTES.md 4.1): which programs have files
//! open that an install / uninstall is about to replace or remove – HLAE.exe, the hook DLLs loaded
//! in a game, a running ffmpeg.exe. We only list them, the user closes them (no `RmShutdown`).

use std::ffi::OsString;
use std::fmt;
use std::fs;
use std::io;
use std::os::windows::ffi::{OsStrExt, OsStringExt};
use std::path::{Path, PathBuf};

use serde::Serialize;
use windows_sys::Win32::Foundation::{CloseHandle, ERROR_MORE_DATA, ERROR_SUCCESS, WIN32_ERROR};
use windows_sys::Win32::System::RestartManager::{
    CCH_RM_SESSION_KEY, RM_PROCESS_INFO, RmEndSession, RmGetList, RmRegisterResources,
    RmStartSession,
};
use windows_sys::Win32::System::Threading::{
    OpenProcess, PROCESS_NAME_WIN32, PROCESS_QUERY_LIMITED_INFORMATION, QueryFullProcessImageNameW,
};
use windows_sys::core::PCWSTR;

use crate::error::{Error, Result};
use crate::legacy::{self, LegacyInstall};
use crate::state::State;

/// A program that has one of the checked files open.
#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Holder {
    pub pid: u32,
    /// Display name from the Restart Manager, e.g. "Counter-Strike 2".
    pub name: String,
    /// File name of the exe, e.g. `cs2.exe`, if it could be read.
    pub exe: Option<String>,
}

impl fmt::Display for Holder {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &self.exe {
            Some(exe) if !exe.eq_ignore_ascii_case(&self.name) => {
                write!(f, "{} ({exe}, PID {})", self.name, self.pid)
            }
            _ => write!(f, "{} (PID {})", self.name, self.pid),
        }
    }
}

/// Stop collecting files of the old setup's folders after this many (a sane bound, HLAE has ~250).
const MAX_LEGACY_FILES: usize = 10_000;

/// Files an operation could replace or remove: the tracked files of our install (HLAE + FFMPEG)
/// and, for a migration, all files in the old setup's folders.
pub fn files_to_check(state: Option<&State>, legacy: Option<&LegacyInstall>) -> Vec<PathBuf> {
    let mut files: Vec<PathBuf> = state
        .map(|s| {
            s.files
                .iter()
                .map(|f| f.path.to_path(&s.install_dir))
                .collect()
        })
        .unwrap_or_default();
    if let Some(legacy) = legacy {
        let dirs = legacy
            .install_dir
            .iter()
            .cloned()
            .chain(legacy::old_ffmpeg_dir());
        for dir in dirs {
            collect_files(&dir, &mut files);
        }
    }
    files
}

/// Recursively, without following junctions / symbolic links.
fn collect_files(dir: &Path, files: &mut Vec<PathBuf>) {
    let Ok(entries) = fs::read_dir(dir) else {
        return;
    };
    for entry in entries.flatten() {
        if files.len() >= MAX_LEGACY_FILES {
            return;
        }
        match entry.file_type() {
            Ok(t) if t.is_dir() => collect_files(&entry.path(), files),
            Ok(t) if t.is_file() => files.push(entry.path()),
            _ => {}
        }
    }
}

/// Programs that have one of `files` open, without this process. If the Restart Manager itself
/// fails (e.g. not implemented under Wine), that's logged and treated as "nothing in use" – a broken
/// check shouldn't block installing; renaming in-use files works anyway (NOTES.md 6.10 d).
///
/// The Restart Manager costs ~18 ms per file with ~200 running processes (~4 s for HLAE), so it only
/// gets the candidates (decided 2026-10-17): [`is_key_file`]s – a loaded runtime DLL is found via the
/// HLAE.exe / hook DLL that loaded it – and the files [`is_open`] flags. A missed file only costs
/// convenience: renaming works anyway, an uninstall keeps what it couldn't delete tracked.
pub fn programs_using(files: &[PathBuf]) -> Vec<Holder> {
    let candidates: Vec<PathBuf> = files
        .iter()
        .filter(|file| is_key_file(file) || is_open(file))
        .cloned()
        .collect();
    match holders(&candidates) {
        Ok(holders) => holders,
        Err(e) => {
            log::warn!("files-in-use check failed, continuing without it: {e}");
            Vec::new()
        }
    }
}

/// What runs as a program or gets loaded into one on its own: `*.exe` (HLAE.exe, the injectors,
/// ffmpeg.exe) and the hook DLLs `AfxHook*.dll` (loaded into games).
fn is_key_file(path: &Path) -> bool {
    let name = path
        .file_name()
        .map(|n| n.to_string_lossy().to_lowercase())
        .unwrap_or_default();
    name.ends_with(".exe") || (name.starts_with("afxhook") && name.ends_with(".dll"))
}

/// Quick probe (~0.1 ms): opening without sharing fails with a sharing violation if anyone has the
/// file open with data access. Doesn't see loaded program images (Windows doesn't apply share modes
/// to them) – those are the key files.
fn is_open(path: &Path) -> bool {
    use std::os::windows::fs::OpenOptionsExt;
    const ERROR_SHARING_VIOLATION: i32 = 32;
    const FILE_FLAG_OPEN_REPARSE_POINT: u32 = 0x0020_0000;
    fs::OpenOptions::new()
        .read(true)
        .share_mode(0)
        .custom_flags(FILE_FLAG_OPEN_REPARSE_POINT)
        .open(path)
        .is_err_and(|e| e.raw_os_error() == Some(ERROR_SHARING_VIOLATION))
}

/// Fails with [`Error::FilesInUse`] if a program has one of `files` open.
pub fn ensure_not_in_use(files: &[PathBuf]) -> Result<()> {
    let holders = programs_using(files);
    if holders.is_empty() {
        Ok(())
    } else {
        Err(Error::FilesInUse(holders))
    }
}

fn check(error: WIN32_ERROR) -> io::Result<()> {
    if error == ERROR_SUCCESS {
        Ok(())
    } else {
        Err(io::Error::from_raw_os_error(error as i32))
    }
}

/// Ends the Restart Manager session.
struct RmSession(u32);

impl Drop for RmSession {
    fn drop(&mut self) {
        // SAFETY: a session handle from RmStartSession, ended once.
        unsafe { RmEndSession(self.0) };
    }
}

fn holders(files: &[PathBuf]) -> io::Result<Vec<Holder>> {
    if files.is_empty() {
        return Ok(Vec::new());
    }
    let mut handle = 0u32;
    let mut key = [0u16; CCH_RM_SESSION_KEY as usize + 1];
    // SAFETY: valid out pointers, `key` has room for CCH_RM_SESSION_KEY + NUL.
    check(unsafe { RmStartSession(&mut handle, 0, key.as_mut_ptr()) })?;
    let session = RmSession(handle);

    let wide: Vec<Vec<u16>> = files
        .iter()
        .map(|f| f.as_os_str().encode_wide().chain(Some(0)).collect())
        .collect();
    let names: Vec<PCWSTR> = wide.iter().map(|w| w.as_ptr()).collect();
    for chunk in names.chunks(1000) {
        // SAFETY: `chunk` points to NUL-terminated strings that outlive the call.
        check(unsafe {
            RmRegisterResources(
                session.0,
                chunk.len() as u32,
                chunk.as_ptr(),
                0,
                std::ptr::null(),
                0,
                std::ptr::null(),
            )
        })?;
    }

    let mut infos: Vec<RM_PROCESS_INFO> = Vec::new();
    loop {
        let mut needed = 0u32;
        let mut count = infos.len() as u32;
        let mut reasons = 0u32;
        // SAFETY: `infos` has room for `count` entries (null and 0 when empty).
        let error = unsafe {
            RmGetList(
                session.0,
                &mut needed,
                &mut count,
                if infos.is_empty() {
                    std::ptr::null_mut()
                } else {
                    infos.as_mut_ptr()
                },
                &mut reasons,
            )
        };
        match error {
            ERROR_SUCCESS => {
                infos.truncate(count as usize);
                break;
            }
            // The list can grow between the calls, so leave some room.
            ERROR_MORE_DATA => infos.resize(needed as usize + 4, RM_PROCESS_INFO::default()),
            e => return Err(io::Error::from_raw_os_error(e as i32)),
        }
    }

    let own = std::process::id();
    let mut holders: Vec<Holder> = infos
        .iter()
        .filter(|info| info.Process.dwProcessId != own)
        .map(|info| Holder {
            pid: info.Process.dwProcessId,
            name: from_wide(&info.strAppName),
            exe: exe_name(info.Process.dwProcessId),
        })
        .collect();
    holders.sort_by(|a, b| a.name.cmp(&b.name).then(a.pid.cmp(&b.pid)));
    holders.dedup_by_key(|h| h.pid);
    Ok(holders)
}

fn from_wide(buffer: &[u16]) -> String {
    let len = buffer.iter().position(|&c| c == 0).unwrap_or(buffer.len());
    String::from_utf16_lossy(&buffer[..len])
}

/// `cs2.exe` for the process, `None` if we may not query it (e.g. another user's elevated process).
fn exe_name(pid: u32) -> Option<String> {
    // SAFETY: plain Win32 calls; the handle is closed, the buffer size is passed.
    unsafe {
        let process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
        if process.is_null() {
            return None;
        }
        let mut buffer = vec![0u16; 32_768];
        let mut len = buffer.len() as u32;
        let ok =
            QueryFullProcessImageNameW(process, PROCESS_NAME_WIN32, buffer.as_mut_ptr(), &mut len);
        CloseHandle(process);
        if ok == 0 {
            return None;
        }
        let path = PathBuf::from(OsString::from_wide(&buffer[..len as usize]));
        path.file_name().map(|n| n.to_string_lossy().into_owned())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn finds_a_process_holding_a_file() {
        let dir = tempfile::tempdir().unwrap();
        let file = dir.path().join("held.txt");
        fs::write(&file, b"x").unwrap();
        // Our own open handle isn't reported (we filter our pid), so let a child hold it.
        let mut child = std::process::Command::new("powershell.exe")
            .args([
                "-NoProfile",
                "-Command",
                &format!(
                    "$f = [IO.File]::Open('{}', 'Open', 'Read', 'Read'); Start-Sleep 30",
                    file.display()
                ),
            ])
            .spawn()
            .unwrap();
        let mut found = Vec::new();
        for _ in 0..100 {
            found = holders(std::slice::from_ref(&file)).unwrap();
            if !found.is_empty() {
                break;
            }
            std::thread::sleep(std::time::Duration::from_millis(100));
        }
        let _ = child.kill();
        let _ = child.wait();
        assert_eq!(found.len(), 1, "{found:?}");
        assert_eq!(found[0].pid, child.id());
        assert_eq!(found[0].exe.as_deref(), Some("powershell.exe"));
        assert!(holders(&[file]).unwrap().is_empty());
    }

    #[test]
    fn key_files() {
        for key in [
            "HLAE.exe",
            "x64/injector.exe",
            "x64/AfxHookSource2.dll",
            "AFXHOOKGOLDSRC.DLL",
        ] {
            assert!(is_key_file(Path::new(key)), "{key}");
        }
        for other in [
            "x64/vcruntime140.dll",
            "AfxHook.dat",
            "locales/de/hlae.mo",
            "exe",
        ] {
            assert!(!is_key_file(Path::new(other)), "{other}");
        }
    }

    /// A running exe isn't seen by the probe (no share modes on program images) but as a key file.
    #[test]
    fn finds_a_running_exe() {
        let dir = tempfile::tempdir().unwrap();
        let exe = dir.path().join("HLAE.exe");
        fs::copy(r"C:\Windows\System32\PING.EXE", &exe).unwrap();
        let mut child = std::process::Command::new(&exe)
            .args(["-n", "30", "127.0.0.1"])
            .stdout(std::process::Stdio::null())
            .spawn()
            .unwrap();
        assert!(!is_open(&exe), "the probe doesn't see program images");
        let found = programs_using(std::slice::from_ref(&exe));
        let _ = child.kill();
        let _ = child.wait();
        assert!(found.iter().any(|h| h.pid == child.id()), "{found:?}");
    }

    #[test]
    fn nothing_to_check() {
        assert!(holders(&[]).unwrap().is_empty());
        let dir = tempfile::tempdir().unwrap();
        assert!(
            holders(&[dir.path().join("missing.dll")])
                .unwrap()
                .is_empty()
        );
    }

    #[test]
    fn display() {
        let holder = Holder {
            pid: 42,
            name: "Counter-Strike 2".into(),
            exe: Some("cs2.exe".into()),
        };
        assert_eq!(holder.to_string(), "Counter-Strike 2 (cs2.exe, PID 42)");
        let holder = Holder {
            pid: 7,
            name: "HLAE.exe".into(),
            exe: Some("hlae.exe".into()),
        };
        assert_eq!(holder.to_string(), "HLAE.exe (PID 7)");
    }
}
