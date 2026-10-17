# installer2 – HLAE Setup

`HLAE_Setup.exe`: the HLAE installer, one self-contained exe written in Rust with a
[Tauri 2](https://tauri.app/) wizard (plain HTML / CSS / JS, no framework). It replaced the WiX
setup in `installer/`. Requirements, decisions and the dev log are in [NOTES.md](NOTES.md).

## Layout

| Crate / folder | What it is |
|---|---|
| [`core/`](core/) `hlae-installer-core` | All install logic, no UI: payload, transactions, state, Windows integration, FFMPEG, migration, protection |
| [`app/`](app/) `hlae-installer` | The exe: wizard (`src/gui.rs`, `ui/`), command line (`src/cli.rs`), elevated worker (`src/worker.rs`) |
| [`build-support/`](build-support/) | Build time only (`app/build.rs`): packs the payload, turns the `.po` files into catalogs |
| [`tools/`](tools/) | Hand-run dev tool: generates [`data/ffmpeg.json`](data/ffmpeg.json) from a BtbN FFmpeg-Builds release |
| [`data/ffmpeg.json`](data/ffmpeg.json) | The FFMPEG builds offered (URLs, SHA-256 of archives and files), committed |

The crates are members of the root Cargo workspace (`../Cargo.toml`). The UI texts live in the
l10n repo: `../deps/release/l10n/locales/en-US/installer2/messages.pot`.

## Building

The release build is part of `cmake -P cmake/MultiBuild.cmake` (see [`../BUILDING.md`](../BUILDING.md)),
which runs, after staging:

```batch
set HLAE_PAYLOAD_DIR=<package>\bin
set HLAE_VERSION=2.192.8
cargo build --locked --profile release-installer -p hlae-installer --target-dir build/cargo-installer
```

Without `HLAE_PAYLOAD_DIR` an empty payload is embedded, enough for working on the wizard:
`cargo build -p hlae-installer` → `target/debug/HLAE_Setup.exe`.

Tests: `cargo test -p hlae-installer-core -p hlae-installer` (no admin needed). Before committing:
`cargo fmt` and `cargo clippy --all-targets`.

## Testing without touching the real install

| Environment variable | Effect |
|---|---|
| `HLAE_SETUP_STATE_FILE=<file>` | Debug builds: own state file, no Windows integration, no admin rights needed |
| `HLAE_SETUP_TEST_WORKER=1` | Debug builds, with the above: still use the worker process, started unelevated |
| `HLAE_SETUP_TEST_PROTECT=1` | Debug builds, with the above, from an admin shell: protect the install dir like a real install |
| `HLAE_SETUP_THEME=light\|dark` | Debug builds: force a color scheme |
| `HLAE_SETUP_LANG=<lang>` | UI language instead of the Windows display language |
| `HLAE_SETUP_L10N_DIR=<dir>` | At build time: translations from `<dir>/<lang>/installer2/messages.po` instead of the l10n repo |

Unattended use: `HLAE_Setup.exe --help`. Exit codes are MSI-compatible (0, 1223 UAC declined,
1602 cancelled, 1603 error, 1605 not installed, 1639 invalid command line).

## Key points

- One exe, no prerequisites except the WebView2 runtime (wizard only; `--quiet` works without).
- The wizard runs as the normal user; only an elevated worker (UAC) changes anything, and it
  installs only its own embedded payload.
- Install state: `%ProgramData%\advancedfx\HLAE\state.json`, admin-only, the single source of truth.
- Every installed file is tracked by SHA-256; we never delete what we didn't install unchanged.
- Version: only `hlae/CMakeLists.txt`; no downgrades (uninstall first).
- License notices of all crates are embedded: `HLAE_Setup.exe --licenses <file>`, "Licenses" in the wizard.
