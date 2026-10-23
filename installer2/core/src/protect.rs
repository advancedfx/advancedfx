//! Protects the install dir like `Program Files` and pins it while we work in it (NOTES.md 6.7).
//!
//! The setup works there elevated, by path. Two things keep a normal user from redirecting it with a
//! junction (e.g. into `C:\Windows\System32`), decided 2026-10-17:
//! - **Protected**: outside `Program Files` the install dir gets owner Administrators and
//!   [`acl::ADMIN_ONLY_DACL`] (users read & execute only), everything in it inherits that, so
//!   nobody else can create or replace anything inside. Under `Program Files` Windows' inherited
//!   permissions do the same and are left alone.
//! - **Pinned**: [`PinnedDir`] keeps a handle to the install dir open without `FILE_SHARE_DELETE`, so
//!   it (and every dir above it) can't be renamed or deleted while we work – which a user with full
//!   control over a parent dir could otherwise do to swap it. The path must also lead to the dir
//!   directly (no junction / symbolic link / short name in it), checked via the handle.

use std::fs;
use std::io;
use std::os::windows::ffi::{OsStrExt, OsStringExt};
use std::os::windows::io::{AsHandle, AsRawHandle, FromRawHandle, OwnedHandle};
use std::path::{Path, PathBuf};

use windows_sys::Win32::Foundation::{HANDLE, INVALID_HANDLE_VALUE};
use windows_sys::Win32::Storage::FileSystem::{
    BY_HANDLE_FILE_INFORMATION, CreateFileW, FILE_ATTRIBUTE_DIRECTORY,
    FILE_ATTRIBUTE_REPARSE_POINT, FILE_FLAG_BACKUP_SEMANTICS, FILE_FLAG_OPEN_REPARSE_POINT,
    FILE_LIST_DIRECTORY, FILE_NAME_NORMALIZED, FILE_READ_ATTRIBUTES, FILE_SHARE_DELETE,
    FILE_SHARE_READ, FILE_SHARE_WRITE, GetFileInformationByHandle, GetFinalPathNameByHandleW,
    OPEN_EXISTING, READ_CONTROL, VOLUME_NAME_DOS, WRITE_DAC, WRITE_OWNER,
};

use crate::acl;
use crate::error::{Error, IoContext, Result};
use crate::win::{KnownFolder, known_folder};

/// How the install dir comes about, see [`prepare`].
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Prepare {
    /// Our install (there's a state): protect it again, recreate it if it's gone.
    Ours,
    /// A new install: the dir must not exist yet or be empty (we don't take over someone's folder).
    Fresh,
    /// A new install over the old setup's dir (migration, NOTES.md 5): may hold its leftovers.
    Migration,
}

/// The install dir, pinned until dropped (see the module docs).
pub struct PinnedDir {
    handle: OwnedHandle,
    path: PathBuf,
    /// Dirs [`prepare`] created, outermost first.
    created: Vec<PathBuf>,
}

impl PinnedDir {
    pub fn path(&self) -> &Path {
        &self.path
    }

    fn info(&self) -> io::Result<BY_HANDLE_FILE_INFORMATION> {
        file_info(&self.handle)
    }

    /// Unpins it and removes the dirs [`prepare`] created – only those that are empty (after a
    /// failed fresh install).
    pub fn remove_created_if_empty(self) {
        let created = self.created.clone();
        drop(self);
        for dir in created.iter().rev() {
            let _ = fs::remove_dir(dir);
        }
    }
}

fn wide(path: &Path) -> Vec<u16> {
    path.as_os_str().encode_wide().chain(Some(0)).collect()
}

/// Opens `path` itself (a junction / link is opened, not followed). `share_delete = false` pins it.
fn open(path: &Path, access: u32, share_delete: bool) -> io::Result<OwnedHandle> {
    let share =
        FILE_SHARE_READ | FILE_SHARE_WRITE | if share_delete { FILE_SHARE_DELETE } else { 0 };
    let path = wide(path);
    // SAFETY: valid NUL-terminated path; the handle is owned (closed) by the returned OwnedHandle.
    unsafe {
        let handle = CreateFileW(
            path.as_ptr(),
            access,
            share,
            std::ptr::null(),
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            std::ptr::null_mut(),
        );
        if handle == INVALID_HANDLE_VALUE {
            return Err(io::Error::last_os_error());
        }
        Ok(OwnedHandle::from_raw_handle(handle))
    }
}

fn raw(handle: &OwnedHandle) -> HANDLE {
    handle.as_raw_handle() as HANDLE
}

