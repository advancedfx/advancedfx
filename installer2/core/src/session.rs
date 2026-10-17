//! Top-level operations as used by CLI / GUI / worker: state file and Windows integration around
//! [`crate::install`].

use std::fs;
use std::io;
use std::path::{Path, PathBuf};

use crate::acl;
use crate::error::{Error, IoContext, Result};
use crate::ffmpeg::FfmpegData;
use crate::ffmpeg_install::{self, FfmpegOutcome, FfmpegPlan};
use crate::fsutil;
use crate::in_use;
use crate::install::{self, InstallRequest, UninstallReport, install_hlae};
use crate::integration;
use crate::legacy::{self, LegacyInstall};
use crate::manifest::Manifest;
use crate::relpath::RelPath;
use crate::state::{Features, State};
use crate::win::{KnownFolder, known_folder};

/// `C:\Program Files (x86)\HLAE` (same as the old installer).
pub fn default_install_dir() -> Result<PathBuf> {
    let program_files =
        known_folder(KnownFolder::ProgramFilesX86).ctx("resolve", Path::new("ProgramFilesX86"))?;
    Ok(program_files.join("HLAE"))
}

/// Like the old `ValidateAppFolderIsLatin`: absolute path with basic latin (7-bit ASCII) characters only.
pub fn validate_install_dir(dir: &Path) -> Result<()> {
    let invalid = |reason| {
        Err(Error::InvalidInstallDir {
            dir: dir.to_path_buf(),
            reason,
        })
    };
    if !dir.is_absolute() {
        return invalid("must be an absolute path");
    }
    let text = dir.to_string_lossy();
    if !text.is_ascii() {
        return invalid("only basic latin characters (7-bit ASCII) are allowed");
    }
    if text
        .chars()
        .any(|c| c.is_ascii_control() || "<>\"|?*".contains(c))
    {
        return invalid("contains invalid characters");
    }
    Ok(())
}

/// Where the state lives and whether the system is touched outside the install dir.
pub struct Session {
    /// `%ProgramData%\advancedfx\HLAE\state.json`
    pub state_file: PathBuf,
    /// Admin-only state dir, setup cache, Apps & features, `APPLICATIONFOLDER`, shortcut.
    /// Off only for tests without admin rights.
    pub integrate: bool,
}

pub struct InstallOptions {
    /// `None` → installed location, else the default.
    pub install_dir: Option<PathBuf>,
    /// `None` → as installed, else on.
    pub locales: Option<bool>,
    /// Already resolved by the client (defaults, download), see [`crate::ffmpeg_install`].
    pub ffmpeg: FfmpegPlan,
}

#[derive(Debug)]
pub struct InstallSummary {
    pub action: &'static str,
    pub previous_version: Option<String>,
    pub version: String,
    pub install_dir: PathBuf,
    pub removed: Vec<RelPath>,
    pub kept: Vec<RelPath>,
    /// E.g. "install win64-gpl (…)", `None` = FFMPEG kept as it was.
    pub ffmpeg_action: Option<String>,
    pub ffmpeg_warnings: Vec<String>,
}

#[derive(Debug)]
pub struct UninstallSummary {
    pub files: UninstallReport,
    pub integration_errors: Vec<String>,
}

impl Session {
    /// The real thing: state under `%ProgramData%`, full integration.
    pub fn system() -> Result<Self> {
        let program_data =
            known_folder(KnownFolder::ProgramData).ctx("resolve", Path::new("ProgramData"))?;
        Ok(Self {
            state_file: program_data
                .join("advancedfx")
                .join("HLAE")
                .join("state.json"),
            integrate: true,
        })
    }

    fn state_dir(&self) -> &Path {
        self.state_file.parent().expect("state file has a parent")
    }

    /// Makes sure `advancedfx\HLAE` (state dir) is admin-only before anything in it is trusted
    /// (NOTES.md 6.7). Refuses if `advancedfx` was pre-created by a normal user.
    fn secure_state_dir(&self) -> Result<()> {
        if !self.integrate {
            return Ok(());
        }
        let dir = self.state_dir();
        let parent = dir.parent().expect("state dir has a parent");
        for path in [parent, dir] {
            if fsutil::is_reparse_point(path).ctx("stat", path)? {
                return Err(Error::ReparsePoint {
                    path: path.to_path_buf(),
                });
            }
        }
        if parent.exists() && !acl::owner_is_trusted(parent).ctx("check owner", parent)? {
            return Err(Error::Conflict {
                path: parent.to_path_buf(),
                reason: "exists, but isn't owned by SYSTEM or Administrators – refusing to use it",
            });
        }
        fs::create_dir_all(dir).ctx("create", dir)?;
        acl::secure_dir(parent, true).ctx("secure", parent)?;
        acl::secure_dir(dir, true).ctx("secure", dir)
    }

