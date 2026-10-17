//! Developer tools for the HLAE installer (not shipped).
//!
//! `ffmpeg`: generates `installer2/data/ffmpeg.json` (NOTES.md 6.10 b) from a BtbN FFmpeg-Builds
//! release. Run by hand when updating FFMPEG, the result is committed; normal builds never download.
//!
//! ```text
//! cargo run -p hlae-installer-tools --release -- ffmpeg \
//!     --tag autobuild-2026-09-30-13-08 --build n9.0.2-17-g2a571b6068 --branch 9.0 \
//!     [--expect-sha512 win64-gpl=<hex> ...] [--cache <dir>] [--out <file>]
//! ```

use std::collections::HashMap;
use std::fs::File;
use std::io::{self, BufReader};
use std::path::{Path, PathBuf};
use std::sync::atomic::AtomicBool;

use hlae_installer_core::download::download;
use hlae_installer_core::ffmpeg::{self, FfmpegData, Variant};
use hlae_installer_core::hash;
use sha2::{Digest, Sha256, Sha512};

const RELEASES: &str = "https://github.com/BtbN/FFmpeg-Builds/releases/download";
/// (id, linking, license); the id is also the part of the BtbN file name.
const VARIANTS: [(&str, &str, &str); 4] = [
    ("win64-gpl", "static", "gpl"),
    ("win64-gpl-shared", "shared", "gpl"),
    ("win64-lgpl", "static", "lgpl"),
    ("win64-lgpl-shared", "shared", "lgpl"),
];
const DEFAULT_VARIANT: &str = "win64-gpl";

fn main() {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let result = match args.first().map(String::as_str) {
        Some("ffmpeg") => ffmpeg_command(&args[1..]),
        _ => Err(
            "usage: hlae-installer-tools ffmpeg --tag <tag> --build <build> --branch <branch> \
                  [--expect-sha512 <id>=<hex>]... [--cache <dir>] [--out <file>]"
                .into(),
        ),
    };
    if let Err(e) = result {
        eprintln!("error: {e}");
        std::process::exit(1);
    }
}

struct Options {
    tag: String,
    build: String,
    branch: String,
    expect_sha512: HashMap<String, String>,
    cache: PathBuf,
    out: PathBuf,
}

fn parse(args: &[String]) -> Result<Options, String> {
    let manifest_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    let mut options = Options {
        tag: String::new(),
        build: String::new(),
        branch: String::new(),
        expect_sha512: HashMap::new(),
        cache: manifest_dir.join("../../target/ffmpeg-cache"),
        out: manifest_dir.join("../data/ffmpeg.json"),
    };
    let mut args = args.iter();
    while let Some(arg) = args.next() {
        let mut value = || args.next().cloned().ok_or(format!("{arg} needs a value"));
        match arg.as_str() {
            "--tag" => options.tag = value()?,
            "--build" => options.build = value()?,
            "--branch" => options.branch = value()?,
            "--cache" => options.cache = value()?.into(),
            "--out" => options.out = value()?.into(),
            "--expect-sha512" => {
                let value = value()?;
                let (id, hex) = value
                    .split_once('=')
                    .ok_or("--expect-sha512 needs <id>=<hex>")?;
                options
                    .expect_sha512
                    .insert(id.to_owned(), hex.to_lowercase());
            }
            _ => return Err(format!("unknown argument {arg}")),
        }
    }
    if options.tag.is_empty() || options.build.is_empty() || options.branch.is_empty() {
        return Err("--tag, --build and --branch are required".into());
    }
    Ok(options)
}