fn file_info(handle: &OwnedHandle) -> io::Result<BY_HANDLE_FILE_INFORMATION> {
    let mut info = BY_HANDLE_FILE_INFORMATION::default();
    // SAFETY: valid handle and out pointer.
    if unsafe { GetFileInformationByHandle(raw(handle), &mut info) } == 0 {
        return Err(io::Error::last_os_error());
    }
    Ok(info)
}

/// `C:\real\path` of the open object (`\\?\` removed; network paths as `UNC\server\…`).
fn final_path(handle: &OwnedHandle) -> io::Result<String> {
    let mut buffer = vec![0u16; 1024];
    loop {
        // SAFETY: valid handle, buffer size passed.
        let len = unsafe {
            GetFinalPathNameByHandleW(
                raw(handle),
                buffer.as_mut_ptr(),
                buffer.len() as u32,
                FILE_NAME_NORMALIZED | VOLUME_NAME_DOS,
            )
        } as usize;
        if len == 0 {
            return Err(io::Error::last_os_error());
        }
        if len < buffer.len() {
            let path = std::ffi::OsString::from_wide(&buffer[..len]);
            let path = path.to_string_lossy();
            return Ok(path.strip_prefix(r"\\?\").unwrap_or(&path).to_owned());
        }
        buffer.resize(len + 1, 0);
    }
}

fn normalize(path: &str) -> String {
    path.trim_end_matches(['\\', '/'])
        .replace('/', "\\")
        .to_lowercase()
}

/// Whether `path` is below `Program Files` / `Program Files (x86)` (by its text).
pub fn is_under_program_files(path: &Path) -> bool {
    let path = normalize(&path.to_string_lossy());
    [KnownFolder::ProgramFiles, KnownFolder::ProgramFilesX86]
        .into_iter()
        .filter_map(|folder| known_folder(folder).ok())
        .any(|base| {
            let base = normalize(&base.to_string_lossy());
            path.strip_prefix(&base)
                .is_some_and(|rest| rest.starts_with('\\'))
        })
}

/// `true` if `dir` doesn't exist or has nothing in it.
pub fn is_missing_or_empty(dir: &Path) -> io::Result<bool> {
    match fs::read_dir(dir) {
        Ok(mut entries) => Ok(entries.next().is_none()),
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(true),
        Err(e) => Err(e),
    }
}

/// Pins the existing dir `dir` and checks that its path leads to it directly.
fn pin_existing(dir: &Path, created: Vec<PathBuf>) -> Result<PinnedDir> {
    // Windows enforces share modes only against handles with data access, so `FILE_LIST_DIRECTORY`
    // (a dir's "read data") is needed for the pin to work, not just `FILE_READ_ATTRIBUTES`.
    let handle = open(dir, FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES, false).ctx("open", dir)?;
    let info = file_info(&handle).ctx("stat", dir)?;
    if info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
        return Err(Error::ReparsePoint {
            path: dir.to_path_buf(),
        });
    }
    if info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY == 0 {
        return Err(Error::Conflict {
            path: dir.to_path_buf(),
            reason: "is a file, not a folder",
        });
    }
    let real = final_path(&handle).ctx("resolve", dir)?;
    if real.starts_with(r"UNC\") {
        return Err(Error::InvalidInstallDir {
            dir: dir.to_path_buf(),
            reason: "network folders are not supported",
        });
    }
    if normalize(&real) != normalize(&dir.to_string_lossy()) {
        log::warn!("{} is really {real}", dir.display());
        return Err(Error::InvalidInstallDir {
            dir: dir.to_path_buf(),
            reason: "the path goes through a junction, symbolic link or short name – please use the real folder path",
        });
    }
    Ok(PinnedDir {
        handle,
        path: dir.to_path_buf(),
        created,
    })
}

/// Pins `dir` for an uninstall (no permission changes); `None` if it's gone.
pub fn pin(dir: &Path) -> Result<Option<PinnedDir>> {
    if !dir.try_exists().ctx("stat", dir)? {
        return Ok(None);
    }
    pin_existing(dir, Vec::new()).map(Some)
}

/// Checks `dir` like [`prepare`] would without changing anything (before the old setup is removed,
/// so a migration doesn't fail half way).
pub fn check(dir: &Path) -> Result<()> {
    pin(dir).map(|_| ())
}

/// Creates (if needed), protects (with `protect`; off in the debug test mode, which runs without admin
/// rights) and pins the install dir `dir`.
pub fn prepare(dir: &Path, how: Prepare, protect: bool) -> Result<PinnedDir> {
    let dir = PathBuf::from(dir.to_string_lossy().trim_end_matches(['\\', '/']));
    if crate::fsutil::is_reparse_point(&dir).ctx("stat", &dir)? {
        return Err(Error::ReparsePoint { path: dir });
    }
    // Before creating anything: the existing part of the path must lead there directly, else the
    // new dirs would end up wherever a junction points.
    if let Some(existing) = dir.ancestors().find(|p| p.exists()) {
        pin_existing(existing, Vec::new())?;
    }
    let created = create_missing(&dir)?;
    let existed = created.is_empty();
    let pinned = pin_existing(&dir, created.clone());
    let result = pinned.and_then(|pinned| {
        if !protect || is_under_program_files(&dir) {
            return Ok(pinned);
        }
        let not_empty = || -> Result<bool> { Ok(!is_missing_or_empty(&dir).ctx("list", &dir)?) };
        if how == Prepare::Fresh && existed && not_empty()? {
            return Err(Error::InstallDirNotEmpty { dir: dir.clone() });
        }
        protect_tree(&pinned)?;
        // Created by us, but the parent may let others write: something may have appeared before
        // the dir was protected.
        if how == Prepare::Fresh && not_empty()? {
            return Err(Error::InstallDirNotEmpty { dir: dir.clone() });
        }
        Ok(pinned)
    });
    if result.is_err() {
        for dir in created.iter().rev() {
            let _ = fs::remove_dir(dir);
        }
    }
    result
}

/// `create_dir_all`, returning the dirs it created (outermost first).
fn create_missing(dir: &Path) -> Result<Vec<PathBuf>> {
    let mut missing = Vec::new();
    let mut current = Some(dir);
    while let Some(path) = current {
        if path.as_os_str().is_empty() || path.try_exists().ctx("stat", path)? {
            break;
        }
        missing.push(path.to_path_buf());
        current = path.parent();
    }
    missing.reverse();
    for (i, path) in missing.iter().enumerate() {
        if let Err(e) = fs::create_dir(path) {
            for created in missing[..i].iter().rev() {
                let _ = fs::remove_dir(created);
            }
            return Err(e).ctx("create", path);
        }
    }
    Ok(missing)
}

const SET_SECURITY: u32 = READ_CONTROL | WRITE_DAC | WRITE_OWNER | FILE_READ_ATTRIBUTES;

/// Owner Administrators + protected admin-only DACL on the pinned dir, then everything below reset
/// to owner Administrators + only the inherited entries (removes what someone else may have been
/// allowed before, e.g. in the old setup's dir). Top-down, so a dir is protected before we look
/// inside. Junctions / links and hard-linked files (could be someone else's file) are skipped.
fn protect_tree(pinned: &PinnedDir) -> Result<()> {
    let dir = pinned.path();
    let root = open(dir, SET_SECURITY, true).ctx("open", dir)?;
    // The same dir as the pinned one (the path can't have changed, but cheap to be sure).
    let (a, b) = (
        pinned.info().ctx("stat", dir)?,
        file_info(&root).ctx("stat", dir)?,
    );
    if (a.dwVolumeSerialNumber, a.nFileIndexHigh, a.nFileIndexLow)
        != (b.dwVolumeSerialNumber, b.nFileIndexHigh, b.nFileIndexLow)
    {
        return Err(Error::Conflict {
            path: dir.to_path_buf(),
            reason: "changed while being prepared",
        });
    }
    acl::set_handle_security(root.as_handle(), acl::ADMIN_ONLY_DACL, true).ctx("protect", dir)?;
    reset_children(dir)
}

fn reset_children(dir: &Path) -> Result<()> {
    for entry in fs::read_dir(dir).ctx("list", dir)? {
        let path = entry.ctx("list", dir)?.path();
        let handle = open(&path, SET_SECURITY, true).ctx("open", &path)?;
        let info = file_info(&handle).ctx("stat", &path)?;
        let is_dir = info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0;
        if info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
            log::warn!("leaving {} alone: junction / symbolic link", path.display());
            continue;
        }
        if !is_dir && info.nNumberOfLinks > 1 {
            log::warn!("leaving {} alone: hard link", path.display());
            continue;
        }
        let dacl = if is_dir {
            acl::INHERITED_DIR_DACL
        } else {
            acl::INHERITED_FILE_DACL
        };
        acl::set_handle_security(handle.as_handle(), dacl, true).ctx("protect", &path)?;
        drop(handle);
        if is_dir {
            reset_children(&path)?;
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn pinned_dir_and_its_parents_cant_be_renamed() {
        let tmp = tempfile::tempdir().unwrap();
        let parent = tmp.path().join("parent");
        let dir = parent.join("HLAE");
        let pinned = prepare(&dir, Prepare::Fresh, false).unwrap();
        assert_eq!(pinned.created, [parent.clone(), dir.clone()]);
        // Working inside is fine.
        fs::write(dir.join("HLAE.exe"), b"x").unwrap();
        fs::rename(dir.join("HLAE.exe"), dir.join("moved.exe")).unwrap();
        assert!(fs::rename(&dir, parent.join("swapped")).is_err());
        assert!(fs::rename(&parent, tmp.path().join("swapped")).is_err());
        assert!(fs::remove_dir_all(&parent).is_err());
        drop(pinned);
        fs::rename(&parent, tmp.path().join("swapped")).unwrap();
    }

    #[test]
    fn junctions_and_files_are_refused() {
        let tmp = tempfile::tempdir().unwrap();
        let real = tmp.path().join("real");
        fs::create_dir(&real).unwrap();
        let link = tmp.path().join("link");
        let status = std::process::Command::new("cmd")
            .args(["/C", "mklink", "/J"])
            .arg(&link)
            .arg(&real)
            .output()
            .unwrap()
            .status;
        assert!(status.success());
        // The junction itself, a new dir right below it, a new dir below an existing one behind it.
        assert!(matches!(
            prepare(&link, Prepare::Ours, false),
            Err(Error::ReparsePoint { .. })
        ));
        assert!(matches!(
            prepare(&link.join("HLAE"), Prepare::Fresh, false),
            Err(Error::ReparsePoint { .. })
        ));
        fs::create_dir(real.join("Games")).unwrap();
        assert!(matches!(
            prepare(&link.join("Games").join("HLAE"), Prepare::Fresh, false),
            Err(Error::InvalidInstallDir { .. })
        ));
        assert!(
            !real.join("HLAE").exists() && !real.join("Games").join("HLAE").exists(),
            "nothing created below the junction"
        );
        fs::write(tmp.path().join("file"), b"x").unwrap();
        assert!(matches!(
            prepare(&tmp.path().join("file"), Prepare::Ours, false),
            Err(Error::Conflict { .. })
        ));
    }

    #[test]
    fn program_files_detection() {
        let x86 = known_folder(KnownFolder::ProgramFilesX86).unwrap();
        assert!(is_under_program_files(&x86.join("HLAE")));
        assert!(is_under_program_files(&PathBuf::from(
            x86.to_string_lossy().to_uppercase() + "\\HLAE\\"
        )));
        assert!(!is_under_program_files(&x86));
        assert!(!is_under_program_files(Path::new(
            "C:\\Program Files (x86)2\\HLAE"
        )));
        assert!(!is_under_program_files(Path::new("D:\\Games\\HLAE")));
    }

    /// Without admin rights the owner can't be set to Administrators, so only the DACL part is
    /// checked: the children get exactly the inherited entries, own entries are gone.
    #[test]
    fn children_get_only_inherited_permissions() {
        if crate::win::is_elevated().unwrap() {
            return; // checked as normal user (admins could set the owner, but keep the test simple)
        }
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("HLAE");
        fs::create_dir_all(dir.join("x64")).unwrap();
        fs::write(dir.join("x64").join("a.dll"), b"x").unwrap();
        let status = std::process::Command::new("icacls")
            .arg(dir.join("x64"))
            .args(["/grant", "*S-1-1-0:(OI)(CI)F", "/Q"])
            .status()
            .unwrap();
        assert!(status.success());

        for (path, dacl) in [
            (dir.join("x64"), acl::INHERITED_DIR_DACL),
            (dir.join("x64").join("a.dll"), acl::INHERITED_FILE_DACL),
        ] {
            let handle = open(&path, READ_CONTROL | WRITE_DAC, true).unwrap();
            acl::set_handle_security(handle.as_handle(), dacl, false).unwrap();
            let sddl = acl::handle_sddl(handle.as_handle()).unwrap();
            assert!(!sddl.contains("WD"), "Everyone entry gone: {sddl}");
            // Exactly the inherited entries (the cosmetic `AI` flag isn't stored by this API).
            assert!(sddl.ends_with(&dacl[4..]), "{sddl}");
        }
        let root = open(&dir, READ_CONTROL | WRITE_DAC, true).unwrap();
        acl::set_handle_security(root.as_handle(), acl::ADMIN_ONLY_DACL, false).unwrap();
        let sddl = acl::handle_sddl(root.as_handle()).unwrap();
        assert!(sddl.contains("D:P"), "protected: {sddl}");
        drop(root);
        // Normal users can't write any more.
        assert!(fs::write(dir.join("x64").join("b.dll"), b"x").is_err());
        let status = std::process::Command::new("icacls")
            .arg(&dir)
            .args(["/reset", "/T", "/Q"])
            .status()
            .unwrap();
        assert!(status.success());
    }
}
