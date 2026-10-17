//! Command line / silent mode (NOTES.md 6.3). Never creates a webview.

use std::ffi::OsString;
use std::path::PathBuf;

use hlae_installer_core::{session, win};

use crate::ops::{self, Request};
use crate::worker;

/// MSI-compatible exit codes (NOTES.md 6.4), deployment tools understand them.
#[allow(dead_code)] // cancel / reboot are used from M6 / M7 on.
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

Without administrator rights a UAC prompt asks for them.
Options may also be written as /option. Exit codes: 0 ok, 1223 UAC prompt
declined, 1603 error, 1605 not installed (uninstall), 1639 invalid command line.";

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

    let request = match args.action {
        Some(Action::Uninstall) => Request::Uninstall,
        Some(Action::Install | Action::Modify) | None => Request::Install {
            install_dir: args.install_dir.clone(),
            locales: args.locales,
        },
    };

    // Elevated already (admin shell, deployment tool) → in-process, else via the elevated worker.
    let in_process = if ops::test_mode() {
        !ops::test_worker()
    } else {
        win::is_elevated().unwrap_or(false)
    };
    let code = if in_process {
        ops::run(&request)
    } else {
        worker::run_elevated(&request)
    };

    // Done here, unelevated, so it's the logged-in user's %APPDATA% (NOTES.md 6.4).
    if request == Request::Uninstall && args.remove_app_data && code == exit_code::SUCCESS {
        remove_app_data();
    }
    code
}

fn remove_app_data() {
    if win::is_local_system().unwrap_or(false) {
        log::warn!("--remove-app-data is ignored when running as SYSTEM (no logged-in user)");
        return;
    }
    match session::remove_app_data() {
        Ok(true) => log::info!("removed %APPDATA%\\HLAE"),
        Ok(false) => log::info!("no %APPDATA%\\HLAE to remove"),
        Err(e) => log::error!("removing %APPDATA%\\HLAE: {e}"),
    }
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
