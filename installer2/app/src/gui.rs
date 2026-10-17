//! The setup wizard (NOTES.md M6): Tauri commands for `ui/`; install / uninstall run on a background
//! thread (same flow as the command line, `actions`) and report through events.

use std::path::{Path, PathBuf};
use std::sync::OnceLock;
use std::sync::atomic::{AtomicBool, Ordering};

use hlae_installer_core::ffmpeg_install::{self, FfmpegSelection, MAX_CUSTOM_PATH};
use hlae_installer_core::in_use::Holder;
use hlae_installer_core::state::FfmpegChoice;
use hlae_installer_core::{Error, dotnet, protect, session, version};
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
    /// HLAE installed by the old WiX setup (only before our first install, NOTES.md 5).
    legacy: Option<Legacy>,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Legacy {
    version: String,
    /// Installed by the WiX v3 setup → not migrated, the user has to uninstall it first.
    wix_v3: bool,
}

#[tauri::command]
fn info(gui: State<'_, Gui>) -> Result<Info, String> {
    info_inner(&gui).map_err(|e| e.to_string())
}

fn info_inner(gui: &Gui) -> hlae_installer_core::Result<Info> {
    let manifest = payload::manifest()?;
    let data = payload::ffmpeg_data()?;
    let state = actions::installed_state();
    let newer_installed = state.as_ref().is_some_and(|s| {
        version::compare(&s.hlae_version, &manifest.hlae_version).is_ok_and(|o| o.is_gt())
    });
    let legacy = actions::legacy();
    let selection = match (&state, &legacy) {
        (None, Some(legacy)) => ffmpeg_install::legacy_selection(&data, legacy),
        _ => ffmpeg_install::default_selection(&data, state.as_ref()),
    };
    let default_install_dir = match legacy.as_ref().and_then(|l| l.install_dir.clone()) {
        Some(dir) => dir,
        None => session::default_install_dir()?,
    };
    let default_ffmpeg = match selection {
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
        default_install_dir: default_install_dir.display().to_string(),
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
        legacy: legacy.map(|l| Legacy {
            version: l.version().unwrap_or_default(),
            wix_v3: l.is_wix_v3(),
        }),
    })
}

/// Programs to close before the install (`true`) / uninstall can start; the page offers Retry.
#[tauri::command]
async fn programs_in_use(install: bool) -> Vec<Holder> {
    let holders = actions::programs_in_use(install);
    if !holders.is_empty() {
        log::warn!("{}", Error::FilesInUse(holders.clone()));
    }
    holders
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
    } else if !new_install_dir_ok(dir) {
        Some("InstallDirNotEmpty")
    } else {
        None
    }
}

/// Like the worker decides it (`protect::prepare`): outside `Program Files` a new install needs a new
/// or empty folder, except the old setup's folder (migration).
fn new_install_dir_ok(dir: &Path) -> bool {
    static LEGACY_DIR: OnceLock<Option<PathBuf>> = OnceLock::new();
    let protects = ops::session().is_ok_and(|s| s.protect);
    if !protects || protect::is_under_program_files(dir) {
        return true;
    }
    let legacy_dir = LEGACY_DIR.get_or_init(|| actions::legacy().and_then(|l| l.install_dir));
    let same = |a: &Path, b: &Path| {
        let normalize = |p: &Path| {
            p.to_string_lossy()
                .trim_end_matches(['\\', '/'])
                .to_lowercase()
        };
        normalize(a) == normalize(b)
    };
    if legacy_dir
        .as_deref()
        .is_some_and(|legacy| same(legacy, dir))
    {
        return true;
    }
    protect::is_missing_or_empty(dir).unwrap_or(true)
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
    let state = actions::installed_state().ok_or("HLAE is not installed")?;
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

    // Keep WebView2 user data out of %LOCALAPPDATA% (see NOTES.md 6.2). The WebView2 processes
    // only end when this process ends, so the dir can't be removed from here: a hidden helper
    // (same exe) waits for us to exit and removes it then. Started now, while our exe still has its
    // name – an uninstall from the cached setup renames it (self-delete, NOTES.md 6.5).
    let webview_data_dir =
        std::env::temp_dir().join(format!("{WEBVIEW_DIR_PREFIX}{}", std::process::id()));
    spawn_cleanup(&webview_data_dir);
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
            programs_in_use,
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
    log::info!("wizard closed, exit code {code}");
    log::logger().flush();
    code
}

pub const CLEANUP_ARG: &str = "--cleanup-webview-data";
const WEBVIEW_DIR_PREFIX: &str = "hlae-setup-webview-";

