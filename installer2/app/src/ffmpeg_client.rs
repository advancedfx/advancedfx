//! Client side (unelevated) of FFMPEG: resolve what to do (defaults / options, NOTES.md 4.2) and
//! download it as the logged-in user, with their proxy settings (NOTES.md 6.8a). The worker then
//! verifies the archive itself.

use std::path::{Path, PathBuf};
use std::sync::atomic::AtomicBool;

use hlae_installer_core::download::download;
use hlae_installer_core::ffmpeg::{self, FfmpegData};
use hlae_installer_core::ffmpeg_install::{FfmpegPlan, FfmpegSelection, default_selection};
use hlae_installer_core::state::State;
use hlae_installer_core::{Error, Result, hash};

/// What the user asked for on the command line.
#[derive(Debug, Default, PartialEq, Eq)]
pub struct FfmpegArgs {
    /// `--ffmpeg <variant|none|keep>`; `None` = defaults.
    pub selection: Option<FfmpegSelection>,
    /// `--ffmpeg-archive <zip>`: already downloaded archive.
    pub archive: Option<PathBuf>,
}

/// Downloads go here (user-writable on purpose: the worker copies + verifies).
fn download_dir() -> PathBuf {
    std::env::temp_dir().join("HLAE_Setup_ffmpeg")
}

/// Resolves the plan; downloads if needed. Returns the plan and a temporary download to delete
/// after a successful install.
pub fn resolve(
    args: &FfmpegArgs,
    data: &FfmpegData,
    state_file: &Path,
    cancel: &AtomicBool,
) -> Result<(FfmpegPlan, Option<PathBuf>)> {
    // Readable for users (NOTES.md 6.7); if it can't be read, behave like a fresh install.
    let state = State::load(state_file).unwrap_or_else(|e| {
        log::warn!("could not read the install state ({e}), using the defaults for FFMPEG");
        None
    });

    let selection = match (&args.selection, &args.archive) {
        (Some(selection), _) => selection.clone(),
        // An archive without --ffmpeg: install whatever variant it is.
        (None, Some(archive)) => FfmpegSelection::Variant(variant_of_archive(data, archive)?),
        (None, None) => default_selection(data, state.as_ref()),
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
            let dir = download_dir();
            std::fs::create_dir_all(&dir).map_err(|e| Error::Download {
                url: variant.url.clone(),
                reason: e.to_string(),
            })?;
            let file_name = variant.url.rsplit('/').next().unwrap_or("ffmpeg.zip");
            let archive = dir.join(file_name);
            if ffmpeg::verify_archive(&archive, variant).is_ok() {
                log::info!("using the already downloaded {}", archive.display());
            } else {
                log::info!(
                    "downloading FFMPEG {} ({} MB) from {}",
                    variant.id,
                    variant.archive_size / 1_000_000,
                    variant.url
                );
                crate::events::phase("download");
                let mut last = None;
                download(&variant.url, &archive, cancel, |done, total| {
                    let total = total.unwrap_or(variant.archive_size).max(1);
                    crate::events::emit(crate::events::Event::Download { done, total });
                    let step = done * 10 / total;
                    if last != Some(step) {
                        last = Some(step);
                        log::info!("downloaded {}% ({} MB)", step * 10, done / 1_000_000);
                    }
                })?;
                // Early, friendly error; the worker checks again on its own copy.
                ffmpeg::verify_archive(&archive, variant)?;
            }
            Ok((
                FfmpegPlan::Download {
                    variant: id,
                    archive: archive.clone(),
                },
                Some(archive),
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
