use std::collections::BTreeSet;
use std::fs;
use std::io;
use std::os::windows::fs::MetadataExt;
use std::path::Path;

use crate::error::{Error, IoContext, Result};
use crate::relpath::RelPath;

pub const FILE_ATTRIBUTE_REPARSE_POINT: u32 = 0x400;

/// Whether `path` itself is a reparse point (junction, symbolic link, …); `false` if it doesn't exist.
pub fn is_reparse_point(path: &Path) -> io::Result<bool> {
    match fs::symlink_metadata(path) {
        Ok(meta) => Ok(meta.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0),
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(false),
        Err(e) => Err(e),
    }
}

/// Refuses if any existing component of `rel` below `root` is a reparse point (NOTES.md 6.7),
/// so we never write / delete through a junction or symbolic link planted inside the install dir.
pub fn check_no_reparse(root: &Path, rel: &RelPath) -> Result<()> {
    let mut path = root.to_path_buf();
    for component in rel.components() {
        path.push(component);
        match fs::symlink_metadata(&path) {
            Ok(meta) if meta.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0 => {
                return Err(Error::ReparsePoint { path });
            }
            Ok(meta) if !meta.is_dir() => break,
            Ok(_) => {}
            Err(e) if e.kind() == io::ErrorKind::NotFound => break,
            Err(e) => return Err(e).ctx("stat", &path),
        }
    }
    Ok(())
}

/// Removes a file, clearing the read-only attribute if needed.
pub fn remove_file_force(path: &Path) -> io::Result<()> {
    match fs::remove_file(path) {
        Err(e) if e.kind() == io::ErrorKind::PermissionDenied => {
            let mut permissions = fs::symlink_metadata(path)?.permissions();
            #[allow(clippy::permissions_set_readonly_false)]
            permissions.set_readonly(false);
            fs::set_permissions(path, permissions)?;
            fs::remove_file(path)
        }
        result => result,
    }
}

/// Removes the now empty parent dirs of `removed` files below `root`, deepest first.
/// Non-empty dirs and reparse points are left alone.
pub fn remove_empty_dirs<'a>(root: &Path, removed: impl IntoIterator<Item = &'a RelPath>) {
    let mut dirs = BTreeSet::new();
    for path in removed {
        let components: Vec<&str> = path.components().collect();
        for depth in 1..components.len() {
            dirs.insert((depth, components[..depth].join("/")));
        }
    }
    for (_, dir) in dirs.into_iter().rev() {
        let Ok(rel) = RelPath::new(&dir) else {
            continue;
        };
        if check_no_reparse(root, &rel).is_ok() {
            // Fails if not empty, which is fine.
            let _ = fs::remove_dir(rel.to_path(root));
        }
    }
}
