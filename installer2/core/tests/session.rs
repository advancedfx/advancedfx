//! `Session` (test mode: own state file, no Windows integration) with files in use.

use std::fs;
use std::path::{Path, PathBuf};
use std::process::{Child, Command};
use std::time::Duration;

use hlae_installer_build_support::pack::{ZSTD_LEVEL, pack_dir};
use hlae_installer_core::Error;
use hlae_installer_core::ffmpeg::{self, FfmpegData};
use hlae_installer_core::ffmpeg_install::FfmpegPlan;
use hlae_installer_core::in_use;
use hlae_installer_core::session::{InstallOptions, Session};
use hlae_installer_core::state::State;
use tempfile::TempDir;

/// A PowerShell that keeps `file` open until it's killed.
struct Holder(Child);

impl Holder {
    fn open(file: &Path) -> Self {
        // Wrapped at once, so the child is killed also if we panic below.
        let holder = Self(
            Command::new("powershell.exe")
            .args([
                "-NoProfile",
                "-Command",
                &format!(
                    "$f = [IO.File]::Open('{}', 'Open', 'Read', 'ReadWrite, Delete'); Start-Sleep 60",
                    file.display()
                ),
            ])
            .spawn()
            .unwrap(),
        );
        let files = [file.to_path_buf()];
        for _ in 0..100 {
            if !in_use::programs_using(&files).is_empty() {
                return holder;
            }
            std::thread::sleep(Duration::from_millis(100));
        }
        panic!("the child never opened {}", file.display());
    }
}

impl Drop for Holder {
    fn drop(&mut self) {
        let _ = self.0.kill();
        let _ = self.0.wait();
    }
}

#[test]
fn install_and_uninstall_refuse_while_files_are_in_use() {
    let tmp = TempDir::new().unwrap();
    let src = tmp.path().join("src");
    fs::create_dir_all(src.join("x64")).unwrap();
    fs::write(src.join("HLAE.exe"), "hlae").unwrap();
    fs::write(src.join("x64").join("AfxHookSource2.dll"), "hook").unwrap();
    let mut payload = Vec::new();
    let manifest = pack_dir(&src, "2.192.8", ZSTD_LEVEL, &mut payload).unwrap();
    let data = FfmpegData {
        format: ffmpeg::FORMAT,
        version: "n9".into(),
        default_variant: "none".into(),
        variants: Vec::new(),
    };
    let session = Session {
        state_file: tmp.path().join("state").join("state.json"),
        integrate: false,
        protect: false,
    };
    let install_dir = tmp.path().join("HLAE");
    let options = InstallOptions {
        install_dir: Some(install_dir.clone()),
        locales: None,
        ffmpeg: FfmpegPlan::Keep,
    };
    session
        .install(&manifest, &payload, &data, &options, "test")
        .unwrap();
    let state_before = fs::read_to_string(&session.state_file).unwrap();

    let hook: PathBuf = install_dir.join("x64").join("AfxHookSource2.dll");
    let holder = Holder::open(&hook);
    let pid = holder.0.id();
    let in_use_by_holder = |e: Error| match e {
        Error::FilesInUse(holders) => assert!(holders.iter().any(|h| h.pid == pid), "{holders:?}"),
        e => panic!("expected FilesInUse, got {e}"),
    };

    // Repair and uninstall are refused before anything is touched.
    in_use_by_holder(
        session
            .install(&manifest, &payload, &data, &options, "test")
            .unwrap_err(),
    );
    in_use_by_holder(session.uninstall().unwrap_err());
    assert_eq!(
        fs::read_to_string(&session.state_file).unwrap(),
        state_before
    );
    assert!(hook.exists() && install_dir.join("HLAE.exe").exists());
    assert!(!install_dir.join(".hlae-setup-tmp").exists());

    drop(holder);
    let summary = session.uninstall().unwrap().expect("installed");
    assert!(summary.files.failed.is_empty(), "{:?}", summary.files);
    assert!(!install_dir.exists());
    assert!(State::load(&session.state_file).unwrap().is_none());
}
