# installer2 – working notes

Shared notes (human + Claude) for replacing the WiX Toolset v5 based `HLAE_Setup.exe`
(sources in [`../installer`](../installer)) with a custom installer (Tauri 2 / Rust).

How to use this file:
- Keep it rough. Bullet points over prose.
- Sections 1–3: facts about the *old* installer. Sections 4–7: requirements and design of the *new* one (all decided).
  Section 8: plan / progress. Section 9: reference data.
- Tick `[x]` in 4 when a requirement is implemented, and in 8 when a milestone step is done.
- New questions go to 7 as `- **Open:** …`; once settled, turn them into a log entry and update 4–6.

---

## 1. Old installer – architecture overview

`HLAE_Setup.exe` is a WiX v5 **Burn bundle** (x86) that chains several packages. Built by
[`build_installer.bat`](../installer/build_installer.bat), invoked from
[`cmake/MultiBuild.cmake`](../cmake/MultiBuild.cmake) (`AFX_MULTIBUILD_INSTALLER`, default ON).

| Project | Type | What it does |
|---|---|---|
| [`setup/Bundle.wxs`](../installer/setup/Bundle.wxs) | Burn bundle (WixStdBA, custom theme) | Detection, UI (install / modify / uninstall), chains the packages below |
| [`HlaeCore/Package.wxs`](../installer/HlaeCore/Package.wxs) | MSI "HLAE" | HLAE files, locales feature, start menu shortcut, registry, copies `ffmpeg.ini` |
| [`HlaeCoreExtension`](../installer/HlaeCoreExtension/CustomActions.cs) | C# DTF custom actions | `ValidateAppFolderIsLatin`, `ValidateTargetPath` (browse dlg) |
| [`HlaeFfmpeg/Package.wxs`](../installer/HlaeFfmpeg/Package.wxs) | MSI "HLAE FFMPEG" | Downloads/extracts FFMPEG, or points to custom ffmpeg.exe |
| [`HlaeFfmpegExtension`](../installer/HlaeFfmpegExtension/CustomActions.cs) | C# DTF custom actions | download + SHA-512 + unzip, migrate from WiX v3, write `ffmpeg.ini`, remove folder |
| [`UninstallHlaeWixV3`](../installer/UninstallHlaeWixV3/Program.cs) | .NET exe in chain | `msiexec /x` all HLAE products `< 2.181.0` (WiX v3 era) |
| [`DeleteHlaeAppData`](../installer/DeleteHlaeAppData/Program.cs) | .NET exe in chain | On bundle uninstall + opt-in: delete `%APPDATA%\HLAE` |
| [`shared/Dependency`](../installer/shared/Dependency/README.md) | wxl | Workaround: WiX Dependency ext only ships English strings |

Chain order: `NetFx472WebAsPrereq` → rollback boundary → `HlaeFfmpeg.msi` → `UninstallHlaeWixV3.exe`
→ `DeleteHlaeAppData.exe` → `HlaeCore.msi`.

Build staging (MultiBuild.cmake): `bin/` is copied to `installer/hlae`, then `ffmpeg/` dir removed,
`locales/` and `HLAE.exe` moved to `installer/hlae-opt` (so they can be separate feature / component).
Output: `<prefix>/HLAE_Setup.exe`.

## 2. Old installer – what it does (feature inventory)

### 2.1 Install location / per-machine
- Per-machine only (`WixUISupportPerUser=0`, `PerMachine`), requires elevation.
- Default dir: `[ProgramFilesFolder]HLAE` → x86 → `C:\Program Files (x86)\HLAE`.
- Bundle variable `InstallFolder` is `Persisted="yes"` + overridable from command line.
- Writes `HKLM\Software\advancedfx\HLAE` `APPLICATIONFOLDER = <install dir>`
  (x86 MSI → actually `HKLM\SOFTWARE\WOW6432Node\advancedfx\HLAE`). Nothing in this repo reads it (checked:
  hlae, AfxCppCli, AfxHookGoldSrc, AfxHookSource, AfxHookSource2, shared, injector). The hooks find the HLAE folder
  from their own DLL path (e.g. [`AfxHookSource/hlaeFolder.cpp`](../AfxHookSource/hlaeFolder.cpp)).
- **Path validation** (`ValidateAppFolderIsLatin`, before `CostFinalize`, and in browse dialog):
  regex `^\p{IsBasicLatin}*$` → every char must be U+0000..U+007F. Error 25001
  "The HLAE application folder path contains invalid characters: Only basic latin character set is allowed (7bit ASCII)."

### 2.2 HLAE core files
- Everything from build `bin/` except `ffmpeg/`, `locales/` (feature `Hlae`, cannot be deselected).
- `HLAE.exe` is its own component (KeyPath, checksum) because the shortcut hangs on it.
- `MajorUpgrade` with downgrade error "A newer version of HLAE is already installed." → **no downgrades**.
- Package version == HLAE version (e.g. `2.192.8`). Bumped by hand on release (see [`BUILDING.md`](../BUILDING.md) L131-133).

### 2.3 Optional: Translations (locales)
- Feature `HlaeLocales` "Translations", default on → `<install>\locales\**`
  (gettext `.mo`, created by [`l10n_update_to_source.bat`](../l10n_update_to_source.bat):
  `locales\<lang>\hlae\messages.mo` for de fi fr-FR hu it ja nl pl pt-BR pt-PT ru zh-CN).
- Only selectable through MSI "Custom" setup type / Change → realistically hidden from most users
  (bundle sets `EnableFeatureSelection="no"`, MSI internal UI only shown on full UI level).

### 2.4 Start menu shortcut
- `ProgramMenuFolder\HLAE\HLAE.lnk` (all users start menu), **advertised** shortcut, icon from `hlae/app.ico`,
  working dir = install dir, description "Half-Life Advanced Effects".
- Dummy `HKCU\Software\advancedfx\HLAE` `scut=1` as KeyPath for the folder component (MSI ICE hack).

