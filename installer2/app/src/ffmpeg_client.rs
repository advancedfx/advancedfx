//! Client side (unelevated) of FFMPEG: resolve what to do (defaults / options, NOTES.md 4.2) and
//! download it as the logged-in user, with their proxy settings (NOTES.md 6.8a). The worker then
//! verifies the archive itself.

use std::path::{Path, PathBuf};
use std::sync::atomic::AtomicBool;

use hlae_installer_core::download::download;
use hlae_installer_core::error::IoContext;
use hlae_installer_core::ffmpeg::{self, FfmpegData};
use hlae_installer_core::ffmpeg_install::{self, FfmpegPlan, FfmpegSelection, default_selection};
use hlae_installer_core::legacy::LegacyInstall;
use hlae_installer_core::state::State;
use hlae_installer_core::{Error, Result, acl, hash};

/// What the user asked for on the command line.
#[derive(Debug, Default, PartialEq, Eq)]
pub struct FfmpegArgs {
    /// `--ffmpeg <variant|none|keep>`; `None` = defaults.
    pub selection: Option<FfmpegSelection>,
    /// `--ffmpeg-archive <zip>`: already downloaded archive.
    pub archive: Option<PathBuf>,
}

/// A downloaded FFMPEG archive, to clean up after the install.
pub struct Download {
    pub archive: PathBuf,
    /// Elevated: the archive's own private dir, removed with it afterwards in any case.
    pub private_dir: Option<PathBuf>,
}

impl Download {
    /// After the install: a private dir always goes; the shared one keeps the archive after a
    /// failure, so a retry can reuse it.
    pub fn clean_up(&self, succeeded: bool) {
        match &self.private_dir {
            Some(dir) => {
                let _ = std::fs::remove_dir_all(dir);
            }
            None if succeeded => {
                let _ = std::fs::remove_file(&self.archive);
            }
            None => {}
        }
    }
}

/// Where to download to. Unelevated: `%TEMP%\HLAE_Setup_ffmpeg` of the user (only theirs; the
/// worker copies + verifies anyway). Elevated – e.g. as SYSTEM, where `%TEMP%` is the shared
/// `C:\Windows\Temp` – a new private dir with a random name instead: a dir someone planted there
/// (e.g. a junction) must never be written into or deleted from (NOTES.md M8 audit).
fn download_dir() -> Result<(PathBuf, bool)> {
    let elevated = hlae_installer_core::win::is_elevated().unwrap_or(true);
    if !elevated {
        let dir = std::env::temp_dir().join("HLAE_Setup_ffmpeg");
        std::fs::create_dir_all(&dir).ctx("create", &dir)?;
        return Ok((dir, false));
    }
    let random = hlae_installer_core::win::random_hex().ctx("create", Path::new("random name"))?;
    let dir = std::env::temp_dir().join(format!("HLAE_Setup_ffmpeg-{random}"));
    acl::create_private_dir(&dir).ctx("create", &dir)?;
    Ok((dir, true))
}

