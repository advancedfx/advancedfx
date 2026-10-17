//! The old WiX based HLAE_Setup.exe (NOTES.md 5): detection, its settings, and removing it.
//!
//! - WiX v5 (HLAE ≥ 2.181.0): Burn bundle + MSIs "HLAE" (HlaeCore) and "HLAE FFMPEG" → migrated:
//!   settings are taken over, the old installation is uninstalled before the new one is installed.
//! - WiX v3 (HLAE < 2.181.0): not migrated (decided), only detected → the user uninstalls it first.

use std::ffi::OsStr;
use std::os::windows::ffi::OsStrExt;
use std::path::PathBuf;
use std::process::Command;

use windows_sys::Win32::System::ApplicationInstallationAndServicing::{
    INSTALLPROPERTY_VERSIONSTRING, MsiEnumRelatedProductsW, MsiGetProductInfoW,
};
use winreg::RegKey;
use winreg::enums::{HKEY_LOCAL_MACHINE, KEY_READ, KEY_WOW64_32KEY};

use crate::error::{Error, Result};
use crate::version;

pub const BUNDLE_UPGRADE_CODE: &str = "{35C8D990-6C4F-4639-ADF7-D259B06E070B}";
pub const CORE_UPGRADE_CODE: &str = "{33FEF63B-4A17-4D59-ABC9-B7A06BD07F07}";
pub const FFMPEG_UPGRADE_CODE: &str = "{9090AE9A-4E9D-46C5-B595-9F2FBCF96945}";
/// First HLAE version installed by the WiX v5 setup.
pub const WIX_V5_FIRST: &str = "2.181.0";

const UNINSTALL_KEY: &str = r"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";
const APP_KEY: &str = r"SOFTWARE\advancedfx\HLAE";
const FFMPEG_KEY: &str = r"SOFTWARE\advancedfx\HLAE FFMPEG";

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Product {
    pub code: String,
    pub version: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Bundle {
    /// Apps & features key name (the bundle id).
    pub key: String,
    pub version: String,
    pub quiet_uninstall: String,
}

/// FFMPEG as the old setup had it.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum LegacyFfmpeg {
    None,
    /// New variant id (mapped from the old registry value).
    Variant(String),
    Custom(PathBuf),
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct LegacyInstall {
    pub bundle: Option<Bundle>,
    pub core: Vec<Product>,
    pub ffmpeg_products: Vec<Product>,
    /// `APPLICATIONFOLDER` (without the trailing `\`).
    pub install_dir: Option<PathBuf>,
    pub ffmpeg: LegacyFfmpeg,
}

impl LegacyInstall {
    /// HLAE version installed by the old setup (highest HlaeCore product, else the bundle's).
    pub fn version(&self) -> Option<String> {
        self.core
            .iter()
            .map(|p| p.version.clone())
            .max_by(|a, b| version::compare(a, b).unwrap_or(std::cmp::Ordering::Equal))
            .or_else(|| self.bundle.as_ref().map(|b| b.version.clone()))
    }

    /// Installed by the WiX v3 setup (< 2.181.0) → not migrated.
    pub fn is_wix_v3(&self) -> bool {
        self.core
            .iter()
            .any(|p| version::compare(&p.version, WIX_V5_FIRST).is_ok_and(|o| o.is_lt()))
    }
}

/// Old FFMPEG variant names (registry `version` of the old setup) → new ids (NOTES.md 6.10 b).
pub fn map_old_variant(old: &str) -> Option<&'static str> {
    Some(match old {
        "latest-win64-static" => "win64-gpl",
        "latest-win64-shared" => "win64-gpl-shared",
        "latest-win64-static-lgpl" => "win64-lgpl",
        "latest-win64-shared-lgpl" => "win64-lgpl-shared",
        _ => return None,
    })
}

fn wide(s: &str) -> Vec<u16> {
    OsStr::new(s).encode_wide().chain(Some(0)).collect()
}

/// Product codes of the installed MSIs with this upgrade code, with their versions.
fn related_products(upgrade_code: &str) -> Vec<Product> {
    let code = wide(upgrade_code);
    let mut products = Vec::new();
    for index in 0.. {
        let mut buffer = [0u16; 39]; // GUID + NUL
        // SAFETY: valid upgrade code string and a 39-character buffer as the API requires.
        if unsafe { MsiEnumRelatedProductsW(code.as_ptr(), 0, index, buffer.as_mut_ptr()) } != 0 {
            break; // ERROR_NO_MORE_ITEMS or an error
        }
        let product = String::from_utf16_lossy(&buffer[..38]);
        let product_wide = wide(&product);
        let mut value = [0u16; 64];
        let mut len = value.len() as u32;
        // SAFETY: valid strings, buffer with its length.
        let ok = unsafe {
            MsiGetProductInfoW(
                product_wide.as_ptr(),
                INSTALLPROPERTY_VERSIONSTRING,
                value.as_mut_ptr(),
                &mut len,
            )
        } == 0;
        let version = if ok {
            String::from_utf16_lossy(&value[..len as usize])
        } else {
            String::new()
        };
        products.push(Product {
            code: product,
            version,
        });
    }
    products
}

