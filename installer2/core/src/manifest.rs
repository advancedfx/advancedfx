//! Payload manifest, see NOTES.md 6.10 a).

use std::collections::HashSet;

use serde::{Deserialize, Serialize};

use crate::error::{Error, Result, check_format};
use crate::hash;
use crate::relpath::RelPath;
use crate::transaction::WORK_DIR_NAME;
use crate::version;

pub const FORMAT: u32 = 1;

/// Top-level dir of the install dir reserved for FFMPEG (NOTES.md 6.8), never part of the HLAE payload.
pub const FFMPEG_DIR: &str = "ffmpeg";
/// Top-level dir holding the optional translations.
pub const LOCALES_DIR: &str = "locales";

/// What an installed file belongs to.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash, PartialOrd, Ord, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum Component {
    Core,
    Locales,
    Ffmpeg,
    FfmpegIni,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct ManifestFile {
    pub path: RelPath,
    pub size: u64,
    pub sha256: String,
    pub feature: Component,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct Manifest {
    pub format: u32,
    pub hlae_version: String,
    pub files: Vec<ManifestFile>,
}

impl Manifest {
    pub fn from_json(json: &str) -> Result<Self> {
        check_format(json, "manifest", FORMAT)?;
        let manifest: Self = serde_json::from_str(json)?;
        manifest.validate()?;
        Ok(manifest)
    }

    pub fn to_json(&self) -> Result<String> {
        Ok(serde_json::to_string_pretty(self)?)
    }

    /// Feature a payload path belongs to.
    pub fn feature_of(path: &RelPath) -> Component {
        if path.is_below(LOCALES_DIR) {
            Component::Locales
        } else {
            Component::Core
        }
    }

    pub fn validate(&self) -> Result<()> {
        let invalid = |msg: String| Err(Error::Manifest(msg));
        if self.format != FORMAT {
            return invalid(format!("format {}", self.format));
        }
        version::parse(&self.hlae_version)?;
        let mut keys = HashSet::new();
        for file in &self.files {
            let path = &file.path;
            if path.is_below(FFMPEG_DIR) || path.key() == FFMPEG_DIR {
                return invalid(format!("{path}: '{FFMPEG_DIR}/' is reserved for FFMPEG"));
            }
            if path
                .components()
                .next()
                .is_some_and(|c| c.eq_ignore_ascii_case(WORK_DIR_NAME))
            {
                return invalid(format!(
                    "{path}: '{WORK_DIR_NAME}' is reserved for the installer"
                ));
            }
            if file.feature != Self::feature_of(path) {
                return invalid(format!("{path}: wrong feature {:?}", file.feature));
            }
            if !hash::is_sha256_hex(&file.sha256) {
                return invalid(format!("{path}: invalid sha256"));
            }
            if !keys.insert(path.key()) {
                return invalid(format!("{path}: duplicate (paths are case-insensitive)"));
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn file(path: &str, feature: Component) -> ManifestFile {
        ManifestFile {
            path: RelPath::new(path).unwrap(),
            size: 1,
            sha256: "0".repeat(64),
            feature,
        }
    }

    fn manifest(files: Vec<ManifestFile>) -> Manifest {
        Manifest {
            format: FORMAT,
            hlae_version: "2.192.8".into(),
            files,
        }
    }

    #[test]
    fn json_roundtrip() {
        let m = manifest(vec![
            file("HLAE.exe", Component::Core),
            file("locales/de/x.mo", Component::Locales),
        ]);
        assert_eq!(Manifest::from_json(&m.to_json().unwrap()).unwrap(), m);
    }

    #[test]
    fn rejects_invalid() {
        for files in [
            vec![file("ffmpeg/bin/ffmpeg.exe", Component::Core)],
            vec![file(".hlae-setup-tmp/x", Component::Core)],
            vec![file("locales/de/x.mo", Component::Core)],
            vec![file("HLAE.exe", Component::Locales)],
            vec![
                file("a.dll", Component::Core),
                file("A.DLL", Component::Core),
            ],
        ] {
            assert!(manifest(files).validate().is_err());
        }
    }

    #[test]
    fn refuses_newer_format() {
        let json = r#"{ "format": 2, "hlae_version": "1.0", "files": [], "new_field": 1 }"#;
        assert!(matches!(
            Manifest::from_json(json),
            Err(Error::UnsupportedFormat { found: 2, .. })
        ));
    }
}
