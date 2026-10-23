//! Native folder / file pickers (`IFileOpenDialog`), modal to the wizard window.

use std::ffi::c_void;
use std::path::{Path, PathBuf};

use windows::Win32::Foundation::HWND;
use windows::Win32::System::Com::{
    CLSCTX_INPROC_SERVER, COINIT_APARTMENTTHREADED, CoCreateInstance, CoInitializeEx,
    CoTaskMemFree, CoUninitialize,
};
use windows::Win32::UI::Shell::Common::COMDLG_FILTERSPEC;
use windows::Win32::UI::Shell::{
    FOS_FILEMUSTEXIST, FOS_FORCEFILESYSTEM, FOS_PATHMUSTEXIST, FOS_PICKFOLDERS, FileOpenDialog,
    IFileOpenDialog, IShellItem, SHCreateItemFromParsingName, SIGDN_FILESYSPATH,
};
use windows::core::{HSTRING, PCWSTR};

pub enum Kind {
    Folder,
    /// A file with exactly this name, e.g. `ffmpeg.exe`.
    File(&'static str),
}

/// Shows the picker on its own COM (STA) thread; `None` if cancelled or on error.
pub fn pick(owner: isize, kind: Kind, title: String, initial: Option<PathBuf>) -> Option<PathBuf> {
    std::thread::spawn(move || {
        // SAFETY: COM initialized for this thread and uninitialized after; all COM objects are
        // dropped inside `show` before that.
        unsafe {
            let init = CoInitializeEx(None, COINIT_APARTMENTTHREADED);
            let result = show(
                HWND(owner as *mut c_void),
                &kind,
                &title,
                initial.as_deref(),
            );
            if init.is_ok() {
                CoUninitialize();
            }
            result.ok().flatten()
        }
    })
    .join()
    .ok()
    .flatten()
}

unsafe fn show(
    owner: HWND,
    kind: &Kind,
    title: &str,
    initial: Option<&Path>,
) -> windows::core::Result<Option<PathBuf>> {
    // SAFETY: plain COM calls on this STA thread.
    unsafe {
        let dialog: IFileOpenDialog =
            CoCreateInstance(&FileOpenDialog, None, CLSCTX_INPROC_SERVER)?;
        let mut options = dialog.GetOptions()? | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
        let name;
        match kind {
            Kind::Folder => options |= FOS_PICKFOLDERS,
            Kind::File(file) => {
                options |= FOS_FILEMUSTEXIST;
                name = HSTRING::from(*file);
                let filter = [COMDLG_FILTERSPEC {
                    pszName: PCWSTR(name.as_ptr()),
                    pszSpec: PCWSTR(name.as_ptr()),
                }];
                dialog.SetFileTypes(&filter)?;
            }
        }
        dialog.SetOptions(options)?;
        dialog.SetTitle(&HSTRING::from(title))?;
        if let Some(dir) = initial.filter(|d| d.is_dir())
            && let Ok(item) =
                SHCreateItemFromParsingName::<_, _, IShellItem>(&HSTRING::from(dir), None)
        {
            let _ = dialog.SetFolder(&item);
        }
        if dialog.Show(Some(owner)).is_err() {
            return Ok(None); // cancelled
        }
        let path = dialog.GetResult()?.GetDisplayName(SIGDN_FILESYSPATH)?;
        let text = path.to_string();
        CoTaskMemFree(Some(path.0 as *const c_void));
        Ok(text.ok().map(PathBuf::from))
    }
}
