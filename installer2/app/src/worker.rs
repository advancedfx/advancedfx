//! Elevated worker process (NOTES.md 6.4): the unelevated process (CLI / GUI) starts
//! `HLAE_Setup.exe --worker <pipe> <client pid>` via UAC, sends one [`Request`] line over the pipe
//! and gets log lines and finally the exit code back.

use std::ffi::OsString;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Read, Write};
use std::os::windows::io::OwnedHandle;
use std::process::Command;
use std::sync::Mutex;
use std::sync::atomic::{AtomicBool, Ordering};
use std::time::Duration;

use hlae_installer_core::{pipe, win};
use serde::{Deserialize, Serialize};

use crate::cli::exit_code;
use crate::ops::{self, Request};

pub const WORKER_ARG: &str = "--worker";
/// The request is a single short JSON line.
const MAX_REQUEST_LEN: u64 = 64 * 1024;
/// `ERROR_CANCELLED`: the UAC prompt was declined.
const ERROR_CANCELLED: i32 = 1223;

#[derive(Debug, Serialize, Deserialize)]
#[serde(tag = "event", rename_all = "kebab-case")]
enum Event {
    Log { level: String, message: String },
    Done { exit_code: i32 },
}

// ---- client side (unelevated) ----

/// Runs `request` in the elevated worker, forwards its log, returns its exit code.
pub fn run_elevated(request: &Request) -> i32 {
    match run_elevated_inner(request) {
        Ok(code) => code,
        Err(e) => {
            log::error!("elevated worker: {e}");
            exit_code::FATAL
        }
    }
}

fn run_elevated_inner(request: &Request) -> io::Result<i32> {
    let name = pipe::random_pipe_name()?;
    let process = match spawn_worker(&name) {
        Err(e) if e.raw_os_error() == Some(ERROR_CANCELLED) => {
            log::error!("administrator rights were declined (UAC prompt)");
            return Ok(exit_code::UAC_DECLINED);
        }
        result => result?,
    };
    let pipe = pipe::connect(
        &name,
        pipe::process_id(&process),
        pipe::CONNECT_TIMEOUT,
        || pipe::exit_code(&process).is_none(),
    )?;

    let mut line = serde_json::to_string(request).map_err(io::Error::other)?;
    line.push('\n');
    (&pipe).write_all(line.as_bytes())?;

    // A closed pipe reads as end of file.
    for line in BufReader::new(&pipe).lines() {
        match serde_json::from_str::<Event>(&line?) {
            Ok(Event::Log { level, message }) => {
                log::log!(level.parse().unwrap_or(log::Level::Info), "{message}")
            }
            Ok(Event::Done { exit_code }) => {
                pipe::wait(&process);
                return Ok(exit_code);
            }
            Err(e) => log::warn!("unexpected message from the worker: {e}"),
        }
    }
    let code = pipe::wait(&process);
    log::error!("the worker ended without a result (exit code {code:?})");
    Ok(exit_code::FATAL)
}

fn spawn_worker(pipe_name: &str) -> io::Result<OwnedHandle> {
    let exe = std::env::current_exe()?;
    let pid = std::process::id().to_string();
    if ops::test_worker() {
        let child = Command::new(&exe)
            .args([WORKER_ARG, pipe_name, &pid])
            .spawn()?;
        log::info!("started the worker unelevated (test mode), pid {}", child.id());
        return Ok(child.into());
    }
    log::info!("starting the elevated worker (UAC prompt)");
    pipe::spawn_elevated(&exe, &format!("{WORKER_ARG} {pipe_name} {pid}"))
}

// ---- worker side (elevated) ----

/// `--worker <pipe> <client pid>`, `args` = the arguments after `--worker`.
pub fn main(args: &[OsString]) -> i32 {
    let (Some(name), Some(client_pid), None) = (
        args.first().and_then(|a| a.to_str()),
        args.get(1)
            .and_then(|a| a.to_str())
            .and_then(|a| a.parse::<u32>().ok()),
        args.get(2),
    ) else {
        return exit_code::INVALID_COMMAND_LINE;
    };
    if !ops::test_worker() && !win::is_elevated().unwrap_or(false) {
        return exit_code::FATAL;
    }

    // Nobody connecting (client gone) must not leave us waiting forever.
    static CONNECTED: AtomicBool = AtomicBool::new(false);
    std::thread::spawn(|| {
        std::thread::sleep(pipe::CONNECT_TIMEOUT + Duration::from_secs(10));
        if !CONNECTED.load(Ordering::SeqCst) {
            std::process::exit(exit_code::FATAL);
        }
    });
    let pipe = match pipe::serve(name, client_pid) {
        Ok(pipe) => pipe,
        Err(_) => return exit_code::FATAL, // nobody to report to
    };
    CONNECTED.store(true, Ordering::SeqCst);

    let mut line = String::new();
    let read = BufReader::new((&pipe).take(MAX_REQUEST_LEN)).read_line(&mut line);
    let writer: &'static Mutex<File> = Box::leak(Box::new(Mutex::new(pipe)));
    let request = match read
        .map_err(|e| e.to_string())
        .and_then(|_| serde_json::from_str::<Request>(&line).map_err(|e| e.to_string()))
    {
        Ok(request) => request,
        Err(e) => {
            send(
                writer,
                &Event::Log {
                    level: "ERROR".into(),
                    message: format!("worker: invalid request: {e}"),
                },
            );
            send(
                writer,
                &Event::Done {
                    exit_code: exit_code::INVALID_COMMAND_LINE,
                },
            );
            return exit_code::INVALID_COMMAND_LINE;
        }
    };

    let _ = log::set_logger(Box::leak(Box::new(PipeLogger { writer })));
    log::set_max_level(log::LevelFilter::Info);
    let code = ops::run(&request);
    send(writer, &Event::Done { exit_code: code });
    code
}

fn send(writer: &Mutex<File>, event: &Event) {
    let Ok(mut line) = serde_json::to_string(event) else {
        return;
    };
    line.push('\n');
    if let Ok(mut file) = writer.lock() {
        let _ = file.write_all(line.as_bytes());
    }
}

/// Forwards log records to the client.
struct PipeLogger {
    writer: &'static Mutex<File>,
}

impl log::Log for PipeLogger {
    fn enabled(&self, metadata: &log::Metadata) -> bool {
        metadata.level() <= log::Level::Info
    }

    fn log(&self, record: &log::Record) {
        if self.enabled(record.metadata()) {
            send(
                self.writer,
                &Event::Log {
                    level: record.level().to_string(),
                    message: record.args().to_string(),
                },
            );
        }
    }

    fn flush(&self) {}
}
