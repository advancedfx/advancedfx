//! Progress events for the GUI (log lines, phases, download progress). Without a sink (command
//! line mode) they go nowhere; log lines still go to the log file / console via `logging`.

use std::sync::OnceLock;

use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
#[serde(tag = "kind", rename_all = "kebab-case")]
pub enum Event {
    Log {
        level: String,
        message: String,
    },
    /// `download`, `elevate`, `work`
    Phase {
        phase: &'static str,
    },
    Download {
        done: u64,
        total: u64,
    },
}

type Sink = Box<dyn Fn(Event) + Send + Sync>;
static SINK: OnceLock<Sink> = OnceLock::new();

pub fn set_sink(sink: impl Fn(Event) + Send + Sync + 'static) {
    let _ = SINK.set(Box::new(sink));
}

pub fn emit(event: Event) {
    if let Some(sink) = SINK.get() {
        sink(event);
    }
}

pub fn phase(phase: &'static str) {
    emit(Event::Phase { phase });
}