### 2.5 Optional: FFMPEG (separate MSI "HLAE FFMPEG", own version, e.g. `1.1.67`)
- Installed to **separate folder** `C:\Program Files (x86)\HLAE FFMPEG\ffmpeg\` (+ `.keep` file).
  Note: bundle passes `APPLICATIONFOLDER` but this MSI uses `INSTALLFOLDER` → always default location.
- Bundle checkbox "FFMPEG can be used for encoding mirv_streams to videos." on Install and Modify page.
  Default checked if: fresh install, or HLAE FFMPEG MSI already installed, or WiX v3 ffmpeg component present.
  Unchecking on Modify uninstalls the FFMPEG MSI.
- Variants (dialog [`FfmpegDlg.wxs`](../installer/HlaeFfmpeg/FfmpegDlg.wxs), only shown in MSI maintenance UI):
  - `latest-win64-static` (gpl, default, "~100 MB, faster, recommended")
  - `latest-win64-shared` (gpl-shared, "~50 MB")
  - `latest-win64-static-lgpl`, `latest-win64-shared-lgpl` ("less features, more permissive")
  - `custom` → user picks existing `ffmpeg.exe` (file must exist)
  - "Reinstall FFMPEG" checkbox (re-download on repair/upgrade or keep existing)
- Download: BtbN GitHub autobuilds, URL + **SHA-512 of the zip** hard-coded per variant in Package.wxs.
  Uses system proxy + default credentials, TLS 1.2. Downloads to temp, verifies hash, extracts.
- Extraction: strips the zip's top-level dir, zip-slip check (`fullPath` must start with target dir).
- Progress UI phases: Connecting → Downloading `[n] of [m] Bytes` → Verifying checksum → Extracting `Item [n] of [m]`.
- Writes `HLAE FFMPEG\ffmpeg\ffmpeg.ini`:
  ```ini
  [Ffmpeg]
  Path=<...>\HLAE FFMPEG\ffmpeg\bin\ffmpeg.exe   ; or custom path
  ```
- State in `HKLM\Software\advancedfx\HLAE FFMPEG` (WOW6432Node): `reinstall` (DWORD), `version` (variant id), `custom` (path).
- Uninstall / reinstall: **deletes whole `ffmpeg` folder recursively** (`RemoveFolder`).
- HlaeCore MSI then copies that `ffmpeg.ini` into `<HLAE>\ffmpeg\ffmpeg.ini` (`NeverOverwrite`, removed on uninstall).

How HLAE consumes it ([`shared/AfxOutStreams.cpp`](../shared/AfxOutStreams.cpp) ~L209-231):
read `<HLAE>\ffmpeg\ffmpeg.ini` `[Ffmpeg] Path=` (≤ MAX_PATH), else fallback `<HLAE>\ffmpeg\bin\ffmpeg.exe`.
See also [`resources/ffmpeg/readme.advancedfx.txt`](../resources/ffmpeg/readme.advancedfx.txt) (manual zip users).

### 2.6 Delete HLAE app data on uninstall
- Bundle Modify page checkbox "Check to remove HLAE %APPDATA% folder upon uinstall (not recommended)." (default off).
- Deletes `%APPDATA%\HLAE` recursively (contains `hlaeconfig.xml`, panorama zips, … see [`hlae/Program.cs`](../hlae/Program.cs) ~L361).
- Runs **elevated** → deletes the elevated account's APPDATA, which may not be the logged-in user's (bug, see 3).

### 2.7 Upgrades / legacy handling
- WiX v3 era HLAE (`< 2.181.0`, same UpgradeCode as current HlaeCore): bundle sets `HlaeRemoveWixV3=1`
  → `UninstallHlaeWixV3.exe` runs `msiexec /x {product} /passive /qn` for each. HlaeCore has LaunchCondition blocking if v3 present.
- WiX v3 FFMPEG: registry `HKLM\SOFTWARE\advancedfx\HLAE\FFMPEG` (+ component `{2D1C78F6-...}`) →
  `DetectOldFfmpeg` / `MigrateFfmpegInstall` move old ffmpeg folder into new location and delete the old reg key.
- Bundle version == HLAE version, own UpgradeCode → Burn handles bundle major upgrades.

### 2.8 General installer behaviour (comes "for free" with MSI/Burn, must be re-built)
- ARP ("Apps & features") entries: **three** – bundle "HLAE Installer", MSI "HLAE", MSI "HLAE FFMPEG" (`Visible="yes"`), with HLAE icon.
- Modify / Repair / Uninstall via re-running setup or ARP.
- Transactional install with rollback (rollback boundary after .NET prereq).
- Files-in-use detection (Restart Manager dialog) – relevant: HLAE.exe running, hook DLLs loaded in game processes.
- Logging to `%TEMP%` (Burn + MSI logs), `/log` switch.
- Command line: `/install /repair /uninstall /layout /passive /quiet /norestart /log <file>`,
  overridable vars `InstallFolder`, `LangId`, `HlaeFfmpeg`, `HlaeRemoveAppData`, `HlaeRemoveWixV3`.
- Prerequisite: .NET Framework 4.7.2 (web bootstrapper). Note: `hlae.csproj` targets `net462`, x86.
- UI localisation: 14 languages (en-US de-DE fi-FI fr-FR hu-HU it-IT ja-JP nl-NL pl-PL pt-BR pt-PT ru-RU tr-TR zh-CN),
  picked from `UserUILanguageID`. Strings come from the l10n `.po` pipeline
  (`deps/release/l10n/locales/<lang>/installer/<project>/messages.po` → `.wxl` via `wxl-po-tools`, see `l10n_update_to_source.bat`).
- Update button (`WixStdBAUpdateAvailable`) exists in the theme but no update feed is configured → never shown.
- Disclaimer strings exist ("This software is for your enjoyment and you are using it at your own risk.") but no license page is shown (`LicenseUrl=""`).

## 3. Known quirks / bugs in the old installer (don't replicate)
- `FinalizeFfmpegInstall` / `MigrateFfmpegInstall` compare `FFMPEG_CUSTOM == "custom"` (path vs. literal) → custom path handling is broken.
- FFMPEG MSI ignores the chosen install folder (`APPLICATIONFOLDER` vs `INSTALLFOLDER`).
- `ffmpeg.ini` written with `WritePrivateProfileString` → ANSI → non-ASCII custom paths get mangled.
- Changelog 2.180.0: upgrading + reinstalling FFMPEG installs new then old FFMPEG again (needed repair).
- Changelog: "HLAE Setup upgrade uninstalls HLAE due to wrong HlaeCore version number" → version handling is fragile.
- AppData deletion runs elevated → wrong user profile when admin ≠ logged-in user.
- FFMPEG variant dialog effectively unreachable on first install (only MSI maintenance UI).
- Prereq .NET 4.7.2 vs app targets 4.6.2.
- `tr-TR` missing from the bundle's `-culture` list in `build_installer.bat`.
- Three ARP entries are confusing; MSI state can get "borked" (see [`doc/how_to_clean_up_borked_HLAE_test_install.txt`](../doc/how_to_clean_up_borked_HLAE_test_install.txt)).
- Three version numbers to bump by hand per release (HlaeCore, HlaeFfmpeg, Bundle).

## 4. Requirements

### 4.1 Features & behaviour
- [ ] Per-machine install, default location `C:\Program Files (x86)\HLAE` (same as before).
- [ ] Validate the HLAE target folder like `ValidateAppFolderIsLatin` (basic Latin / 7-bit ASCII only).
- [ ] Optional features: FFMPEG, locale files (Translations).
- [ ] Every installed file tracked with a SHA-256 checksum (manifest).
- [ ] Uninstall removes a file only if path **and** checksum match the manifest (user-modified/added files stay); remove empty dirs.
- [ ] Upgrade / repair **overwrite** tracked files, even if the user modified them.
- [ ] On upgrade, remove files that were in the old manifest but not in the new one (only if checksum unchanged).
- [ ] Versioning of HLAE and FFMPEG → detect upgrade and from which version.
- [ ] No downgrade: refuse with a message, users must uninstall first.
- [ ] Keep install location on upgrade (from our state, else old `APPLICATIONFOLDER` reg value, else default).
- [ ] Modify (toggle FFMPEG / locales / change FFMPEG variant), Repair (re-verify checksums, restore missing/changed files), Uninstall.
- [ ] Single ARP entry (`HKLM\...\Uninstall\<key>`): DisplayName, DisplayVersion, Publisher `advancedfx.org`, DisplayIcon,
      InstallLocation, UninstallString / QuietUninstallString, EstimatedSize, ModifyPath. Uninstaller = cached installer copy (6.5).
- [ ] Write `APPLICATIONFOLDER` to `HKLM\SOFTWARE\WOW6432Node\advancedfx\HLAE` for third-party tools (same place as before), remove on uninstall.
- [ ] Start menu shortcut `%ProgramData%\Microsoft\Windows\Start Menu\Programs\HLAE.lnk` (no sub-folder, plain non-advertised),
      created/updated on install, removed on uninstall.
- [ ] Option to delete HLAE app data (`%APPDATA%\HLAE`) on uninstall – for the **logged-in** user (done by the unelevated GUI, 6.4).
- [ ] "Launch HLAE" on finish (unelevated).
- [x] Files-in-use check before touching files (HLAE.exe, AfxHook*.dll loaded in game processes) → ask to close / retry
      (M7: Restart Manager; wizard Retry / Back, quiet mode 1603, no automatic closing).
- [x] Rollback on failure: stage new files, swap, restore on error (never leave a half-installed state)
      (M1; crash-safe journal M7, not power-loss-safe by decision, 6.10 d).
- [ ] Silent / unattended mode via command line (install dir, features, FFMPEG variant, remove app data, log file), exit codes (6.4).
- [ ] Log file.
- [ ] Localised UI in the 14 languages via the `.po` l10n pipeline (6.9).
- [ ] .NET Framework check + message only (no prereq install): minimum **4.6.2**
      (`Release >= 394802` under `HKLM\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full`).
      Nothing shipped needs more: `hlae.csproj` `net462`, `AfxCppCli.vcxproj` `v4.6.2`,
      `injector.exe` (prebuilt, advancedfx/injector 1.0.5) `v4.0 Client`. HLAE forces TLS 1.2 itself (`hlae/Program.cs` ~L351).
      If a target is ever raised, raise this check with it.
- VC++ runtime is shipped app-local (`CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS`, x86 + `x64\`) → no VC++ redist prereq.
- [ ] Single source of truth for versions (HLAE version from build, FFMPEG version from its data file).
- [ ] Build integration: replace the `build_installer.bat` call in `cmake/MultiBuild.cmake`, output stays `HLAE_Setup.exe`.

### 4.2 FFMPEG
- [ ] Variants: static/shared × gpl/lgpl, "use existing ffmpeg.exe" (writes `ffmpeg.ini`, 6.8), "keep current", "none".
- [ ] Per variant a data file: download URL, archive SHA-256, version, list of resulting files with SHA-256.
- [ ] Download: system proxy, HTTPS, progress (bytes), archive hash verify, strip root dir, zip-slip protection, temp cleanup, cancel.
- [ ] Remember the FFMPEG choice (variant / custom path / none) across upgrades.
- [ ] Default choice:
  - custom FFMPEG installed → **keep current**.
  - downloaded FFMPEG installed, same version as offered → **keep current**.
  - downloaded FFMPEG installed, different version → **upgrade** (preselect the previously installed variant).
  - fresh install (no HLAE installed) → **install** (static gpl, like before).
  - HLAE installed, FFMPEG deliberately not installed → **don't install** (like the old installer).
  - migration from old WiX v5 installer with FFMPEG → counts as "different version" → **upgrade**,
    preselect the old variant / custom path (5).

### 4.3 Non-goals (for now)
- Code signing (would be a `signtool sign` step at the end of the build).
- Installer self-update / "newer installer available" – HLAE.exe has its own update check (`hlae/MainForm.cs`, `UpdateCheck`).
- Migration from WiX v3 installs (< 2.181.0) – only detected, see 5.
- Installing .NET Framework or WebView2 (WebView2 missing → we open Microsoft's download page, 6.2).
- Downgrades.

## 5. Migration from old installers
- **WiX v5 (2.181.0 … current 2.192.x)**:
  - Detect bundle via `Uninstall` keys (WOW6432Node) where `BundleUpgradeCode` = `{35C8D990-6C4F-4639-ADF7-D259B06E070B}`
    → run its `QuietUninstallString` (cached exe in `C:\ProgramData\Package Cache\{bundle id}\`). Removes both MSIs, keeps AppData (`HlaeRemoveAppData=0`).
  - Fallback if bundle missing / broken: `MsiEnumRelatedProducts` for HlaeCore `{33FEF63B-...}` and HlaeFfmpeg `{9090AE9A-...}` → `msiexec /x`.
  - Before uninstalling read: install dir (`APPLICATIONFOLDER`), HlaeCore version, FFMPEG installed?, FFMPEG `version`/`custom` reg values → preselect in new UI.
  - FFMPEG MSI uninstall deletes the FFMPEG folder → download it again (preselect the old variant / custom path).
- **WiX v3 (< 2.181.0)**: no migration. Detect it (HlaeCore UpgradeCode product with version < 2.181.0)
  and refuse with a message to uninstall the old HLAE manually first (like the old LaunchCondition).
- Clean leftovers: `HKCU\Software\advancedfx\HLAE\scut`, `HKLM\Software\advancedfx\HLAE FFMPEG`, empty `Program Files (x86)\HLAE FFMPEG`,
  old start menu folder `Programs\HLAE\` if still there.

## 6. Design

### 6.0 Overview
- GUI: **Tauri 2** (WebView2, Rust backend), frontend **vanilla** HTML/CSS/JS (no framework).
  Why: polished wizard UI with little effort; system fonts → CJK (ja-JP, zh-CN) work without bundling fonts.
- One self-contained `HLAE_Setup.exe` (6.1) that is installer, uninstaller (cached copy, 6.5), CLI (6.3) and elevated worker (6.4).

### 6.1 Single self-contained exe
- A Tauri app already builds to one exe (frontend assets compiled in).
  No NSIS / 7-Zip SFX (Tauri's NSIS would install the *installer app* and register *its own* ARP entry;
  wrappers add a temp extraction step + more AV false positives).
- Payload = HLAE files as one compressed archive (`payload.tar.zst`, zstd high level + long mode)
  + `manifest.json` (path, size, SHA-256, feature: `core` / `locales`).
- Embedded with `include_bytes!`: `build.rs` packs the staged dir from an env var (e.g. `HLAE_PAYLOAD_DIR`).
  - Memory: same as an RT_RCDATA resource – the exe image is memory-mapped, pages are only read when touched and are
    file-backed (can be dropped again). Never copy the payload into a `Vec`, always stream from the `&[u8]`.
  - Difference is build time only: payload changes recompile/relink, rustc/LLVM hold the data in memory
    → fine at ~10 MB, would hurt at hundreds of MB.
  - Keep payload access behind one function (`fn payload() -> &'static [u8]`) in its own module, so switching to
    a post-build-injected resource (`FindResource`/`LoadResource`) later only touches that function + the build step.
