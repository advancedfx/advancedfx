use std::path::{Path, PathBuf};

use serde::Deserialize;

#[derive(Debug, thiserror::Error)]
pub enum Error {
    #[error("{op} {}: {source}", .path.display())]
    Io {
        op: &'static str,
        path: PathBuf,
        #[source]
        source: std::io::Error,
    },
    #[error("invalid path {path:?}: {reason}")]
    InvalidPath { path: String, reason: &'static str },
    #[error("invalid version {0:?}")]
    InvalidVersion(String),
    #[error("invalid manifest: {0}")]
    Manifest(String),
    #[error("invalid state: {0}")]
    State(String),
    #[error("unsupported {what} format {found} (this installer supports up to {known})")]
    UnsupportedFormat {
        what: &'static str,
        found: u32,
        known: u32,
    },
    #[error("invalid payload: {0}")]
    Payload(String),
    #[error("verification failed for {path}: {reason}")]
    Verify { path: String, reason: String },
    #[error("{} is a reparse point (junction / symbolic link), refusing to touch it", .path.display())]
    ReparsePoint { path: PathBuf },
    #[error("{}: {reason}", .path.display())]
    Conflict { path: PathBuf, reason: &'static str },
    #[error("downgrade from {installed} to {new} is not supported, uninstall first")]
    Downgrade { installed: String, new: String },
    #[error("invalid install dir {}: {reason}", .dir.display())]
    InvalidInstallDir { dir: PathBuf, reason: &'static str },
    #[error("{} already exists and isn't empty – please choose a new or empty folder", .dir.display())]
    InstallDirNotEmpty { dir: PathBuf },
    #[error("install dir {} differs from the installed location {}", .requested.display(), .installed.display())]
    InstallDirMismatch {
        installed: PathBuf,
        requested: PathBuf,
    },
    #[error("invalid custom ffmpeg.exe {}: {reason}", .exe.display())]
    InvalidCustomFfmpeg { exe: PathBuf, reason: &'static str },
    #[error("{0}")]
    Legacy(String),
    #[error("download of {url} failed: {reason}")]
    Download { url: String, reason: String },
    #[error("cancelled")]
    Cancelled,
    #[error(
        "HLAE files are in use by: {}. Please close these programs and try again.",
        .0.iter().map(|h| h.to_string()).collect::<Vec<_>>().join(", ")
    )]
    FilesInUse(Vec<crate::in_use::Holder>),
    #[error("JSON: {0}")]
    Json(#[from] serde_json::Error),
}

pub type Result<T> = std::result::Result<T, Error>;

/// Attaches the operation and path to an `io::Error`.
pub trait IoContext<T> {
    fn ctx(self, op: &'static str, path: &Path) -> Result<T>;
}

impl<T> IoContext<T> for std::io::Result<T> {
    fn ctx(self, op: &'static str, path: &Path) -> Result<T> {
        self.map_err(|source| Error::Io {
            op,
            path: path.to_path_buf(),
            source,
        })
    }
}

/// Refuses JSON whose top-level `format` is unknown (0 or newer than `known`).
pub(crate) fn check_format(json: &str, what: &'static str, known: u32) -> Result<()> {
    #[derive(Deserialize)]
    struct Format {
        format: u32,
    }
    let found = serde_json::from_str::<Format>(json)?.format;
    if found == 0 || found > known {
        return Err(Error::UnsupportedFormat { what, found, known });
    }
    Ok(())
}
