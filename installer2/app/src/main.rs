// No console window in release builds.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod actions;
mod cli;
mod dialog;
mod events;
mod ffmpeg_client;
mod gui;
mod l10n;
mod logging;
mod ops;
mod payload;
mod worker;

fn main() {
    // Elevated worker (internal, NOTES.md 6.4): no webview, no console, no log file.
    let raw_args: Vec<_> = std::env::args_os().skip(1).collect();
    if raw_args.first().is_some_and(|a| a == worker::WORKER_ARG) {
        harden_dll_search();
        std::process::exit(worker::main(&raw_args[1..]));
    }
    // Hidden helper removing the wizard's WebView2 data dir after it exited (internal).
    if raw_args.first().is_some_and(|a| a == gui::CLEANUP_ARG) {
        harden_dll_search();
        std::process::exit(gui::cleanup_main(&raw_args[1..]));
    }

    // Arguments first: the command line mode never creates a webview (NOTES.md 6.3).
    let args = cli::parse(raw_args);
    if let Some(mode) = args.as_ref().ok().and_then(cli::Args::gui_mode) {
        std::process::exit(gui::run(mode));
    }

    harden_dll_search();
    attach_parent_console();
    let code = match args {
        Ok(args) => {
            let (log_file, opened) = logging::init(args.log.as_deref());
            if let Err(e) = opened {
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
