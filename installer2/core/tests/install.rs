use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

use hlae_installer_build_support::pack::{ZSTD_LEVEL, pack_dir};
use hlae_installer_core::Error;
use hlae_installer_core::install::{InstallRequest, install_hlae, uninstall};
use hlae_installer_core::manifest::{Component, Manifest};
use hlae_installer_core::relpath::RelPath;
use hlae_installer_core::state::{Features, State};
use hlae_installer_core::transaction::{WORK_DIR_NAME, journal_path, recover};
use tempfile::TempDir;

const ALL: Features = Features { locales: true };
const NO_LOCALES: Features = Features { locales: false };

fn write(root: &Path, rel: &str, content: &str) {
    let path = root.join(rel);
    fs::create_dir_all(path.parent().unwrap()).unwrap();
    fs::write(path, content).unwrap();
}

fn read(root: &Path, rel: &str) -> Option<String> {
    fs::read_to_string(root.join(rel)).ok()
}

/// A HLAE "build" packed into a payload.
struct Build {
    manifest: Manifest,
    payload: Vec<u8>,
}

fn build(version: &str, files: &[(&str, &str)]) -> Build {
    let src = TempDir::new().unwrap();
    for (rel, content) in files {
        write(src.path(), rel, content);
    }
    let mut payload = Vec::new();
    let manifest = pack_dir(src.path(), version, ZSTD_LEVEL, &mut payload).unwrap();
    Build { manifest, payload }
}

/// Next to the install dir (the journal goes next to it).
fn state_file(dir: &Path) -> PathBuf {
    dir.parent().unwrap().join("state").join("state.json")
}

fn request<'a>(dir: &'a Path, build: &'a Build, features: Features) -> InstallRequest<'a> {
    InstallRequest {
        install_dir: dir,
        manifest: &build.manifest,
        payload: &build.payload,
        features,
        written_by: "test",
        state_file: state_file(dir),
    }
}

/// Installs and commits, like the CLI will do (state save in between).
fn install(dir: &Path, build: &Build, features: Features, old: Option<&State>) -> State {
    let outcome = install_hlae(&request(dir, build, features), old).unwrap();
    outcome.tx.commit().unwrap();
    outcome.state
}

fn paths(state: &State) -> Vec<&str> {
    state.files.iter().map(|f| f.path.as_str()).collect()
}

fn v1() -> Build {
    build(
        "2.192.8",
        &[
            ("HLAE.exe", "hlae v1"),
            ("x64/AfxHookSource2.dll", "hook v1"),
            ("old.txt", "obsolete in v2"),
            ("oldmod.txt", "obsolete in v2, user modifies it"),
            ("locales/de/hlae/messages.mo", "de v1"),
            ("ffmpeg/readme.txt", "skipped by the packer"),
        ],
    )
}

fn v2() -> Build {
    build(
        "2.192.9",
        &[
            ("HLAE.exe", "hlae v2"),
            ("x64/AfxHookSource2.dll", "hook v1"),
            ("new/new.txt", "new in v2"),
            ("locales/de/hlae/messages.mo", "de v2"),
        ],
    )
}

fn temp_install_dir() -> (TempDir, PathBuf) {
    let tmp = TempDir::new().unwrap();
    let dir = tmp.path().join("HLAE");
    (tmp, dir)
}

#[test]
fn pack_assigns_features_and_skips_ffmpeg() {
    let build = v1();
    let features: Vec<_> = build
        .manifest
        .files
        .iter()
        .map(|f| (f.path.as_str(), f.feature))
        .collect();
    assert_eq!(
        features,
        [
            ("HLAE.exe", Component::Core),
            ("locales/de/hlae/messages.mo", Component::Locales),
            ("old.txt", Component::Core),
            ("oldmod.txt", Component::Core),
            ("x64/AfxHookSource2.dll", Component::Core),
        ]
    );
    // Deterministic output (reproducible builds).
    assert_eq!(v1().payload, build.payload);
}

