//! Registry entries: Apps & features (ARP) and `APPLICATIONFOLDER` (NOTES.md 4.1).
//!
//! All functions take the root key and base path, so tests can use `HKEY_CURRENT_USER` instead of `HKLM`.

use std::io;
use std::path::Path;

use winreg::RegKey;
use winreg::enums::{
    HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS, KEY_READ, KEY_WOW64_32KEY, KEY_WOW64_64KEY,
};

/// Parent of all ARP entries (64-bit view; the old 32-bit MSI used the WOW6432Node one).
pub const UNINSTALL_BASE: &str = r"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";
pub const ARP_KEY_NAME: &str = "HLAE";
/// Read by third-party tools; 32-bit view (`WOW6432Node`) like the old installer.
pub const APP_KEY: &str = r"SOFTWARE\advancedfx\HLAE";
pub const APP_FOLDER_VALUE: &str = "APPLICATIONFOLDER";

pub fn hklm() -> RegKey {
    RegKey::predef(HKEY_LOCAL_MACHINE)
}

pub struct ArpEntry {
    pub display_name: String,
    pub display_version: String,
    pub publisher: String,
    pub display_icon: String,
    pub install_location: String,
    pub uninstall_string: String,
    pub quiet_uninstall_string: String,
    pub modify_path: String,
    pub url_info_about: String,
    pub estimated_size_kb: u32,
    /// `YYYYMMDD`
    pub install_date: String,
}

pub fn write_arp(root: &RegKey, base: &str, key_name: &str, entry: &ArpEntry) -> io::Result<()> {
    let (key, _) = root.create_subkey_with_flags(
        format!(r"{base}\{key_name}"),
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
    )?;
    for (name, value) in [
        ("DisplayName", &entry.display_name),
        ("DisplayVersion", &entry.display_version),
        ("Publisher", &entry.publisher),
        ("DisplayIcon", &entry.display_icon),
        ("InstallLocation", &entry.install_location),
        ("UninstallString", &entry.uninstall_string),
        ("QuietUninstallString", &entry.quiet_uninstall_string),
        ("ModifyPath", &entry.modify_path),
        ("URLInfoAbout", &entry.url_info_about),
        ("InstallDate", &entry.install_date),
    ] {
        key.set_value(name, value)?;
    }
    key.set_value("EstimatedSize", &entry.estimated_size_kb)?;
    key.set_value("NoRepair", &1u32)?; // repair is part of "Modify"
    Ok(())
}

pub fn remove_arp(root: &RegKey, base: &str, key_name: &str) -> io::Result<()> {
    let base = root.open_subkey_with_flags(base, KEY_ALL_ACCESS | KEY_WOW64_64KEY)?;
    ignore_not_found(base.delete_subkey_all(key_name))
}

/// Writes `APPLICATIONFOLDER` with a trailing `\`, exactly like the old MSI (`[APPLICATIONFOLDER]`).
pub fn write_app_folder(root: &RegKey, key_path: &str, install_dir: &Path) -> io::Result<()> {
    let (key, _) = root.create_subkey_with_flags(key_path, KEY_ALL_ACCESS | KEY_WOW64_32KEY)?;
    let dir = install_dir.to_string_lossy();
    key.set_value(
        APP_FOLDER_VALUE,
        &format!("{}\\", dir.trim_end_matches(['\\', '/'])),
    )
}

/// Removes `APPLICATIONFOLDER`, then `key_path` and its parent if they ended up empty.
pub fn remove_app_folder(root: &RegKey, key_path: &str) -> io::Result<()> {
    match root.open_subkey_with_flags(key_path, KEY_ALL_ACCESS | KEY_WOW64_32KEY) {
        Ok(key) => ignore_not_found(key.delete_value(APP_FOLDER_VALUE))?,
        Err(e) => return ignore_not_found(Err(e)),
    }
    let mut path = key_path;
    while let Some((parent, _)) = path.rsplit_once('\\') {
        if !delete_key_if_empty(root, path)? {
            break;
        }
        path = parent;
        if path.eq_ignore_ascii_case("SOFTWARE") {
            break;
        }
    }
    Ok(())
}

pub fn read_app_folder(root: &RegKey, key_path: &str) -> io::Result<Option<String>> {
    match root.open_subkey_with_flags(key_path, KEY_READ | KEY_WOW64_32KEY) {
        Ok(key) => match key.get_value::<String, _>(APP_FOLDER_VALUE) {
            Ok(value) => Ok(Some(value)),
            Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(None),
            Err(e) => Err(e),
        },
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(None),
        Err(e) => Err(e),
    }
}

