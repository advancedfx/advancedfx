//! The setup wizard (NOTES.md M6): Tauri commands for `ui/`; install / uninstall run on a background
//! thread (same flow as the command line, `actions`) and report through events.

use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, Ordering};

use hlae_installer_core::ffmpeg_install::{self, FfmpegSelection, MAX_CUSTOM_PATH};
use hlae_installer_core::state::{FfmpegChoice, State as InstallState};
use hlae_installer_core::{dotnet, session, version};
use serde::{Deserialize, Serialize};
use tauri::{AppHandle, Emitter, Manager, State, WebviewUrl, WebviewWindowBuilder, WindowEvent};

use crate::actions::{self, InstallChoice};
use crate::cli::{GuiMode, exit_code};
use crate::ffmpeg_client::FfmpegArgs;
use crate::{dialog, events, l10n, logging, ops, payload};

struct Gui {
    mode: GuiMode,
    log_file: PathBuf,
    cancel: AtomicBool,
    busy: AtomicBool,
}

/// FFMPEG choice as the page sends / shows it.
#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(tag = "kind", rename_all = "kebab-case")]
enum Selection {
    Keep,
    None,
    Download { variant: String },
    Custom { exe: String },
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Installed {
    version: String,
    install_dir: String,
    locales: bool,
    /// `none`, `download`, `custom`
    ffmpeg_choice: &'static str,
    ffmpeg_variant: Option<String>,
    ffmpeg_custom: Option<String>,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Variant {
    id: String,
    linking: String,
    license: String,
    size_mb: u64,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Info {
    mode: GuiMode,
    language: &'static str,
    version: String,
    has_payload: bool,
    installed: Option<Installed>,
    newer_installed: bool,
    default_install_dir: String,
    variants: Vec<Variant>,
    default_ffmpeg: Selection,
    dotnet_ok: bool,
    dotnet_version: &'static str,
}

#[tauri::command]
fn info(gui: State<'_, Gui>) -> Result<Info, String> {
    info_inner(&gui).map_err(|e| e.to_string())
}

fn info_inner(gui: &Gui) -> hlae_installer_core::Result<Info> {
    let manifest = payload::manifest()?;
    let data = payload::ffmpeg_data()?;
    let state = load_state();
    let newer_installed = state.as_ref().is_some_and(|s| {
        version::compare(&s.hlae_version, &manifest.hlae_version).is_ok_and(|o| o.is_gt())
    });
    let default_ffmpeg = match ffmpeg_install::default_selection(&data, state.as_ref()) {
        FfmpegSelection::Keep => Selection::Keep,
        FfmpegSelection::None => Selection::None,
        FfmpegSelection::Variant(variant) => Selection::Download { variant },
        FfmpegSelection::Custom(exe) => Selection::Custom {
            exe: exe.display().to_string(),
        },
    };
    Ok(Info {
        mode: gui.mode,
        language: l10n::language(),
        version: manifest.hlae_version.clone(),
        has_payload: !manifest.files.is_empty(),
        installed: state.map(|s| Installed {
            version: s.hlae_version,
            install_dir: s.install_dir.display().to_string(),
            locales: s.features.locales,
            ffmpeg_choice: match s.ffmpeg.choice {
                FfmpegChoice::None => "none",
                FfmpegChoice::Download => "download",
                FfmpegChoice::Custom => "custom",
            },
            ffmpeg_variant: s.ffmpeg.variant,
            ffmpeg_custom: s.ffmpeg.custom_path,
        }),
        newer_installed,
        default_install_dir: session::default_install_dir()?.display().to_string(),
        variants: data
            .variants
            .iter()
            .map(|v| Variant {
                id: v.id.clone(),
                linking: v.linking.clone(),
                license: v.license.clone(),
                size_mb: (v.archive_size + 500_000) / 1_000_000,
            })
            .collect(),
        default_ffmpeg,
        dotnet_ok: dotnet::framework_ok(),
        dotnet_version: dotnet::MIN_VERSION,
    })
}

/// The installed state (readable for users), `None` if not installed or unreadable.
fn load_state() -> Option<InstallState> {
    let session = ops::session().ok()?;
    InstallState::load(&session.state_file).unwrap_or_else(|e| {
        log::warn!("could not read the install state: {e}");
        None
    })
}

#[tauri::command]
fn l10n_catalog() -> &'static std::collections::HashMap<String, String> {
    l10n::catalog()
}

/// `None` if fine, else the message key.
#[tauri::command]
fn check_install_dir(path: String) -> Option<&'static str> {
    let dir = Path::new(&path);
    if !dir.is_absolute() {
        Some("InstallDirNotAbsolute")
    } else if !path.is_ascii() {
        Some("InstallDirNotAscii")
    } else if session::validate_install_dir(dir).is_err() {
        Some("InstallDirInvalid")
    } else {
        None
    }
}

/// `None` if fine, else the message key.
#[tauri::command]
fn check_ffmpeg_exe(path: String) -> Option<&'static str> {
    if path.encode_utf16().count() > MAX_CUSTOM_PATH {
        Some("FfmpegCustomTooLong")
    } else if ffmpeg_install::validate_custom_exe(Path::new(&path)).is_err() {
        Some("FfmpegCustomMissing")
    } else {
        None
    }
}

