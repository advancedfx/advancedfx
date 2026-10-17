//! Install / uninstall as started by the user, shared by the command line mode and the GUI:
//! FFMPEG is resolved (and downloaded) unelevated first, then the elevated part runs in-process
//! (already elevated) or in the worker (NOTES.md 6.4, 6.8a).

use std::path::PathBuf;
use std::sync::atomic::AtomicBool;

use hlae_installer_core::legacy::{self, LegacyInstall};
use hlae_installer_core::state::State;
use hlae_installer_core::{Error, session, win};

use crate::cli::exit_code;
use crate::ffmpeg_client::{self, FfmpegArgs};
use crate::ops::{self, Request};
use crate::{payload, worker};

pub struct InstallChoice {
    pub install_dir: Option<PathBuf>,
    pub locales: Option<bool>,
    pub ffmpeg: FfmpegArgs,
}

/// Returns the exit code; `cancel` aborts the FFMPEG download (exit code 1602).
pub fn install(choice: &InstallChoice, cancel: &AtomicBool) -> i32 {
    let legacy = legacy();
    if let Some(legacy) = legacy.as_ref().filter(|l| l.is_wix_v3()) {
        log::error!("{}", session::legacy_v3_message(legacy));
        return exit_code::FATAL;
    }
    let resolved = payload::ffmpeg_data().and_then(|data| {
        let session = ops::session()?;
        ffmpeg_client::resolve(
            &choice.ffmpeg,
            &data,
            &session.state_file,
            legacy.as_ref(),
            cancel,
        )
    });
    let (plan, download) = match resolved {
        Ok(resolved) => resolved,
        Err(Error::Cancelled) => {
            log::warn!("cancelled");
            return exit_code::USER_CANCEL;
        }
        Err(e) => {
            log::error!("{e}");
            return exit_code::FATAL;
        }
    };
    let code = dispatch(&Request::Install {
        install_dir: choice.install_dir.clone(),
        locales: choice.locales,
        ffmpeg: plan,
    });
    // Keep the download on failure, a retry can reuse it.
    if let Some(download) = download.filter(|_| code == exit_code::SUCCESS) {
        let _ = std::fs::remove_file(&download);
    }
    code
}

pub fn uninstall(remove_app_data: bool) -> i32 {
    let code = dispatch(&Request::Uninstall);
    // Done here, unelevated, so it's the logged-in user's %APPDATA% (NOTES.md 6.4).
    if remove_app_data && code == exit_code::SUCCESS {
        remove_hlae_app_data();
    }
    code
}

/// The old WiX setup's installation if it matters: only before our first install, never in the
/// debug test mode (the worker doesn't migrate there either).
pub fn legacy() -> Option<LegacyInstall> {
    if ops::test_mode() {
        return None;
    }
    let installed = ops::session()
        .ok()
        .and_then(|s| State::load(&s.state_file).ok().flatten())
        .is_some();
    if installed { None } else { legacy::detect() }
}

/// Elevated already (admin shell, deployment tool) → in-process, else via the elevated worker.
fn dispatch(request: &Request) -> i32 {
    let in_process = if ops::test_mode() {
        !ops::test_worker()
    } else {
        win::is_elevated().unwrap_or(false)
    };
    if in_process {
        crate::events::phase("work");
        ops::run(request)
    } else {
        worker::run_elevated(request)
    }
}

fn remove_hlae_app_data() {
    if win::is_local_system().unwrap_or(false) {
        log::warn!(
            "removing %APPDATA%\\HLAE is skipped when running as SYSTEM (no logged-in user)"
        );
        return;
    }
    match session::remove_app_data() {
        Ok(true) => log::info!("removed %APPDATA%\\HLAE"),
        Ok(false) => log::info!("no %APPDATA%\\HLAE to remove"),
        Err(e) => log::error!("removing %APPDATA%\\HLAE: {e}"),
    }
}
