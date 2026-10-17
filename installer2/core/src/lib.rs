//! Install logic of the HLAE installer, independent of GUI / CLI (see installer2/NOTES.md).

#[cfg(not(windows))]
compile_error!("hlae-installer-core only supports Windows.");

pub mod acl;
pub mod dotnet;
pub mod download;
pub mod error;
pub mod ffmpeg;
pub mod ffmpeg_install;
pub mod fsutil;
pub mod hash;
pub mod install;
pub mod integration;
pub mod legacy;
pub mod manifest;
pub mod payload;
pub mod pipe;
pub mod registry;
pub mod relpath;
pub mod session;
pub mod shortcut;
pub mod state;
pub mod time;
pub mod transaction;
pub mod version;
pub mod win;

pub use error::{Error, Result};

pub fn version() -> &'static str {
    env!("CARGO_PKG_VERSION")
}