fn owner(window: &tauri::WebviewWindow) -> isize {
    window.hwnd().map(|h| h.0 as isize).unwrap_or_default()
}

#[tauri::command]
async fn browse_folder(window: tauri::WebviewWindow, initial: String) -> Option<String> {
    let initial = PathBuf::from(initial);
    // Start in the deepest existing parent of what's entered.
    let start = initial
        .ancestors()
        .find(|p| p.is_dir())
        .map(Path::to_path_buf);
    dialog::pick(
        owner(&window),
        dialog::Kind::Folder,
        l10n::t("BrowseInstallDirTitle", &[]),
        start,
    )
    .map(|p| p.display().to_string())
}

#[tauri::command]
async fn browse_ffmpeg_exe(window: tauri::WebviewWindow, initial: String) -> Option<String> {
    let start = Path::new(&initial).parent().map(Path::to_path_buf);
    dialog::pick(
        owner(&window),
        dialog::Kind::File("ffmpeg.exe"),
        l10n::t("BrowseFfmpegTitle", &[]),
        start,
    )
    .map(|p| p.display().to_string())
}

#[tauri::command]
fn start_install(
    app: AppHandle,
    gui: State<'_, Gui>,
    install_dir: String,
    locales: bool,
    ffmpeg: Selection,
) -> Result<(), String> {
    if gui.busy.swap(true, Ordering::SeqCst) {
        return Err("busy".into());
    }
    gui.cancel.store(false, Ordering::SeqCst);
    let selection = match ffmpeg {
        Selection::Keep => FfmpegSelection::Keep,
        Selection::None => FfmpegSelection::None,
        Selection::Download { variant } => FfmpegSelection::Variant(variant),
        Selection::Custom { exe } => FfmpegSelection::Custom(PathBuf::from(exe)),
    };
    let choice = InstallChoice {
        install_dir: Some(PathBuf::from(install_dir)),
        locales: Some(locales),
        ffmpeg: FfmpegArgs {
            selection: Some(selection),
            archive: None,
        },
    };
    std::thread::spawn(move || {
        let gui = app.state::<Gui>();
        let code = actions::install(&choice, &gui.cancel);
        finished(&app, &gui, code);
    });
    Ok(())
}

#[tauri::command]
fn start_uninstall(
    app: AppHandle,
    gui: State<'_, Gui>,
    remove_app_data: bool,
) -> Result<(), String> {
    if gui.busy.swap(true, Ordering::SeqCst) {
        return Err("busy".into());
    }
    std::thread::spawn(move || {
        let gui = app.state::<Gui>();
        let code = actions::uninstall(remove_app_data);
        finished(&app, &gui, code);
    });
    Ok(())
}

fn finished(app: &AppHandle, gui: &Gui, code: i32) {
    log::info!("exit code {code}");
    log::logger().flush();
    gui.busy.store(false, Ordering::SeqCst);
    let _ = app.emit("setup-finished", code);
}

/// Cancels the FFMPEG download (the elevated part can't be cancelled).
#[tauri::command]
fn cancel(gui: State<'_, Gui>) {
    gui.cancel.store(true, Ordering::SeqCst);
}

/// Starts the installed HLAE (unelevated, like the wizard itself).
#[tauri::command]
fn launch_hlae() -> Result<(), String> {
    let state = load_state().ok_or("HLAE is not installed")?;
    std::process::Command::new(state.install_dir.join("HLAE.exe"))
        .current_dir(&state.install_dir)
        .spawn()
        .map(|_| ())
        .map_err(|e| e.to_string())
}