#[test]
fn fresh_install_and_uninstall() {
    let (_tmp, dir) = temp_install_dir();
    let state = install(&dir, &v1(), ALL, None);

    assert_eq!(
        read(&dir, "x64/AfxHookSource2.dll").as_deref(),
        Some("hook v1")
    );
    assert_eq!(
        read(&dir, "locales/de/hlae/messages.mo").as_deref(),
        Some("de v1")
    );
    assert!(!dir.join("ffmpeg").exists());
    assert!(!dir.join(WORK_DIR_NAME).exists());
    assert_eq!(state.history.last().unwrap().action, "install");
    assert_eq!(state.files.len(), 5);

    let report = uninstall(&state);
    assert_eq!(report.removed.len(), 5);
    assert!(report.kept.is_empty() && report.failed.is_empty());
    assert!(!dir.exists(), "empty install dir is removed");
}

#[test]
fn uninstall_keeps_modified_and_untracked_files() {
    let (_tmp, dir) = temp_install_dir();
    let state = install(&dir, &v1(), ALL, None);
    write(&dir, "HLAE.exe", "modified by user");
    write(&dir, "x64/user.cfg", "untracked");

    let report = uninstall(&state);
    assert_eq!(report.kept, [RelPath::new("HLAE.exe").unwrap()]);
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("modified by user"));
    assert_eq!(read(&dir, "x64/user.cfg").as_deref(), Some("untracked"));
    assert!(!dir.join("x64/AfxHookSource2.dll").exists());
    assert!(!dir.join("locales").exists(), "empty dirs are removed");
}

#[test]
fn upgrade_overwrites_removes_obsolete_and_keeps_modified_obsolete() {
    let (_tmp, dir) = temp_install_dir();
    let old = install(&dir, &v1(), ALL, None);
    write(&dir, "HLAE.exe", "modified by user"); // tracked, still in v2 → overwritten
    write(&dir, "oldmod.txt", "modified by user"); // tracked, gone in v2 → kept

    let outcome = install_hlae(&request(&dir, &v2(), ALL), Some(&old)).unwrap();
    assert_eq!(outcome.action, "upgrade");
    assert_eq!(outcome.applied.removed, [RelPath::new("old.txt").unwrap()]);
    assert_eq!(outcome.applied.kept, [RelPath::new("oldmod.txt").unwrap()]);
    let state = outcome.state;
    outcome.tx.commit().unwrap();

    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("hlae v2"));
    assert_eq!(read(&dir, "new/new.txt").as_deref(), Some("new in v2"));
    assert_eq!(
        read(&dir, "locales/de/hlae/messages.mo").as_deref(),
        Some("de v2")
    );
    assert!(!dir.join("old.txt").exists());
    assert_eq!(
        read(&dir, "oldmod.txt").as_deref(),
        Some("modified by user")
    );
    assert_eq!(
        paths(&state),
        [
            "HLAE.exe",
            "locales/de/hlae/messages.mo",
            "new/new.txt",
            "x64/AfxHookSource2.dll"
        ]
    );
    assert_eq!(state.hlae_version, "2.192.9");
    assert_eq!(state.history.last().unwrap().detail, "2.192.8 -> 2.192.9");
}

#[test]
fn modify_removes_deselected_locales_and_repair_restores_files() {
    let (_tmp, dir) = temp_install_dir();
    let build = v1();
    let state = install(&dir, &build, ALL, None);

    let outcome = install_hlae(&request(&dir, &build, NO_LOCALES), Some(&state)).unwrap();
    assert_eq!(outcome.action, "modify");
    let state = outcome.state;
    outcome.tx.commit().unwrap();
    assert!(!dir.join("locales").exists());
    assert!(state.files.iter().all(|f| f.component == Component::Core));

    fs::remove_file(dir.join("HLAE.exe")).unwrap();
    write(&dir, "old.txt", "corrupted");
    let outcome = install_hlae(&request(&dir, &build, NO_LOCALES), Some(&state)).unwrap();
    assert_eq!(outcome.action, "repair");
    outcome.tx.commit().unwrap();
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("hlae v1"));
    assert_eq!(read(&dir, "old.txt").as_deref(), Some("obsolete in v2"));
}

