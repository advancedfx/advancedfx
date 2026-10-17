//! Install / upgrade / repair / modify / uninstall of the HLAE payload.

use std::cmp::Ordering;
use std::fs;
use std::io;
use std::path::Path;

use crate::error::{Error, IoContext, Result};
use crate::fsutil;
use crate::hash;
use crate::manifest::{Component, Manifest};
use crate::payload;
use crate::relpath::RelPath;
use crate::state::{Features, FileRecord, State};
use crate::transaction::{Applied, Transaction, WORK_DIR_NAME};
use crate::version;

/// Components the HLAE payload operation manages; files of other components (FFMPEG) are left alone.
pub const HLAE_COMPONENTS: [Component; 2] = [Component::Core, Component::Locales];

pub struct InstallRequest<'a> {
    pub install_dir: &'a Path,
    pub manifest: &'a Manifest,
    pub payload: &'a [u8],
    pub features: Features,
    /// E.g. `HLAE_Setup 2.192.9`, stored in the state.
    pub written_by: &'a str,
}

/// Result of [`install_hlae`]. The files are in place, but not committed yet:
/// save `state`, then [`Transaction::commit`] – or drop / [`Transaction::rollback`] to undo.
pub struct Outcome {
    pub state: State,
    pub tx: Transaction,
    pub applied: Applied,
    /// `install`, `upgrade`, `repair` or `modify`.
    pub action: &'static str,
}

/// Installs, upgrades, repairs or modifies HLAE, depending on `old`.
pub fn install_hlae(req: &InstallRequest, old: Option<&State>) -> Result<Outcome> {
    req.manifest.validate()?;
    let action = match old {
        None => "install",
        Some(old) => {
            if !same_dir(&old.install_dir, req.install_dir) {
                return Err(Error::InstallDirMismatch {
                    installed: old.install_dir.clone(),
                    requested: req.install_dir.to_path_buf(),
                });
            }
            match version::compare(&req.manifest.hlae_version, &old.hlae_version)? {
                Ordering::Less => {
                    return Err(Error::Downgrade {
                        installed: old.hlae_version.clone(),
                        new: req.manifest.hlae_version.clone(),
                    });
                }
                Ordering::Greater => "upgrade",
                Ordering::Equal if old.features != req.features => "modify",
                Ordering::Equal => "repair",
            }
        }
    };

    let mut components = vec![Component::Core];
    if req.features.locales {
        components.push(Component::Locales);
    }

    let mut tx = Transaction::begin(req.install_dir)?;
    let staged = payload::stage(req.payload, req.manifest, &components, &tx.staging_dir())?;
    let old_files = old.map(|s| s.files.as_slice()).unwrap_or_default();
    let applied = tx.apply(staged, &HLAE_COMPONENTS, old_files)?;

    let mut state = match old {
        Some(old) => old.clone(),
        None => State::new(req.written_by, req.install_dir, &req.manifest.hlae_version, req.features),
    };
    let detail = match old {
        Some(old) if action == "upgrade" => format!("{} -> {}", old.hlae_version, req.manifest.hlae_version),
        _ => req.manifest.hlae_version.clone(),
    };
    state.written_by = req.written_by.to_owned();
    state.hlae_version = req.manifest.hlae_version.clone();
    state.features = req.features;
    state.replace_files(&HLAE_COMPONENTS, applied.files.clone());
    state.push_history(action, detail);

    Ok(Outcome {
        state,
        tx,
        applied,
        action,
    })
}

#[derive(Debug, Default)]
pub struct UninstallReport {
    pub removed: Vec<RelPath>,
    /// Modified by the user (checksum differs) → left in place.
    pub kept: Vec<RelPath>,
    pub missing: Vec<RelPath>,
    pub failed: Vec<(RelPath, String)>,
}

/// Removes the tracked `files` from `install_dir` if their checksum still matches (NOTES.md 4.1),
/// then the now empty dirs.
pub fn uninstall_files(install_dir: &Path, files: &[FileRecord]) -> UninstallReport {
    let mut report = UninstallReport::default();
    for file in files {
        match remove_tracked(install_dir, file) {
            Ok(Removal::Removed) => report.removed.push(file.path.clone()),
            Ok(Removal::Kept) => report.kept.push(file.path.clone()),
            Ok(Removal::Missing) => report.missing.push(file.path.clone()),
            Err(e) => {
                log::error!("{e}");
                report.failed.push((file.path.clone(), e.to_string()));
            }
        }
    }
    fsutil::remove_empty_dirs(install_dir, &report.removed);
    report
}

/// Uninstalls everything tracked in `state` and removes the install dir if it ended up empty.
pub fn uninstall(state: &State) -> UninstallReport {
    let install_dir = &state.install_dir;
    let work_dir = install_dir.join(WORK_DIR_NAME);
    if matches!(fsutil::is_reparse_point(&work_dir), Ok(false)) && work_dir.exists() {
        let _ = fs::remove_dir_all(&work_dir);
    }
    let report = uninstall_files(install_dir, &state.files);
    // Fails if not empty (user files, kept modified files), which is fine.
    let _ = fs::remove_dir(install_dir);
    report
}

enum Removal {
    Removed,
    Kept,
    Missing,
}

fn remove_tracked(install_dir: &Path, file: &FileRecord) -> Result<Removal> {
    fsutil::check_no_reparse(install_dir, &file.path)?;
    let target = file.path.to_path(install_dir);
    match fs::symlink_metadata(&target) {
        Ok(meta) if meta.is_file() && hash::sha256_file(&target)?.0 == file.sha256 => {
            fsutil::remove_file_force(&target).ctx("remove", &target)?;
            Ok(Removal::Removed)
        }
        Ok(_) => Ok(Removal::Kept),
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(Removal::Missing),
        Err(e) => Err(e).ctx("stat", &target),
    }
}

fn same_dir(a: &Path, b: &Path) -> bool {
    let normalize = |p: &Path| p.to_string_lossy().trim_end_matches(['\\', '/']).to_lowercase();
    normalize(a) == normalize(b)
}
