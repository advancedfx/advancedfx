//! The embedded HLAE payload, packed by `build.rs` (see NOTES.md 6.1).
//! All access goes through here, so the embedding can be swapped (e.g. for a resource) later.

use hlae_installer_core::ffmpeg::FfmpegData;
use hlae_installer_core::manifest::Manifest;

/// `payload.tar.zst`; memory-mapped as part of the exe, stream from it, never copy it.
pub fn payload() -> &'static [u8] {
    include_bytes!(concat!(env!("OUT_DIR"), "/payload.tar.zst"))
}

pub fn manifest() -> hlae_installer_core::Result<Manifest> {
    Manifest::from_json(include_str!(concat!(env!("OUT_DIR"), "/manifest.json")))
}

/// License notices of the project and all its crates (`build.rs` `embed_licenses`).
pub fn licenses() -> String {
    let compressed = include_bytes!(concat!(env!("OUT_DIR"), "/licenses.txt.zst"));
    let text = zstd::decode_all(&compressed[..]).expect("embedded licenses are valid zstd");
    String::from_utf8(text).expect("embedded licenses are UTF-8")
}

/// The FFMPEG variants this build offers (`installer2/data/ffmpeg.json`, NOTES.md 6.10 b).
pub fn ffmpeg_data() -> hlae_installer_core::Result<FfmpegData> {
    FfmpegData::from_json(include_str!("../../data/ffmpeg.json"))
}