#[test]
fn repair_only_unpacks_what_differs() {
    let (_tmp, dir) = temp_install_dir();
    let state = install(&dir, &v1(), ALL, None);
    // Same manifest, but a payload that can't be unpacked: fine as long as nothing differs,
    // because then the payload isn't read at all.
    let garbage = Build {
        manifest: v1().manifest,
        payload: b"not a payload".to_vec(),
    };
    let outcome = install_hlae(&request(&dir, &garbage, ALL), Some(&state)).unwrap();
    assert_eq!(outcome.action, "repair");
    assert_eq!(
        paths(&outcome.state),
        paths(&state),
        "unchanged files recorded"
    );
    let state = outcome.state;
    outcome.tx.commit().unwrap();

    write(&dir, "HLAE.exe", "corrupted");
    assert!(install_hlae(&request(&dir, &garbage, ALL), Some(&state)).is_err());
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("corrupted"));
    let state = install(&dir, &v1(), ALL, Some(&state));
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("hlae v1"));
    assert_eq!(state.files.len(), 5);
}

#[test]
fn hlae_upgrade_keeps_files_of_other_components() {
    let (_tmp, dir) = temp_install_dir();
    let mut state = install(&dir, &v1(), ALL, None);
    write(&dir, "ffmpeg/bin/ffmpeg.exe", "ffmpeg");
    state.files.push(hlae_installer_core::state::FileRecord {
        path: RelPath::new("ffmpeg/bin/ffmpeg.exe").unwrap(),
        sha256: hlae_installer_core::hash::sha256_file(&dir.join("ffmpeg/bin/ffmpeg.exe"))
            .unwrap()
            .0,
        component: Component::Ffmpeg,
    });

    let state = install(&dir, &v2(), ALL, Some(&state));
    assert_eq!(
        read(&dir, "ffmpeg/bin/ffmpeg.exe").as_deref(),
        Some("ffmpeg")
    );
    assert!(paths(&state).contains(&"ffmpeg/bin/ffmpeg.exe"));
}

#[test]
fn downgrade_and_moving_are_refused() {
    let (_tmp, dir) = temp_install_dir();
    let state = install(&dir, &v2(), ALL, None);
    assert!(matches!(
        install_hlae(&request(&dir, &v1(), ALL), Some(&state)),
        Err(Error::Downgrade { .. })
    ));
    let other = dir.with_file_name("Other");
    assert!(matches!(
        install_hlae(&request(&other, &v2(), ALL), Some(&state)),
        Err(Error::InstallDirMismatch { .. })
    ));
    // Same dir with different case / trailing separator is fine.
    let same = PathBuf::from(format!("{}\\", dir.display().to_string().to_uppercase()));
    assert!(install_hlae(&request(&same, &v2(), ALL), Some(&state)).is_ok());
}

#[test]
fn failed_apply_rolls_back() {
    let (_tmp, dir) = temp_install_dir();
    let old = install(&dir, &v1(), ALL, None);
    // "new/new.txt" can't be placed because a file is in the way of the "new" dir.
    write(&dir, "new", "blocker");

    let result = install_hlae(&request(&dir, &v2(), ALL), Some(&old));
    assert!(result.is_err(), "{:?}", result.err());
    assert_eq!(
        read(&dir, "HLAE.exe").as_deref(),
        Some("hlae v1"),
        "already replaced file restored"
    );
    assert_eq!(
        read(&dir, "locales/de/hlae/messages.mo").as_deref(),
        Some("de v1")
    );
    assert_eq!(read(&dir, "old.txt").as_deref(), Some("obsolete in v2"));
    assert!(!dir.join(WORK_DIR_NAME).exists());
}

