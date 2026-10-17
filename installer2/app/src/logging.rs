//! Log to a file and the console (NOTES.md 4.1).

use std::fs::File;
use std::io::{self, Write};
use std::path::{Path, PathBuf};
use std::sync::Mutex;

use hlae_installer_core::time;

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
        let line = format!("{} {:5} {}", time::utc_now(), record.level(), record.args());
        eprintln!("{line}");
        if let Ok(mut file) = self.file.lock() {
            if let Some(file) = file.as_mut() {
                let _ = writeln!(file, "{line}");
            }
        }
    }

    fn flush(&self) {
        if let Ok(mut file) = self.file.lock() {
            if let Some(file) = file.as_mut() {
                let _ = file.flush();
            }
        }
    }
}

/// `%TEMP%\HLAE_Setup_<time>.log`
pub fn default_log_file() -> PathBuf {
    let stamp = time::utc_now().replace([':', '-'], "");
    std::env::temp_dir().join(format!("HLAE_Setup_{stamp}.log"))
}

/// Installs the logger; returns an error if the log file can't be created (logging to the console then).
pub fn init(path: &Path) -> io::Result<()> {
    let file = File::create(path);
    let opened = file.as_ref().map(|_| ()).map_err(|e| io::Error::new(e.kind(), e.to_string()));
    let logger = Box::leak(Box::new(Logger {
        file: Mutex::new(file.ok()),
    }));
    let _ = log::set_logger(logger);
    log::set_max_level(log::LevelFilter::Info);
    opened
}