fn ffmpeg_command(args: &[String]) -> Result<(), String> {
    let options = parse(args)?;
    std::fs::create_dir_all(&options.cache).map_err(|e| e.to_string())?;
    let cancel = AtomicBool::new(false);

    // BtbN's checksum list for the release ("<sha256>  <file name>" per line).
    let checksums_file = options
        .cache
        .join(format!("{}-checksums.sha256", options.tag));
    if !checksums_file.exists() {
        let url = format!("{RELEASES}/{}/checksums.sha256", options.tag);
        download(&url, &checksums_file, &cancel, |_, _| {}).map_err(|e| e.to_string())?;
    }
    let checksums: HashMap<String, String> = std::fs::read_to_string(&checksums_file)
        .map_err(|e| e.to_string())?
        .lines()
        .filter_map(|line| line.split_once(char::is_whitespace))
        .map(|(hash, name)| (name.trim().to_owned(), hash.to_lowercase()))
        .collect();

    let mut variants = Vec::new();
    for (id, linking, license) in VARIANTS {
        let name = format!("ffmpeg-{}-{id}-{}.zip", options.build, options.branch);
        let url = format!("{RELEASES}/{}/{name}", options.tag);
        let archive = options.cache.join(&name);
        if archive.exists() {
            println!("{id}: using cached {}", archive.display());
        } else {
            println!("{id}: downloading {url}");
            let mut last_percent = None;
            download(&url, &archive, &cancel, |done, total| {
                if let Some(total) = total.filter(|&t| t > 0) {
                    let percent = done * 100 / total / 10 * 10;
                    if last_percent != Some(percent) {
                        last_percent = Some(percent);
                        println!("  {percent:3}% ({} MB)", done / 1_000_000);
                    }
                }
            })
            .map_err(|e| e.to_string())?;
        }

        let (sha256, sha512, size) = hash_file(&archive).map_err(|e| e.to_string())?;
        match checksums.get(&name) {
            Some(expected) if *expected == sha256 => {
                println!("{id}: SHA-256 matches BtbN's checksums.sha256")
            }
            Some(expected) => return Err(format!("{id}: SHA-256 {sha256} != BtbN's {expected}")),
            None => return Err(format!("{id}: {name} missing in BtbN's checksums.sha256")),
        }
        match options.expect_sha512.get(id) {
            Some(expected) if *expected == sha512 => {
                println!("{id}: SHA-512 matches --expect-sha512")
            }
            Some(expected) => return Err(format!("{id}: SHA-512 {sha512} != expected {expected}")),
            None => println!("{id}: SHA-512 {sha512}"),
        }

        let strip_prefix = ffmpeg::archive_prefix(&archive).map_err(|e| e.to_string())?;
        let files = ffmpeg::inspect_archive(&archive, &strip_prefix).map_err(|e| e.to_string())?;
        println!(
            "{id}: {} files below {strip_prefix}, {} MB archive",
            files.len(),
            size / 1_000_000
        );
        variants.push(Variant {
            id: id.to_owned(),
            linking: linking.to_owned(),
            license: license.to_owned(),
            url,
            archive_size: size,
            archive_sha256: sha256,
            strip_prefix,
            files,
        });
    }

    let data = FfmpegData {
        format: ffmpeg::FORMAT,
        version: format!("{} ({})", options.build, options.tag),
        default_variant: DEFAULT_VARIANT.to_owned(),
        variants,
    };
    data.validate().map_err(|e| e.to_string())?;
    if let Some(dir) = options.out.parent() {
        std::fs::create_dir_all(dir).map_err(|e| e.to_string())?;
    }
    std::fs::write(&options.out, data.to_json().map_err(|e| e.to_string())?)
        .map_err(|e| e.to_string())?;
    println!("wrote {}", options.out.display());
    Ok(())
}

/// SHA-256, SHA-512 and size in one pass.
fn hash_file(path: &Path) -> io::Result<(String, String, u64)> {
    struct Both(Sha256, Sha512);
    impl io::Write for Both {
        fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
            self.0.update(buf);
            self.1.update(buf);
            Ok(buf.len())
        }
        fn flush(&mut self) -> io::Result<()> {
            Ok(())
        }
    }
    let mut both = Both(Sha256::new(), Sha512::new());
    let size = io::copy(&mut BufReader::new(File::open(path)?), &mut both)?;
    Ok((
        hash::to_hex(&both.0.finalize()),
        hash::to_hex(&both.1.finalize()),
        size,
    ))
}