#[test]
fn dropping_without_commit_rolls_back() {
    let (_tmp, dir) = temp_install_dir();
    let old = install(&dir, &v1(), ALL, None);
    // E.g. saving the state failed → caller drops the outcome.
    drop(install_hlae(&request(&dir, &v2(), ALL), Some(&old)).unwrap());
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("hlae v1"));
    assert_eq!(read(&dir, "old.txt").as_deref(), Some("obsolete in v2"));
    assert!(!dir.join("new").exists(), "created dirs removed again");

    let (_tmp, fresh) = temp_install_dir();
    drop(install_hlae(&request(&fresh, &v1(), ALL), None).unwrap());
    assert!(!fresh.exists(), "fresh install fully undone");
}

fn assert_v1(dir: &Path) {
    assert_eq!(read(dir, "HLAE.exe").as_deref(), Some("hlae v1"));
    assert_eq!(read(dir, "old.txt").as_deref(), Some("obsolete in v2"));
    assert_eq!(
        read(dir, "locales/de/hlae/messages.mo").as_deref(),
        Some("de v1")
    );
    assert!(!dir.join("new").exists());
}

fn assert_cleaned_up(dir: &Path) {
    assert!(!dir.join(WORK_DIR_NAME).exists());
    assert!(!journal_path(&state_file(dir)).exists());
}

#[test]
fn crash_before_saving_the_state_rolls_back() {
    let (_tmp, dir) = temp_install_dir();
    let old = install(&dir, &v1(), ALL, None);
    let outcome = install_hlae(&request(&dir, &v2(), ALL), Some(&old)).unwrap();
    outcome.tx.simulate_crash();
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("hlae v2"));

    recover(&state_file(&dir)).unwrap();
    assert_v1(&dir);
    assert_cleaned_up(&dir);
    recover(&state_file(&dir)).unwrap(); // nothing left to do

    // Same for a fresh install: everything goes, including the created install dir.
    let (_tmp, fresh) = temp_install_dir();
    install_hlae(&request(&fresh, &v1(), ALL), None)
        .unwrap()
        .tx
        .simulate_crash();
    recover(&state_file(&fresh)).unwrap();
    assert!(!fresh.exists());
}

#[test]
fn crash_after_saving_the_state_keeps_the_new_files() {
    let (_tmp, dir) = temp_install_dir();
    let old = install(&dir, &v1(), ALL, None);
    let outcome = install_hlae(&request(&dir, &v2(), ALL), Some(&old)).unwrap();
    let state_file = state_file(&dir);
    fs::create_dir_all(state_file.parent().unwrap()).unwrap();
    outcome.state.save(&state_file).unwrap();
    outcome.tx.simulate_crash();

    // Also via the next transaction's begin (here: a repair of v2).
    let state = State::load(&state_file).unwrap().unwrap();
    install_hlae(&request(&dir, &v2(), ALL), Some(&state))
        .unwrap()
        .tx
        .commit()
        .unwrap();
    assert_eq!(read(&dir, "HLAE.exe").as_deref(), Some("hlae v2"));
    assert_eq!(read(&dir, "new/new.txt").as_deref(), Some("new in v2"));
    assert!(!dir.join("old.txt").exists());
    assert_cleaned_up(&dir);
}

