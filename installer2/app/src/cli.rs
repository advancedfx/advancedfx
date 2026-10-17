//! Command line / silent mode (NOTES.md 6.3). Never creates a webview.

use std::ffi::OsString;
use std::path::PathBuf;

use hlae_installer_core::dotnet;
use hlae_installer_core::session::{self, InstallOptions, Session};
use hlae_installer_core::win;

use crate::payload;

/// MSI-compatible exit codes (NOTES.md 6.4), deployment tools understand them.
#[allow(dead_code)] // UAC / cancel / reboot are used from M4 / M6 / M7 on.
pub mod exit_code {
    pub const SUCCESS: i32 = 0;
    pub const UAC_DECLINED: i32 = 1223;
    pub const USER_CANCEL: i32 = 1602;
    pub const FATAL: i32 = 1603;
    pub const NOT_INSTALLED: i32 = 1605;
    pub const INVALID_COMMAND_LINE: i32 = 1639;
    pub const REBOOT_REQUIRED: i32 = 3010;
}

pub const USAGE: &str = "\
HLAE Setup

Usage: HLAE_Setup.exe [options]

Without options the setup wizard is shown.

  --quiet                 Unattended, no user interface.
  --install               Install, upgrade or repair HLAE (default action).
  --modify                Change the installed features (with --quiet: like --install).
  --uninstall             Uninstall HLAE.
  --install-dir <dir>     Install location (default: installed location, else
                          \"C:\\Program Files (x86)\\HLAE\"). Basic latin (ASCII) only.
  --locales               Install the translations (default: as installed, else on).
  --no-locales            Don't install the translations.
  --remove-app-data       With --uninstall: also delete %APPDATA%\\HLAE (settings).
  --log <file>            Log file (default: %TEMP%\\HLAE_Setup_<time>.log).
  --help                  Show this help.

Options may also be written as /option. Exit codes: 0 ok, 1603 error,
1605 not installed (uninstall), 1639 invalid command line.";

#[derive(Debug, PartialEq, Eq)]
pub enum Action {
    Install,
    Modify,
    Uninstall,
}

#[derive(Debug, Default, PartialEq, Eq)]
pub struct Args {
    pub quiet: bool,
    pub help: bool,
    pub action: Option<Action>,
    pub install_dir: Option<PathBuf>,
    pub locales: Option<bool>,
    pub remove_app_data: bool,
    pub log: Option<PathBuf>,
}

impl Args {
    /// No options → GUI.
    pub fn is_gui(&self) -> bool {
        *self == Args::default()
    }
}

pub fn parse(args: impl IntoIterator<Item = OsString>) -> Result<Args, String> {
    let mut parsed = Args::default();
    let mut args = args.into_iter();
    while let Some(arg) = args.next() {
        let arg = arg
            .to_str()
            .ok_or("invalid characters in command line")?
            .to_owned();
        let Some(option) = arg.strip_prefix("--").or_else(|| arg.strip_prefix('/')) else {
            return Err(format!("unexpected argument {arg:?}"));
        };
        let (name, inline_value) = match option.split_once('=') {
            Some((name, value)) => (name.to_ascii_lowercase(), Some(value.to_owned())),
            None => (option.to_ascii_lowercase(), None),
        };
        let mut value = || -> Result<String, String> {
            match &inline_value {
                Some(value) => Ok(value.clone()),
                None => args
                    .next()
                    .and_then(|v| v.into_string().ok())
                    .ok_or_else(|| format!("--{name} needs a value")),
            }
        };
        let set_action = |parsed: &mut Args, action| match &parsed.action {
            Some(previous) if *previous != action => {
                Err("--install, --modify and --uninstall exclude each other".to_owned())
            }
            _ => {
                parsed.action = Some(action);
                Ok(())
            }
        };
        match name.as_str() {
            "quiet" => parsed.quiet = true,
            "help" | "?" => parsed.help = true,
            "install" => set_action(&mut parsed, Action::Install)?,
            "modify" => set_action(&mut parsed, Action::Modify)?,
            "uninstall" => set_action(&mut parsed, Action::Uninstall)?,
            "install-dir" => parsed.install_dir = Some(PathBuf::from(value()?)),
            "locales" => parsed.locales = Some(true),
            "no-locales" => parsed.locales = Some(false),
            "remove-app-data" => parsed.remove_app_data = true,
            "log" => parsed.log = Some(PathBuf::from(value()?)),
            _ => return Err(format!("unknown option {arg:?}")),
        }
    }
    if parsed.remove_app_data && parsed.action != Some(Action::Uninstall) {
        return Err("--remove-app-data only works with --uninstall".into());
    }
    Ok(parsed)
}

/// Runs the command line mode, returns the exit code.
pub fn run(args: Args) -> i32 {
    if args.help {
        println!("{USAGE}");
        return exit_code::SUCCESS;
    }
    if !args.quiet {
        // TODO(M6): pass the options to the wizard instead.
        eprintln!("Options without --quiet are not supported yet (wizard: M6).\n\n{USAGE}");
        return exit_code::INVALID_COMMAND_LINE;
    }

    match run_quiet(&args) {
        Ok(code) => code,
        Err(e) => {
            log::error!("{e}");
            exit_code::FATAL
        }
    }
}

fn run_quiet(args: &Args) -> hlae_installer_core::Result<i32> {
    let session = session()?;
    if session.integrate && !win::is_elevated().unwrap_or(false) {
        // TODO(M4): self-elevate via the worker process.
        log::error!("HLAE Setup must be run as administrator");
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

    match args.action.as_ref().unwrap_or(&Action::Install) {
        Action::Install | Action::Modify => {
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
                install_dir: args.install_dir.clone(),
                locales: args.locales,
            };
            let summary =
                session.install(&manifest, payload::payload(), &options, &written_by())?;
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
        Action::Uninstall => {
            let Some(summary) = session.uninstall()? else {
                log::warn!("HLAE is not installed");
                return Ok(exit_code::NOT_INSTALLED);
            };
            let report = &summary.files;
            for path in &report.kept {
                log::warn!("kept modified file: {path}");
            }
            if args.remove_app_data {
                match session::remove_app_data() {
                    Ok(true) => log::info!("removed %APPDATA%\\HLAE"),
                    Ok(false) => log::info!("no %APPDATA%\\HLAE to remove"),
                    Err(e) => log::error!("removing %APPDATA%\\HLAE: {e}"),
                }
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

/// The system session; debug builds only: `HLAE_SETUP_STATE_FILE` overrides the state file and turns
/// off the Windows integration (and with it the administrator check), so the CLI can be tested without
/// admin rights.
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

pub fn written_by() -> String {
    format!("HLAE_Setup {}", env!("CARGO_PKG_VERSION"))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn parse_str(args: &[&str]) -> Result<Args, String> {
        parse(args.iter().map(OsString::from))
    }

    #[test]
    fn no_args_is_gui() {
        assert!(parse_str(&[]).unwrap().is_gui());
        assert!(!parse_str(&["--quiet"]).unwrap().is_gui());
    }

    #[test]
    fn parses_options() {
        let args = parse_str(&[
            "/quiet",
            "--install-dir",
            "D:\\HLAE",
            "--NO-LOCALES",
            "--log=C:\\x.log",
        ])
        .unwrap();
        assert!(args.quiet);
        assert_eq!(args.install_dir, Some(PathBuf::from("D:\\HLAE")));
        assert_eq!(args.locales, Some(false));
        assert_eq!(args.log, Some(PathBuf::from("C:\\x.log")));
        let args = parse_str(&["--uninstall", "--remove-app-data", "--quiet"]).unwrap();
        assert_eq!(args.action, Some(Action::Uninstall));
        assert!(args.remove_app_data);
    }

    #[test]
    fn rejects_invalid() {
        for args in [
            &["--bogus"][..],
            &["install"],
            &["--install-dir"],
            &["--install", "--uninstall"],
            &["--remove-app-data"],
        ] {
            assert!(parse_str(args).is_err(), "{args:?}");
        }
    }
}
