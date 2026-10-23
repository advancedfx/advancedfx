//! Start menu shortcut (NOTES.md 4.1): plain (non-advertised) `.lnk` via `IShellLinkW`.

use std::path::{Path, PathBuf};

use windows::Win32::Storage::FileSystem::WIN32_FIND_DATAW;
use windows::Win32::System::Com::{
    CLSCTX_INPROC_SERVER, COINIT_APARTMENTTHREADED, CoCreateInstance, CoInitializeEx,
    CoUninitialize, IPersistFile, STGM_READ,
};
use windows::Win32::UI::Shell::{IShellLinkW, ShellLink};
use windows::core::{HSTRING, Interface};

pub struct Shortcut<'a> {
    pub target: &'a Path,
    pub working_dir: &'a Path,
    pub description: &'a str,
}

/// Runs `f` with COM initialized on this thread.
fn with_com<T>(f: impl FnOnce() -> windows::core::Result<T>) -> windows::core::Result<T> {
    // SAFETY: balanced with CoUninitialize if it succeeded (S_FALSE = already initialized also counts).
    let init = unsafe { CoInitializeEx(None, COINIT_APARTMENTTHREADED) };
    let result = f();
    if init.is_ok() {
        unsafe { CoUninitialize() };
    }
    result
}

/// Creates or replaces the shortcut file `lnk`.
pub fn create(lnk: &Path, shortcut: &Shortcut) -> windows::core::Result<()> {
    with_com(|| unsafe {
        let link: IShellLinkW = CoCreateInstance(&ShellLink, None, CLSCTX_INPROC_SERVER)?;
        link.SetPath(&HSTRING::from(shortcut.target))?;
        link.SetWorkingDirectory(&HSTRING::from(shortcut.working_dir))?;
        link.SetDescription(&HSTRING::from(shortcut.description))?;
        link.SetIconLocation(&HSTRING::from(shortcut.target), 0)?;
        link.cast::<IPersistFile>()?.Save(&HSTRING::from(lnk), true)
    })
}

/// Target of an existing shortcut file.
pub fn read_target(lnk: &Path) -> windows::core::Result<PathBuf> {
    with_com(|| unsafe {
        let link: IShellLinkW = CoCreateInstance(&ShellLink, None, CLSCTX_INPROC_SERVER)?;
        link.cast::<IPersistFile>()?
            .Load(&HSTRING::from(lnk), STGM_READ)?;
        let mut buffer = [0u16; 32768];
        let mut find_data = WIN32_FIND_DATAW::default();
        link.GetPath(&mut buffer, &mut find_data, 0)?;
        let len = buffer.iter().position(|&c| c == 0).unwrap_or(buffer.len());
        Ok(PathBuf::from(String::from_utf16_lossy(&buffer[..len])))
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn create_and_read_back() {
        let dir = tempfile::tempdir().unwrap();
        let target = dir.path().join("HLAE.exe");
        std::fs::write(&target, b"").unwrap();
        let lnk = dir.path().join("HLAE.lnk");
        let shortcut = Shortcut {
            target: &target,
            working_dir: dir.path(),
            description: "Half-Life Advanced Effects",
        };
        create(&lnk, &shortcut).unwrap();
        create(&lnk, &shortcut).unwrap(); // replacing works
        assert_eq!(read_target(&lnk).unwrap(), target);
    }
}
