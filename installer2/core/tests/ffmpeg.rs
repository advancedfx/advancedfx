use std::fs::{self, File};
use std::io::Write;
use std::path::{Path, PathBuf};

use hlae_installer_build_support::pack::{ZSTD_LEVEL, pack_dir};
use hlae_installer_core::Error;
use hlae_installer_core::ffmpeg::{self, FfmpegData, Variant};
use hlae_installer_core::ffmpeg_install::{
    FfmpegPlan, FfmpegSelection, INI_PATH, apply, default_selection, ini_content, warnings,
};
use hlae_installer_core::install::{InstallRequest, install_hlae, uninstall};
use hlae_installer_core::state::{Features, FfmpegChoice, State};
use tempfile::TempDir;

fn write_zip(path: &Path, prefix: &str, files: &[(&str, &str)]) {
    let mut zip = zip::ZipWriter::new(File::create(path).unwrap());
    let options = zip::write::SimpleFileOptions::default();
    zip.add_directory(prefix, options).unwrap();
    for (name, content) in files {
        zip.start_file(format!("{prefix}{name}"), options).unwrap();
        zip.write_all(content.as_bytes()).unwrap();
    }
    zip.finish().unwrap();
}

fn variant(dir: &Path, id: &str, files: &[(&str, &str)]) -> (Variant, PathBuf) {
    let prefix = format!("ffmpeg-{id}/");
    let archive = dir.join(format!("{id}.zip"));
    write_zip(&archive, &prefix, files);
    let (archive_sha256, archive_size) = hlae_installer_core::hash::sha256_file(&archive).unwrap();
    let files = ffmpeg::inspect_archive(&archive, &prefix).unwrap();
    let variant = Variant {
        id: id.into(),
        linking: "static".into(),
        license: "gpl".into(),
        url: format!("https://example.invalid/{id}.zip"),
        archive_size,
        archive_sha256,
        strip_prefix: prefix,
        files,
    };
    (variant, archive)
}

struct Setup {
    _tmp: TempDir,
    install_dir: PathBuf,
    download_dir: PathBuf,
    state_file: PathBuf,
    data: FfmpegData,
    gpl: PathBuf,
    shared: PathBuf,
    state: State,
}

/// HLAE installed + an FFMPEG data set with two variants.
fn setup() -> Setup {
    let tmp = TempDir::new().unwrap();
    let src = tmp.path().join("src");
    fs::create_dir_all(&src).unwrap();
    fs::write(src.join("HLAE.exe"), "hlae").unwrap();
    let mut payload = Vec::new();
    let manifest = pack_dir(&src, "2.192.8", ZSTD_LEVEL, &mut payload).unwrap();
    let install_dir = tmp.path().join("HLAE");
    let outcome = install_hlae(
        &InstallRequest {
            install_dir: &install_dir,
            manifest: &manifest,
            payload: &payload,
            features: Features { locales: true },
            written_by: "test",
            state_file: tmp.path().join("state").join("state.json"),
        },
        None,
    )
    .unwrap();
    outcome.tx.commit().unwrap();

    let (gpl, gpl_archive) = variant(
        tmp.path(),
        "win64-gpl",
        &[("bin/ffmpeg.exe", "static exe"), ("LICENSE.txt", "gpl")],
    );
    let (shared, shared_archive) = variant(
        tmp.path(),
        "win64-gpl-shared",
        &[
            ("bin/ffmpeg.exe", "shared exe"),
            ("bin/avcodec.dll", "dll"),
            ("LICENSE.txt", "gpl"),
        ],
    );
    let data = FfmpegData {
        format: ffmpeg::FORMAT,
        version: "n9 (tag)".into(),
        default_variant: "win64-gpl".into(),
        variants: vec![gpl, shared],
    };
    data.validate().unwrap();
    Setup {
        download_dir: tmp.path().join("admin-only-download"),
        state_file: tmp.path().join("state").join("state.json"),
        install_dir,
        data,
        gpl: gpl_archive,
        shared: shared_archive,
        state: outcome.state,
        _tmp: tmp,
    }
}

fn run(s: &Setup, state: &State, plan: FfmpegPlan) -> State {
    let outcome = apply(state, &s.data, &plan, &s.download_dir, &s.state_file)
        .unwrap()
        .expect("something to do");
    outcome.tx.commit().unwrap();
    outcome.state
}

fn read(dir: &Path, rel: &str) -> Option<String> {
    fs::read_to_string(dir.join(rel)).ok()
}

fn download(variant: &str, archive: &Path) -> FfmpegPlan {
    FfmpegPlan::Download {
        variant: variant.into(),
        archive: archive.into(),
    }
}

#[test]
fn default_rules() {
    let s = setup();
    let data = &s.data;
    let variant = |id: &str| FfmpegSelection::Variant(id.into());
    assert_eq!(
        default_selection(data, None),
        variant("win64-gpl"),
        "fresh install"
    );

    let mut state = s.state.clone();
    assert_eq!(
        default_selection(data, Some(&state)),
        FfmpegSelection::Keep,
        "deliberately none → keep it that way"
    );

    state.ffmpeg.choice = FfmpegChoice::Custom;
    assert_eq!(
        default_selection(data, Some(&state)),
        FfmpegSelection::Keep,
        "custom"
    );

    state.ffmpeg.choice = FfmpegChoice::Download;
    state.ffmpeg.variant = Some("win64-gpl-shared".into());
    state.ffmpeg.version = Some("n9 (tag)".into());
    assert_eq!(
        default_selection(data, Some(&state)),
        FfmpegSelection::Keep,
        "same version"
    );

    state.ffmpeg.version = Some("n8 (old)".into());
    assert_eq!(
        default_selection(data, Some(&state)),
        variant("win64-gpl-shared"),
        "upgrade, same variant"
    );

    state.ffmpeg.variant = Some("gone-variant".into());
    assert_eq!(
        default_selection(data, Some(&state)),
        variant("win64-gpl"),
        "unknown variant → default"
    );
}

