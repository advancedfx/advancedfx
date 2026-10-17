//! Everything outside the install dir: setup cache, Apps & features entry, `APPLICATIONFOLDER`,
//! start menu shortcut (NOTES.md 4.1, 6.5). What was done is recorded in [`State::integration`].

use std::ffi::OsStr;
use std::fs;
use std::io;
use std::os::windows::ffi::OsStrExt;
use std::path::{Path, PathBuf};

use windows_sys::Win32::Storage::FileSystem::{MOVEFILE_DELAY_UNTIL_REBOOT, MoveFileExW};

use crate::error::{IoContext, Result};
use crate::registry::{self, APP_KEY, ARP_KEY_NAME, ArpEntry, UNINSTALL_BASE};
use crate::shortcut::{self, Shortcut};
use crate::state::State;
use crate::win::{KnownFolder, known_folder};

pub const SETUP_EXE_NAME: &str = "HLAE_Setup.exe";
pub const SHORTCUT_NAME: &str = "HLAE.lnk";

/// Creates / updates all integration for the installed `state`, running from `setup_exe`.
/// `cache_dir` is `<state dir>\setup`. Updates `state.integration` as it goes, also on error.
pub fn apply(state: &mut State, setup_exe: &Path, cache_dir: &Path) -> Result<()> {
    let cached_exe = cache_dir.join(SETUP_EXE_NAME);
    update_cache(setup_exe, &cached_exe)?;
    state.integration.setup_cache = Some(cached_exe.clone());

    let install_dir = state.install_dir.clone();
    let hlae_exe = install_dir.join("HLAE.exe");
    let quoted = format!("\"{}\"", cached_exe.display());
    let entry = ArpEntry {
        display_name: "HLAE".into(),
        display_version: state.hlae_version.clone(),
        publisher: "advancedfx.org".into(),
        display_icon: format!("{},0", hlae_exe.display()),
        install_location: install_dir.display().to_string(),
        uninstall_string: format!("{quoted} --uninstall"),
        quiet_uninstall_string: format!("{quoted} --quiet --uninstall"),
        modify_path: format!("{quoted} --modify"),
        url_info_about: "https://www.advancedfx.org/".into(),
        estimated_size_kb: estimated_size_kb(state),
        install_date: crate::time::utc_now()[..10].replace('-', ""),
    };
    registry::write_arp(&registry::hklm(), UNINSTALL_BASE, ARP_KEY_NAME, &entry)
        .ctx("write Apps & features entry", Path::new(ARP_KEY_NAME))?;
    state.integration.arp_key = Some(ARP_KEY_NAME.into());

    registry::write_app_folder(&registry::hklm(), APP_KEY, &install_dir)
        .ctx("write APPLICATIONFOLDER", Path::new(APP_KEY))?;
    state.integration.app_folder_reg = true;

    registry::write_legacy_guard(
        &registry::hklm(),
        UNINSTALL_BASE,
        crate::legacy::BUNDLE_UPGRADE_CODE,
        &state.hlae_version,
        &cached_exe,
    )
    .ctx(
        "write the guard against the old setup",
        Path::new(registry::LEGACY_GUARD_KEY_NAME),
    )?;
    state.integration.legacy_guard = true;

    let lnk = known_folder(KnownFolder::CommonPrograms)
        .ctx("resolve", Path::new("CommonPrograms"))?
        .join(SHORTCUT_NAME);
    let shortcut = Shortcut {
        target: &hlae_exe,
        working_dir: &install_dir,
        description: "Half-Life Advanced Effects",
    };
    shortcut::create(&lnk, &shortcut)
        .map_err(io::Error::from)
        .ctx("create shortcut", &lnk)?;
    state.integration.shortcut = Some(lnk);
    Ok(())
}

/// Removes everything recorded in `state.integration` (best effort), returns the errors.
pub fn remove(state: &State) -> Vec<String> {
    let mut errors = Vec::new();
    let integration = &state.integration;
    if let Some(lnk) = &integration.shortcut {
        match fs::remove_file(lnk) {
            Err(e) if e.kind() != io::ErrorKind::NotFound => {
                errors.push(format!("{}: {e}", lnk.display()))
            }
            _ => {}
        }
    }
    if integration.app_folder_reg
        && let Err(e) = registry::remove_app_folder(&registry::hklm(), APP_KEY)
    {
        errors.push(format!("{APP_KEY}: {e}"));
    }
    if integration.legacy_guard
        && let Err(e) = registry::remove_legacy_guard(&registry::hklm(), UNINSTALL_BASE)
    {
        errors.push(format!(
            "guard entry {}: {e}",
            registry::LEGACY_GUARD_KEY_NAME
        ));
    }
    if let Some(key) = &integration.arp_key
        && let Err(e) = registry::remove_arp(&registry::hklm(), UNINSTALL_BASE, key)
    {
        errors.push(format!("Apps & features entry {key}: {e}"));
    }
    if let Some(cached_exe) = &integration.setup_cache
        && let Err(e) = remove_cache(cached_exe)
    {
        errors.push(format!("{}: {e}", cached_exe.display()));
    }
    for error in &errors {
        log::error!("{error}");
    }
    errors
}

