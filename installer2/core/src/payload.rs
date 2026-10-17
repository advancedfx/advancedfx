//! Verified extraction of the payload into a staging dir.

use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::io::{self, BufWriter};
use std::path::{Path, PathBuf};

use crate::error::{Error, IoContext, Result};
use crate::hash::HashingWriter;
use crate::manifest::{Component, Manifest, ManifestFile};
use crate::relpath::RelPath;

/// zstd window of the payload (2^27 = 128 MiB): the packer (build-support) compresses with it,
/// the decoder here must allow at least this.
pub const ZSTD_WINDOW_LOG: u32 = 27;

/// A verified file waiting in the staging dir to be moved into place.
#[derive(Debug)]
pub struct StagedFile {
    pub path: RelPath,
    pub sha256: String,
    pub size: u64,
    pub component: Component,
    pub staged: PathBuf,
}

/// Extracts the files of `components` from `payload` into `staging_dir`, except those in `skip`
/// ([`RelPath::key`]s of files already installed exactly like this).
///
/// Streams the archive (never holds it in memory) and verifies every file against `manifest`:
/// entries not in the manifest, non-regular entries, duplicates, size / hash mismatches and
/// missing files are errors. If every file is skipped, the payload isn't read at all.
pub fn stage(
    payload: &[u8],
    manifest: &Manifest,
    components: &[Component],
    staging_dir: &Path,
    skip: &HashSet<String>,
) -> Result<Vec<StagedFile>> {
    let invalid = |msg: String| Error::Payload(msg);
    let expected: HashMap<String, &ManifestFile> =
        manifest.files.iter().map(|f| (f.path.key(), f)).collect();
    let wanted = |file: &ManifestFile| {
        components.contains(&file.feature) && !skip.contains(&file.path.key())
    };
    if !manifest.files.iter().any(wanted) {
        return Ok(Vec::new());
    }

    let mut decoder = zstd::Decoder::with_buffer(payload).ctx("decompress", staging_dir)?;
    decoder
        .window_log_max(ZSTD_WINDOW_LOG)
        .ctx("decompress", staging_dir)?;
    let mut archive = tar::Archive::new(decoder);

    let mut seen = HashSet::new();
    let mut staged = Vec::new();
    for entry in archive.entries().ctx("read payload", staging_dir)? {
        let mut entry = entry.ctx("read payload", staging_dir)?;
        let name = String::from_utf8(entry.path_bytes().into_owned())
            .map_err(|_| invalid("entry name is not UTF-8".into()))?;
        if entry.header().entry_type() != tar::EntryType::Regular {
            return Err(invalid(format!("{name}: not a regular file")));
        }
        let path = RelPath::new(&name)?;
        let file = expected
            .get(&path.key())
            .ok_or_else(|| invalid(format!("{name}: not in manifest")))?;
        if !seen.insert(path.key()) {
            return Err(invalid(format!("{name}: duplicate entry")));
        }
        if !wanted(file) {
            continue;
        }

        let target = staging_dir.join(format!("{:06}", staged.len()));
        let out = File::create_new(&target).ctx("create", &target)?;
        let mut writer = HashingWriter::new(BufWriter::new(out));
        io::copy(&mut entry, &mut writer).ctx("extract", &target)?;
        let (sha256, size, _) = writer.finish().ctx("write", &target)?;
        if size != file.size || sha256 != file.sha256 {
            return Err(Error::Verify {
                path: name,
                reason: format!(
                    "expected {} bytes / {}, got {size} bytes / {sha256}",
                    file.size, file.sha256
                ),
            });
        }
        staged.push(StagedFile {
            path: file.path.clone(),
            sha256,
            size,
            component: file.feature,
            staged: target,
        });
    }

    if let Some(missing) = manifest
        .files
        .iter()
        .find(|f| components.contains(&f.feature) && !seen.contains(&f.path.key()))
    {
        return Err(invalid(format!("{}: missing from payload", missing.path)));
    }
    Ok(staged)
}