/// Deletes a key without values and subkeys; returns whether it was deleted.
fn delete_key_if_empty(root: &RegKey, path: &str) -> io::Result<bool> {
    let key = match root.open_subkey_with_flags(path, KEY_READ | KEY_WOW64_32KEY) {
        Ok(key) => key,
        Err(e) if e.kind() == io::ErrorKind::NotFound => return Ok(true),
        Err(e) => return Err(e),
    };
    let info = key.query_info()?;
    if info.sub_keys != 0 || info.values != 0 {
        return Ok(false);
    }
    drop(key);
    ignore_not_found(root.delete_subkey_with_flags(path, KEY_WOW64_32KEY))?;
    Ok(true)
}

fn ignore_not_found(result: io::Result<()>) -> io::Result<()> {
    match result {
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(()),
        result => result,
    }
}

#[cfg(test)]
mod tests {
    use winreg::enums::HKEY_CURRENT_USER;

    use super::*;

    fn test_base() -> String {
        format!(
            r"Software\advancedfx-installer2-test-{}",
            std::process::id()
        )
    }

    #[test]
    fn arp_roundtrip() {
        let root = RegKey::predef(HKEY_CURRENT_USER);
        let base = format!(r"{}\Uninstall", test_base());
        let entry = ArpEntry {
            display_name: "HLAE".into(),
            display_version: "2.192.8".into(),
            publisher: "advancedfx.org".into(),
            display_icon: r"C:\HLAE\HLAE.exe,0".into(),
            install_location: r"C:\HLAE".into(),
            uninstall_string: r#""C:\setup\HLAE_Setup.exe" --uninstall"#.into(),
            quiet_uninstall_string: r#""C:\setup\HLAE_Setup.exe" --quiet --uninstall"#.into(),
            modify_path: r#""C:\setup\HLAE_Setup.exe" --modify"#.into(),
            url_info_about: "https://www.advancedfx.org/".into(),
            estimated_size_kb: 25_000,
            install_date: "20261017".into(),
        };
        write_arp(&root, &base, ARP_KEY_NAME, &entry).unwrap();
        let key = root.open_subkey(format!(r"{base}\{ARP_KEY_NAME}")).unwrap();
        assert_eq!(
            key.get_value::<String, _>("DisplayVersion").unwrap(),
            "2.192.8"
        );
        assert_eq!(key.get_value::<u32, _>("EstimatedSize").unwrap(), 25_000);
        remove_arp(&root, &base, ARP_KEY_NAME).unwrap();
        remove_arp(&root, &base, ARP_KEY_NAME).unwrap(); // idempotent
        assert!(root.open_subkey(format!(r"{base}\{ARP_KEY_NAME}")).is_err());
        root.delete_subkey_all(test_base()).unwrap();
    }

    #[test]
    fn app_folder_roundtrip_removes_empty_keys_only() {
        let root = RegKey::predef(HKEY_CURRENT_USER);
        let parent = format!(r"{}-app", test_base());
        let key_path = format!(r"{parent}\HLAE");
        write_app_folder(&root, &key_path, Path::new(r"C:\Program Files (x86)\HLAE")).unwrap();
        assert_eq!(
            read_app_folder(&root, &key_path).unwrap().as_deref(),
            Some(r"C:\Program Files (x86)\HLAE\")
        );

        // A foreign value keeps the key alive.
        let (key, _) = root
            .create_subkey_with_flags(&key_path, KEY_ALL_ACCESS | KEY_WOW64_32KEY)
            .unwrap();
        key.set_value("other", &"x").unwrap();
        remove_app_folder(&root, &key_path).unwrap();
        assert_eq!(read_app_folder(&root, &key_path).unwrap(), None);
        assert!(
            root.open_subkey_with_flags(&key_path, KEY_READ | KEY_WOW64_32KEY)
                .is_ok()
        );

        key.delete_value("other").unwrap();
        drop(key);
        remove_app_folder(&root, &key_path).unwrap();
        assert!(
            root.open_subkey_with_flags(&parent, KEY_READ | KEY_WOW64_32KEY)
                .is_err(),
            "empty parent removed"
        );
        remove_app_folder(&root, &key_path).unwrap(); // idempotent
    }
}
