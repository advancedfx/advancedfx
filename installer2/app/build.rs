use std::fs::{self, File};
use std::io::BufWriter;
use std::path::PathBuf;

fn main() {
    pack_payload();
    tauri_build::build()
}

/// Packs the staged HLAE build (`HLAE_PAYLOAD_DIR`, version `HLAE_VERSION`) into `$OUT_DIR`
/// for `src/payload.rs` (see NOTES.md 6.1 / 6.10 a).
/// Without `HLAE_PAYLOAD_DIR` an empty payload is embedded, so the GUI can be worked on without a HLAE build.
fn pack_payload() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-env-changed=HLAE_PAYLOAD_DIR");
    println!("cargo:rerun-if-env-changed=HLAE_VERSION");

    let out_dir = PathBuf::from(std::env::var_os("OUT_DIR").expect("OUT_DIR"));
    let (src_dir, version) = match std::env::var_os("HLAE_PAYLOAD_DIR") {
        Some(dir) => {
            let dir = PathBuf::from(dir);
            // Cargo scans a directory recursively for changes.
            println!("cargo:rerun-if-changed={}", dir.display());
            let version = std::env::var("HLAE_VERSION")
                .expect("HLAE_VERSION must be set together with HLAE_PAYLOAD_DIR");
            (dir, version)
        }
        None => {
            println!("cargo:warning=HLAE_PAYLOAD_DIR not set, embedding an empty HLAE payload");
            let empty = out_dir.join("empty-payload");
            fs::create_dir_all(&empty).expect("create empty payload dir");
            (empty, "0.0.0".to_owned())
        }
    };

    // Unoptimized (debug) builds: fast compression instead of the small one.
    let level = match std::env::var("OPT_LEVEL").as_deref() {
        Ok("0") => 3,
        _ => hlae_installer_core::pack::ZSTD_LEVEL,
    };
    let payload_path = out_dir.join("payload.tar.zst");
    let file = File::create(&payload_path).expect("create payload");
    let manifest = hlae_installer_core::pack::pack_dir(&src_dir, &version, level, BufWriter::new(file))
        .unwrap_or_else(|e| panic!("packing {}: {e}", src_dir.display()));
    let json = manifest.to_json().expect("serialize manifest");
    fs::write(out_dir.join("manifest.json"), json).expect("write manifest");
}
