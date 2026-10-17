// No console window in release builds.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod cli;
mod logging;
mod payload;

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
    // Arguments first: the command line mode never creates a webview (NOTES.md 6.3).
    let args = cli::parse(std::env::args_os().skip(1));
    if matches!(&args, Ok(args) if args.is_gui()) {
        run_gui();
        return;
    }

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
