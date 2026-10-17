//! Install state `%ProgramData%\advancedfx\HLAE\state.json`, see NOTES.md 6.7 / 6.10 c).

use std::collections::HashSet;
use std::fs::{self, File};
use std::io::{self, Write};
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::error::{Error, IoContext, Result, check_format};
use crate::manifest::Component;
use crate::relpath::RelPath;
use crate::time;

pub const FORMAT: u32 = 1;
pub const HISTORY_MAX: usize = 20;

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct FileRecord {
    pub path: RelPath,
    pub sha256: String,
    pub component: Component,
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Features {
    pub locales: bool,
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum FfmpegChoice {
    #[default]
    None,
    Download,
    Custom,
}

#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct FfmpegState {
    pub choice: FfmpegChoice,
    pub variant: Option<String>,
    pub version: Option<String>,
    pub custom_path: Option<String>,
}

/// What we registered outside the install dir, so uninstall removes exactly that.
#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Integration {
    pub arp_key: Option<String>,
    pub shortcut: Option<PathBuf>,
    pub app_folder_reg: bool,
    /// Cached copy of the installer, used as uninstaller (NOTES.md 6.5).
    pub setup_cache: Option<PathBuf>,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct HistoryEntry {
    pub action: String,
    pub time: String,
    pub detail: String,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct State {
    pub format: u32,
    pub written_by: String,
    pub install_dir: PathBuf,
    pub hlae_version: String,
    pub features: Features,
    pub ffmpeg: FfmpegState,
    pub files: Vec<FileRecord>,
    pub integration: Integration,
    pub history: Vec<HistoryEntry>,
}

impl State {
    pub fn new(
        written_by: &str,
        install_dir: &Path,
        hlae_version: &str,
        features: Features,
    ) -> Self {
        Self {
            format: FORMAT,
            written_by: written_by.to_owned(),
            install_dir: install_dir.to_path_buf(),
            hlae_version: hlae_version.to_owned(),
            features,
            ffmpeg: FfmpegState::default(),
            files: Vec::new(),
            integration: Integration::default(),
            history: Vec::new(),
        }
    }

    /// Loads the state, `None` if there is none.
    pub fn load(path: &Path) -> Result<Option<Self>> {
        match fs::read_to_string(path) {
            Ok(json) => Self::from_json(&json).map(Some),
            Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(None),
            Err(e) => Err(e).ctx("read", path),
        }
    }

    pub fn from_json(json: &str) -> Result<Self> {
        check_format(json, "state", FORMAT)?;
        let state: Self = serde_json::from_str(json)?;
        state.validate()?;
        Ok(state)
    }

    pub fn to_json(&self) -> Result<String> {
        Ok(serde_json::to_string_pretty(self)?)
    }

    /// Writes the state atomically (temp file + rename), so a crash never leaves a half-written state.
    pub fn save(&self, path: &Path) -> Result<()> {
        self.validate()?;
        let json = self.to_json()?;
        let mut tmp = path.as_os_str().to_owned();
        tmp.push(".tmp");
        let tmp = PathBuf::from(tmp);
        {
            let mut file = File::create(&tmp).ctx("create", &tmp)?;
            file.write_all(json.as_bytes()).ctx("write", &tmp)?;
            file.sync_all().ctx("flush", &tmp)?;
        }
        fs::rename(&tmp, path).ctx("replace", path)
    }

    pub fn validate(&self) -> Result<()> {
        if self.format != FORMAT {
            return Err(Error::State(format!("format {}", self.format)));
        }
        let mut keys = HashSet::new();
        for file in &self.files {
            if !keys.insert(file.path.key()) {
                return Err(Error::State(format!("{}: duplicate file", file.path)));
            }
        }
        Ok(())
    }

    pub fn push_history(&mut self, action: &str, detail: String) {
        self.history.push(HistoryEntry {
            action: action.to_owned(),
            time: time::utc_now(),
            detail,
        });
        let excess = self.history.len().saturating_sub(HISTORY_MAX);
        self.history.drain(..excess);
    }

    /// Replaces the files of `components` with `files`, keeps the others (e.g. FFMPEG on an HLAE upgrade).
    pub fn replace_files(&mut self, components: &[Component], files: Vec<FileRecord>) {
        self.files.retain(|f| !components.contains(&f.component));
        self.files.extend(files);
        self.files
            .sort_by(|a, b| a.path.as_str().cmp(b.path.as_str()));
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn state() -> State {
        let mut state = State::new(
            "test",
            Path::new("C:\\HLAE"),
            "2.192.8",
            Features { locales: true },
        );
        state.files.push(FileRecord {
            path: RelPath::new("HLAE.exe").unwrap(),
            sha256: "0".repeat(64),
            component: Component::Core,
        });
        state
    }

    #[test]
    fn save_load_roundtrip() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("state.json");
        assert_eq!(State::load(&path).unwrap(), None);
        let state = state();
        state.save(&path).unwrap();
        state.save(&path).unwrap(); // replacing an existing file works too
        assert_eq!(State::load(&path).unwrap(), Some(state));
        assert!(!dir.path().join("state.json.tmp").exists());
    }

    #[test]
    fn refuses_newer_format() {
        let json = state()
            .to_json()
            .unwrap()
            .replace("\"format\": 1", "\"format\": 2");
        assert!(matches!(
            State::from_json(&json),
            Err(Error::UnsupportedFormat { found: 2, .. })
        ));
    }

    #[test]
    fn history_is_capped() {
        let mut state = state();
        for i in 0..HISTORY_MAX + 5 {
            state.push_history("repair", i.to_string());
        }
        assert_eq!(state.history.len(), HISTORY_MAX);
        assert_eq!(state.history[0].detail, "5");
    }
}