/// Starts the hidden `--cleanup-webview-data <dir> <our pid>` helper.
fn spawn_cleanup(dir: &Path) {
    use std::os::windows::process::CommandExt;
    const CREATE_NO_WINDOW: u32 = 0x0800_0000;
    let result = std::env::current_exe().and_then(|exe| {
        std::process::Command::new(exe)
            .arg(CLEANUP_ARG)
            .arg(dir)
            .arg(std::process::id().to_string())
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

/// `--cleanup-webview-data <dir> <wizard pid>` (internal): waits until the wizard process has
/// ended (also if it crashed), then removes its WebView2 data dir, retrying for up to
/// [`WEBVIEW_CLEANUP_TIMEOUT`] while the WebView2 processes shut down (slow PCs).
/// Only accepts `%TEMP%\hlae-setup-webview-*`, so it can't be used to delete anything else.
pub fn cleanup_main(args: &[std::ffi::OsString]) -> i32 {
    let [dir, pid] = args else {
        return exit_code::INVALID_COMMAND_LINE;
    };
    let dir = Path::new(dir);
    let ours = dir.parent() == Some(std::env::temp_dir().as_path())
        && dir
            .file_name()
            .and_then(|n| n.to_str())
            .is_some_and(|n| n.starts_with(WEBVIEW_DIR_PREFIX));
    let Some(pid) = pid
        .to_str()
        .and_then(|p| p.parse::<u32>().ok())
        .filter(|_| ours)
    else {
        return exit_code::INVALID_COMMAND_LINE;
    };
    wait_for_exit(pid);
    let deadline = std::time::Instant::now() + WEBVIEW_CLEANUP_TIMEOUT;
    while std::time::Instant::now() < deadline {
        if std::fs::remove_dir_all(dir).is_ok() || !dir.exists() {
            return exit_code::SUCCESS;
        }
        std::thread::sleep(std::time::Duration::from_millis(250));
    }
    exit_code::FATAL
}

/// Blocks until process `pid` has ended (returns at once if it's gone already).
fn wait_for_exit(pid: u32) {
    use windows_sys::Win32::Foundation::CloseHandle;
    use windows_sys::Win32::System::Threading::{
        INFINITE, OpenProcess, PROCESS_SYNCHRONIZE, WaitForSingleObject,
    };
    // SAFETY: plain Win32 calls; the handle is closed.
    unsafe {
        let process = OpenProcess(PROCESS_SYNCHRONIZE, 0, pid);
        if !process.is_null() {
            WaitForSingleObject(process, INFINITE);
            CloseHandle(process);
        }
    }
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

/// WebView2 Evergreen runtime check before any window is created (NOTES.md 6.2). If it's missing,
/// we only open Microsoft's download page – no automatic download / run (decided 2026-10-17: keeps
/// working when Microsoft changes the bootstrapper or its link, and we never run downloaded code).
mod webview2 {
    use windows_sys::Win32::UI::Shell::ShellExecuteW;
    use windows_sys::Win32::UI::WindowsAndMessaging::{
        IDYES, MB_ICONQUESTION, MB_YESNO, MessageBoxW, SW_SHOWNORMAL,
    };
    use winreg::RegKey;
    use winreg::enums::{HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, KEY_READ, KEY_WOW64_32KEY};

    use crate::l10n::t;

    const CLIENT_KEY: &str =
        r"SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}";
    /// Microsoft's WebView2 page ("Get the Runtime"); redirects to the user's language.
    const DOWNLOAD_PAGE: &str = "https://developer.microsoft.com/microsoft-edge/webview2/";

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

    /// `true` if the wizard can run. Else tells the user, optionally opens the download page, and
    /// the setup ends (they start it again after installing the runtime).
    pub fn ensure_installed() -> bool {
        if installed() {
            return true;
        }
        log::warn!("WebView2 runtime not found");
        let text = t("WebView2Missing", &[("url", DOWNLOAD_PAGE)]);
        if message_box(&text, MB_YESNO | MB_ICONQUESTION) == IDYES {
            let verb: Vec<u16> = "open".encode_utf16().chain(Some(0)).collect();
            let url: Vec<u16> = DOWNLOAD_PAGE.encode_utf16().chain(Some(0)).collect();
            // SAFETY: valid NUL-terminated strings; opens the URL in the default browser.
            unsafe {
                ShellExecuteW(
                    std::ptr::null_mut(),
                    verb.as_ptr(),
                    url.as_ptr(),
                    std::ptr::null(),
                    std::ptr::null(),
                    SW_SHOWNORMAL,
                )
            };
        }
        false
    }
}
