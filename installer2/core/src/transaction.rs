//! Moves staged files into place with backup, so a failed operation can be rolled back – also after
//! a crash (NOTES.md 6.10 d).
//!
//! Work dir `<install dir>\.hlae-setup-tmp\<id>\` (same volume → renames are cheap and atomic, also
//! for in-use files): `staging\` holds verified new files, `backup\` the replaced / removed old ones.
//! `<id>` is new for every transaction. Nothing is deleted before [`Transaction::commit`].
//!
//! Journal `journal.jsonl` next to the state file (admin-only dir): one JSON line is appended before
//! each step (write-ahead). Not flushed to disk: safe if the setup crashes or is killed; after a power
//! loss the install may be mixed, running the setup again repairs it (decided 2026-10-17).
//! The commit point is the state save: the journal records the state file's hash at the start, so
//! [`recover`] knows whether to roll back (state unchanged) or only clean up (new state saved).

use std::collections::HashSet;
use std::fs::{self, File};
use std::io::{self, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU32, Ordering};

use serde::{Deserialize, Serialize};

use crate::error::{Error, IoContext, Result};
use crate::fsutil;
use crate::hash;
use crate::manifest::{Component, ManifestFile};
use crate::payload::StagedFile;
use crate::relpath::RelPath;
use crate::state::FileRecord;

pub const WORK_DIR_NAME: &str = ".hlae-setup-tmp";
pub const JOURNAL_FILE_NAME: &str = "journal.jsonl";
const JOURNAL_FORMAT: u32 = 1;

/// The journal of a transaction working on the install whose state is `state_file`.
pub fn journal_path(state_file: &Path) -> PathBuf {
    state_file.with_file_name(JOURNAL_FILE_NAME)
}