#[test]
fn recovery_skips_steps_that_never_happened() {
    let (_tmp, dir) = temp_install_dir();
    let old = install(&dir, &v1(), ALL, None);
    let outcome = install_hlae(&request(&dir, &v2(), ALL), Some(&old)).unwrap();
    outcome.tx.simulate_crash();
    // Journaled, but the setup "crashed" before doing them; then a line cut off by the crash.
    let journal = journal_path(&state_file(&dir));
    let mut text = fs::read_to_string(&journal).unwrap();
    text.push_str(r#"{"op":"backup","path":"x64/AfxHookSource2.dll","backup":"000099"}"#);
    text.push('\n');
    text.push_str(r#"{"op":"placed","path":"never.txt","sha256":""#);
    text.push_str(&"0".repeat(64));
    text.push_str("\"}\n");
    text.push_str(r#"{"op":"placed","path":"cut"#);
    fs::write(&journal, text).unwrap();

    recover(&state_file(&dir)).unwrap();
    assert_v1(&dir);
    assert_eq!(
        read(&dir, "x64/AfxHookSource2.dll").as_deref(),
        Some("hook v1")
    );
    assert_cleaned_up(&dir);
}

#[test]
fn tampered_payload_is_rejected() {
    let (_tmp, dir) = temp_install_dir();
    let mut build = v1();
    build.manifest.files[0].sha256 = "0".repeat(64);
    assert!(matches!(
        install_hlae(&request(&dir, &build, ALL), None),
        Err(Error::Verify { .. })
    ));
    assert!(!dir.exists(), "nothing left behind");

    let mut build = v1();
    build.manifest.files.remove(0);
    assert!(matches!(
        install_hlae(&request(&dir, &build, ALL), None),
        Err(Error::Payload(_))
    ));
}

fn junction(link: &Path, target: &Path) {
    let status = Command::new("cmd")
        .args(["/C", "mklink", "/J"])
        .arg(link)
        .arg(target)
        .output()
        .unwrap()
        .status;
    assert!(status.success(), "mklink /J failed");
}

#[test]
fn junctions_inside_install_dir_are_not_followed() {
    let (tmp, dir) = temp_install_dir();
    let state = install(&dir, &v1(), ALL, None);
    // Replace x64 with a junction to an outside dir holding an identical file.
    let outside = tmp.path().join("outside");
    write(&outside, "AfxHookSource2.dll", "hook v1");
    fs::remove_dir_all(dir.join("x64")).unwrap();
    junction(&dir.join("x64"), &outside);

    assert!(matches!(
        install_hlae(&request(&dir, &v1(), ALL), Some(&state)),
        Err(Error::ReparsePoint { .. })
    ));
    let report = uninstall(&state);
    assert_eq!(report.failed.len(), 1);
    assert_eq!(
        read(&outside, "AfxHookSource2.dll").as_deref(),
        Some("hook v1"),
        "outside file untouched"
    );
}

/// Packs and installs a real HLAE build:
/// `set HLAE_TEST_PAYLOAD_DIR=C:\source\advancedfx\build\package-release\bin` then
/// `cargo test -p hlae-installer-core --release -- --ignored --nocapture`
#[test]
#[ignore]
fn real_payload() {
    let Some(src) = std::env::var_os("HLAE_TEST_PAYLOAD_DIR") else {
        panic!("HLAE_TEST_PAYLOAD_DIR not set");
    };
    let started = std::time::Instant::now();
    let mut payload = Vec::new();
    let manifest = pack_dir(Path::new(&src), "2.192.8", ZSTD_LEVEL, &mut payload).unwrap();
    println!(
        "packed {} files into {} bytes in {:?}",
        manifest.files.len(),
        payload.len(),
        started.elapsed()
    );

    let (_tmp, dir) = temp_install_dir();
    let build = Build { manifest, payload };
    let started = std::time::Instant::now();
    let state = install(&dir, &build, ALL, None);
    println!("installed in {:?}", started.elapsed());
    for file in &build.manifest.files {
        let (sha256, _) = hlae_installer_core::hash::sha256_file(&file.path.to_path(&dir)).unwrap();
        assert_eq!(sha256, file.sha256, "{}", file.path);
    }
    let report = uninstall(&state);
    assert_eq!(report.removed.len(), build.manifest.files.len());
    assert!(!dir.exists());
}
