//! Packs a staged HLAE build into the payload (`payload.tar.zst` + manifest), used by the app's `build.rs`.

use std::fs::{self, File};
use std::io::Write;
use std::os::windows::fs::MetadataExt;
use std::path::{Path, PathBuf};

use hlae_installer_core::error::{Error, IoContext, Result};
use hlae_installer_core::fsutil::FILE_ATTRIBUTE_REPARSE_POINT;
use hlae_installer_core::hash;
use hlae_installer_core::manifest::{FFMPEG_DIR, FORMAT, Manifest, ManifestFile};
use hlae_installer_core::payload::ZSTD_WINDOW_LOG;
use hlae_installer_core::relpath::RelPath;

/// Compression level for release builds; debug builds may use a faster one.
pub const ZSTD_LEVEL: i32 = 19;

/// Packs all files below `src_dir` into a tar.zst written to `out` and returns the manifest.
///
/// A top-level `ffmpeg/` dir is skipped (FFMPEG is installed separately, NOTES.md 6.8).
/// Output is deterministic: sorted paths, no timestamps / owners in the tar headers.
pub fn pack_dir<W: Write>(
    src_dir: &Path,
    hlae_version: &str,
    level: i32,
    out: W,
) -> Result<Manifest> {
    let mut sources = Vec::new();
    collect(src_dir, None, &mut sources)?;
    sources.sort_by(|a, b| a.0.as_str().cmp(b.0.as_str()));

    let mut encoder = zstd::Encoder::new(out, level).ctx("compress", src_dir)?;
    encoder
        .long_distance_matching(true)
        .ctx("compress", src_dir)?;
    encoder
        .window_log(ZSTD_WINDOW_LOG)
        .ctx("compress", src_dir)?;
    encoder.include_checksum(true).ctx("compress", src_dir)?;
    let mut tar = tar::Builder::new(encoder);

    let mut files = Vec::with_capacity(sources.len());
    for (path, full) in sources {
        let (sha256, size) = hash::sha256_file(&full)?;
        let mut header = tar::Header::new_gnu();
        header.set_entry_type(tar::EntryType::Regular);
        header.set_size(size);
        header.set_mode(0o644);
        header.set_mtime(0);
        let file = File::open(&full).ctx("open", &full)?;
        tar.append_data(&mut header, path.as_str(), file)
            .ctx("pack", &full)?;
        files.push(ManifestFile {
            feature: Manifest::feature_of(&path),
            path,
            size,
            sha256,
        });
    }

    let encoder = tar.into_inner().ctx("pack", src_dir)?;
    encoder
        .finish()
        .ctx("compress", src_dir)?
        .flush()
        .ctx("write", src_dir)?;

    let manifest = Manifest {
        format: FORMAT,
        hlae_version: hlae_version.to_owned(),
        files,
    };
    manifest.validate()?;
    Ok(manifest)
}

fn collect(dir: &Path, prefix: Option<&str>, out: &mut Vec<(RelPath, PathBuf)>) -> Result<()> {
    for entry in fs::read_dir(dir).ctx("list", dir)? {
        let entry = entry.ctx("list", dir)?;
        let full = entry.path();
        let name = entry.file_name();
        let name = name.to_str().ok_or_else(|| Error::InvalidPath {
            path: full.display().to_string(),
            reason: "not valid Unicode",
        })?;
        if prefix.is_none() && name.eq_ignore_ascii_case(FFMPEG_DIR) {
            continue;
        }
        let rel = match prefix {
            Some(prefix) => format!("{prefix}/{name}"),
            None => name.to_owned(),
        };
        // read_dir metadata doesn't follow reparse points.
        let meta = entry.metadata().ctx("stat", &full)?;
        if meta.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
            return Err(Error::ReparsePoint { path: full });
        }
        if meta.is_dir() {
            collect(&full, Some(&rel), out)?;
        } else {
            out.push((RelPath::new(&rel)?, full));
        }
    }
    Ok(())
}