fn find_bundle() -> Option<Bundle> {
    let uninstall = RegKey::predef(HKEY_LOCAL_MACHINE)
        .open_subkey_with_flags(UNINSTALL_KEY, KEY_READ | KEY_WOW64_32KEY)
        .ok()?;
    // Our own guard entry looks like a bundle on purpose (`registry::write_legacy_guard`).
    let names = uninstall
        .enum_keys()
        .filter_map(|k| k.ok())
        .filter(|name| !name.eq_ignore_ascii_case(crate::registry::LEGACY_GUARD_KEY_NAME));
    for name in names {
        let Ok(key) = uninstall.open_subkey_with_flags(&name, KEY_READ | KEY_WOW64_32KEY) else {
            continue;
        };
        let codes: Vec<String> = key
            .get_value::<Vec<String>, _>("BundleUpgradeCode")
            .or_else(|_| {
                key.get_value::<String, _>("BundleUpgradeCode")
                    .map(|c| vec![c])
            })
            .unwrap_or_default();
        if codes
            .iter()
            .any(|c| c.eq_ignore_ascii_case(BUNDLE_UPGRADE_CODE))
        {
            return Some(Bundle {
                key: name,
                version: key.get_value("DisplayVersion").unwrap_or_default(),
                quiet_uninstall: key.get_value("QuietUninstallString").unwrap_or_default(),
            });
        }
    }
    None
}

fn read_string(key: &str, value: &str) -> Option<String> {
    RegKey::predef(HKEY_LOCAL_MACHINE)
        .open_subkey_with_flags(key, KEY_READ | KEY_WOW64_32KEY)
        .and_then(|k| k.get_value::<String, _>(value))
        .ok()
        .filter(|v| !v.is_empty())
}

/// The old setup's installation, `None` if there is none.
pub fn detect() -> Option<LegacyInstall> {
    let bundle = find_bundle();
    let core = related_products(CORE_UPGRADE_CODE);
    let ffmpeg_products = related_products(FFMPEG_UPGRADE_CODE);
    if bundle.is_none() && core.is_empty() && ffmpeg_products.is_empty() {
        return None;
    }
    let install_dir = read_string(APP_KEY, "APPLICATIONFOLDER")
        .map(|dir| PathBuf::from(dir.trim_end_matches(['\\', '/'])));
    let ffmpeg = if ffmpeg_products.is_empty() {
        LegacyFfmpeg::None
    } else {
        match read_string(FFMPEG_KEY, "version").as_deref() {
            Some("custom") => match read_string(FFMPEG_KEY, "custom") {
                Some(path) => LegacyFfmpeg::Custom(PathBuf::from(path)),
                None => LegacyFfmpeg::Variant("win64-gpl".into()),
            },
            Some(old) => LegacyFfmpeg::Variant(map_old_variant(old).unwrap_or("win64-gpl").into()),
            None => LegacyFfmpeg::Variant("win64-gpl".into()),
        }
    };
    Some(LegacyInstall {
        bundle,
        core,
        ffmpeg_products,
        install_dir,
        ffmpeg,
    })
}

/// Splits `"C:\path\setup.exe" /uninstall /quiet` into program and arguments.
fn split_command(command: &str) -> Option<(String, Vec<String>)> {
    let command = command.trim();
    let (program, rest) = match command.strip_prefix('"') {
        Some(rest) => {
            let end = rest.find('"')?;
            (rest[..end].to_owned(), &rest[end + 1..])
        }
        None => match command.split_once(' ') {
            Some((program, rest)) => (program.to_owned(), rest),
            None => (command.to_owned(), ""),
        },
    };
    Some((
        program,
        rest.split_whitespace().map(str::to_owned).collect(),
    ))
}

