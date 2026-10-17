//! Command line / silent mode (NOTES.md 6.3). Never creates a webview.

use std::ffi::OsString;
use std::path::PathBuf;

use std::sync::atomic::AtomicBool;

use hlae_installer_core::ffmpeg_install::FfmpegSelection;

use crate::actions::{self, InstallChoice};
use crate::ffmpeg_client::{self, FfmpegArgs};
use crate::payload;

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
  --ffmpeg <choice>       FFMPEG: win64-gpl (static, default), win64-gpl-shared,
                          win64-lgpl, win64-lgpl-shared, none or keep. Default: install
                          on a fresh install, update a downloaded one, else keep.
  --ffmpeg-custom <exe>   Use an existing ffmpeg.exe instead (writes ffmpeg\\ffmpeg.ini).
  --ffmpeg-archive <zip>  Use an already downloaded FFMPEG archive (must be one of
                          this setup's archives, it's checked) instead of downloading.
  --remove-app-data       With --uninstall: also delete %APPDATA%\\HLAE (settings).
  --log <file>            Log file (default: %TEMP%\\HLAE_Setup_<time>.log).
  --help                  Show this help.

Without administrator rights a UAC prompt asks for them.
Options may also be written as /option. Exit codes: 0 ok, 1223 UAC prompt
declined, 1603 error, 1605 not installed (uninstall), 1639 invalid command line.";

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Action {
    Install,
    Modify,
    Uninstall,
}

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Args {
    pub quiet: bool,
    pub help: bool,
    pub action: Option<Action>,
    pub install_dir: Option<PathBuf>,
    pub locales: Option<bool>,
    pub remove_app_data: bool,
    pub log: Option<PathBuf>,
    /// `--ffmpeg` value, checked against the embedded variants in [`run`].
    pub ffmpeg: Option<String>,
    pub ffmpeg_custom: Option<PathBuf>,
    pub ffmpeg_archive: Option<PathBuf>,
}

/// Start page of the wizard.
#[derive(Clone, Copy, Debug, PartialEq, Eq, serde::Serialize)]
#[serde(rename_all = "kebab-case")]
pub enum GuiMode {
    /// Normal start: install, or maintenance if installed.
    Start,
    Modify,
    Uninstall,
}

impl Args {
    /// No options, or only `--install` / `--modify` / `--uninstall` without `--quiet`
    /// (e.g. from Apps & features) → wizard.
    pub fn gui_mode(&self) -> Option<GuiMode> {
        let without_action = Args {
            action: None,
            ..self.clone()
        };
        if without_action != Args::default() {
            return None;
        }
        Some(match self.action {
            None | Some(Action::Install) => GuiMode::Start,
            Some(Action::Modify) => GuiMode::Modify,
            Some(Action::Uninstall) => GuiMode::Uninstall,
        })
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
            "ffmpeg" => parsed.ffmpeg = Some(value()?.to_ascii_lowercase()),
            "ffmpeg-custom" => parsed.ffmpeg_custom = Some(PathBuf::from(value()?)),
            "ffmpeg-archive" => parsed.ffmpeg_archive = Some(PathBuf::from(value()?)),
            _ => return Err(format!("unknown option {arg:?}")),
        }
    }
    let uninstall = parsed.action == Some(Action::Uninstall);
    if parsed.remove_app_data && !uninstall {
        return Err("--remove-app-data only works with --uninstall".into());
    }
    let any_ffmpeg = parsed.ffmpeg.is_some()
        || parsed.ffmpeg_custom.is_some()
        || parsed.ffmpeg_archive.is_some();
    if uninstall && any_ffmpeg {
        return Err("--ffmpeg… options don't work with --uninstall".into());
    }
    if parsed.ffmpeg_custom.is_some()
        && (parsed.ffmpeg.is_some() || parsed.ffmpeg_archive.is_some())
    {
        return Err("--ffmpeg-custom excludes --ffmpeg and --ffmpeg-archive".into());
    }
    if parsed.ffmpeg_archive.is_some() && matches!(parsed.ffmpeg.as_deref(), Some("none" | "keep"))
    {
        return Err("--ffmpeg-archive needs an FFMPEG variant, not --ffmpeg none / keep".into());
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
        eprintln!("Options other than --install / --modify / --uninstall need --quiet.\n\n{USAGE}");
        return exit_code::INVALID_COMMAND_LINE;
    }

    match args.action {
        Some(Action::Uninstall) => actions::uninstall(args.remove_app_data),
        Some(Action::Install | Action::Modify) | None => match ffmpeg_args(&args) {
            Ok(ffmpeg) => {
                let choice = InstallChoice {
                    install_dir: args.install_dir.clone(),
                    locales: args.locales,
                    ffmpeg,
                };
                actions::install(&choice, &AtomicBool::new(false))
            }
            Err(code) => code,
        },
    }
}

