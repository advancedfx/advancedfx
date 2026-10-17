// No console window in release builds.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod cli;
mod logging;
mod ops;
mod payload;
mod worker;

use tauri::{WebviewUrl, WebviewWindowBuilder};

#[tauri::command]
fn core_version() -> String {
    format!(
        "{} ({})",
        hlae_installer_core::version(),
        if tauri::is_dev() { "dev" } else { "production" }
    )
}

#[tauri::command]
fn payload_info() -> Result<String, String> {
    let manifest = payload::manifest().map_err(|e| e.to_string())?;
    Ok(format!(
        "HLAE {}, {} files, {} bytes compressed",
        manifest.hlae_version,
        manifest.files.len(),
        payload::payload().len()
    ))
}

fn main() {
    // Elevated worker (internal, NOTES.md 6.4): no webview, no console, no log file.
    let raw_args: Vec<_> = std::env::args_os().skip(1).collect();
    if raw_args.first().is_some_and(|a| a == worker::WORKER_ARG) {
        harden_dll_search();
        std::process::exit(worker::main(&raw_args[1..]));
    }

    // Arguments first: the command line mode never creates a webview (NOTES.md 6.3).
    let args = cli::parse(raw_args);
    if matches!(&args, Ok(args) if args.is_gui()) {
        run_gui();
        return;
    }

    harden_dll_search();
    attach_parent_console();
    let code = match args {
        Ok(args) => {
            let log_file = args.log.clone().unwrap_or_else(logging::default_log_file);
            if let Err(e) = logging::init(&log_file) {
                eprintln!("could not create log file {}: {e}", log_file.display());
            }
            log::info!(
                "{} started: {:?}",
                cli::written_by(),
                std::env::args().collect::<Vec<_>>()
            );
            log::info!("log file: {}", log_file.display());
            let code = cli::run(args);
            log::info!("exit code {code}");
            log::logger().flush();
            code
        }
        Err(e) => {
            eprintln!("{e}\n\n{}", cli::USAGE);
            cli::exit_code::INVALID_COMMAND_LINE
        }
    };
    std::process::exit(code);
}

/// DLL planting protection (NOTES.md 6.4): later `LoadLibrary` calls only search System32, never the
/// directory HLAE_Setup.exe was started from (e.g. Downloads). Static imports are covered by the
/// `/DEPENDENTLOADFLAG` link flag (`build.rs`), which also applies before `main`.
/// Not (yet) for the GUI: WebView2 loads its own DLLs and may rely on the default search order.
fn harden_dll_search() {
    use windows_sys::Win32::System::LibraryLoader::{
        LOAD_LIBRARY_SEARCH_SYSTEM32, SetDefaultDllDirectories,
    };
    // SAFETY: plain Win32 call.
    unsafe { SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32) };
}

/// Release builds are GUI subsystem apps: attach to the console we were started from (if any) for output.
fn attach_parent_console() {
    #[cfg(not(debug_assertions))]
    {
        use windows_sys::Win32::System::Console::{ATTACH_PARENT_PROCESS, AttachConsole};
        // SAFETY: plain Win32 call; failure (no parent console) is fine.
        unsafe { AttachConsole(ATTACH_PARENT_PROCESS) };
    }
}

fn run_gui() {
    // Keep WebView2 user data out of %LOCALAPPDATA% (see NOTES.md 6.2).
    let webview_data_dir =
        std::env::temp_dir().join(format!("hlae-setup-webview-{}", std::process::id()));

    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![core_version, payload_info])
        .setup(move |app| {
            WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()))
                .title("HLAE Setup")
                .inner_size(600.0, 450.0)
                .resizable(false)
                .maximizable(false)
                .data_directory(webview_data_dir.clone())
                .build()?;
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running HLAE Setup");
}