- FFMPEG is *not* embedded: separate data files per variant in `data/` (4.2).
- Build flow (replaces the `build_installer.bat` call in `cmake/MultiBuild.cmake`):
  `stage bin/ → cargo build --profile release-installer -p hlae-installer (build.rs packs payload) → HLAE_Setup.exe`
  No `tauri-cli` needed (requires `tauri` feature `custom-protocol`, see 8 / M0). Crates are part of the root Cargo workspace.

### 6.2 WebView2 dependency
- In `main()`, **before** building the Tauri app: check WebView2 Evergreen runtime
  (`HKLM\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}` `pv`, also HKCU).
  If missing: native `MessageBox` → **decided (2026-10-17, revised):** offer to open Microsoft's WebView2 download page
  (`https://developer.microsoft.com/microsoft-edge/webview2/`) in the browser, the user installs the runtime and starts
  HLAE Setup again; the message also points to `--quiet`. No automatic download / run of the bootstrapper: that would
  break when Microsoft changes the bootstrapper or its link, and would need a signature check before running it.
- Set the WebView data directory to a temp folder and delete it on exit (otherwise leaves `EBWebView` in `%LOCALAPPDATA%`).
  Implemented (M6): `%TEMP%\hlae-setup-webview-<pid>`. The WebView2 processes only end when HLAE_Setup.exe ends, so the
  wizard can't delete it itself. At **start** it launches a hidden helper (`HLAE_Setup.exe --cleanup-webview-data <dir>
  <pid>`, internal) that waits for the wizard process to end (also a crash), then removes the dir, retrying for up to
  90 s; only accepts `%TEMP%\hlae-setup-webview-*`. Started at the beginning on purpose: an uninstall from the cached
  setup renames the running exe (6.5), so starting "our exe" at the end failed (found in the M6 GUI test).
  Measured: dir gone ~0.5 s after the wizard closed / was killed.

### 6.3 Command line / silent mode
- Parse args first; `--quiet` / `--uninstall --quiet` etc. never create a webview → silent mode works without WebView2.
- Exe is GUI subsystem → `AttachConsole(ATTACH_PARENT_PROCESS)` for console output.

### 6.4 Elevation
- GUI runs `asInvoker`; actual install work in an elevated worker = same exe relaunched via
  `ShellExecuteEx` verb `runas` (e.g. `--worker <pipe-name>`), talking over a named pipe.
  Unelevated GUI deletes `%APPDATA%\HLAE` and launches HLAE → correct user (fixes old bug, see 3).
  (Rejected: `requireAdministrator` manifest – would delete/launch as the wrong user.)
- Already elevated (admin shell, deployment tools, winget elevated) → no worker, do the work in-process.
- Not elevated + silent → self-elevate via `runas` → UAC prompt (standard behaviour).
- Running as SYSTEM (Intune / SCCM, session 0): no UAC, no UI, no interactive user →
  ignore/refuse "remove app data", never launch HLAE.
- Security of the elevated worker (same bug class as the 2024 WiX security release, see changelog):
  - Random pipe name, pipe ACL only for the launching user, verify client PID == parent
    (`GetNamedPipeClientProcessId`), accept exactly one client.
  - Worker re-validates all input itself (install path rules, only manifest-driven file ops), never trusts the GUI.
  - FFMPEG download + extraction in an admin-only temp folder (not the user's `%TEMP%`);
    verify hash and extract from the same open handle → no file swap after verification.
  - `SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)` first thing in `main()` → no DLL planting
    from the folder the exe was started from (e.g. Downloads).
  - Worker never creates a WebView.
- Exit codes (MSI-compatible, deployment tools understand them):
  0 ok, 3010 reboot required, 1602 user cancel, 1603 fatal error, 1223 UAC declined (`ERROR_CANCELLED`).

### 6.5 Uninstaller / cache
- Copy full `HLAE_Setup.exe` to `%ProgramData%\advancedfx\HLAE\setup\HLAE_Setup.exe` (like Burn's Package Cache)
  → offline repair / modify; ARP `UninstallString` / `ModifyPath` point there (`--uninstall`, `--modify`).
- Self-delete on uninstall (implemented in M3, [`core/src/integration.rs`](core/src/integration.rs)): if the running exe *is*
  the cached copy (uninstall from Apps & features), rename it to a unique `HLAE_Setup.<pid>.<time>.delete` (renaming a
  running exe is allowed) and schedule it plus the then empty `setup`, `HLAE`, `advancedfx` dirs for deletion at the next
  reboot (`MoveFileExW` + `MOVEFILE_DELAY_UNTIL_REBOOT`; dirs are only deleted if empty, files always → unique name, so a
  reinstall before the reboot isn't affected). Otherwise the cache is deleted directly.
  Rejected: "copy self to `%TEMP%` and relaunch" – an elevated process starting an exe from a user-writable dir is a
  privilege escalation hole (the user can swap the exe in between).

### 6.6 Code layout
```
installer2/
  core/           Rust lib, runs on the user's machine: manifest, hashing, payload extraction, file
                  transactions/rollback, install state, ARP registration, shortcut, legacy WiX
                  detection/uninstall, FFMPEG download + install
  build-support/  Rust lib, build time only (app/build.rs, core's tests): payload packing, translation
                  catalogs (.pot / .po via `polib`) – nothing of it ends up in HLAE_Setup.exe
  app/            Tauri app: thin GUI + CLI/silent mode + elevated worker, all calling core
  tools/          Developer tools, not shipped (generates data/ffmpeg.json)
  data/           FFMPEG variant data files
  l10n/           messages.pot (English source of the GUI texts, moves to the l10n repo later, 6.9)
```
Keep all install logic in `core` (testable without GUI, usable by silent mode and the worker).

### 6.7 Install state
- Single source of truth: `%ProgramData%\advancedfx\HLAE\state.json` – installed HLAE version, install dir,
  features, FFMPEG choice (variant/version/custom path/none), and all file manifests (path + SHA-256, per feature).
  Cached installer next to it: `%ProgramData%\advancedfx\HLAE\setup\HLAE_Setup.exe` (6.5).
- Written atomically (write temp file + rename), so a crash never leaves a half-written state.
- Folder must be admin-only: the worker creates it with an explicit ACL (Administrators + SYSTEM, no inheritance);
  if it already exists with a non-admin owner or a wrong ACL (pre-created / squatted) → reset or refuse.
  (Default `ProgramData` ACL lets normal users create subfolders they then own.)
- Registry is **output only** (written, never read back as our state):
  ARP Uninstall key, `APPLICATIONFOLDER`. Only exception: reading the old WiX installer's keys for detection/migration (5).
- Worker only acts on paths inside the install dir: reject `..`, absolute paths, and don't follow reparse points (junctions/symlinks).
- **Install dir protection** (decided 2026-10-17, M7, [`core/src/protect.rs`](core/src/protect.rs)). The worker
  writes / renames / deletes there as admin, by path. In a folder normal users can change (custom location, e.g.
  `C:\Games\HLAE` – Authenticated Users get "Modify" below `C:\`), a user could swap a folder for a junction between
  our checks and the rename (e.g. `x64\` → `C:\Windows\System32`) → our elevated rename / delete lands there =
  local privilege escalation. Checking for reparse points alone can't prevent that race. Hence:
  - **Protected**: outside `Program Files` / `Program Files (x86)` the install dir gets owner Administrators +
    protected `SYSTEM:F, Administrators:F, Users:RX` (like the state dir), everything below reset to owner
    Administrators + inherited entries only (set per handle with `SetKernelObjectSecurity`, top-down, no
    propagation by path; junctions and hard-linked files are skipped). Under `Program Files` Windows' inherited
    permissions are left alone (already admin-only).
  - **Pinned**: for the whole install / uninstall a handle to the install dir is held open without
    `FILE_SHARE_DELETE` (with `FILE_LIST_DIRECTORY` – share modes are only enforced against handles with data
    access): the dir and every dir above it can't be renamed / deleted meanwhile (a user with full control over a
    parent could otherwise swap the whole dir). The path must lead to the dir directly: the deepest existing part
    is checked before anything is created, then the dir itself (`GetFinalPathNameByHandleW` = the given path, no
    junction / symbolic link / short name; network folders refused).
  - New install into an existing folder: outside `Program Files` it must be empty (taken over), else refused
    (`InstallDirNotEmpty`, also shown in the wizard while typing); the old setup's folder (migration) is taken
    over with its leftovers. Our own install (state) is re-protected on every install / repair / upgrade / modify.
  - Consequence: in a custom folder normal users can't change HLAE's files without admin rights (accepted: users
    grant themselves admin when needed; HLAE's settings are in `%APPDATA%`).
  - Not done on purpose: deleting leftovers at reboot (`MoveFileExW`) in the install dir – between the setup's end
    and the reboot nothing pins it. Leftovers are removed by the next setup run instead.

### 6.8 FFMPEG location
- Downloaded variants are unpacked into `<HLAE>\ffmpeg\` (archive root dir stripped) → `<HLAE>\ffmpeg\bin\ffmpeg.exe`,
  which HLAE finds via its fallback (2.5) → **no `ffmpeg.ini`**, no separate `HLAE FFMPEG` folder anymore.
- Custom `ffmpeg.exe` → write only `<HLAE>\ffmpeg\ffmpeg.ini` (`[Ffmpeg] Path=<custom>`), tracked in the state
  with the SHA-256 of the written content like any other file.
  - Write it as **UTF-16LE with BOM**, otherwise `GetPrivateProfileStringW` mangles non-ASCII paths.
  - Custom path must be ≤ MAX_PATH (HLAE ignores longer values) and the file must exist.
- Switching:
  - downloaded → custom: write ini, remove downloaded files per manifest (checksum rule).
  - custom → downloaded: **must** remove our ini, otherwise it overrides the downloaded FFMPEG.
- Untracked `ffmpeg.ini` (e.g. hand-written per [`readme.advancedfx.txt`](../resources/ffmpeg/readme.advancedfx.txt))
  is never deleted (checksum rule), but would silently override a downloaded FFMPEG → detect and warn / ask.

### 6.8a FFMPEG download (decided)
- **WinHTTP**, synchronous, `WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY` (Windows 8.1+): system + per-user proxy settings
  incl. PAC / WPAD, failover, proxy auth with Windows credentials (autologon) – same coverage as the old installer's
  `GetSystemWebProxy()` + `DefaultNetworkCredentials`. TLS via Schannel + Windows certificate store
  (company root CAs work). Rust HTTP crates (`ureq`, `reqwest`) can't do PAC / WPAD / NTLM proxy auth.
  - Fallback if `AUTOMATIC_PROXY` isn't supported (e.g. Wine): `WinHttpGetIEProxyConfigForCurrentUser` + `WinHttpGetProxyForUrl`.
  - TLS: `WINHTTP_OPTION_SECURE_PROTOCOLS` = TLS 1.2 + 1.3 (retry TLS 1.2 only if 1.3 is unknown, older Windows 10).
    Not `WINHTTP_FLAG_SECURE_DEFAULTS` (forces async mode, no security gain here). Integrity comes from the SHA-256 anyway.
  - Cancel: check a flag between `WinHttpReadData` chunks.
- Download runs in the **unelevated** client (the logged-in user's proxy settings + credentials) into `%TEMP%`;
  the worker copies it into its admin-only temp dir, **then** verifies SHA-256 and extracts from that copy
  (tampering with the `%TEMP%` file can only make the check fail). As SYSTEM (Intune / SCCM) the worker downloads itself.
- Manual fallback: `--ffmpeg-archive <zip>` = use an already downloaded archive (same verification path, no download).
  GUI file picker: decide in M6. Otherwise users can still use the HLAE zip + install FFMPEG by hand.
- Wine / Proton: nice to have; for now the **silent mode** is the target there (WebView2 under Wine is unreliable).

### 6.9 Translations (gettext `.po`)
- New `installer2` project in the l10n repo ([`deps/release/l10n`](../deps/release/l10n), github.com/advancedfx/l10n, translated via Pontoon).
- Source of truth: hand-maintained `locales/en-US/installer2/messages.pot`;
  translations `locales/<lang>/installer2/messages.po` via the usual `update_from_en-US.py` + Pontoon workflow.
  **For now** (while the GUI texts still change a lot) the `.pot` lives in `installer2/l10n/messages.pot` and moves
  to the l10n repo once the GUI is stable (decided 2026-10-17); `build.rs` already reads translations from
  `deps/release/l10n/locales/<lang>/installer2/messages.po` if present.
- Testing a translation without touching the l10n repo: `HLAE_SETUP_L10N_DIR=<dir>` at build time (same layout
  `<dir>/<lang>/installer2/messages.po`), `HLAE_SETUP_LANG=<lang>` at runtime.
- Parsing: `polib` 0.3 (pure Rust, MIT, deps `linereader` + `concat-string`) in `build-support` (build time only).
  Translations whose `{placeholders}` differ from the English text are skipped (→ English) with a build warning.
- Keys: `msgctxt` = stable ID (like the old installer, e.g. `msgctxt "InstallButton"`), `msgid` = English text.
- Build time (`build.rs`): parse all `.po` (`polib`), keep `msgctxt → msgstr` (skip empty/fuzzy),
  en-US from the `.pot` (`msgctxt → msgid`); write `$OUT_DIR/l10n/<lang>.json` + generated
  `l10n_index.rs` with `include_str!` per language → all catalogs embedded in the exe, no gettext at runtime.
- Runtime Rust (`l10n` module): `init(ui_lang)` first thing in `main()` – language from OS UI language (`sys-locale`),
  match exact (`pt-BR`, `zh-CN`) → language only (`de`) → `en-US`; merged over en-US so missing keys show English.
  `t(id)` used for CLI output and the native WebView2 message box (works without webview).
- Frontend (vanilla JS): Tauri command `l10n_catalog` returns the merged map (`withGlobalTauri: true`);
  `t(id, args)` + `data-l10n="<id>"` attributes on static HTML.
- Conventions:
  - Named placeholders `{name}`, replaced identically in Rust and JS (convert old `[1]` / `[AFX_FFMPEGURL]`).
  - Avoid plurals (phrase like "{done} / {total} MB"); real plural forms would need the `.po` plural formula on both sides.
  - Strip `&` accelerators when reusing old WiX strings.
- Reuse existing translations of the old installer (FFMPEG texts, Latin-path error, remove app data, …)
  via `msgmerge` with the old `installer/*` `.po` files as compendium.

### 6.10 File formats
Common rules for all three:
- JSON, UTF-8. Top-level `"format": <int>`; a reader refuses a higher `format` than it knows
  (e.g. state written by a newer installer → "newer installer was used, please use that one").
- Paths: relative to their root (install dir / archive root), `/` as separator, matched **case-insensitively**
  (Windows). Validated on read *and* write: no `..`, no absolute / drive / UNC paths, no `:` (alternate data streams),
  no reserved device names (`CON`, `NUL`, …), no trailing dot / space.
- Hashes: lower-case hex SHA-256. Sizes in bytes.

#### a) Payload manifest – `manifest.json`, generated by `build.rs`, embedded next to `payload.tar.zst`
```json
{
  "format": 1,
  "hlae_version": "2.192.8",
  "files": [
    { "path": "HLAE.exe",                 "size": 1234567, "sha256": "…", "feature": "core" },
    { "path": "x64/AfxHookSource2.dll",   "size": 5555555, "sha256": "…", "feature": "core" },
    { "path": "locales/de/hlae/messages.mo", "size": 23456, "sha256": "…", "feature": "locales" }
  ]
}
```
- Features: `core` (always), `locales` (everything under `locales/`). Assignment by path prefix in `build.rs`.
- `hlae_version` from env var `HLAE_VERSION`, set by CMake from `hlae_VERSION_*` in [`hlae/CMakeLists.txt`](../hlae/CMakeLists.txt)
  (= single source of truth, also what HLAE.exe gets). Build fails if not set.
- The staged dir is the build `bin/` minus `ffmpeg/` (like `MultiBuild.cmake` does today).
- Extraction verifies every file against size + SHA-256 before it's moved into place.

#### b) FFMPEG data file – `installer2/data/ffmpeg.json`, embedded at build time
```json
{
  "format": 1,
  "version": "n9.0.2-17-g2a571b6068 (autobuild-2026-09-30-13-08)",
  "default_variant": "win64-gpl",
  "variants": [
    {
      "id": "win64-gpl",
      "linking": "static",
      "license": "gpl",
      "url": "https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-09-30-13-08/ffmpeg-n9.0.2-17-g2a571b6068-win64-gpl-9.0.zip",
      "archive_size": 123456789,
      "archive_sha256": "…",
      "strip_prefix": "ffmpeg-n9.0.2-17-g2a571b6068-win64-gpl-9.0/",
      "files": [
        { "path": "bin/ffmpeg.exe", "size": 123, "sha256": "…" },
        { "path": "LICENSE.txt",    "size": 456, "sha256": "…" }
      ]
    }
  ]
}
```
- One `version` for the whole set → "same version as offered" = same `version` string (and same variant); no ordering needed.
- Variant ids: `win64-gpl`, `win64-gpl-shared`, `win64-lgpl`, `win64-lgpl-shared`.
  Migration mapping from the old registry `version` value (5): `latest-win64-static` → `win64-gpl`,
  `latest-win64-shared` → `win64-gpl-shared`, `latest-win64-static-lgpl` → `win64-lgpl`,
  `latest-win64-shared-lgpl` → `win64-lgpl-shared`, `custom` → custom path.
- `files` = the **whole archive** (like before: `bin/`, `doc/`, for shared also `include/`, `lib/`) as it ends up in
  `<HLAE>\ffmpeg\` after stripping `strip_prefix`; extraction rejects any archive entry not in the list and any hash mismatch.
- Generated, not hand-written, by the dev-only crate `installer2/tools`
  (`cargo run -p hlae-installer-tools -- ffmpeg …`): downloads each URL, hashes it, lists the archive and writes the file.
  (BtbN also publishes a `checksums.sha256` per release – the tool can cross-check against it.)
  Run **by hand** when updating FFMPEG (like editing `HlaeFfmpeg/Package.wxs` today); `ffmpeg.json` is committed.
  Normal builds only embed the committed file and never download anything.

#### c) Install state – `%ProgramData%\advancedfx\HLAE\state.json`, written by the installer (6.7)
```json
{
  "format": 1,
  "written_by": "HLAE_Setup 2.192.9",
  "install_dir": "C:\\Program Files (x86)\\HLAE",
  "hlae_version": "2.192.9",
  "features": { "locales": true },
  "ffmpeg": {
    "choice": "download",
    "variant": "win64-gpl",
    "version": "n9.0.2-17-g2a571b6068 (autobuild-2026-09-30-13-08)",
    "custom_path": null
  },
  "files": [
    { "path": "HLAE.exe",            "sha256": "…", "component": "core" },
    { "path": "ffmpeg/bin/ffmpeg.exe", "sha256": "…", "component": "ffmpeg" }
  ],
  "integration": {
    "arp_key": "HLAE",
    "shortcut": "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\HLAE.lnk",
    "app_folder_reg": true
  },
  "history": [
    { "action": "migrate", "time": "2026-11-01T12:00:00Z", "detail": "WiX v5 2.192.8" },
    { "action": "install", "time": "2026-11-01T12:00:05Z", "detail": "2.192.9" }
  ]
}
```
- `ffmpeg.choice`: `none` | `download` | `custom`. `none` is remembered so upgrades don't install FFMPEG (4.2).
- `components`: `core`, `locales`, `ffmpeg`, `ffmpeg-ini` (the generated ini for custom, hash of the written content).
- `files` = exactly what we wrote, so uninstall / upgrade apply the checksum rules (4.1) against it.
  Directories are not stored: after removing files, parent dirs are removed bottom-up if empty.
- `integration` = what we registered, so uninstall removes exactly that (and nothing we didn't create).
- `history`: short log for support / debugging (kept, capped, e.g. last 20 entries).
- Rollback journal for an operation in progress is a **separate** file (M7, see d), not part of the state.

#### d) Transaction / work dir (implemented in M1, journal in M7, [`core/src/transaction.rs`](core/src/transaction.rs))
- Work dir `<install dir>\.hlae-setup-tmp\<id>\` (reserved name, never in a manifest; `<id>` = `<pid>-<time>-<n>`,
  new per transaction): `staging\` = verified new files, `backup\` = replaced / removed old files. Same volume as
  the install dir → renames only, cheap and atomic. (`%ProgramData%` isn't on the same volume for e.g. `D:\…`.)
- Order: stage + verify everything → move old aside + move new into place → caller saves state → `commit()` deletes
  journal + backups. Any error, or dropping the transaction without `commit()`, restores the previous files and
  removes created dirs.
- Identical files (same SHA-256) are left untouched on upgrade / repair.
- Old files are never written in place, only renamed → works for in-use DLLs (rename is allowed) and never writes through hard links.
- **Crash-safe journal** (decided 2026-10-17): `journal.jsonl` next to `state.json` (admin-only state dir, so it
  can't be tampered with, and recovery always knows where to look). One JSON line is appended *before* each step
  (`begin` with the state file's SHA-256, `created-dir`, `backup`, `placed` with the file's SHA-256).
  - **No flush** to disk (decided): safe against the setup crashing / being killed. After a power loss / bluescreen
    the journal may miss steps → mixed install, `state.json` still consistent; running the setup again (repair /
    update) fixes it. Accepted.
  - Commit point = the state save: [`recover`] compares the state file's hash with the one in `begin` – unchanged →
    roll back from the journal, changed → the new state was saved, only remove the backups. Runs at the start of
    every install / uninstall and every `Transaction::begin`. Idempotent: steps that never happened are skipped
    (`placed` only removed if its hash still matches, `backup` only restored if the backup exists). A cut-off last
    line is ignored. If restoring hits an I/O error, the journal is kept and the operation fails (retry later).
  - Work dir leftovers that can't be removed now (e.g. a replaced DLL still loaded) stay until the next setup run
    removes them – no deletion at reboot (see 6.7 install dir protection).
- The former known limitation (a normal user tampering with a user-writable install dir during the operation) is
  solved by the install dir protection (6.7); the old note "mitigated by the reparse point checks" was wrong:
  checks before a rename can't prevent a junction swapped in after them.

## 7. Decision log
All questions so far are decided; details live in 4–6.
- Packaging: Tauri 2 GUI, single self-contained exe, no NSIS / 7-Zip SFX (6.1).
- Payload embedding: `include_bytes!` (rejected: RT_RCDATA resource injected post-build, appended overlay) (6.1).
- WebView2 missing: open Microsoft's download page, no automatic download / run (6.2; revised 2026-10-17,
  was "download bootstrapper on demand").
- Frontend: vanilla, no framework.
- Elevation: elevated worker process (6.4).
- Silent mode exit codes: MSI-compatible (6.4).
- Uninstaller: cached copy of the installer in `%ProgramData%` (6.5).
- Install state: `%ProgramData%\advancedfx\HLAE\state.json`, registry output only
  (rejected: inside install dir, registry for manifests, registry + ProgramData hybrid) (6.7).
- Checksums: SHA-256 everywhere (tracked files and FFMPEG archives).
- Upgrade over user-modified tracked files: overwrite.
- FFMPEG location: `<HLAE>\ffmpeg\`, ini only for custom FFMPEG (6.8).
- FFMPEG default choice rules (4.2).
- Old FFMPEG on migration: download again (5).
- WiX v3 migration: dropped, detect + message (5).
- Start menu: `HLAE.lnk` directly in Programs.
- `APPLICATIONFOLDER`: keep writing, WOW6432Node.
- .NET: check + message only, minimum 4.6.2, no target changes.
- Translations: new `installer2` l10n project, stay with gettext `.po` (6.9).
- No downgrades, no self-update, no code signing for now (4.3).
- File formats: payload manifest, `ffmpeg.json`, `state.json` as in 6.10. FFMPEG: install the whole archive.
  HLAE version from CMake env var `HLAE_VERSION`. `ffmpeg.json` generated by hand-run dev tool `installer2/tools`,
  committed, no downloads during builds. `history` kept in the state.
- Payload stays `include_bytes!` for now, behind one `payload()` function; revisit (post-build resource) if
  payload growth makes build times hurt (6.1).
- Old setup over ours: hidden fake "newer bundle" entry as a guard, works for WiX v5 (8, Untested / open TODOs).
- Files in use: Restart Manager, list + Retry only, no auto-close, quiet mode 1603 (M7); Restart Manager only for
  `*.exe`, `AfxHook*.dll` + files a quick share-mode probe flags (speed).
- Rollback journal: in the state dir, no flush, commit point = state save (6.10 d). Rejected: no rollback at all
  (NSIS style) – kept since done and tested.
- Install dir protection: owner Administrators + admin-only ACL outside `Program Files`, pinned handle; new installs
  need a new / empty folder (except migration); no reboot deletion in the install dir (6.7). Rejected: only allowing
  install dirs below `Program Files`.
- FFMPEG download: WinHTTP synchronous + automatic system proxy, TLS 1.2+, download in the unelevated client,
  verify in the worker; `--ffmpeg-archive` manual fallback; Wine: silent mode is enough for now (6.8a).

## 8. Plan / milestones
- [ ] **M0 – Setup & quick checks**
  - [x] Check toolchain (Rust, `tauri-cli`), size of the staged HLAE `bin/`.
        Rust 1.95 stable (targets `x86_64` + `i686-pc-windows-msvc`), Node 24, WebView2 155 present; `tauri-cli` 2 installed via `cargo install`.
        Staged `build/package-release/bin`: ~25 MB, 209 files (`x64\` 13 MB, `locales\` 0.2 MB); `hlae.zip` 9 MB
        → `include_bytes!` payload will be well under 10 MB compressed.
  - [x] Check whether plain `cargo build --release` works for the app (vanilla frontend, no build step)
        → **yes**: the CMake build doesn't need `tauri-cli`. Requires `tauri` feature `custom-protocol` in `app/Cargo.toml`
        (tauri-cli adds it itself; without it `tauri::is_dev()` is true – assets still embedded since we have no `devUrl`).
  - [x] Crates `installer2/{core, app}` as members of the **root** Cargo workspace ([`../Cargo.toml`](../Cargo.toml), next to `lib/*`),
        Tauri 2 vanilla app (frontend in `app/ui/`, no `tauri.conf.json` windows, window created in `setup()`).
        Build: `cargo build --profile release-installer -p hlae-installer` → `target/release-installer/HLAE_Setup.exe`,
        **2.7 MB** without payload (plain `release`: 8.1 MB). Profile = copy of `release-tool` (`opt-level = "z"`, LTO, abort, strip).
        Later via Corrosion with `PROFILE "release-installer"`. Root `Cargo.lock` gained the installer deps (no existing version changed).
        If hashing / unpacking FFMPEG is too slow with `opt-level = "z"`: per-package override `[profile.release-installer.package.<crate>]`.
  - [x] Verify assumptions: `withGlobalTauri` with vanilla JS works (`invoke`), page served from inside the exe
        (started from a different working dir), WebView2 data directory set via `data_directory()` → lands in
        `%TEMP%\hlae-setup-webview-<pid>`, nothing in `%LOCALAPPDATA%`.
  - [ ] Still open: delete the WebView2 data directory on exit (6.2) → M6.
  - [x] `include_bytes!` with the real payload: `build.rs` packs `HLAE_PAYLOAD_DIR` fine (see M1).
- [x] **M1 – `core` library + tests**
  - [x] Specify formats (reviewed): payload manifest, `state.json`, FFMPEG variant data file (6.10).
  - [x] Packer: staged dir → `payload.tar.zst` + manifest (SHA-256), deterministic output (sorted, no timestamps).
        Real build (`build/package-release/bin`): 208 files / 25 MB → **6.0 MB** payload (zstd 19 + long mode) in ~9 s;
        install ~1 s incl. verification.
  - [x] Install / upgrade / repair / modify / uninstall into a target dir: checksum rules, overwrite on upgrade, obsolete file
        removal, keep modified files, downgrade + install dir change refused, path safety (reparse points), rollback (6.10 d).
  - [x] Atomic `state.json` write.
  - [x] `app/build.rs` packs the payload into `$OUT_DIR` (env `HLAE_PAYLOAD_DIR` + `HLAE_VERSION`; without them an empty
        payload), [`app/src/payload.rs`](app/src/payload.rs) embeds it. Packing runs with `build-override` `opt-level = 3`
        (root `Cargo.toml`), otherwise zstd 19 in an unoptimized build script would be very slow.
        Note: as long as nothing reads the payload bytes, LTO drops them (exe stays 2.8 MB) → real size check in M2.
  - Tests: `cargo test -p hlae-installer-core` (13 unit + 11 integration tests, temp dirs, no admin).
    Real payload: `set HLAE_TEST_PAYLOAD_DIR=…\build\package-release\bin` + `cargo test -p hlae-installer-core --release -- --ignored --nocapture`.
  - Build times (`release-installer`, this machine): full 8 min, incremental after app or payload change ~1.5 min (LTO dominates).
- [x] **M2 – CLI / silent mode** on top of core (first end-to-end install + uninstall, from an admin shell).
  - [`app/src/cli.rs`](app/src/cli.rs): `--quiet`, `--install` (default) / `--uninstall`, `--install-dir <dir>`,
    `--locales` / `--no-locales`, `--remove-app-data`, `--log <file>`, `--help`; also as `/option`, `--opt=value`.
    No options → GUI. Options without `--quiet` → 1639 for now (wizard takes them in M6).
  - Exit codes: 0, 1603 error (incl. not elevated, until M4), 1605 not installed (uninstall), 1639 invalid command line.
  - [`app/src/logging.rs`](app/src/logging.rs): log to console + file (default `%TEMP%\HLAE_Setup_<time>.log`).
    Release exe is GUI subsystem → `AttachConsole(ATTACH_PARENT_PROCESS)`; callers must wait explicitly:
    PowerShell `Start-Process … -Wait -PassThru -NoNewWindow`, cmd `start /wait "" HLAE_Setup.exe …`.
  - [`core/src/session.rs`](core/src/session.rs): state file load/save around install / uninstall, defaults via known folders
    (`SHGetKnownFolderPath`, not env vars), install dir validation (absolute, 7-bit ASCII), remove `%APPDATA%\HLAE`.
    Partial uninstall (files in use) keeps tracking only the files that are left, so a retry can finish.
  - Debug builds only: env `HLAE_SETUP_STATE_FILE` overrides the state file and skips the admin check → CLI testable without admin.
    Debug builds pack with zstd level 3 (fast), optimized builds with 19.
  - Tested: scenario install → repair → modify (no locales) → uninstall (kept modified + untracked file) → uninstall again (1605),
    and as admin into `C:\HLAE-test` with the release exe (2026-10-17). Release exe **9.1 MB** (3.1 MB code + 6.0 MB payload;
    old `HLAE_Setup.exe` 8.2 MB), release-installer rebuild after code change ~3.5 min.
  - Not yet: start menu, ARP, `APPLICATIONFOLDER`, state dir ACL (M3), self-elevation (M4),
    **detection of the old WiX installer** (M7) – until then never install into the old `Program Files (x86)\HLAE`.
- [x] **M3 – Windows integration**: ARP entry, `APPLICATIONFOLDER`, shortcut, setup cache, self-delete, .NET check.
  - State dir: `advancedfx` + `advancedfx\HLAE` get owner Administrators + protected DACL
    `SYSTEM:F, Administrators:F, Users:RX` ([`core/src/acl.rs`](core/src/acl.rs)); a pre-existing `advancedfx` not owned by
    SYSTEM / Administrators → refuse. Checked before the state is loaded on every run.
  - ARP: `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\HLAE` (64-bit view), `UninstallString` /
    `QuietUninstallString` / `ModifyPath` (`--uninstall`, `--quiet --uninstall`, `--modify`) → cached setup,
    `EstimatedSize` from the installed files, `NoRepair=1` (repair is part of Modify) ([`core/src/registry.rs`](core/src/registry.rs)).
  - `APPLICATIONFOLDER` with trailing `\` like the old MSI; on uninstall the value and then empty keys are removed.
  - Shortcut via `IShellLinkW` ([`core/src/shortcut.rs`](core/src/shortcut.rs)), `windows` crate 0.62 (same as Tauri).
  - .NET check ([`core/src/dotnet.rs`](core/src/dotnet.rs)): `Release >= 394802` (4.6.2), CLI only warns.
  - Order: install: files → state → commit → integration → state again (records what was done, also on error).
    Uninstall: files first; if some can't be removed, integration + state stay so the uninstall can be retried.
  - `Session` ([`core/src/session.rs`](core/src/session.rs)) bundles state file + integration on/off
    (debug override `HLAE_SETUP_STATE_FILE` = no integration, no admin needed).
  - `cargo clippy` clean, `cargo fmt` (default style, no `rustfmt.toml` in the repo).
  - Tested as admin (2026-10-17, script installs into `C:\HLAE-test`, uninstalls via `QuietUninstallString`, backs up and
    restores the old WiX `APPLICATIONFOLDER`): all as expected. **Still to confirm after a reboot:** `C:\ProgramData\advancedfx` gone.
  - Note: ARP `--uninstall` / `--modify` without `--quiet` still end with 1639 (wizard M6); elevation works since M4.
- [x] **M4 – Elevated worker** + named pipe (incl. security measures, 6.4).
  - [`core/src/pipe.rs`](core/src/pipe.rs): random pipe name (`BCryptGenRandom`, 128 bit), worker creates it with
    `FILE_FLAG_FIRST_PIPE_INSTANCE` + `PIPE_REJECT_REMOTE_CLIENTS`, DACL = read/write for the *client's user SID* only,
    checks `GetNamedPipeClientProcessId`; client connects with `SECURITY_IDENTIFICATION` (no impersonation) and checks
    `GetNamedPipeServerProcessId` == the worker it started. `CONNECT_TIMEOUT` 90 s (worker exits if nobody connects).
    `spawn_elevated` = `ShellExecuteExW` verb `runas`, hidden.
  - [`app/src/worker.rs`](app/src/worker.rs): `HLAE_Setup.exe --worker <pipe> <client pid>` (internal, not in `--help`).
    Protocol: one JSON `Request` line (max 64 KiB) → JSON lines `log` … `done {exit_code}`. Worker validates the request
    itself and only installs its own embedded payload. Client forwards the worker's log into its console + log file.
  - [`app/src/ops.rs`](app/src/ops.rs): the elevated operations (moved out of the CLI). CLI: already elevated → in-process,
    else worker. `--remove-app-data` runs in the unelevated client after the uninstall (right user), ignored as SYSTEM.
  - DLL planting: `/DEPENDENTLOADFLAG:0x800` link flag (static imports from System32 only; all static imports are system
    DLLs incl. the UCRT, no `vcruntime140.dll`), plus `SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)` in the
    worker and CLI mode. Not in the GUI (WebView2 loads its own DLLs) – the GUI runs unelevated anyway. GUI still works.
  - Debug builds: `HLAE_SETUP_STATE_FILE` + `HLAE_SETUP_TEST_WORKER=1` → real worker process + pipe, but started
    unelevated → whole path testable without UAC.
  - Tested 2026-10-17 from a non-elevated shell: UAC declined → 1223; install via worker; uninstall via cached setup's
    `QuietUninstallString` (client + worker both run from the cached exe, self-delete still works).
  - Known limitation: started already elevated by a *different* admin account (over-the-shoulder UAC), "remove app data"
    hits that account's `%APPDATA%`.
- [x] **M5 – FFMPEG**: variants, download, default rules, custom `ffmpeg.ini`;
      dev tool `installer2/tools` generating `data/ffmpeg.json` (6.10 b).
  - [x] [`core/src/download.rs`](core/src/download.rs): WinHTTP download as in 6.8a (tested: GitHub incl. redirect, 404).
        Note: status code must be queried with `WINHTTP_QUERY_FLAG_NUMBER` (32 bit), `NUMBER64` returns nothing for it.
  - [x] [`core/src/ffmpeg.rs`](core/src/ffmpeg.rs): data model + validation, archive prefix + file listing (`zip` 9,
        feature `deflate-flate2-zlib-rs` only – the default `deflate` pulls `zopfli`, which would bump `log` in the shared
        `Cargo.lock` and thereby change the hook build).
  - [x] [`tools`](tools/src/main.rs) + generated [`data/ffmpeg.json`](data/ffmpeg.json) (2026-10-17, autobuild-2026-09-30-13-08):
        all 4 archives match BtbN's `checksums.sha256` **and** the SHA-512 from the old `HlaeFfmpeg/Package.wxs`.
        Sizes: gpl 194 MB / 44 files, gpl-shared 86 MB / 224 files, lgpl 172 MB / 44 files, lgpl-shared 77 MB / 224 files
        (the old dialog texts "100 MB" / "50 MB" are outdated). Cache: `target/ffmpeg-cache` (≈ 530 MB).
        Regenerate: see the doc comment in `tools/src/main.rs` (pass the new tag / build / branch).
  - [x] [`core/src/ffmpeg_install.rs`](core/src/ffmpeg_install.rs): `FfmpegPlan` (keep / none / download / custom),
        default rules (4.2; "deliberately none" = keep it that way, no-op), one transaction for components `ffmpeg` +
        `ffmpeg-ini` (switching removes what's no longer wanted, custom → download removes our ini), ini UTF-16LE + BOM,
        custom path checks (absolute, exists, ≤ 260 UTF-16 units), warning for a foreign `ffmpeg.ini`.
        Archive: worker copies it into `<state dir>\download\`, verifies size + SHA-256 there, extracts verifying every file.
  - [x] Session: HLAE transaction → FFMPEG transaction → integration (also after an FFMPEG error, so a fresh HLAE still
        gets its ARP entry; then exit 1603).
  - [x] Client ([`app/src/ffmpeg_client.rs`](app/src/ffmpeg_client.rs)): resolves the plan unelevated (reads the state),
        downloads to `%TEMP%\HLAE_Setup_ffmpeg\` (reused if already verified, deleted after success, kept for a retry).
        CLI: `--ffmpeg <variant|none|keep>`, `--ffmpeg-custom <exe>`, `--ffmpeg-archive <zip>` (variant detected by hash).
  - Tested 2026-10-17 (debug, test worker, real archives): install gpl (44 files, `ffmpeg.exe -version` works) →
    custom (only ini, BOM `FF FE`) → archive shared (224 files, ini removed) → none → default (no-op) → uninstall.
  - Tested 2026-10-17 with the **release exe as unelevated client** (real download, system proxy settings): fresh install
    into `C:\HLAE-test` with defaults → downloaded win64-gpl (194 MB, ~95 s here), worker verified + installed HLAE +
    FFMPEG in ~10 s, `ffmpeg.exe -version` works, temp download deleted, ARP `EstimatedSize` incl. FFMPEG (~509 MB);
    rerun with defaults → "keep as installed", no download; uninstall via cached setup → 252 files removed.
  - Observation: a repair with unchanged files took ~14 s (stages all 208 HLAE files first, then compares) –
    could compare installed files against the manifest *before* staging and only extract what differs (later, M7).
  - Note for the GUI (M6): the download happens before the UAC prompt (unelevated); show progress there.
- [x] **M6 – GUI wizard + translations** (incl. WebView2 check, launch HLAE). Open points moved to "Untested / open TODOs".
  - Design (decided 2026-10-17): black / white / gray, accent maroon `#990000` (HLAE logo `misc/icon/logo2009.svg`, inlined);
    light + dark following Windows (dark: lighter maroon `#ff9e9e` for text / focus). Color-blind friendly: accent differs
    by brightness, states / errors always also by icon (✓ ✕ ! ⚠) and text, radios by shape; contrast ≥ 4.5:1.
    Design refinement is a separate task later.
  - Pages ([`app/ui/`](app/ui/)): install (location + browse + ASCII check, translations, FFMPEG: download variant /
    existing ffmpeg.exe / keep / none, preselected by 4.2), maintenance (update or repair / change / uninstall; newer
    installed → only uninstall), uninstall (+ remove app data), progress (download bar, "waiting for admin", log),
    done (✓ / ✕ / !, start HLAE, open log). `--uninstall` / `--modify` without `--quiet` (Apps & features) open the
    right page. Window close is blocked while working.
  - Backend: [`app/src/gui.rs`](app/src/gui.rs) (commands, background thread, WebView2 check + bootstrapper download),
    [`actions.rs`](app/src/actions.rs) (shared with the CLI), [`events.rs`](app/src/events.rs) (log / phase / download →
    UI), [`dialog.rs`](app/src/dialog.rs) (`IFileOpenDialog`), [`l10n.rs`](app/src/l10n.rs) (catalogs, UI language via
    `GetUserPreferredUILanguages`, override `HLAE_SETUP_LANG`).
  - Translations: [`l10n/messages.pot`](l10n/messages.pot) (all GUI texts), catalogs built by `build.rs` with
    `build-support` (6.9); only English so far.
  - Debug only: `HLAE_SETUP_THEME=light|dark` to check both color schemes.
  - Tested 2026-10-17 (debug, test worker): install / maintenance / uninstall pages in dark + light, repair through the
    GUI (keyboard) → done page, WebView2 data dir cleanup.
  - [x] Real run as normal user with the release exe (2026-10-17): install into `C:\HLAE-test` with FFMPEG download
        (progress page, UAC), Change → Remove FFMPEG, uninstall from the cached setup (`--uninstall` → uninstall page),
        `%APPDATA%\HLAE` kept. Found + fixed: WebView2 data dir left over after the uninstall from the cached setup (6.2).
  - [x] Test translation (2026-10-17, German, deliberately incomplete; kept outside the repo): via
        `HLAE_SETUP_L10N_DIR=<dir>` (`<dir>/<lang>/installer2/messages.po`, overrides the l10n repo path in `build.rs`)
        + `HLAE_SETUP_LANG=de`. Checked: translated texts incl. window title, fuzzy → English, wrong placeholder →
        build warning + English, untranslated → English, longer German texts fit. Auto detection picked `en-US`
        (first of the user's Windows UI languages `en-US, en-DE, en-GB, de-DE, fr-FR`) – correct.
- [x] **M7 – Migration** from WiX v5, files-in-use check, rollback hardening (done 2026-10-17; the `msiexec`
      fallback test skipped on purpose, see below).
  - [x] Detection ([`core/src/legacy.rs`](core/src/legacy.rs), read-only): bundle via `BundleUpgradeCode` in the 32-bit
        Uninstall keys (+ `QuietUninstallString`), MSIs via `MsiEnumRelatedProductsW` + `MsiGetProductInfoW`,
        `APPLICATIONFOLDER`, old FFMPEG `version` / `custom` → mapped to the new variant ids. Verified on the test
        machine (2026-10-17): bundle 2.192.8, HlaeCore 2.192.8, HlaeFfmpeg 1.1.67 `{CA6490CA-…}`,
        `C:\Program Files (x86)\HLAE`, `win64-gpl`.
  - [x] Migration wiring (not tested for real yet): only before our first install (no state) and never in the debug test
        mode. Worker: WiX v3 (< 2.181.0) → refuse with message; WiX v5 → take over the install dir, run the bundle's
        quiet uninstall, then `msiexec /x /qn /norestart` for any MSI still there, error if anything is left, clean the
        leftovers (5), then install; history entry `migrate`. Client / GUI: FFMPEG preselected from the old setup
        (variant / custom path / none), install dir preset, GUI notice ("will be replaced, settings kept"),
        WiX v3 → blocking notice, Install disabled.
  - [x] Real migration test (2026-10-17, release exe, `--quiet` from a normal shell, 1 UAC prompt, ~2 min incl. the
        194 MB download): old bundle uninstalled silently (exit 0, the `msiexec` fallback wasn't needed), all 3 old ARP
        entries gone, old `HLAE FFMPEG` key + folder and `Programs\HLAE\` removed; new HLAE in
        `C:\Program Files (x86)\HLAE` with FFMPEG `win64-gpl` taken over, `Programs\HLAE.lnk`, `APPLICATIONFOLDER`
        written again, history `migrate` → `install` → `ffmpeg`. The test machine now has the new installer's HLAE
        (old setup can be reinstalled from the public release for further migration tests).
  - [x] WiX v3 refusal test (2026-10-17, release exe, normal shell, old HLAE 2.180.1 installed, 0 UAC prompts):
        detected via the HlaeCore UpgradeCode (product `{E6278F97-…}` 2.180.1); `--quiet` → exit 1603 with the
        message, nothing written (no ARP entry, state, shortcut); wizard shows the v3 notice with Install disabled.
        The v3 bundle ("HLAE Installer",
        `{aa80f90d-…}`) has another bundle UpgradeCode and isn't found as a bundle – fine, HlaeCore is enough.
  - [x] Migration via the GUI (2026-10-17, release exe, normal shell, old v5 2.192.8 without FFMPEG, 1 UAC prompt,
        ~10 s): notice shown, location preset, "Don't install FFMPEG" preselected; bundle
        `{5FF3867E-…}` uninstalled silently (exit 0), both old ARP entries + `Programs\HLAE\` gone, new install with
        FFMPEG `none`, history `migrate` → `install`.
  - Not tested, skipped on purpose (rare): `msiexec /x` fallback (bundle missing / broken). Known gap: the orphaned
    bundle ARP entry (+ its `Package Cache` folder) would stay after the fallback.
  - [x] Files-in-use check ([`core/src/in_use.rs`](core/src/in_use.rs), Restart Manager via `windows-sys`
        `Win32_System_RestartManager`, works unelevated). Decided 2026-10-17: wizard offers only Retry (+ Back), no
        "continue anyway"; no automatic closing (`RmShutdown`); quiet mode → 1603; checked files = what the operation
        touches: tracked files of the state (HLAE + FFMPEG) and, for a migration, all files in the old setup's folders
        (`APPLICATIONFOLDER`, `HLAE FFMPEG`). Checked in the client before the download / UAC prompt (wizard: page
        listing the programs, display name + exe; command `programs_in_use`) and again in the session right before
        anything is touched (programs started meanwhile → `Error::FilesInUse`, nothing changed). If the Restart Manager
        itself fails (e.g. Wine): warning, continue. Tested (2026-10-17, debug test mode, `HLAE.exe` held open by a
        PowerShell): quiet repair / uninstall → 1603 with the program listed, nothing changed; wizard page, Retry while
        still held ("Still in use"), Retry after closing → repair done. Tests: `in_use` unit tests,
        `core/tests/session.rs`.
  - [x] Crash-safe rollback journal (6.10 d). Tests: crash before / after the state save (`simulate_crash`),
        journaled steps that never happened + cut-off line; smoke test in debug test mode: install / repair /
        uninstall leave no journal / work dir.
  - [x] Install dir protection + pinning (6.7, prompted by the journal design: reboot deletion in a user-writable
        install dir would be unsafe → the whole dir has to be safe). Unit tests: pin blocks renaming the dir and its
        parent, junction in / below the path refused before creating anything, children get exactly the inherited
        entries. Debug switch `HLAE_SETUP_TEST_PROTECT=1` (with `HLAE_SETUP_STATE_FILE`, admin shell) protects in
        the test mode too.
  - [x] Admin test of the protection (2026-10-17, debug exe, test mode + `HLAE_SETUP_TEST_PROTECT=1`, admin shell,
        below `C:\HLAE-protect-test`): new folder → owner Administrators, protected, SYSTEM / Administrators full +
        Users read & execute, files / dirs inside only inherited entries; repair keeps it; uninstall removes the
        folder; existing non-empty folder → 1603 "isn't empty", file + permissions untouched; existing empty folder →
        taken over + protected; path through a junction → 1603 before anything is created.
  - [x] Faster repair: before staging, the installed files are compared with the manifest (size, then SHA-256);
        identical ones aren't unpacked, only recorded; if nothing differs the payload isn't read at all
        ([`transaction::unchanged_files`](core/src/transaction.rs), `payload::stage` with `skip`). Real test
        (2026-10-17, release exe, intact install in `Program Files (x86)`): worker part < 1 s (was ~14 s),
        "208 files unchanged, 0 to install", no journal / work dir left. Test: repair of an intact install with an
        unreadable payload succeeds, with one broken file it needs the payload again.
  - [x] Files-in-use check made fast (found in that test: 5 s before the UAC prompt). The Restart Manager costs
        ~18 ms per registered file with ~200 running processes (~4 s for 208 files, twice per operation). Decided
        2026-10-17: quick probe of all files (open without sharing → sharing violation = open as data, ~0.1 ms per
        file; doesn't see loaded program images) + Restart Manager only for the candidates: `*.exe`, `AfxHook*.dll`
        and what the probe flagged. Loaded runtime DLLs are found via the HLAE.exe / hook DLL that loaded them. A
        missed file only costs convenience (rename works anyway; uninstall keeps it tracked for a retry).
        Measured on the install: ~0.3 s (in PowerShell) instead of ~4 s. Rejected: all files (slow), all files but
        only checked once before the UAC prompt (still ~4 s).
  - Test machine has the new installer's HLAE again (after the GUI migration test); old v5 / v3 setups can be
    reinstalled from public releases for further migration tests.
### Untested / open TODOs
Things that exist but haven't been tested yet, or small TODOs outside the milestones.
- [ ] **WebView2 missing** (6.2, `gui.rs` `webview2::ensure_installed`): message box → opens Microsoft's download page →
      setup ends. Not tested – needs a machine / VM without WebView2. Left open on purpose (2026-10-17).
- [ ] **RTL (right-to-left) languages** in the wizard: not supported yet (no RTL language in the l10n repo so far,
      moved here 2026-10-17). Would need: `dir="rtl"` from the language, logical CSS properties
      (`margin-inline-start`, `border-inline-start`, `text-align: start`), paths / log / path inputs always LTR,
      bidi isolation (FSI / PDI) around inserted placeholder values, `MB_RTLREADING` for the native message box.
      Verify with someone who reads the language.
- [x] **Guard against the old setup** (decided 2026-10-17, "better than nothing"; tested for WiX v5 below): the old
      WiX v5 / v3 setups don't see our install (it's neither their bundle nor their MSI) and would install their
      MSIs into the same default folder without any warning (MSI never warns about an existing folder) → mixed
      files, two Apps & features entries. Most likely when someone deliberately wants an older HLAE version.
      Integration now writes a hidden Uninstall entry `{25B334F6-90B9-45C4-ADB4-CCE2BC677FC4}` (32-bit view,
      `SystemComponent=1`) with the old bundle's `BundleUpgradeCode` and our version as `BundleVersion`, so a v5
      bundle should see a *newer* related bundle and refuse ("A newer version of HLAE is already installed").
      Removed on uninstall (`state.integration.legacy_guard`), skipped by our own legacy detection.
      Tested 2026-10-17 with the old WiX v5 setup 2.192.8 (Burn 5.0.2) against our 2.192.9: first Burn skipped the
      entry ("Failed to read cache path from registry") → `BundleCachePath` (= our cached setup) added. Then:
      "Detected related bundle … is a downgrade", the wizard still shows its normal Install page, but clicking
      Install → "Plan skipped due to related bundle of plan type Downgrade", apply 0x80070666, exit 1638, never
      elevated (no UAC prompt), nothing installed. Needs our version > the old setup's (true for every real
      release). WiX v3 setups: not tested, likely another bundle upgrade code → no effect there. Also say in the
      release notes: "to install an older HLAE, uninstall HLAE first".

- [ ] **M8 – Build integration** in `cmake/MultiBuild.cmake`, retire `installer/`.
  - [ ] Third-party licenses: the installer's Rust deps (Tauri etc.) are now in the root `Cargo.lock` →
        `misc/THIRDPARTY.yml` / `cargo bundle-licenses` check from [`BUILDING.md`](../BUILDING.md) must cover them.
  - [ ] Update the release steps in `BUILDING.md` (no more 3 WiX versions to bump).

## 9. Reference data
| What | Value |
|---|---|
| Bundle UpgradeCode | `{35C8D990-6C4F-4639-ADF7-D259B06E070B}` |
| HlaeCore UpgradeCode (v3 + v5) | `{33FEF63B-4A17-4D59-ABC9-B7A06BD07F07}` |
| HlaeFfmpeg UpgradeCode | `{9090AE9A-4E9D-46C5-B595-9F2FBCF96945}` |
| HlaeFfmpeg ProductCode (current) | `{CA6490CA-3B5A-464E-9B4A-84A2B3014FF4}` |
| HlaeFfmpeg folder component (v5) | `{07854568-BA31-4064-BE28-3F79781FA09A}` |
| Old v3 FFMPEG component | `{2D1C78F6-D5B5-499E-B596-47429893F8A8}` |
| WiX v3/v5 boundary version | `2.181.0.0` |
| Registry (HLAE) | `HKLM\SOFTWARE\WOW6432Node\advancedfx\HLAE` `APPLICATIONFOLDER` |
| Registry (FFMPEG v5) | `HKLM\SOFTWARE\WOW6432Node\advancedfx\HLAE FFMPEG` `reinstall`,`version`,`custom` |
| Registry (FFMPEG v3) | `HKLM\SOFTWARE\WOW6432Node\advancedfx\HLAE\FFMPEG` |
| Registry (shortcut dummy) | `HKCU\Software\advancedfx\HLAE` `scut` |
| Default install dir | `C:\Program Files (x86)\HLAE` |
| Old FFMPEG dir | `C:\Program Files (x86)\HLAE FFMPEG\ffmpeg` |
| Shortcut (old) | `%ProgramData%\Microsoft\Windows\Start Menu\Programs\HLAE\HLAE.lnk` |
| Shortcut (new) | `%ProgramData%\Microsoft\Windows\Start Menu\Programs\HLAE.lnk` |
| App data | `%APPDATA%\HLAE` |
| Install state (new) | `%ProgramData%\advancedfx\HLAE\state.json` |
| Setup cache (new) | `%ProgramData%\advancedfx\HLAE\setup\HLAE_Setup.exe` |
| FFMPEG ini | `<HLAE>\ffmpeg\ffmpeg.ini`, `[Ffmpeg] Path=` |
| Current FFMPEG source | BtbN FFmpeg-Builds `autobuild-2026-09-30-13-08`, `n9.0.2-17-g2a571b6068` (URLs + SHA-512 in `HlaeFfmpeg/Package.wxs`) |