#[tauri::command]
fn open_log(gui: State<'_, Gui>) -> Result<(), String> {
    std::process::Command::new("notepad.exe")
        .arg(&gui.log_file)
        .spawn()
        .map(|_| ())
        .map_err(|e| e.to_string())
}

#[tauri::command]
fn quit(app: AppHandle, gui: State<'_, Gui>) {
    if !gui.busy.load(Ordering::SeqCst) {
        app.exit(0);
    }
}

/// How long to keep trying to remove the WebView2 data dir after the window closed.
const WEBVIEW_CLEANUP_TIMEOUT: std::time::Duration = std::time::Duration::from_secs(90);

/// Runs the wizard, returns the exit code.
pub fn run(mode: GuiMode) -> i32 {
    let log_file = logging::default_log_file();
    let _ = logging::init(&log_file);
    log::info!("{} started (wizard, {mode:?})", crate::cli::written_by());

    if !webview2::ensure_installed() {
        return exit_code::USER_CANCEL;
    }

    // Keep WebView2 user data out of %LOCALAPPDATA% (see NOTES.md 6.2), removed again on exit.
    let webview_data_dir =
        std::env::temp_dir().join(format!("{WEBVIEW_DIR_PREFIX}{}", std::process::id()));
    let title = l10n::t("WindowTitle", &[]);
    let data_dir = webview_data_dir.clone();

    let app = tauri::Builder::default()
        .manage(Gui {
            mode,
            log_file,
            cancel: AtomicBool::new(false),
            busy: AtomicBool::new(false),
        })
        .invoke_handler(tauri::generate_handler![
            info,
            l10n_catalog,
            check_install_dir,
            check_ffmpeg_exe,
            browse_folder,
            browse_ffmpeg_exe,
            start_install,
            start_uninstall,
            cancel,
            launch_hlae,
            open_log,
            quit,
        ])
        .setup(move |app| {
            let handle = app.handle().clone();
            events::set_sink(move |event| {
                let _ = handle.emit("setup-event", event);
            });
            WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()))
                .title(&title)
                .inner_size(640.0, 580.0)
                .resizable(false)
                .maximizable(false)
                .theme(debug_theme())
                .data_directory(data_dir.clone())
                .build()?;
            Ok(())
        })
        .on_window_event(|window, event| {
            // Don't close in the middle of an install / uninstall.
            if let WindowEvent::CloseRequested { api, .. } = event
                && window.state::<Gui>().busy.load(Ordering::SeqCst)
            {
                api.prevent_close();
            }
        })
        .build(tauri::generate_context!())
        .expect("error while building HLAE Setup");
    let code = app.run_return(|_, _| {});

    // The WebView2 processes only end when this process ends, so the data dir can't be removed
    // from here: leave it to a hidden helper (same exe) that retries after we're gone.
    log::info!(
        "wizard closed, cleaning up {} after exit",
        webview_data_dir.display()
    );
    log::logger().flush();
    spawn_cleanup(&webview_data_dir);
    code
}

pub const CLEANUP_ARG: &str = "--cleanup-webview-data";
const WEBVIEW_DIR_PREFIX: &str = "hlae-setup-webview-";

fn spawn_cleanup(dir: &Path) {
    use std::os::windows::process::CommandExt;
    const CREATE_NO_WINDOW: u32 = 0x0800_0000;
    let result = std::env::current_exe().and_then(|exe| {
        std::process::Command::new(exe)
            .arg(CLEANUP_ARG)
            .arg(dir)
            .stdin(std::process::Stdio::null())
            .stdout(std::process::Stdio::null())
            .stderr(std::process::Stdio::null())
            .creation_flags(CREATE_NO_WINDOW)
            .spawn()
    });
    if let Err(e) = result {
        log::warn!("could not start the cleanup of {}: {e}", dir.display());
    }
}

