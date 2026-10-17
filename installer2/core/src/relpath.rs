use std::fmt;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Deserializer, Serialize, Serializer};

use crate::error::{Error, Result};

/// Validated path relative to an install dir or archive root, `/` separated (see NOTES.md 6.10).
#[derive(Clone, Debug, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct RelPath(String);

const RESERVED_NAMES: [&str; 22] = [
    "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
    "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
];

impl RelPath {
    pub fn new(path: &str) -> Result<Self> {
        validate(path)?;
        Ok(Self(path.to_owned()))
    }

    pub fn as_str(&self) -> &str {
        &self.0
    }

    /// Key for comparing paths case-insensitively, like Windows file names.
    pub fn key(&self) -> String {
        self.0.to_lowercase()
    }

    pub fn components(&self) -> impl Iterator<Item = &str> {
        self.0.split('/')
    }

    /// Whether the path lies below the top-level directory `dir` (case-insensitive).
    pub fn is_below(&self, dir: &str) -> bool {
        self.key().starts_with(&format!("{}/", dir.to_lowercase()))
    }

    pub fn to_path(&self, root: &Path) -> PathBuf {
        let mut path = root.to_path_buf();
        path.extend(self.components());
        path
    }
}

fn validate(path: &str) -> Result<()> {
    let invalid = |reason| {
        Err(Error::InvalidPath {
            path: path.to_owned(),
            reason,
        })
    };
    if path.is_empty() {
        return invalid("empty");
    }
    for component in path.split('/') {
        if component.is_empty() {
            return invalid("empty component (leading, trailing or double '/')");
        }
        if component == "." || component == ".." {
            return invalid("'.' or '..' component");
        }
        if component.ends_with('.') || component.ends_with(' ') {
            return invalid("component ends with '.' or ' '");
        }
        if component
            .chars()
            .any(|c| (c as u32) < 0x20 || "<>:\"\\|?*".contains(c))
        {
            return invalid("invalid character");
        }
        let stem = component.split('.').next().unwrap_or_default();
        let stem = stem.trim_end_matches(' ').to_ascii_uppercase();
        if RESERVED_NAMES.contains(&stem.as_str()) {
            return invalid("reserved device name");
        }
    }
    Ok(())
}

impl fmt::Display for RelPath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&self.0)
    }
}

impl Serialize for RelPath {
    fn serialize<S: Serializer>(&self, serializer: S) -> std::result::Result<S::Ok, S::Error> {
        serializer.serialize_str(&self.0)
    }
}

impl<'de> Deserialize<'de> for RelPath {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> std::result::Result<Self, D::Error> {
        let path = String::deserialize(deserializer)?;
        RelPath::new(&path).map_err(serde::de::Error::custom)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn accepts_normal_paths() {
        for path in ["HLAE.exe", "x64/AfxHookSource2.dll", "locales/pt-BR/hlae/messages.mo", "a b/c.d.e"] {
            assert!(RelPath::new(path).is_ok(), "{path}");
        }
    }

    #[test]
    fn rejects_dangerous_paths() {
        for path in [
            "", "/abs", "C:/x", "C:", "a//b", "a/", "../x", "a/../b", "./a", "a\\b", "\\\\server\\share",
            "file.txt:stream", "a.", "a ", "CON", "nul.txt", "x/COM1.dll", "lpt9", "a\u{1}b", "a*b",
        ] {
            assert!(RelPath::new(path).is_err(), "{path:?}");
        }
    }

    #[test]
    fn case_insensitive_key_and_below() {
        let path = RelPath::new("Locales/de/hlae/messages.mo").unwrap();
        assert_eq!(path.key(), "locales/de/hlae/messages.mo");
        assert!(path.is_below("locales"));
        assert!(!RelPath::new("localesX/a").unwrap().is_below("locales"));
    }

    #[test]
    fn to_path_uses_native_separators() {
        let path = RelPath::new("x64/a.dll").unwrap();
        assert_eq!(path.to_path(Path::new("C:\\HLAE")), PathBuf::from("C:\\HLAE\\x64\\a.dll"));
    }
}
