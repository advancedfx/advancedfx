//! Build-time code of the HLAE installer, used by `app/build.rs` (and core's tests).
//! Nothing here runs on the user's machine; the install logic is in `hlae-installer-core`.

#[cfg(not(windows))]
compile_error!("hlae-installer-build-support only supports Windows.");

pub mod l10n;
pub mod pack;
