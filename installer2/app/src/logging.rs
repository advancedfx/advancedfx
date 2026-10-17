//! Log to a file and the console (NOTES.md 4.1).

use std::fs::File;
use std::io::{self, Write};
use std::path::{Path, PathBuf};
use std::sync::Mutex;

use hlae_installer_core::{time, win};

struct Logger {
    file: Mutex<Option<File>>,
}

impl log::Log for Logger {
    fn enabled(&self, metadata: &log::Metadata) -> bool {
        metadata.level() <= log::Level::Info
    }

    fn log(&self, record: &log::Record) {
        if !self.enabled(record.metadata()) {
            return;
        }
        let message = record.args().to_string();
        let line = format!("{} {:5} {message}", time::utc_now(), record.level());
        eprintln!("{line}");
        crate::events::emit(crate::events::Event::Log {
            level: record.level().to_string(),
            message,
        });
        if let Ok(mut file) = self.file.lock()
            && let Some(file) = file.as_mut()
        {
            let _ = writeln!(file, "{line}");
        }
    }

    fn flush(&self) {
        if let Ok(mut file) = self.file.lock()
            && let Some(file) = file.as_mut()
        {
            let _ = file.flush();
        }
    }
}

/// The default log `%TEMP%\HLAE_Setup_<time>.log`, only ever created new: as SYSTEM `%TEMP%` is the
/// shared `C:\Windows\Temp`, where someone could have created that name already, e.g. to read along
/// (NOTES.md M8 audit). Then a name with a random part instead.
fn create_default_log() -> (PathBuf, io::Result<File>) {
    let stamp = time::utc_now().replace([':', '-'], "");
    let path = std::env::temp_dir().join(format!("HLAE_Setup_{stamp}.log"));
    match File::create_new(&path) {
        Err(e) if e.kind() == io::ErrorKind::AlreadyExists => match win::random_hex() {
            Ok(random) => {
                let path =
                    std::env::temp_dir().join(format!("HLAE_Setup_{stamp}-{}.log", &random[..8]));
                let file = File::create_new(&path);
                (path, file)
            }
            Err(e) => (path, Err(e)),
        },
        file => (path, file),
    }
}

/// Installs the logger, logging to `explicit` (`--log`, created or overwritten) or the default log.
/// Returns the log file and an error if it couldn't be created (logging to the console then).
pub fn init(explicit: Option<&Path>) -> (PathBuf, io::Result<()>) {
    let (path, file) = match explicit {
        Some(path) => (path.to_path_buf(), File::create(path)),
        None => create_default_log(),
    };
    let opened = file
        .as_ref()
        .map(|_| ())
        .map_err(|e| io::Error::new(e.kind(), e.to_string()));
    let logger = Box::leak(Box::new(Logger {
        file: Mutex::new(file.ok()),
    }));
    let _ = log::set_logger(logger);
    log::set_max_level(log::LevelFilter::Info);
    (path, opened)
}
