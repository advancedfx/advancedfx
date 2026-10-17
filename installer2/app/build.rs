use std::fs::{self, File};
use std::io::BufWriter;
use std::path::PathBuf;

fn main() {
    // DLL planting protection (NOTES.md 6.4): the loader resolves the exe's static imports only from
    // System32 (LOAD_LIBRARY_SEARCH_SYSTEM32), never from the dir the exe was started from.
    // Safe: all static imports are Windows system DLLs (checked with `dumpbin /dependents`).
    println!("cargo:rustc-link-arg-bins=/DEPENDENTLOADFLAG:0x800");
    pack_payload();
    build_catalogs();
    tauri_build::build()
}

/// Translation catalogs for `src/l10n.rs` (NOTES.md 6.9): English from `installer2/l10n/messages.pot`,
/// translations from `deps/release/l10n/locales/<lang>/installer2/messages.po` (if present).
/// Writes `$OUT_DIR/l10n/<lang>.json` and `$OUT_DIR/l10n_index.rs`.
fn build_catalogs() {
    use hlae_installer_build_support::l10n;

    let manifest_dir =
        PathBuf::from(std::env::var_os("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
    let out_dir = PathBuf::from(std::env::var_os("OUT_DIR").expect("OUT_DIR")).join("l10n");
    fs::create_dir_all(&out_dir).expect("create l10n out dir");

    let pot = manifest_dir.join("../l10n/messages.pot");
    println!("cargo:rerun-if-changed={}", pot.display());
    let source = l10n::source_catalog(File::open(&pot).expect("open messages.pot"))
        .unwrap_or_else(|e| panic!("{}: {e}", pot.display()));
    let mut catalogs = vec![("en-US".to_owned(), source.clone())];

    let locales = manifest_dir.join("../../deps/release/l10n/locales");
    if locales.is_dir() {
        let mut langs: Vec<_> = fs::read_dir(&locales)
            .expect("list locales")
            .filter_map(|e| e.ok())
            .map(|e| e.file_name().to_string_lossy().into_owned())
            .filter(|lang| lang != "en-US")
            .collect();
        langs.sort();
        for lang in langs {
            let po = locales.join(&lang).join("installer2").join("messages.po");
            println!("cargo:rerun-if-changed={}", po.display());
            let Ok(file) = File::open(&po) else {
                continue;
            };
            let (catalog, warnings) = l10n::translated_catalog(file, &source)
                .unwrap_or_else(|e| panic!("{}: {e}", po.display()));
            for warning in warnings {
                println!("cargo:warning={lang}: {warning}");
            }
            if !catalog.is_empty() {
                catalogs.push((lang, catalog));
            }
        }
    }

    let mut index = String::from("pub static CATALOGS: &[(&str, &str)] = &[\n");
    for (lang, catalog) in &catalogs {
        let json = serde_json::to_string(catalog).expect("serialize catalog");
        fs::write(out_dir.join(format!("{lang}.json")), json).expect("write catalog");
        index.push_str(&format!(
            "    ({lang:?}, include_str!(concat!(env!(\"OUT_DIR\"), \"/l10n/{lang}.json\"))),\n"
        ));
    }
    index.push_str("];\n");
    fs::write(out_dir.parent().unwrap().join("l10n_index.rs"), index).expect("write l10n index");
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
        _ => hlae_installer_build_support::pack::ZSTD_LEVEL,
    };
    let payload_path = out_dir.join("payload.tar.zst");
    let file = File::create(&payload_path).expect("create payload");
    let manifest = hlae_installer_build_support::pack::pack_dir(
        &src_dir,
        &version,
        level,
        BufWriter::new(file),
    )
    .unwrap_or_else(|e| panic!("packing {}: {e}", src_dir.display()));
    let json = manifest.to_json().expect("serialize manifest");
    fs::write(out_dir.join("manifest.json"), json).expect("write manifest");
}