#[test]
fn install_switch_custom_back_and_remove() {
    let s = setup();
    let dir = &s.install_dir;

    let state = run(&s, &s.state, download("win64-gpl", &s.gpl));
    assert_eq!(
        read(dir, "ffmpeg/bin/ffmpeg.exe").as_deref(),
        Some("static exe")
    );
    assert_eq!(
        read(dir, "HLAE.exe").as_deref(),
        Some("hlae"),
        "HLAE untouched"
    );
    assert_eq!(state.ffmpeg.choice, FfmpegChoice::Download);
    assert_eq!(state.ffmpeg.version.as_deref(), Some("n9 (tag)"));
    assert!(
        !s.download_dir.join("ffmpeg.zip").exists(),
        "verified copy removed again"
    );

    // Switch variant: the obsolete files go, the new ones come.
    let state = run(&s, &state, download("win64-gpl-shared", &s.shared));
    assert_eq!(
        read(dir, "ffmpeg/bin/ffmpeg.exe").as_deref(),
        Some("shared exe")
    );
    assert!(dir.join("ffmpeg/bin/avcodec.dll").exists());

    // Custom: downloaded files removed, ini written.
    let exe = s.install_dir.parent().unwrap().join("my-ffmpeg.exe");
    fs::write(&exe, "mine").unwrap();
    let state = run(&s, &state, FfmpegPlan::Custom { exe: exe.clone() });
    assert!(!dir.join("ffmpeg/bin").exists());
    assert_eq!(fs::read(dir.join(INI_PATH)).unwrap(), ini_content(&exe));
    assert_eq!(state.ffmpeg.choice, FfmpegChoice::Custom);
    assert_eq!(state.ffmpeg.custom_path, Some(exe.display().to_string()));

    // Back to download: our ini must go, else it would override the downloaded FFMPEG.
    let state = run(&s, &state, download("win64-gpl", &s.gpl));
    assert!(!dir.join(INI_PATH).exists());
    assert!(warnings(&state).is_empty());

    // None: everything removed incl. the ffmpeg dir, and remembered.
    let state = run(&s, &state, FfmpegPlan::None);
    assert!(!dir.join("ffmpeg").exists());
    assert_eq!(state.ffmpeg.choice, FfmpegChoice::None);
    assert!(state.files.iter().all(|f| !f.path.is_below("ffmpeg")));

    // Uninstall removes FFMPEG with HLAE.
    let state = run(&s, &state, download("win64-gpl", &s.gpl));
    let report = uninstall(&state);
    assert!(report.failed.is_empty() && report.kept.is_empty());
    assert!(!dir.exists());
}

#[test]
fn keep_and_none_without_ffmpeg_do_nothing() {
    let s = setup();
    for plan in [FfmpegPlan::Keep, FfmpegPlan::None] {
        assert!(
            apply(&s.state, &s.data, &plan, &s.download_dir, &s.state_file)
                .unwrap()
                .is_none(),
            "{plan:?}"
        );
    }
}

#[test]
fn wrong_or_tampered_archive_is_rejected() {
    let s = setup();
    let state = run(&s, &s.state, download("win64-gpl", &s.gpl));

    // The shared archive offered as the gpl variant.
    let result = apply(
        &state,
        &s.data,
        &download("win64-gpl", &s.shared),
        &s.download_dir,
        &s.state_file,
    );
    assert!(
        matches!(result, Err(Error::Verify { .. })),
        "{:?}",
        result.err()
    );

    let mut data = s.data.clone();
    data.variants[0].files[0].sha256 = "0".repeat(64);
    let result = apply(
        &state,
        &data,
        &download("win64-gpl", &s.gpl),
        &s.download_dir,
        &s.state_file,
    );
    assert!(
        matches!(result, Err(Error::Verify { .. })),
        "{:?}",
        result.err()
    );

    assert_eq!(
        read(&s.install_dir, "ffmpeg/bin/ffmpeg.exe").as_deref(),
        Some("static exe"),
        "unchanged"
    );
    assert!(!s.download_dir.join("ffmpeg.zip").exists());

    let unknown = apply(
        &state,
        &s.data,
        &download("win32-nope", &s.gpl),
        &s.download_dir,
        &s.state_file,
    );
    assert!(unknown.is_err());
    let missing = apply(
        &state,
        &s.data,
        &FfmpegPlan::Custom {
            exe: "C:\\does\\not\\exist\\ffmpeg.exe".into(),
        },
        &s.download_dir,
        &s.state_file,
    );
    assert!(matches!(missing, Err(Error::InvalidCustomFfmpeg { .. })));
}

#[test]
fn warns_about_foreign_ini_and_never_deletes_it() {
    let s = setup();
    let state = run(&s, &s.state, download("win64-gpl", &s.gpl));
    fs::write(
        s.install_dir.join(INI_PATH),
        "[Ffmpeg]\nPath=C:\\elsewhere\\ffmpeg.exe\n",
    )
    .unwrap();
    assert_eq!(warnings(&state).len(), 1);

    let state = run(&s, &state, FfmpegPlan::None);
    assert!(s.install_dir.join(INI_PATH).exists(), "untracked file kept");
    assert_eq!(state.ffmpeg.choice, FfmpegChoice::None);
}