/// One journal line; also what [`Transaction`] keeps in memory to undo.
#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(tag = "op", rename_all = "kebab-case")]
enum Entry {
    /// First line. `state_sha256` = hash of the state file at the start, `None` = there was none.
    Begin {
        format: u32,
        install_dir: PathBuf,
        id: String,
        state_sha256: Option<String>,
    },
    /// About to create `dir` (absolute; the install dir itself, its parents or dirs inside it).
    CreatedDir { dir: PathBuf },
    /// About to move the file at `path` to `backup\<backup>`.
    Backup { path: RelPath, backup: String },
    /// About to move a staged file with this hash to `path`.
    Placed { path: RelPath, sha256: String },
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

struct Journal {
    path: PathBuf,
    file: File,
}

impl Journal {
    fn append(&mut self, entry: &Entry) -> Result<()> {
        let mut line = serde_json::to_vec(entry)?;
        line.push(b'\n');
        // Unbuffered: each line is handed to Windows at once, so it survives a crash of the setup.
        self.file.write_all(&line).ctx("write", &self.path)
    }
}

pub struct Transaction {
    install_dir: PathBuf,
    work_dir: PathBuf,
    /// `None` once committed / rolled back.
    journal: Option<Journal>,
    /// Steps done so far, oldest first.
    undo: Vec<Entry>,
    removed: Vec<RelPath>,
    backups: usize,
    finished: bool,
}

impl Transaction {
    /// Finishes an interrupted earlier transaction ([`recover`]), then creates the install dir if
    /// needed and a fresh work dir inside it. `state_file` = the install's state (may not exist yet).
    pub fn begin(install_dir: &Path, state_file: &Path) -> Result<Self> {
        recover(state_file)?;
        let journal_path = journal_path(state_file);
        if let Some(dir) = journal_path.parent() {
            fs::create_dir_all(dir).ctx("create", dir)?;
        }
        let state_sha256 = state_hash(state_file)?;
        let file = File::create(&journal_path).ctx("create", &journal_path)?;
        let id = new_id();
        let root = install_dir.join(WORK_DIR_NAME);
        let mut tx = Self {
            install_dir: install_dir.to_path_buf(),
            work_dir: root.join(&id),
            journal: Some(Journal {
                path: journal_path,
                file,
            }),
            undo: Vec::new(),
            removed: Vec::new(),
            backups: 0,
            finished: false,
        };
        tx.log(Entry::Begin {
            format: JOURNAL_FORMAT,
            install_dir: install_dir.to_path_buf(),
            id,
            state_sha256,
        })?;
        tx.create_dirs(install_dir)?;
        if fsutil::is_reparse_point(&root).ctx("stat", &root)? {
            return Err(Error::ReparsePoint { path: root });
        }
        remove_stale_work_dirs(&root);
        tx.create_dirs(&root)?;
        for dir in [tx.work_dir.clone(), tx.staging_dir(), tx.backup_dir()] {
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

    fn log(&mut self, entry: Entry) -> Result<()> {
        self.journal
            .as_mut()
            .expect("transaction still open")
            .append(&entry)
    }

    /// Moves `staged` into place and moves obsolete files of the `managed` components aside.
    /// `unchanged` = files of the new set that are installed exactly like this already (not staged,
    /// see [`unchanged_files`]); they're kept and recorded.
    ///
    /// - Tracked files are overwritten even if modified (NOTES.md 4.1); identical files are left untouched.
    /// - Files in `old_files` of a `managed` component that aren't in `staged` / `unchanged` are
    ///   removed if their checksum still matches, else kept.
    ///
    /// On error everything done so far is rolled back.
    pub fn apply(
        &mut self,
        staged: Vec<StagedFile>,
        unchanged: Vec<FileRecord>,
        managed: &[Component],
        old_files: &[FileRecord],
    ) -> Result<Applied> {
        let result = self.apply_inner(staged, unchanged, managed, old_files);
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
        unchanged: Vec<FileRecord>,
        managed: &[Component],
        old_files: &[FileRecord],
    ) -> Result<Applied> {
        let mut applied = Applied::default();
        let mut new_keys: HashSet<String> = unchanged.iter().map(|f| f.path.key()).collect();
        applied.files.extend(unchanged);

        for file in staged {
            new_keys.insert(file.path.key());
            fsutil::check_no_reparse(&self.install_dir, &file.path)?;
            let target = file.path.to_path(&self.install_dir);
            if let Some(parent) = target.parent() {
                self.create_dirs(parent)?;
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
                        self.move_to_backup(&file.path)?;
                        self.place(&file)?;
                    }
                }
                Err(e) if e.kind() == io::ErrorKind::NotFound => self.place(&file)?,
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
                    self.move_to_backup(&old.path)?;
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

    fn move_to_backup(&mut self, path: &RelPath) -> Result<()> {
        self.backups += 1;
        let entry = Entry::Backup {
            path: path.clone(),
            backup: format!("{:06}", self.backups),
        };
        self.log(entry.clone())?;
        let target = path.to_path(&self.install_dir);
        let backup = self.backup_dir().join(format!("{:06}", self.backups));
        fs::rename(&target, &backup).ctx("move aside", &target)?;
        self.undo.push(entry);
        Ok(())
    }

    fn place(&mut self, file: &StagedFile) -> Result<()> {
        let entry = Entry::Placed {
            path: file.path.clone(),
            sha256: file.sha256.clone(),
        };
        self.log(entry.clone())?;
        let target = file.path.to_path(&self.install_dir);
        fs::rename(&file.staged, &target).ctx("place", &target)?;
        self.undo.push(entry);
        Ok(())
    }

    /// `create_dir_all` that journals and records every dir it creates, for rollback.
    fn create_dirs(&mut self, dir: &Path) -> Result<()> {
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
            let entry = Entry::CreatedDir { dir: path.clone() };
            self.log(entry.clone())?;
            fs::create_dir(&path).ctx("create", &path)?;
            self.undo.push(entry);
        }
        Ok(())
    }

    /// Makes the changes permanent: call after the new state is saved. Deletes the journal, the
    /// backups and the work dir, removes now empty dirs.
    pub fn commit(mut self) -> Result<()> {
        self.finished = true;
        self.undo.clear();
        self.remove_journal();
        remove_work_dir(&self.work_dir);
        fsutil::remove_empty_dirs(&self.install_dir, &self.removed);
        Ok(())
    }

    /// Restores the state before [`Transaction::begin`].
    pub fn rollback(mut self) -> Result<()> {
        self.rollback_inner()
    }

    /// Tests only: ends the transaction as if the setup had crashed now (journal + work dir stay).
    #[doc(hidden)]
    pub fn simulate_crash(mut self) {
        self.finished = true;
    }

    fn rollback_inner(&mut self) -> Result<()> {
        self.finished = true;
        let undo = std::mem::take(&mut self.undo);
        undo_all(&self.install_dir, &self.work_dir, undo)?;
        self.remove_journal();
        Ok(())
    }

    fn remove_journal(&mut self) {
        if let Some(journal) = self.journal.take() {
            drop(journal.file);
            if let Err(e) = fs::remove_file(&journal.path) {
                // Harmless: the next run finds the work done (or undone) and only cleans up.
                log::warn!("could not remove {}: {e}", journal.path.display());
            }
        }
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

/// The files of `wanted` that are installed in `install_dir` exactly like this already (same size and
/// SHA-256, no junction in the path): they needn't be staged (fast repair / upgrade). A file that
/// can't be read counts as changed.
pub fn unchanged_files<'a>(
    install_dir: &Path,
    wanted: impl IntoIterator<Item = &'a ManifestFile>,
) -> Vec<FileRecord> {
    wanted
        .into_iter()
        .filter(|file| {
            if fsutil::check_no_reparse(install_dir, &file.path).is_err() {
                return false;
            }
            let target = file.path.to_path(install_dir);
            // Size first: cheap, and most changed files differ in size.
            fs::symlink_metadata(&target)
                .is_ok_and(|meta| meta.is_file() && meta.len() == file.size)
                && hash::sha256_file(&target).is_ok_and(|(sha256, _)| sha256 == file.sha256)
        })
        .map(|file| FileRecord {
            path: file.path.clone(),
            sha256: file.sha256.clone(),
            component: file.feature,
        })
        .collect()
}

/// Finishes a transaction that was interrupted (setup crashed / killed): if the state file is still
/// the one from before, the previous files are restored, else (the new state was saved) only the
/// backups are removed. No-op without a journal. If restoring fails, the journal is kept for the next try.
pub fn recover(state_file: &Path) -> Result<()> {
    let path = journal_path(state_file);
    let text = match fs::read_to_string(&path) {
        Ok(text) => text,
        Err(e) if e.kind() == io::ErrorKind::NotFound => return Ok(()),
        Err(e) => return Err(e).ctx("read", &path),
    };
    let mut entries = Vec::new();
    for line in text.lines() {
        match serde_json::from_str::<Entry>(line) {
            Ok(entry) => entries.push(entry),
            // The last line may be cut off by the crash; nothing after it can have happened.
            Err(e) => {
                log::warn!("journal {}: ignoring the rest ({e})", path.display());
                break;
            }
        }
    }
    let Some(Entry::Begin {
        format,
        install_dir,
        id,
        state_sha256,
    }) = entries.first().cloned()
    else {
        log::warn!("removing unusable journal {}", path.display());
        return fs::remove_file(&path).ctx("remove", &path);
    };
    if format != JOURNAL_FORMAT {
        return Err(Error::UnsupportedFormat {
            what: "journal",
            found: format,
            known: JOURNAL_FORMAT,
        });
    }
    if !is_valid_id(&id) {
        log::warn!("removing journal {} with invalid id {id:?}", path.display());
        return fs::remove_file(&path).ctx("remove", &path);
    }
    let work_dir = install_dir.join(WORK_DIR_NAME).join(&id);
    if state_hash(state_file)? == state_sha256 {
        log::warn!(
            "an earlier operation in {} was interrupted, restoring the previous files",
            install_dir.display()
        );
        if let Err(e) = undo_all(
            &install_dir,
            &work_dir,
            entries.into_iter().skip(1).collect(),
        ) {
            log::error!(
                "could not restore everything, will try again next time (journal {} kept)",
                path.display()
            );
            return Err(e);
        }
    } else {
        log::info!("an earlier operation was interrupted after saving its result, cleaning up");
        remove_work_dir(&work_dir);
    }
    fs::remove_file(&path).ctx("remove", &path)
}

/// Undoes `steps` (oldest first, undone newest first), then removes the work dir and created dirs.
/// Works for steps that never happened or were undone already (after a crash): those are skipped.
/// On I/O errors the work dir is kept (it still holds backups) and the first error is returned.
fn undo_all(install_dir: &Path, work_dir: &Path, steps: Vec<Entry>) -> Result<()> {
    let mut first_error = None;
    let mut created_dirs = Vec::new();
    for step in steps.into_iter().rev() {
        let result = match step {
            Entry::Placed { path, sha256 } => undo_placed(install_dir, &path, &sha256),
            Entry::Backup { path, backup } => undo_backup(install_dir, work_dir, &path, &backup),
            Entry::CreatedDir { dir } => {
                created_dirs.push(dir);
                Ok(())
            }
            Entry::Begin { .. } => Ok(()),
        };
        if let Err(e) = result {
            log::error!("{e}");
            first_error.get_or_insert(e);
        }
    }
    if let Some(e) = first_error {
        return Err(e);
    }
    remove_work_dir(work_dir);
    // Deepest first already (undone in reverse); `remove_dir` only removes empty dirs.
    for dir in created_dirs {
        let _ = fs::remove_dir(&dir);
    }
    Ok(())
}

/// Removes a file we placed – only if it's still exactly that file.
fn undo_placed(install_dir: &Path, path: &RelPath, sha256: &str) -> Result<()> {
    fsutil::check_no_reparse(install_dir, path)?;
    let target = path.to_path(install_dir);
    match fs::symlink_metadata(&target) {
        Ok(meta) if meta.is_file() && hash::sha256_file(&target)?.0 == sha256 => {
            fsutil::remove_file_force(&target).ctx("rollback remove", &target)
        }
        // Never placed (crash before the rename) or already undone / replaced by the old file.
        Ok(_) => Ok(()),
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(()),
        Err(e) => Err(e).ctx("stat", &target),
    }
}

fn undo_backup(install_dir: &Path, work_dir: &Path, path: &RelPath, backup: &str) -> Result<()> {
    if backup.is_empty() || !backup.bytes().all(|b| b.is_ascii_digit()) {
        log::warn!("journal: ignoring invalid backup name {backup:?}");
        return Ok(());
    }
    let backup = work_dir.join("backup").join(backup);
    if !backup.try_exists().ctx("stat", &backup)? {
        return Ok(()); // never moved (crash before the rename) or already restored
    }
    fsutil::check_no_reparse(install_dir, path)?;
    let target = path.to_path(install_dir);
    if target.try_exists().ctx("stat", &target)? {
        // Only after a crash: the new file was changed since, so it wasn't removed. Keep it.
        log::warn!(
            "not restoring {path}: a different file is there now (run the setup again to repair)"
        );
        return Ok(());
    }
    fs::rename(&backup, &target).ctx("rollback restore", &target)
}

/// SHA-256 of the state file, `None` if there is none.
fn state_hash(state_file: &Path) -> Result<Option<String>> {
    match fs::symlink_metadata(state_file) {
        Ok(_) => Ok(Some(hash::sha256_file(state_file)?.0)),
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(None),
        Err(e) => Err(e).ctx("stat", state_file),
    }
}

/// `<pid>-<time>-<n>`: unique per transaction (two per install run within the same second).
fn new_id() -> String {
    static COUNTER: AtomicU32 = AtomicU32::new(0);
    format!(
        "{}-{}-{}",
        std::process::id(),
        crate::time::utc_now().replace([':', '-'], ""),
        COUNTER.fetch_add(1, Ordering::Relaxed)
    )
}

fn is_valid_id(id: &str) -> bool {
    !id.is_empty() && id.len() <= 64 && id.bytes().all(|b| b.is_ascii_alphanumeric() || b == b'-')
}

/// Removes the work dir; if that fails (e.g. a replaced DLL is still loaded), the next run does it.
/// Also removes `.hlae-setup-tmp` if empty then.
fn remove_work_dir(work_dir: &Path) {
    match fs::remove_dir_all(work_dir) {
        Ok(()) => {}
        Err(e) if e.kind() == io::ErrorKind::NotFound => {}
        Err(e) => log::warn!(
            "could not remove {} ({e}), the next run of HLAE Setup removes it",
            work_dir.display()
        ),
    }
    if let Some(root) = work_dir.parent() {
        let _ = fs::remove_dir(root);
    }
}

/// Work dirs of earlier transactions that couldn't be removed then (no journal refers to them any
/// more, [`recover`] ran before). Best effort.
fn remove_stale_work_dirs(root: &Path) {
    let Ok(entries) = fs::read_dir(root) else {
        return;
    };
    for entry in entries.flatten() {
        let path = entry.path();
        log::info!("removing old work dir {}", path.display());
        let result = match entry.file_type() {
            // Not ours (we only create plain dirs here): a junction / link is removed, not followed.
            Ok(t) if t.is_symlink() => fs::remove_dir(&path).or_else(|_| fs::remove_file(&path)),
            Ok(t) if t.is_dir() => fs::remove_dir_all(&path),
            _ => fs::remove_file(&path),
        };
        if let Err(e) = result {
            log::warn!("could not remove {}: {e}", path.display());
        }
    }
}
