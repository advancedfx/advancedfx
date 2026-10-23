//! Installing / switching / removing FFMPEG in `<HLAE>\ffmpeg\` (NOTES.md 4.2, 6.8, 6.8a).
//!
//! One transaction manages both FFMPEG components (`ffmpeg`, `ffmpeg-ini`), so switching between
//! variants, to / from a custom `ffmpeg.exe` or to "none" removes what's no longer wanted
//! (with the usual checksum rule) – e.g. custom → download removes our `ffmpeg.ini`.

use std::fs;
use std::io::Write;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::error::{Error, IoContext, Result};
use crate::ffmpeg::{self, FfmpegData};
use crate::hash;
use crate::manifest::Component;
use crate::payload::StagedFile;
use crate::relpath::RelPath;
use crate::state::{FfmpegChoice, FfmpegState, State};
use crate::transaction::{Applied, Transaction};

pub const FFMPEG_COMPONENTS: [Component; 2] = [Component::Ffmpeg, Component::FfmpegIni];
pub const INI_PATH: &str = "ffmpeg/ffmpeg.ini";
/// HLAE reads `Path=` into a `MAX_PATH` buffer and ignores longer values (AfxOutStreams.cpp).
pub const MAX_CUSTOM_PATH: usize = 260;

/// What should happen with FFMPEG. Built by the unelevated client (which also downloads), sent to the
/// worker, which re-validates it against its own embedded `ffmpeg.json`.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum FfmpegPlan {
    /// Leave FFMPEG as it is.
    Keep,
    /// Remove FFMPEG (and remember "no FFMPEG").
    None,
    /// Install `variant` from the already downloaded `archive` (verified by the worker).
    Download { variant: String, archive: PathBuf },
    /// Use an existing `ffmpeg.exe` via `ffmpeg.ini`.
    Custom { exe: PathBuf },
}

/// What the user / defaults want, before downloading.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum FfmpegSelection {
    Keep,
    None,
    Variant(String),
    Custom(PathBuf),
}

/// Default choice (NOTES.md 4.2), `state` = installed state, if any.
pub fn default_selection(data: &FfmpegData, state: Option<&State>) -> FfmpegSelection {
    let Some(state) = state else {
        return FfmpegSelection::Variant(data.default_variant.clone()); // fresh install
    };
    let installed = &state.ffmpeg;
    match installed.choice {
        FfmpegChoice::Custom => FfmpegSelection::Keep,
        // Deliberately not installed → leave it like that (nothing to do).
        FfmpegChoice::None => FfmpegSelection::Keep,
        FfmpegChoice::Download => {
            let variant = installed
                .variant
                .as_deref()
                .filter(|id| data.variant(id).is_some())
                .unwrap_or(&data.default_variant);
            if installed.version.as_deref() == Some(data.version.as_str())
                && installed.variant.as_deref() == Some(variant)
            {
                FfmpegSelection::Keep
            } else {
                FfmpegSelection::Variant(variant.to_owned()) // upgrade, same variant
            }
        }
    }
}

/// Default choice when migrating from the old WiX setup (NOTES.md 4.2): take over its FFMPEG
/// (same variant / custom path, downloaded again), none if it had none.
pub fn legacy_selection(
    data: &FfmpegData,
    legacy: &crate::legacy::LegacyInstall,
) -> FfmpegSelection {
    use crate::legacy::LegacyFfmpeg;
    match &legacy.ffmpeg {
        LegacyFfmpeg::None => FfmpegSelection::None,
        LegacyFfmpeg::Custom(exe) if validate_custom_exe(exe).is_ok() => {
            FfmpegSelection::Custom(exe.clone())
        }
        LegacyFfmpeg::Custom(_) => FfmpegSelection::Variant(data.default_variant.clone()),
        LegacyFfmpeg::Variant(id) if data.variant(id).is_some() => {
            FfmpegSelection::Variant(id.clone())
        }
        LegacyFfmpeg::Variant(_) => FfmpegSelection::Variant(data.default_variant.clone()),
    }
}

/// Checks a custom `ffmpeg.exe` path (absolute, existing file, short enough for HLAE).
pub fn validate_custom_exe(exe: &Path) -> Result<()> {
    use std::os::windows::ffi::OsStrExt;
    let invalid = |reason| {
        Err(Error::InvalidCustomFfmpeg {
            exe: exe.to_path_buf(),
            reason,
        })
    };
    if !exe.is_absolute() {
        return invalid("must be an absolute path");
    }
    if exe.as_os_str().encode_wide().count() > MAX_CUSTOM_PATH {
        return invalid("path longer than 260 characters, HLAE can't use it");
    }
    if !exe.is_file() {
        return invalid("file does not exist");
    }
    Ok(())
}

/// `ffmpeg.ini` content: UTF-16LE with BOM, else `GetPrivateProfileStringW` mangles non-ASCII paths.
pub fn ini_content(exe: &Path) -> Vec<u8> {
    let text = format!("[Ffmpeg]\r\nPath={}\r\n", exe.display());
    let mut bytes = vec![0xFF, 0xFE];
    bytes.extend(text.encode_utf16().flat_map(u16::to_le_bytes));
    bytes
}

/// Result of [`apply`]: files are in place but not committed (like [`crate::install::Outcome`]):
/// save `state`, then commit `tx` – or drop it to roll back.
pub struct FfmpegOutcome {
    pub state: State,
    pub tx: Transaction,
    pub applied: Applied,
    /// E.g. "install win64-gpl".
    pub action: String,
}