/// `--cleanup-webview-data <dir>` (internal): removes the wizard's WebView2 data dir once the
/// wizard's WebView2 processes are gone, retrying for up to [`WEBVIEW_CLEANUP_TIMEOUT`] (slow PCs).
/// Only accepts `%TEMP%\hlae-setup-webview-*`, so it can't be used to delete anything else.
pub fn cleanup_main(args: &[std::ffi::OsString]) -> i32 {
    let [dir] = args else {
        return exit_code::INVALID_COMMAND_LINE;
    };
    let dir = Path::new(dir);
    let ours = dir.parent() == Some(std::env::temp_dir().as_path())
        && dir
            .file_name()
            .and_then(|n| n.to_str())
            .is_some_and(|n| n.starts_with(WEBVIEW_DIR_PREFIX));
    if !ours {
        return exit_code::INVALID_COMMAND_LINE;
    }
    let deadline = std::time::Instant::now() + WEBVIEW_CLEANUP_TIMEOUT;
    while std::time::Instant::now() < deadline {
        if std::fs::remove_dir_all(dir).is_ok() || !dir.exists() {
            return exit_code::SUCCESS;
        }
        std::thread::sleep(std::time::Duration::from_millis(250));
    }
    exit_code::FATAL
}

/// Debug builds only: `HLAE_SETUP_THEME=light|dark` overrides the Windows theme (to check both
/// color schemes); otherwise follow Windows.
fn debug_theme() -> Option<tauri::Theme> {
    if !cfg!(debug_assertions) {
        return None;
    }
    match std::env::var("HLAE_SETUP_THEME").as_deref() {
        Ok("light") => Some(tauri::Theme::Light),
        Ok("dark") => Some(tauri::Theme::Dark),
        _ => None,
    }
}

/// WebView2 Evergreen runtime check before any window is created (NOTES.md 6.2).
mod webview2 {
    use std::sync::atomic::AtomicBool;

    use windows_sys::Win32::UI::WindowsAndMessaging::{
        IDYES, MB_ICONERROR, MB_ICONQUESTION, MB_OK, MB_YESNO, MessageBoxW,
    };
    use winreg::RegKey;
    use winreg::enums::{HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, KEY_READ, KEY_WOW64_32KEY};

    use crate::l10n::t;

    const CLIENT_KEY: &str =
        r"SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}";
    const BOOTSTRAPPER_URL: &str = "https://go.microsoft.com/fwlink/p/?LinkId=2124703";

    fn installed() -> bool {
        let has_version = |root, flags| {
            RegKey::predef(root)
                .open_subkey_with_flags(CLIENT_KEY, KEY_READ | flags)
                .and_then(|key| key.get_value::<String, _>("pv"))
                .is_ok_and(|pv| !pv.is_empty() && pv != "0.0.0.0")
        };
        has_version(HKEY_LOCAL_MACHINE, KEY_WOW64_32KEY) || has_version(HKEY_CURRENT_USER, 0)
    }

    fn message_box(text: &str, flags: u32) -> i32 {
        let text: Vec<u16> = text.encode_utf16().chain(Some(0)).collect();
        let title: Vec<u16> = t("WindowTitle", &[])
            .encode_utf16()
            .chain(Some(0))
            .collect();
        // SAFETY: valid NUL-terminated strings, no owner window.
        unsafe { MessageBoxW(std::ptr::null_mut(), text.as_ptr(), title.as_ptr(), flags) }
    }

    /// `true` if the wizard can run; else the user declined or the install failed (told already).
    pub fn ensure_installed() -> bool {
        if installed() {
            return true;
        }
        log::warn!("WebView2 runtime not found");
        if message_box(&t("WebView2Missing", &[]), MB_YESNO | MB_ICONQUESTION) != IDYES {
            return false;
        }
        let setup = std::env::temp_dir().join("MicrosoftEdgeWebview2Setup.exe");
        let result = hlae_installer_core::download::download(
            BOOTSTRAPPER_URL,
            &setup,
            &AtomicBool::new(false),
            |_, _| {},
        )
        .map_err(|e| e.to_string())
        .and_then(|_| {
            // TODO(M7): verify the Authenticode signature (Microsoft) before running it.
            let status = std::process::Command::new(&setup)
                .status()
                .map_err(|e| e.to_string())?;
            log::info!("WebView2 bootstrapper exit status {status}");
            Ok(())
        });
        let _ = std::fs::remove_file(&setup);
        if let Err(e) = &result {
            log::error!("WebView2 install: {e}");
        }
        if installed() {
            return true;
        }
        let error = result.err().unwrap_or_else(|| "?".into());
        message_box(
            &t("WebView2Failed", &[("error", &error)]),
            MB_OK | MB_ICONERROR,
        );
        false
    }
}