fn ffmpeg_args(args: &Args) -> Result<FfmpegArgs, i32> {
    let data = payload::ffmpeg_data().map_err(|e| {
        log::error!("{e}");
        exit_code::FATAL
    })?;
    if let Some(exe) = &args.ffmpeg_custom {
        hlae_installer_core::ffmpeg_install::validate_custom_exe(exe).map_err(|e| {
            log::error!("{e}");
            exit_code::INVALID_COMMAND_LINE
        })?;
    }
    let selection = match (&args.ffmpeg_custom, &args.ffmpeg) {
        (Some(exe), _) => Some(FfmpegSelection::Custom(exe.clone())),
        (None, Some(value)) => Some(ffmpeg_client::parse_selection(value, &data).map_err(|e| {
            log::error!("{e}");
            exit_code::INVALID_COMMAND_LINE
        })?),
        (None, None) => None,
    };
    Ok(FfmpegArgs {
        selection,
        archive: args.ffmpeg_archive.clone(),
    })
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
    fn gui_modes() {
        assert_eq!(parse_str(&[]).unwrap().gui_mode(), Some(GuiMode::Start));
        assert_eq!(
            parse_str(&["--uninstall"]).unwrap().gui_mode(),
            Some(GuiMode::Uninstall)
        );
        assert_eq!(
            parse_str(&["/modify"]).unwrap().gui_mode(),
            Some(GuiMode::Modify)
        );
        assert_eq!(parse_str(&["--quiet"]).unwrap().gui_mode(), None);
        assert_eq!(
            parse_str(&["--uninstall", "--quiet"]).unwrap().gui_mode(),
            None
        );
        assert_eq!(parse_str(&["--no-locales"]).unwrap().gui_mode(), None);
        assert_eq!(parse_str(&["--help"]).unwrap().gui_mode(), None);
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
            &["--uninstall", "--ffmpeg", "none"],
            &["--ffmpeg-custom", "C:\\f.exe", "--ffmpeg", "win64-gpl"],
            &[
                "--ffmpeg-custom",
                "C:\\f.exe",
                "--ffmpeg-archive",
                "C:\\f.zip",
            ],
            &["--ffmpeg", "keep", "--ffmpeg-archive", "C:\\f.zip"],
        ] {
            assert!(parse_str(args).is_err(), "{args:?}");
        }
    }

    #[test]
    fn parses_ffmpeg_options() {
        let args = parse_str(&["--quiet", "--ffmpeg", "Win64-GPL-Shared"]).unwrap();
        assert_eq!(args.ffmpeg.as_deref(), Some("win64-gpl-shared"));
        let args = parse_str(&["--ffmpeg-archive=D:\\dl\\ffmpeg.zip"]).unwrap();
        assert_eq!(
            args.ffmpeg_archive,
            Some(PathBuf::from("D:\\dl\\ffmpeg.zip"))
        );
        let data = payload::ffmpeg_data().unwrap();
        assert!(ffmpeg_client::parse_selection("win64-lgpl", &data).is_ok());
        assert!(ffmpeg_client::parse_selection("win32-gpl", &data).is_err());
    }
}
