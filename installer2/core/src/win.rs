//! Small wrappers around Windows APIs.

use std::ffi::{OsString, c_void};
use std::io;
use std::os::windows::ffi::OsStringExt;
use std::path::PathBuf;
use std::ptr::null_mut;

use windows_sys::Win32::Foundation::{CloseHandle, HANDLE};
use windows_sys::Win32::Security::{GetTokenInformation, TOKEN_ELEVATION, TOKEN_QUERY, TokenElevation};
use windows_sys::Win32::System::Com::CoTaskMemFree;
use windows_sys::Win32::System::Threading::{GetCurrentProcess, OpenProcessToken};
use windows_sys::Win32::UI::Shell::{
    FOLDERID_ProgramData, FOLDERID_ProgramFilesX86, FOLDERID_RoamingAppData, SHGetKnownFolderPath,
};
use windows_sys::core::{GUID, PWSTR};

#[derive(Clone, Copy, Debug)]
pub enum KnownFolder {
    /// `C:\ProgramData`
    ProgramData,
    /// `C:\Program Files (x86)`
    ProgramFilesX86,
    /// `%APPDATA%` of the current user.
    RoamingAppData,
}

/// Resolves a known folder via the shell API (not via environment variables, which the caller controls).
pub fn known_folder(folder: KnownFolder) -> io::Result<PathBuf> {
    let id: &GUID = match folder {
        KnownFolder::ProgramData => &FOLDERID_ProgramData,
        KnownFolder::ProgramFilesX86 => &FOLDERID_ProgramFilesX86,
        KnownFolder::RoamingAppData => &FOLDERID_RoamingAppData,
    };
    let mut path: PWSTR = null_mut();
    // SAFETY: valid GUID pointer and out pointer; `path` is freed with CoTaskMemFree in any case.
    let hr = unsafe { SHGetKnownFolderPath(id, 0, null_mut(), &mut path) };
    let result = if hr >= 0 && !path.is_null() {
        // SAFETY: on success `path` is a NUL-terminated wide string.
        let wide = unsafe {
            let len = (0..).take_while(|&i| *path.add(i) != 0).count();
            std::slice::from_raw_parts(path, len)
        };
        Ok(PathBuf::from(OsString::from_wide(wide)))
    } else {
        Err(io::Error::from_raw_os_error(hr))
    };
    // SAFETY: `path` was allocated by SHGetKnownFolderPath (or is null).
    unsafe { CoTaskMemFree(path as *const c_void) };
    result
}

/// Whether the current process runs elevated (as administrator).
pub fn is_elevated() -> io::Result<bool> {
    // SAFETY: plain Win32 calls with valid out pointers, the token handle is closed.
    unsafe {
        let mut token: HANDLE = null_mut();
        if OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &mut token) == 0 {
            return Err(io::Error::last_os_error());
        }
        let mut elevation = TOKEN_ELEVATION { TokenIsElevated: 0 };
        let mut len = 0u32;
        let ok = GetTokenInformation(
            token,
            TokenElevation,
            &mut elevation as *mut TOKEN_ELEVATION as *mut c_void,
            size_of::<TOKEN_ELEVATION>() as u32,
            &mut len,
        );
        let error = io::Error::last_os_error();
        CloseHandle(token);
        if ok == 0 {
            return Err(error);
        }
        Ok(elevation.TokenIsElevated != 0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn resolves_known_folders() {
        for folder in [KnownFolder::ProgramData, KnownFolder::ProgramFilesX86, KnownFolder::RoamingAppData] {
            let path = known_folder(folder).unwrap();
            assert!(path.is_absolute() && path.is_dir(), "{folder:?}: {}", path.display());
        }
    }

    #[test]
    fn elevation_check_works() {
        is_elevated().unwrap();
    }
}