    /// Installs / upgrades / repairs / modifies HLAE and FFMPEG, saves the state and integrates it
    /// into Windows.
    pub fn install(
        &self,
        manifest: &Manifest,
        payload: &[u8],
        ffmpeg_data: &FfmpegData,
        options: &InstallOptions,
        written_by: &str,
    ) -> Result<InstallSummary> {
        self.secure_state_dir()?;
        let old = State::load(&self.state_file)?;
        // The old WiX based setup (NOTES.md 5), only relevant before our first install.
        let legacy = (self.integrate && old.is_none())
            .then(legacy::detect)
            .flatten();
        if let Some(legacy) = legacy.as_ref().filter(|l| l.is_wix_v3()) {
            return Err(Error::Legacy(legacy_v3_message(legacy)));
        }
        let install_dir = match (&options.install_dir, &old, &legacy) {
            (Some(dir), _, _) => dir.clone(),
            (None, Some(old), _) => old.install_dir.clone(),
            (None, None, Some(legacy)) if legacy.install_dir.is_some() => {
                legacy.install_dir.clone().expect("checked")
            }
            (None, None, _) => default_install_dir()?,
        };
        validate_install_dir(&install_dir)?;
        // The client checked already (before elevating); this catches programs started since.
        in_use::ensure_not_in_use(&in_use::files_to_check(old.as_ref(), legacy.as_ref()))?;
        if let Some(legacy) = &legacy {
            legacy::uninstall(legacy)?;
        }
        let features = Features {
            locales: options
                .locales
                .or(old.as_ref().map(|s| s.features.locales))
                .unwrap_or(true),
        };
        log::info!(
            "{} HLAE {} to {} (translations: {})",
            if old.is_some() {
                "updating"
            } else {
                "installing"
            },
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
        let mut outcome = outcome;
        if let Some(legacy) = &legacy {
            let history = std::mem::take(&mut outcome.state.history);
            outcome.state.push_history(
                "migrate",
                format!("WiX v5 HLAE Setup {}", legacy.version().unwrap_or_default()),
            );
            outcome.state.history.extend(history);
        }
        fs::create_dir_all(self.state_dir()).ctx("create", self.state_dir())?;
        outcome.state.save(&self.state_file)?;
        let mut state = outcome.state;
        let mut summary = InstallSummary {
            action: outcome.action,
            previous_version: old.map(|s| s.hlae_version),
            version: state.hlae_version.clone(),
            install_dir,
            removed: outcome.applied.removed,
            kept: outcome.applied.kept,
            ffmpeg_action: None,
            ffmpeg_warnings: Vec::new(),
        };
        outcome.tx.commit()?;

        // FFMPEG after HLAE (separate transaction): if it fails, HLAE stays installed and a retry
        // only has to redo FFMPEG.
        let download_dir = self.state_dir().join("download");
        let ffmpeg = ffmpeg_install::apply(&state, ffmpeg_data, &options.ffmpeg, &download_dir);
        let _ = fs::remove_dir(&download_dir);
        let ffmpeg_error = match ffmpeg {
            Ok(Some(FfmpegOutcome {
                state: new_state,
                tx,
                action,
                ..
            })) => {
                new_state.save(&self.state_file)?;
                tx.commit()?;
                state = new_state;
                summary.ffmpeg_action = Some(action);
                None
            }
            Ok(None) => None,
            Err(e) => Some(e),
        };
        summary.ffmpeg_warnings = ffmpeg_install::warnings(&state);

        // Also after an FFMPEG error: HLAE is installed and needs its Apps & features entry etc.
        if self.integrate {
            let setup_exe = std::env::current_exe().ctx("locate", Path::new("HLAE_Setup.exe"))?;
            let result =
                integration::apply(&mut state, &setup_exe, &self.state_dir().join("setup"));
            // Record what was done, also if it failed half way (a repair fixes the rest).
            state.save(&self.state_file)?;
            result?;
        }
        match ffmpeg_error {
            Some(e) => Err(e),
            None => Ok(summary),
        }
    }

    /// Uninstalls HLAE; `None` if not installed (no state).
    ///
    /// If some files couldn't be removed (e.g. in use), the state keeps tracking just those and the
    /// integration (Apps & features entry, …) stays, so a retry can finish.
    pub fn uninstall(&self) -> Result<Option<UninstallSummary>> {
        self.secure_state_dir()?;
        let Some(mut state) = State::load(&self.state_file)? else {
            return Ok(None);
        };
        log::info!(
            "uninstalling HLAE {} from {}",
            state.hlae_version,
            state.install_dir.display()
        );
        in_use::ensure_not_in_use(&in_use::files_to_check(Some(&state), None))?;
        let files = install::uninstall(&state);
        if !files.failed.is_empty() {
            let failed: Vec<&RelPath> = files.failed.iter().map(|(path, _)| path).collect();
            state.files.retain(|f| failed.contains(&&f.path));
            state.push_history(
                "uninstall",
                format!("incomplete, {} files left", failed.len()),
            );
            state.save(&self.state_file)?;
            return Ok(Some(UninstallSummary {
                files,
                integration_errors: Vec::new(),
            }));
        }

        let integration_errors = integration::remove(&state);
        match fs::remove_file(&self.state_file) {
            Err(e) if e.kind() != io::ErrorKind::NotFound => {
                return Err(e).ctx("remove", &self.state_file);
            }
            _ => {}
        }
        // `advancedfx\HLAE`, each only if empty.
        let dir = self.state_dir();
        let _ = fs::remove_dir(dir);
        if let Some(parent) = dir.parent().filter(|p| p.ends_with("advancedfx")) {
            let _ = fs::remove_dir(parent);
        }
        Ok(Some(UninstallSummary {
            files,
            integration_errors,
        }))
    }
}

/// HLAE installed by the WiX v3 setup isn't migrated (NOTES.md 5): the user uninstalls it first.
pub fn legacy_v3_message(legacy: &LegacyInstall) -> String {
    format!(
        "HLAE {} was installed by an old HLAE Setup (before {}) that can't be updated automatically. \
         Please uninstall it in Apps & features first, then run this setup again.",
        legacy.version().unwrap_or_default(),
        legacy::WIX_V5_FIRST
    )
}

/// Deletes `%APPDATA%\HLAE` of the current user (uninstall option, NOTES.md 4.1).
pub fn remove_app_data() -> Result<bool> {
    let app_data =
        known_folder(KnownFolder::RoamingAppData).ctx("resolve", Path::new("RoamingAppData"))?;
    let dir = app_data.join("HLAE");
    if fsutil::is_reparse_point(&dir).ctx("stat", &dir)? {
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
