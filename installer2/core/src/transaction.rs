//! Moves staged files into place with backup, so a failed operation can be rolled back.
//!
//! Work dir `<install dir>\.hlae-setup-tmp\` (same volume → renames are cheap and atomic):
//! `staging\` holds verified new files, `backup\` the replaced / removed old ones.
//! Nothing is deleted before [`Transaction::commit`]. A crash-safe journal comes later (NOTES.md M7).

use std::collections::HashSet;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

use crate::error::{Error, IoContext, Result};
use crate::fsutil;
use crate::hash;
use crate::manifest::Component;
use crate::payload::StagedFile;
use crate::relpath::RelPath;
use crate::state::FileRecord;

pub const WORK_DIR_NAME: &str = ".hlae-setup-tmp";

enum Undo {
    CreatedDir(PathBuf),
    MovedToBackup { target: PathBuf, backup: PathBuf },
    Placed { target: PathBuf },
}

/// Result of [`Transaction::apply`].
#[derive(Debug, Default)]
pub struct Applied {
    /// Files now installed for the managed components (what goes into the state).
    pub files: Vec<FileRecord>,
    /// Obsolete files removed (unchanged since we installed them).
    pub removed: Vec<RelPath>,
    /// Obsolete files left in place because the user modified them (or they're behind a reparse point).
    pub kept: Vec<RelPath>,
}

pub struct Transaction {
    install_dir: PathBuf,
    work_dir: PathBuf,
    undo: Vec<Undo>,
    removed: Vec<RelPath>,
    backups: usize,
    finished: bool,
}

impl Transaction {
    /// Creates the install dir if needed and a fresh work dir inside it.
    pub fn begin(install_dir: &Path) -> Result<Self> {
        let mut undo = Vec::new();
        create_dirs(install_dir, &mut undo)?;
        let work_dir = install_dir.join(WORK_DIR_NAME);
        let tx = Self {
            install_dir: install_dir.to_path_buf(),
            work_dir,
            undo,
            removed: Vec::new(),
            backups: 0,
            finished: false,
        };
        let work_dir = tx.work_dir.clone();
        if fsutil::is_reparse_point(&work_dir).ctx("stat", &work_dir)? {
            return Err(Error::ReparsePoint { path: work_dir });
        }
        if work_dir.exists() {
            log::warn!("removing stale work dir {}", work_dir.display());
            fs::remove_dir_all(&work_dir).ctx("remove", &work_dir)?;
        }
        for dir in [work_dir.clone(), tx.staging_dir(), tx.backup_dir()] {
            fs::create_dir(&dir).ctx("create", &dir)?;
        }
        Ok(tx)
    }

    pub fn install_dir(&self) -> &Path {
        &self.install_dir
    }

    pub fn staging_dir(&self) -> PathBuf {
        self.work_dir.join("staging")
    }

    fn backup_dir(&self) -> PathBuf {
        self.work_dir.join("backup")
    }

    /// Moves `staged` into place and moves obsolete files of the `managed` components aside.
    ///
    /// - Tracked files are overwritten even if modified (NOTES.md 4.1); identical files are left untouched.
    /// - Files in `old_files` of a `managed` component that aren't in `staged` are removed if their
    ///   checksum still matches, else kept.
    ///
    /// On error everything done so far is rolled back.
    pub fn apply(
        &mut self,
        staged: Vec<StagedFile>,
        managed: &[Component],
        old_files: &[FileRecord],
    ) -> Result<Applied> {
        let result = self.apply_inner(staged, managed, old_files);
        if result.is_err()
            && let Err(e) = self.rollback_inner()
        {
            log::error!("rollback failed: {e}");
        }
        result
    }