/// Copies the running installer into the cache (unless it runs from there).
/// An existing cached exe is moved aside first, since it may be running.
fn update_cache(setup_exe: &Path, cached_exe: &Path) -> Result<()> {
    if same_file(setup_exe, cached_exe) {
        return Ok(());
    }
    let dir = cached_exe.parent().expect("cache dir");
    fs::create_dir_all(dir).ctx("create", dir)?;
    let tmp = cached_exe.with_extension("exe.tmp");
    fs::copy(setup_exe, &tmp).ctx("copy installer to", &tmp)?;
    if cached_exe.exists() {
        let aside = unique_aside(cached_exe, "old");
        fs::rename(cached_exe, &aside).ctx("move aside", cached_exe)?;
        if fs::remove_file(&aside).is_err() {
            delete_on_reboot(&aside);
        }
    }
    fs::rename(&tmp, cached_exe).ctx("place", cached_exe)
}

/// Removes the cached installer and its dir. If we *are* the cached installer (uninstall from
/// Apps & features), it can't be deleted while running: rename it to a unique name (so a reinstall
/// before the reboot isn't affected) and delete it on the next reboot.
fn remove_cache(cached_exe: &Path) -> io::Result<()> {
    let running = std::env::current_exe().is_ok_and(|exe| same_file(&exe, cached_exe));
    if running {
        let aside = unique_aside(cached_exe, "delete");
        fs::rename(cached_exe, &aside)?;
        delete_on_reboot(&aside);
        // Also the dirs above it, innermost first: `setup`, `HLAE`, `advancedfx` (skip(1) = the exe
        // itself, handled above). Safe, see [`delete_on_reboot`]: e.g. after a reinstall before the
        // reboot `setup` isn't empty, so nothing is deleted.
        for dir in cached_exe.ancestors().skip(1).take(3) {
            delete_on_reboot(dir);
        }
        return Ok(());
    }
    match fs::remove_file(cached_exe) {
        Err(e) if e.kind() != io::ErrorKind::NotFound => return Err(e),
        _ => {}
    }
    if let Some(dir) = cached_exe.parent() {
        let _ = fs::remove_dir(dir);
    }
    Ok(())
}

fn unique_aside(path: &Path, suffix: &str) -> PathBuf {
    path.with_extension(format!(
        "{}.{}.{suffix}",
        std::process::id(),
        crate::time::utc_now().replace(':', "")
    ))
}

/// Schedules `path` for deletion at the next reboot via `MoveFileExW(path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)`.
///
/// Behaviour as documented for `MoveFileExW`:
/// - The entries are stored in `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\PendingFileRenameOperations`
///   and run early during boot, in the order they were issued.
/// - A *directory* is only deleted if it is empty at that time, else the entry fails silently and the
///   directory stays → scheduling dirs is always safe.
/// - A *file* is deleted unconditionally, whatever it contains by then → only schedule files with a
///   unique name nobody else will reuse (see [`unique_aside`]), never e.g. `HLAE_Setup.exe` itself,
///   which a reinstall before the reboot would recreate.
/// - Only works for administrators / LocalSystem; success only means "registered", not "deleted".
fn delete_on_reboot(path: &Path) {
    let wide: Vec<u16> = OsStr::new(path).encode_wide().chain(Some(0)).collect();
    // SAFETY: valid NUL-terminated path; NULL target = delete.
    if unsafe { MoveFileExW(wide.as_ptr(), std::ptr::null(), MOVEFILE_DELAY_UNTIL_REBOOT) } == 0 {
        log::warn!(
            "could not schedule {} for deletion: {}",
            path.display(),
            io::Error::last_os_error()
        );
    } else {
        log::info!("{} will be deleted on the next reboot", path.display());
    }
}

fn same_file(a: &Path, b: &Path) -> bool {
    match (fs::canonicalize(a), fs::canonicalize(b)) {
        (Ok(a), Ok(b)) => a == b,
        _ => false,
    }
}

fn estimated_size_kb(state: &State) -> u32 {
    let bytes: u64 = state
        .files
        .iter()
        .filter_map(|f| fs::metadata(f.path.to_path(&state.install_dir)).ok())
        .map(|m| m.len())
        .sum();
    u32::try_from(bytes / 1024).unwrap_or(u32::MAX)
}