/// Resolves the plan; downloads if needed. Returns the plan and the download to clean up after the
/// install ([`Download::clean_up`]).
pub fn resolve(
    args: &FfmpegArgs,
    data: &FfmpegData,
    state_file: &Path,
    legacy: Option<&LegacyInstall>,
    cancel: &AtomicBool,
) -> Result<(FfmpegPlan, Option<Download>)> {
    // Readable for users (NOTES.md 6.7); if it can't be read, behave like a fresh install.
    let state = State::load(state_file).unwrap_or_else(|e| {
        log::warn!("could not read the install state ({e}), using the defaults for FFMPEG");
        None
    });

    let selection = match (&args.selection, &args.archive) {
        (Some(selection), _) => selection.clone(),
        // An archive without --ffmpeg: install whatever variant it is.
        (None, Some(archive)) => FfmpegSelection::Variant(variant_of_archive(data, archive)?),
        (None, None) => match (&state, legacy) {
            // Migrating from the old WiX setup: take over its FFMPEG.
            (None, Some(legacy)) => ffmpeg_install::legacy_selection(data, legacy),
            _ => default_selection(data, state.as_ref()),
        },
    };
    log::info!("FFMPEG: {}", describe(&selection));

    match selection {
        FfmpegSelection::Keep => Ok((FfmpegPlan::Keep, None)),
        FfmpegSelection::None => Ok((FfmpegPlan::None, None)),
        FfmpegSelection::Custom(exe) => Ok((FfmpegPlan::Custom { exe }, None)),
        FfmpegSelection::Variant(id) => {
            let variant = data
                .variant(&id)
                .ok_or_else(|| Error::Manifest(format!("unknown FFMPEG variant {id}")))?;
            if let Some(archive) = &args.archive {
                ffmpeg::verify_archive(archive, variant)?;
                return Ok((
                    FfmpegPlan::Download {
                        variant: id,
                        archive: archive.clone(),
                    },
                    None,
                ));
            }
            let (dir, private) = download_dir().map_err(|e| Error::Download {
                url: variant.url.clone(),
                reason: e.to_string(),
            })?;
            let file_name = variant.url.rsplit('/').next().unwrap_or("ffmpeg.zip");
            let downloaded = Download {
                archive: dir.join(file_name),
                private_dir: private.then_some(dir),
            };
            let archive = &downloaded.archive;
            let result = if ffmpeg::verify_archive(archive, variant).is_ok() {
                log::info!("using the already downloaded {}", archive.display());
                Ok(())
            } else {
                log::info!(
                    "downloading FFMPEG {} ({} MB) from {}",
                    variant.id,
                    variant.archive_size / 1_000_000,
                    variant.url
                );
                crate::events::phase("download");
                let mut last = None;
                download(&variant.url, archive, cancel, |done, total| {
                    let total = total.unwrap_or(variant.archive_size).max(1);
                    crate::events::emit(crate::events::Event::Download { done, total });
                    let step = done * 10 / total;
                    if last != Some(step) {
                        last = Some(step);
                        log::info!("downloaded {}% ({} MB)", step * 10, done / 1_000_000);
                    }
                })
                // Early, friendly error; the worker checks again on its own copy.
                .and_then(|_| ffmpeg::verify_archive(archive, variant))
            };
            if let Err(e) = result {
                downloaded.clean_up(false);
                return Err(e);
            }
            Ok((
                FfmpegPlan::Download {
                    variant: id,
                    archive: archive.clone(),
                },
                Some(downloaded),
            ))
        }
    }
}

/// The variant whose archive this is (by SHA-256).
fn variant_of_archive(data: &FfmpegData, archive: &Path) -> Result<String> {
    let (sha256, _) = hash::sha256_file(archive)?;
    data.variants
        .iter()
        .find(|v| v.archive_sha256 == sha256)
        .map(|v| v.id.clone())
        .ok_or_else(|| Error::Verify {
            path: archive.display().to_string(),
            reason: format!(
                "not one of the FFMPEG archives of this setup ({})",
                data.version
            ),
        })
}

fn describe(selection: &FfmpegSelection) -> String {
    match selection {
        FfmpegSelection::Keep => "keep as installed".into(),
        FfmpegSelection::None => "none".into(),
        FfmpegSelection::Variant(id) => format!("install {id}"),
        FfmpegSelection::Custom(exe) => format!("use {}", exe.display()),
    }
}

/// `--ffmpeg <value>`
pub fn parse_selection(
    value: &str,
    data: &FfmpegData,
) -> std::result::Result<FfmpegSelection, String> {
    match value {
        "keep" => Ok(FfmpegSelection::Keep),
        "none" => Ok(FfmpegSelection::None),
        id if data.variant(id).is_some() => Ok(FfmpegSelection::Variant(id.to_owned())),
        _ => Err(format!(
            "--ffmpeg: unknown value {value:?}, use keep, none or one of: {}",
            data.variants
                .iter()
                .map(|v| v.id.as_str())
                .collect::<Vec<_>>()
                .join(", ")
        )),
    }
}
