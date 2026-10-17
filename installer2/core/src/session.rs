//! Top-level operations as used by CLI / GUI / worker: state file handling around [`crate::install`].

use std::fs;
use std::io;
use std::path::{Path, PathBuf};

use crate::error::{Error, IoContext, Result};
use crate::install::{self, InstallRequest, UninstallReport, install_hlae};
use crate::manifest::Manifest;
use crate::relpath::RelPath;
use crate::state::{Features, State};
use crate::win::{KnownFolder, known_folder};

/// `%ProgramData%\advancedfx\HLAE\state.json` (NOTES.md 6.7).
pub fn default_state_file() -> Result<PathBuf> {
    let program_data = known_folder(KnownFolder::ProgramData).ctx("resolve", Path::new("ProgramData"))?;
    Ok(program_data.join("advancedfx").join("HLAE").join("state.json"))
}

/// `C:\Program Files (x86)\HLAE` (same as the old installer).
pub fn default_install_dir() -> Result<PathBuf> {
    let program_files = known_folder(KnownFolder::ProgramFilesX86).ctx("resolve", Path::new("ProgramFilesX86"))?;
    Ok(program_files.join("HLAE"))
}

/// Like the old `ValidateAppFolderIsLatin`: absolute path with basic latin (7-bit ASCII) characters only.
pub fn validate_install_dir(dir: &Path) -> Result<()> {
    let invalid = |reason| Err(Error::InvalidInstallDir { dir: dir.to_path_buf(), reason });
    if !dir.is_absolute() {
        return invalid("must be an absolute path");
    }
    let text = dir.to_string_lossy();
    if !text.is_ascii() {
        return invalid("only basic latin characters (7-bit ASCII) are allowed");
    }
    if text.chars().any(|c| c.is_ascii_control() || "<>\"|?*".contains(c)) {
        return invalid("contains invalid characters");
    }
    Ok(())
}

pub struct InstallOptions {
    /// `None` → installed location, else the default.
    pub install_dir: Option<PathBuf>,
    /// `None` → as installed, else on.
    pub locales: Option<bool>,
}

#[derive(Debug)]
pub struct InstallSummary {
    pub action: &'static str,
    pub previous_version: Option<String>,
    pub version: String,
    pub install_dir: PathBuf,
    pub removed: Vec<RelPath>,
    pub kept: Vec<RelPath>,
}

/// Installs / upgrades / repairs / modifies HLAE and saves the state.
pub fn install(
    state_file: &Path,
    manifest: &Manifest,
    payload: &[u8],
    options: &InstallOptions,
    written_by: &str,
) -> Result<InstallSummary> {
    let old = State::load(state_file)?;
    let install_dir = match (&options.install_dir, &old) {
        (Some(dir), _) => dir.clone(),
        (None, Some(old)) => old.install_dir.clone(),
        (None, None) => default_install_dir()?,
    };
    validate_install_dir(&install_dir)?;
    let features = Features {
        locales: options
            .locales
            .or(old.as_ref().map(|s| s.features.locales))
            .unwrap_or(true),
    };
    log::info!(
        "{} HLAE {} to {} (translations: {})",
        if old.is_some() { "updating" } else { "installing" },
        manifest.hlae_version,
        install_dir.display(),
        features.locales
    );

    let request = InstallRequest {
        install_dir: &install_dir,
        manifest,
        payload,
        features,
        written_by,
    };
    let outcome = install_hlae(&request, old.as_ref())?;
    // Until `commit()` everything can still be undone: dropping `outcome` on error rolls back.
    if let Some(dir) = state_file.parent() {
        fs::create_dir_all(dir).ctx("create", dir)?; // TODO(M3): admin-only ACL (NOTES.md 6.7)
    }
    outcome.state.save(state_file)?;
    let summary = InstallSummary {
        action: outcome.action,
        previous_version: old.map(|s| s.hlae_version),
        version: outcome.state.hlae_version.clone(),
        install_dir,
        removed: outcome.applied.removed,
        kept: outcome.applied.kept,
    };
    outcome.tx.commit()?;
    Ok(summary)
}

/// Uninstalls HLAE; `None` if not installed (no state).
///
/// If some files couldn't be removed (e.g. in use), the state keeps tracking just those, so a retry can finish.
pub fn uninstall(state_file: &Path) -> Result<Option<UninstallReport>> {
    let Some(mut state) = State::load(state_file)? else {
        return Ok(None);
    };
    log::info!("uninstalling HLAE {} from {}", state.hlae_version, state.install_dir.display());
    let report = install::uninstall(&state);
    if report.failed.is_empty() {
        match fs::remove_file(state_file) {
            Err(e) if e.kind() != io::ErrorKind::NotFound => return Err(e).ctx("remove", state_file),
            _ => {}
        }
        // `advancedfx\HLAE`, each only if empty.
        if let Some(dir) = state_file.parent() {
            let _ = fs::remove_dir(dir);
            if let Some(parent) = dir.parent().filter(|p| p.ends_with("advancedfx")) {
                let _ = fs::remove_dir(parent);
            }
        }
    } else {
        let failed: Vec<&RelPath> = report.failed.iter().map(|(path, _)| path).collect();
        state.files.retain(|f| failed.contains(&&f.path));
        state.push_history("uninstall", format!("incomplete, {} files left", failed.len()));
        state.save(state_file)?;
    }
    Ok(Some(report))
}

/// Deletes `%APPDATA%\HLAE` of the current user (uninstall option, NOTES.md 4.1).
pub fn remove_app_data() -> Result<bool> {
    let app_data = known_folder(KnownFolder::RoamingAppData).ctx("resolve", Path::new("RoamingAppData"))?;
    let dir = app_data.join("HLAE");
    if crate::fsutil::is_reparse_point(&dir).ctx("stat", &dir)? {
        return Err(Error::ReparsePoint { path: dir });
    }
    match fs::remove_dir_all(&dir) {
        Ok(()) => Ok(true),
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(false),
        Err(e) => Err(e).ctx("remove", &dir),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn install_dir_validation() {
        assert!(validate_install_dir(Path::new("C:\\Program Files (x86)\\HLAE")).is_ok());
        assert!(validate_install_dir(Path::new("D:\\Games\\HLAE 2")).is_ok());
        assert!(validate_install_dir(Path::new("HLAE")).is_err());
        assert!(validate_install_dir(Path::new("C:\\Spiele\\Größe")).is_err());
        assert!(validate_install_dir(Path::new("C:\\Users\\日本\\HLAE")).is_err());
        assert!(validate_install_dir(Path::new("C:\\a|b")).is_err());
    }
}