/// Applies `plan` to the installed HLAE in `state`. `Keep` → `None` (nothing to do).
/// `download_dir` must be admin-only: the archive is copied there before it's verified and extracted.
/// `state_file` = where `state` is saved (the transaction journal goes next to it).
pub fn apply(
    state: &State,
    data: &FfmpegData,
    plan: &FfmpegPlan,
    download_dir: &Path,
    state_file: &Path,
) -> Result<Option<FfmpegOutcome>> {
    let install_dir = &state.install_dir;
    let nothing_installed = state.ffmpeg.choice == FfmpegChoice::None
        && !state
            .files
            .iter()
            .any(|f| FFMPEG_COMPONENTS.contains(&f.component));
    let (new_ffmpeg, action) = match plan {
        FfmpegPlan::Keep => return Ok(None),
        FfmpegPlan::None if nothing_installed => return Ok(None),
        FfmpegPlan::None => (FfmpegState::default(), "remove".to_owned()),
        FfmpegPlan::Download { variant, .. } => {
            let found = data
                .variant(variant)
                .ok_or_else(|| Error::Manifest(format!("unknown FFMPEG variant {variant}")))?;
            let ffmpeg = FfmpegState {
                choice: FfmpegChoice::Download,
                variant: Some(found.id.clone()),
                version: Some(data.version.clone()),
                custom_path: None,
            };
            (ffmpeg, format!("install {} ({})", found.id, data.version))
        }
        FfmpegPlan::Custom { exe } => {
            validate_custom_exe(exe)?;
            let ffmpeg = FfmpegState {
                choice: FfmpegChoice::Custom,
                variant: None,
                version: None,
                custom_path: Some(exe.display().to_string()),
            };
            (ffmpeg, format!("use custom {}", exe.display()))
        }
    };

    let mut tx = Transaction::begin(install_dir, state_file)?;
    let staged = match plan {
        FfmpegPlan::Keep | FfmpegPlan::None => Vec::new(),
        FfmpegPlan::Download { variant, archive } => {
            let variant = data.variant(variant).expect("checked above");
            // Copy first, then verify: the source may be in a user-writable dir (%TEMP%).
            fs::create_dir_all(download_dir).ctx("create", download_dir)?;
            let copy = download_dir.join("ffmpeg.zip");
            fs::copy(archive, &copy).ctx("copy FFMPEG archive", archive)?;
            let result = ffmpeg::verify_archive(&copy, variant)
                .and_then(|()| ffmpeg::stage_archive(&copy, variant, &tx.staging_dir()));
            let _ = fs::remove_file(&copy);
            result?
        }
        FfmpegPlan::Custom { exe } => vec![stage_ini(&tx.staging_dir(), exe)?],
    };
    let applied = tx.apply(staged, Vec::new(), &FFMPEG_COMPONENTS, &state.files)?;

    let mut state = state.clone();
    state.ffmpeg = new_ffmpeg;
    state.replace_files(&FFMPEG_COMPONENTS, applied.files.clone());
    state.push_history("ffmpeg", action.clone());
    Ok(Some(FfmpegOutcome {
        state,
        tx,
        applied,
        action,
    }))
}

fn stage_ini(staging_dir: &Path, exe: &Path) -> Result<StagedFile> {
    let content = ini_content(exe);
    let staged = staging_dir.join("ffmpeg.ini");
    let mut file = fs::File::create_new(&staged).ctx("create", &staged)?;
    file.write_all(&content).ctx("write", &staged)?;
    let mut hasher = hash::HashingWriter::new(std::io::sink());
    hasher.write_all(&content).ctx("hash", &staged)?;
    let (sha256, size, _) = hasher.finish().ctx("hash", &staged)?;
    Ok(StagedFile {
        path: RelPath::new(INI_PATH)?,
        sha256,
        size,
        component: Component::FfmpegIni,
        staged,
    })
}

/// Warnings about the FFMPEG setup in `state` (e.g. a hand-written `ffmpeg.ini` overriding the
/// downloaded FFMPEG, NOTES.md 6.8).
pub fn warnings(state: &State) -> Vec<String> {
    let mut warnings = Vec::new();
    let ini = RelPath::new(INI_PATH)
        .expect("valid")
        .to_path(&state.install_dir);
    let tracked = state
        .files
        .iter()
        .any(|f| f.component == Component::FfmpegIni);
    if state.ffmpeg.choice == FfmpegChoice::Download && !tracked && ini.exists() {
        warnings.push(format!(
            "{} was not written by HLAE Setup and overrides the installed FFMPEG – delete it to use the installed one",
            ini.display()
        ));
    }
    warnings
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ini_is_utf16le_with_bom() {
        let bytes = ini_content(Path::new("C:\\Größe\\ffmpeg.exe"));
        assert_eq!(&bytes[..2], &[0xFF, 0xFE]);
        let text: Vec<u16> = bytes[2..]
            .chunks(2)
            .map(|c| u16::from_le_bytes([c[0], c[1]]))
            .collect();
        assert_eq!(
            String::from_utf16(&text).unwrap(),
            "[Ffmpeg]\r\nPath=C:\\Größe\\ffmpeg.exe\r\n"
        );
    }
}
