//! FFMPEG variant data `installer2/data/ffmpeg.json` (NOTES.md 6.10 b) and ZIP archive inspection.

use std::collections::HashSet;
use std::fs::File;
use std::io::{self, BufReader};
use std::path::Path;

use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};

use crate::error::{Error, IoContext, Result, check_format};
use crate::hash;
use crate::relpath::RelPath;

pub const FORMAT: u32 = 1;

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct FfmpegFile {
    pub path: RelPath,
    pub size: u64,
    pub sha256: String,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct Variant {
    /// `win64-gpl`, `win64-gpl-shared`, `win64-lgpl`, `win64-lgpl-shared`
    pub id: String,
    /// `static` or `shared`
    pub linking: String,
    /// `gpl` or `lgpl`
    pub license: String,
    pub url: String,
    pub archive_size: u64,
    pub archive_sha256: String,
    /// Top-level dir inside the archive, stripped when extracting (ends with `/`).
    pub strip_prefix: String,
    /// What ends up in `<HLAE>\ffmpeg\` (paths after stripping `strip_prefix`).
    pub files: Vec<FfmpegFile>,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct FfmpegData {
    pub format: u32,
    /// One version for the whole set: same `version` (and variant) = "same version as offered".
    pub version: String,
    pub default_variant: String,
    pub variants: Vec<Variant>,
}

impl FfmpegData {
    pub fn from_json(json: &str) -> Result<Self> {
        check_format(json, "ffmpeg data", FORMAT)?;
        let data: Self = serde_json::from_str(json)?;
        data.validate()?;
        Ok(data)
    }

    pub fn to_json(&self) -> Result<String> {
        Ok(serde_json::to_string_pretty(self)? + "\n")
    }

    pub fn variant(&self, id: &str) -> Option<&Variant> {
        self.variants.iter().find(|v| v.id == id)
    }

    pub fn validate(&self) -> Result<()> {
        let invalid = |msg: String| Err(Error::Manifest(format!("ffmpeg data: {msg}")));
        if self.format != FORMAT {
            return invalid(format!("format {}", self.format));
        }
        if self.version.is_empty() {
            return invalid("empty version".into());
        }
        if self.variant(&self.default_variant).is_none() {
            return invalid(format!("unknown default variant {}", self.default_variant));
        }
        let mut ids = HashSet::new();
        for variant in &self.variants {
            let id = &variant.id;
            if !ids.insert(id.as_str()) {
                return invalid(format!("duplicate variant {id}"));
            }
            if !variant.url.starts_with("https://") {
                return invalid(format!("{id}: URL must be https"));
            }
            if !hash::is_sha256_hex(&variant.archive_sha256) {
                return invalid(format!("{id}: invalid archive_sha256"));
            }
            if !variant.strip_prefix.ends_with('/')
                || variant.strip_prefix.matches('/').count() != 1
            {
                return invalid(format!(
                    "{id}: strip_prefix must be one top-level dir ending with '/'"
                ));
            }
            RelPath::new(variant.strip_prefix.trim_end_matches('/'))?;
            let mut keys = HashSet::new();
            for file in &variant.files {
                if !hash::is_sha256_hex(&file.sha256) {
                    return invalid(format!("{id}: {}: invalid sha256", file.path));
                }
                if !keys.insert(file.path.key()) {
                    return invalid(format!(
                        "{id}: {}: duplicate (paths are case-insensitive)",
                        file.path
                    ));
                }
            }
        }
        Ok(())
    }
}

/// Top-level dir of a ZIP archive (the common first path component of all entries), with `/`.
pub fn archive_prefix(archive_path: &Path) -> Result<String> {
    let file = File::open(archive_path).ctx("open", archive_path)?;
    let mut archive =
        zip::ZipArchive::new(BufReader::new(file)).map_err(|e| zip_error(archive_path, e))?;
    let mut prefix: Option<String> = None;
    for i in 0..archive.len() {
        let entry = archive
            .by_index_raw(i)
            .map_err(|e| zip_error(archive_path, e))?;
        let name = entry.name().map_err(|e| zip_error(archive_path, e))?;
        let top = name.split('/').next().unwrap_or_default().to_owned();
        match &prefix {
            None => prefix = Some(top),
            Some(p) if *p == top => {}
            Some(_) => {
                return Err(Error::Payload(format!(
                    "{}: no single top-level dir",
                    archive_path.display()
                )));
            }
        }
    }
    prefix
        .filter(|p| !p.is_empty())
        .map(|p| p + "/")
        .ok_or_else(|| Error::Payload(format!("{}: empty archive", archive_path.display())))
}

/// Lists the files of a ZIP archive below `strip_prefix` with size + SHA-256 (decompresses each entry).
/// Entries outside `strip_prefix`, invalid paths and non-file entries (except dirs) are errors.
pub fn inspect_archive(archive_path: &Path, strip_prefix: &str) -> Result<Vec<FfmpegFile>> {
    let file = File::open(archive_path).ctx("open", archive_path)?;
    let mut archive =
        zip::ZipArchive::new(BufReader::new(file)).map_err(|e| zip_error(archive_path, e))?;
    let mut files = Vec::new();
    for i in 0..archive.len() {
        let mut entry = archive
            .by_index(i)
            .map_err(|e| zip_error(archive_path, e))?;
        let name = entry
            .name()
            .map_err(|e| zip_error(archive_path, e))?
            .into_owned();
        let Some(rel) = name.strip_prefix(strip_prefix) else {
            return Err(Error::Payload(format!("{name}: outside of {strip_prefix}")));
        };
        if entry.is_dir() || rel.is_empty() {
            continue;
        }
        if !entry.is_file() {
            return Err(Error::Payload(format!("{name}: not a regular file")));
        }
        let path = RelPath::new(rel)?;
        let mut hasher = Sha256::new();
        let size = io::copy(&mut entry, &mut hasher).ctx("read", archive_path)?;
        files.push(FfmpegFile {
            path,
            size,
            sha256: hash::to_hex(&hasher.finalize()),
        });
    }
    files.sort_by(|a, b| a.path.as_str().cmp(b.path.as_str()));
    Ok(files)
}

/// Extracts `variant`'s archive into `staging_dir`, verifying every file against `variant.files`
/// (like [`crate::payload::stage`]): entries not in the list, size / hash mismatches and missing
/// files are errors. The archive itself must already be verified (size + SHA-256).
pub fn stage_archive(
    archive_path: &Path,
    variant: &Variant,
    staging_dir: &Path,
) -> Result<Vec<crate::payload::StagedFile>> {
    use std::collections::HashMap;

    use crate::hash::HashingWriter;
    use crate::manifest::{Component, FFMPEG_DIR};

    let expected: HashMap<String, &FfmpegFile> =
        variant.files.iter().map(|f| (f.path.key(), f)).collect();
    let file = File::open(archive_path).ctx("open", archive_path)?;
    let mut archive =
        zip::ZipArchive::new(BufReader::new(file)).map_err(|e| zip_error(archive_path, e))?;
    let mut seen = HashSet::new();
    let mut staged = Vec::new();
    for i in 0..archive.len() {
        let mut entry = archive
            .by_index(i)
            .map_err(|e| zip_error(archive_path, e))?;
        let name = entry
            .name()
            .map_err(|e| zip_error(archive_path, e))?
            .into_owned();
        let Some(rel) = name.strip_prefix(&variant.strip_prefix) else {
            return Err(Error::Payload(format!(
                "{name}: outside of {}",
                variant.strip_prefix
            )));
        };
        if entry.is_dir() || rel.is_empty() {
            continue;
        }
        let path = RelPath::new(rel)?;
        let file = expected
            .get(&path.key())
            .ok_or_else(|| Error::Payload(format!("{name}: not in the FFMPEG file list")))?;
        if !entry.is_file() || !seen.insert(path.key()) {
            return Err(Error::Payload(format!(
                "{name}: not a regular file or duplicate"
            )));
        }
        let target = staging_dir.join(format!("ffmpeg-{:06}", staged.len()));
        let out = File::create_new(&target).ctx("create", &target)?;
        let mut writer = HashingWriter::new(std::io::BufWriter::new(out));
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
        staged.push(crate::payload::StagedFile {
            // Installed below `<HLAE>\ffmpeg\`.
            path: RelPath::new(&format!("{FFMPEG_DIR}/{}", file.path))?,
            sha256,
            size,
            component: Component::Ffmpeg,
            staged: target,
        });
    }
    if let Some(missing) = variant.files.iter().find(|f| !seen.contains(&f.path.key())) {
        return Err(Error::Payload(format!(
            "{}: missing from the archive",
            missing.path
        )));
    }
    Ok(staged)
}

/// SHA-256 and size of a file, checked against what `variant` expects.
pub fn verify_archive(archive_path: &Path, variant: &Variant) -> Result<()> {
    let (sha256, size) = hash::sha256_file(archive_path)?;
    if size != variant.archive_size || sha256 != variant.archive_sha256 {
        return Err(Error::Verify {
            path: archive_path.display().to_string(),
            reason: format!(
                "not the expected {} archive: expected {} bytes / {}, got {size} bytes / {sha256}",
                variant.id, variant.archive_size, variant.archive_sha256
            ),
        });
    }
    Ok(())
}

fn zip_error(path: &Path, error: zip::result::ZipError) -> Error {
    Error::Payload(format!("{}: {error}", path.display()))
}

#[cfg(test)]
mod tests {
    use std::io::Write;

    use super::*;

    fn write_zip(path: &Path, entries: &[(&str, &[u8])]) {
        let mut zip = zip::ZipWriter::new(File::create(path).unwrap());
        let options = zip::write::SimpleFileOptions::default()
            .compression_method(zip::CompressionMethod::Deflated);
        for (name, data) in entries {
            if name.ends_with('/') {
                zip.add_directory(*name, options).unwrap();
            } else {
                zip.start_file(*name, options).unwrap();
                zip.write_all(data).unwrap();
            }
        }
        zip.finish().unwrap();
    }

    #[test]
    fn inspects_archive_below_prefix() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("ffmpeg.zip");
        write_zip(
            &path,
            &[
                ("ffmpeg-x/", b""),
                ("ffmpeg-x/bin/", b""),
                ("ffmpeg-x/bin/ffmpeg.exe", b"abc"),
                ("ffmpeg-x/LICENSE.txt", b"license"),
            ],
        );
        assert_eq!(archive_prefix(&path).unwrap(), "ffmpeg-x/");
        let files = inspect_archive(&path, "ffmpeg-x/").unwrap();
        let paths: Vec<_> = files.iter().map(|f| f.path.as_str()).collect();
        assert_eq!(paths, ["LICENSE.txt", "bin/ffmpeg.exe"]);
        assert_eq!(files[1].size, 3);
        assert_eq!(
            files[1].sha256,
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
        );
    }

    #[test]
    fn rejects_entries_outside_prefix_and_bad_paths() {
        let dir = tempfile::tempdir().unwrap();
        let other = dir.path().join("other.zip");
        write_zip(&other, &[("ffmpeg-x/a", b"a"), ("evil/b", b"b")]);
        assert!(archive_prefix(&other).is_err());
        assert!(inspect_archive(&other, "ffmpeg-x/").is_err());
    }
}