/// Uninstalls the old WiX v5 installation (elevated): the bundle's quiet uninstall, then
/// `msiexec /x` for any MSI that's still there. Keeps `%APPDATA%\HLAE` (the old bundle's
/// "remove app data" option is off by default). Errors if something is left.
pub fn uninstall(legacy: &LegacyInstall) -> Result<()> {
    if let Some(bundle) = &legacy.bundle {
        log::info!(
            "uninstalling the old HLAE Setup (bundle {} {})",
            bundle.key,
            bundle.version
        );
        match split_command(&bundle.quiet_uninstall) {
            Some((program, args)) if !program.is_empty() => {
                match Command::new(&program).args(&args).status() {
                    Ok(status) => log::info!("old bundle uninstall: {status}"),
                    Err(e) => log::warn!("could not run the old bundle's uninstall {program}: {e}"),
                }
            }
            _ => log::warn!("the old bundle has no usable QuietUninstallString"),
        }
    }
    let msiexec = crate::win::known_folder(crate::win::KnownFolder::System)
        .map(|dir| dir.join("msiexec.exe"))
        .unwrap_or_else(|_| PathBuf::from("msiexec.exe"));
    for product in related_products(CORE_UPGRADE_CODE)
        .into_iter()
        .chain(related_products(FFMPEG_UPGRADE_CODE))
    {
        log::info!("uninstalling old MSI {} {}", product.code, product.version);
        match Command::new(&msiexec)
            .args(["/x", &product.code, "/qn", "/norestart"])
            .status()
        {
            Ok(status) => log::info!("msiexec /x {}: {status}", product.code),
            Err(e) => log::warn!("could not run msiexec: {e}"),
        }
    }
    let left: Vec<Product> = related_products(CORE_UPGRADE_CODE)
        .into_iter()
        .chain(related_products(FFMPEG_UPGRADE_CODE))
        .collect();
    if !left.is_empty() {
        return Err(Error::Legacy(format!(
            "the old HLAE installation could not be removed completely ({}), please uninstall it in Apps & features",
            left.iter()
                .map(|p| format!("{} {}", p.code, p.version))
                .collect::<Vec<_>>()
                .join(", ")
        )));
    }
    cleanup_leftovers();
    Ok(())
}

/// `C:\Program Files (x86)\HLAE FFMPEG`, where the old setup put FFMPEG.
pub fn old_ffmpeg_dir() -> Option<PathBuf> {
    crate::win::known_folder(crate::win::KnownFolder::ProgramFilesX86)
        .ok()
        .map(|dir| dir.join("HLAE FFMPEG"))
}

/// Leftovers of the old setup (NOTES.md 5), best effort.
fn cleanup_leftovers() {
    let hklm = RegKey::predef(HKEY_LOCAL_MACHINE);
    if let Ok(software) = hklm.open_subkey_with_flags(
        "SOFTWARE",
        KEY_READ | winreg::enums::KEY_WRITE | KEY_WOW64_32KEY,
    ) {
        let _ = software.delete_subkey_all(r"advancedfx\HLAE FFMPEG");
    }
    let hkcu = RegKey::predef(winreg::enums::HKEY_CURRENT_USER);
    if let Ok(key) = hkcu.open_subkey_with_flags(
        r"Software\advancedfx\HLAE",
        KEY_READ | winreg::enums::KEY_WRITE,
    ) {
        let _ = key.delete_value("scut");
    }
    if let Some(dir) = old_ffmpeg_dir() {
        let _ = std::fs::remove_dir(dir.join("ffmpeg"));
        let _ = std::fs::remove_dir(&dir);
    }
    if let Ok(programs) = crate::win::known_folder(crate::win::KnownFolder::CommonPrograms) {
        let _ = std::fs::remove_dir(programs.join("HLAE")); // only if empty
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn maps_old_variants() {
        assert_eq!(map_old_variant("latest-win64-static"), Some("win64-gpl"));
        assert_eq!(
            map_old_variant("latest-win64-shared-lgpl"),
            Some("win64-lgpl-shared")
        );
        assert_eq!(map_old_variant("custom"), None);
    }

    #[test]
    fn splits_uninstall_commands() {
        assert_eq!(
            split_command(r#""C:\ProgramData\Package Cache\{x}\HLAE_Setup.exe" /uninstall /quiet"#),
            Some((
                r"C:\ProgramData\Package Cache\{x}\HLAE_Setup.exe".into(),
                vec!["/uninstall".into(), "/quiet".into()]
            ))
        );
        assert_eq!(
            split_command("setup.exe /x"),
            Some(("setup.exe".into(), vec!["/x".into()]))
        );
        assert_eq!(split_command(r#""unterminated"#), None);
    }

    #[test]
    fn wix_v3_by_version() {
        let product = |version: &str| Product {
            code: "{x}".into(),
            version: version.into(),
        };
        let install = |core| LegacyInstall {
            bundle: None,
            core,
            ffmpeg_products: Vec::new(),
            install_dir: None,
            ffmpeg: LegacyFfmpeg::None,
        };
        assert!(install(vec![product("2.180.1")]).is_wix_v3());
        assert!(!install(vec![product("2.192.8")]).is_wix_v3());
        assert_eq!(
            install(vec![product("2.181.1"), product("2.192.8")])
                .version()
                .as_deref(),
            Some("2.192.8")
        );
    }

    /// Read-only: prints what's detected on this machine.
    /// `cargo test -p hlae-installer-core legacy -- --ignored --nocapture`
    #[test]
    #[ignore]
    fn print_detected() {
        println!("{:#?}", detect());
    }
}
