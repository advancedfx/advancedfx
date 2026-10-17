//! The actual install / uninstall operations. They need admin rights: they run in the elevated
//! worker, or in-process if HLAE Setup was started elevated already (NOTES.md 6.4).

use std::path::PathBuf;

use hlae_installer_core::dotnet;
use hlae_installer_core::session::{InstallOptions, Session};
use hlae_installer_core::win;
use serde::{Deserialize, Serialize};

use crate::cli::exit_code;
use crate::payload;

/// What the unelevated process asks the worker to do. The worker validates it itself and only ever
/// installs its own embedded payload.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(tag = "op", rename_all = "kebab-case")]
pub enum Request {
    Install {
        install_dir: Option<PathBuf>,
        locales: Option<bool>,
    },
    Uninstall,
}

/// Debug builds only: `HLAE_SETUP_STATE_FILE` → test session without Windows integration and
/// without admin rights.
pub fn test_mode() -> bool {
    cfg!(debug_assertions) && std::env::var_os("HLAE_SETUP_STATE_FILE").is_some()
}

/// Debug builds only, with [`test_mode`]: `HLAE_SETUP_TEST_WORKER=1` → still use a worker process,
/// but started without elevation, so the pipe path can be tested without a UAC prompt.
pub fn test_worker() -> bool {
    test_mode() && std::env::var("HLAE_SETUP_TEST_WORKER").as_deref() == Ok("1")
}

fn session() -> hlae_installer_core::Result<Session> {
    #[cfg(debug_assertions)]
    if let Some(path) = std::env::var_os("HLAE_SETUP_STATE_FILE") {
        return Ok(Session {
            state_file: PathBuf::from(path),
            integrate: false,
        });
    }
    Session::system()
}

/// Runs `request`, returns the exit code.
pub fn run(request: &Request) -> i32 {
    match run_inner(request) {
        Ok(code) => code,
        Err(e) => {
            log::error!("{e}");
            exit_code::FATAL
        }
    }
}

fn run_inner(request: &Request) -> hlae_installer_core::Result<i32> {
    let session = session()?;
    if session.integrate && !win::is_elevated().unwrap_or(false) {
        log::error!("installing / uninstalling needs administrator rights");
        return Ok(exit_code::FATAL);
    }
    log::info!(
        "state file: {}{}",
        session.state_file.display(),
        if session.integrate {
            ""
        } else {
            " (test mode: no Windows integration)"
        }
    );

    match request {
        Request::Install {
            install_dir,
            locales,
        } => {
            if !dotnet::framework_ok() {
                log::warn!(
                    "HLAE needs .NET Framework {} or newer, please install it (found release {:?})",
                    dotnet::MIN_VERSION,
                    dotnet::framework_release()
                );
            }
            let manifest = payload::manifest()?;
            if manifest.files.is_empty() {
                log::error!(
                    "this build of HLAE Setup contains no HLAE files (built without HLAE_PAYLOAD_DIR)"
                );
                return Ok(exit_code::FATAL);
            }
            let options = InstallOptions {
                install_dir: install_dir.clone(),
                locales: *locales,
            };
            let summary = session.install(
                &manifest,
                payload::payload(),
                &options,
                &crate::cli::written_by(),
            )?;
            match &summary.previous_version {
                Some(previous) if summary.action == "upgrade" => {
                    log::info!("upgrade: HLAE {previous} -> {}", summary.version)
                }
                _ => log::info!("{}: HLAE {}", summary.action, summary.version),
            }
            for path in &summary.kept {
                log::warn!("kept modified file no longer part of HLAE: {path}");
            }
            log::info!("done: {}", summary.install_dir.display());
            Ok(exit_code::SUCCESS)
        }
        Request::Uninstall => {
            let Some(summary) = session.uninstall()? else {
                log::warn!("HLAE is not installed");
                return Ok(exit_code::NOT_INSTALLED);
            };
            let report = &summary.files;
            for path in &report.kept {
                log::warn!("kept modified file: {path}");
            }
            if !report.failed.is_empty() {
                for (path, error) in &report.failed {
                    log::error!("could not remove {path}: {error}");
                }
                log::error!(
                    "uninstall incomplete, close HLAE / games using it and run the uninstall again"
                );
                return Ok(exit_code::FATAL);
            }
            if !summary.integration_errors.is_empty() {
                return Ok(exit_code::FATAL);
            }
            log::info!("done: removed {} files", report.removed.len());
            Ok(exit_code::SUCCESS)
        }
    }
}