    fn apply_inner(
        &mut self,
        staged: Vec<StagedFile>,
        managed: &[Component],
        old_files: &[FileRecord],
    ) -> Result<Applied> {
        let mut applied = Applied::default();
        let mut new_keys = HashSet::new();

        for file in staged {
            new_keys.insert(file.path.key());
            fsutil::check_no_reparse(&self.install_dir, &file.path)?;
            let target = file.path.to_path(&self.install_dir);
            if let Some(parent) = target.parent() {
                create_dirs(parent, &mut self.undo)?;
            }
            match fs::symlink_metadata(&target) {
                Ok(meta) if meta.is_dir() => {
                    return Err(Error::Conflict {
                        path: target,
                        reason: "a directory is in the way of a file",
                    });
                }
                Ok(meta) => {
                    let identical = meta.is_file() && hash::sha256_file(&target)?.0 == file.sha256;
                    if !identical {
                        self.move_to_backup(&target)?;
                        self.place(&file.staged, target)?;
                    }
                }
                Err(e) if e.kind() == io::ErrorKind::NotFound => {
                    self.place(&file.staged, target)?
                }
                Err(e) => return Err(e).ctx("stat", &target),
            }
            applied.files.push(FileRecord {
                path: file.path,
                sha256: file.sha256,
                component: file.component,
            });
        }

        let obsolete = old_files
            .iter()
            .filter(|f| managed.contains(&f.component) && !new_keys.contains(&f.path.key()));
        for old in obsolete {
            if let Err(e) = fsutil::check_no_reparse(&self.install_dir, &old.path) {
                log::warn!("keeping {}: {e}", old.path);
                applied.kept.push(old.path.clone());
                continue;
            }
            let target = old.path.to_path(&self.install_dir);
            match fs::symlink_metadata(&target) {
                Ok(meta) if meta.is_file() && hash::sha256_file(&target)?.0 == old.sha256 => {
                    self.move_to_backup(&target)?;
                    self.removed.push(old.path.clone());
                    applied.removed.push(old.path.clone());
                }
                Ok(_) => {
                    log::info!("keeping modified {}", old.path);
                    applied.kept.push(old.path.clone());
                }
                Err(e) if e.kind() == io::ErrorKind::NotFound => {}
                Err(e) => return Err(e).ctx("stat", &target),
            }
        }
        Ok(applied)
    }

    fn move_to_backup(&mut self, target: &Path) -> Result<()> {
        self.backups += 1;
        let backup = self.backup_dir().join(format!("{:06}", self.backups));
        fs::rename(target, &backup).ctx("move aside", target)?;
        self.undo.push(Undo::MovedToBackup {
            target: target.to_path_buf(),
            backup,
        });
        Ok(())
    }

    fn place(&mut self, staged: &Path, target: PathBuf) -> Result<()> {
        fs::rename(staged, &target).ctx("place", &target)?;
        self.undo.push(Undo::Placed { target });
        Ok(())
    }

    /// Makes the changes permanent: deletes backups and work dir, removes now empty dirs.
    pub fn commit(mut self) -> Result<()> {
        self.finished = true;
        self.undo.clear();
        if let Err(e) = fs::remove_dir_all(&self.work_dir) {
            // E.g. a replaced DLL still loaded somewhere; cleaned up on the next run (M7: delete on reboot).
            log::warn!("could not remove {}: {e}", self.work_dir.display());
        }
        fsutil::remove_empty_dirs(&self.install_dir, &self.removed);
        Ok(())
    }

    /// Restores the state before [`Transaction::begin`].
    pub fn rollback(mut self) -> Result<()> {
        self.rollback_inner()
    }

    fn rollback_inner(&mut self) -> Result<()> {
        self.finished = true;
        let mut first_error = None;
        let mut created_dirs = Vec::new();
        while let Some(undo) = self.undo.pop() {
            let result = match undo {
                Undo::Placed { target } => {
                    fsutil::remove_file_force(&target).ctx("rollback remove", &target)
                }
                Undo::MovedToBackup { target, backup } => {
                    fs::rename(&backup, &target).ctx("rollback restore", &target)
                }
                Undo::CreatedDir(dir) => {
                    created_dirs.push(dir);
                    Ok(())
                }
            };
            if let Err(e) = result {
                log::error!("{e}");
                first_error.get_or_insert(e);
            }
        }
        if first_error.is_none()
            && let Err(e) = fs::remove_dir_all(&self.work_dir)
        {
            log::warn!("could not remove {}: {e}", self.work_dir.display());
        }
        // `created_dirs` is deepest first already (undo popped in reverse).
        for dir in created_dirs {
            let _ = fs::remove_dir(&dir);
        }
        first_error.map_or(Ok(()), Err)
    }
}

impl Drop for Transaction {
    fn drop(&mut self) {
        if !self.finished {
            log::warn!("transaction dropped without commit, rolling back");
            if let Err(e) = self.rollback_inner() {
                log::error!("rollback failed: {e}");
            }
        }
    }
}

/// `create_dir_all` that records every dir it created, for rollback.
fn create_dirs(dir: &Path, undo: &mut Vec<Undo>) -> Result<()> {
    let mut missing = Vec::new();
    let mut current = Some(dir);
    while let Some(path) = current {
        if path.as_os_str().is_empty() || path.try_exists().ctx("stat", path)? {
            break;
        }
        missing.push(path.to_path_buf());
        current = path.parent();
    }
    for path in missing.into_iter().rev() {
        fs::create_dir(&path).ctx("create", &path)?;
        undo.push(Undo::CreatedDir(path));
    }
    Ok(())
}
