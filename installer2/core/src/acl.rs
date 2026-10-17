//! Admin-only directories for the install state and setup cache (NOTES.md 6.7).

use std::io;
use std::os::windows::ffi::OsStrExt;
use std::path::Path;
use std::ptr::null_mut;

use windows_sys::Win32::Foundation::{ERROR_SUCCESS, LocalFree};
use windows_sys::Win32::Security::Authorization::{
    ConvertStringSecurityDescriptorToSecurityDescriptorW, GetNamedSecurityInfoW, SDDL_REVISION_1,
    SE_FILE_OBJECT, SetNamedSecurityInfoW,
};
use windows_sys::Win32::Security::{
    ACL, DACL_SECURITY_INFORMATION, GetSecurityDescriptorDacl, GetSecurityDescriptorOwner,
    IsWellKnownSid, OWNER_SECURITY_INFORMATION, PROTECTED_DACL_SECURITY_INFORMATION,
    PSECURITY_DESCRIPTOR, PSID, WinBuiltinAdministratorsSid, WinLocalSystemSid,
};

/// SYSTEM + Administrators full control, Users read & execute (the cached setup must be startable
/// from Apps & features), inherited by everything below, nothing inherited from the parent.
const ADMIN_ONLY_DACL: &str = "D:P(A;OICI;FA;;;SY)(A;OICI;FA;;;BA)(A;OICI;0x1200a9;;;BU)";

fn wide(s: &std::ffi::OsStr) -> Vec<u16> {
    s.encode_wide().chain(Some(0)).collect()
}

/// Applies the admin-only ACL to `dir`; with `set_owner` also makes Administrators the owner
/// (needs admin rights).
pub fn secure_dir(dir: &Path, set_owner: bool) -> io::Result<()> {
    let sddl = if set_owner {
        format!("O:BA{ADMIN_ONLY_DACL}")
    } else {
        ADMIN_ONLY_DACL.to_owned()
    };
    let sddl = wide(sddl.as_ref());
    let path = wide(dir.as_os_str());
    let mut descriptor: PSECURITY_DESCRIPTOR = null_mut();
    // SAFETY: valid NUL-terminated strings and out pointers; `descriptor` is freed with LocalFree.
    unsafe {
        if ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl.as_ptr(),
            SDDL_REVISION_1,
            &mut descriptor,
            null_mut(),
        ) == 0
        {
            return Err(io::Error::last_os_error());
        }
        let result = (|| {
            let (mut present, mut defaulted) = (0, 0);
            let mut dacl: *mut ACL = null_mut();
            if GetSecurityDescriptorDacl(descriptor, &mut present, &mut dacl, &mut defaulted) == 0 {
                return Err(io::Error::last_os_error());
            }
            let mut owner: PSID = null_mut();
            let mut info = DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION;
            if set_owner {
                if GetSecurityDescriptorOwner(descriptor, &mut owner, &mut defaulted) == 0 {
                    return Err(io::Error::last_os_error());
                }
                info |= OWNER_SECURITY_INFORMATION;
            }
            match SetNamedSecurityInfoW(
                path.as_ptr(),
                SE_FILE_OBJECT,
                info,
                owner,
                null_mut(),
                dacl,
                null_mut(),
            ) {
                ERROR_SUCCESS => Ok(()),
                error => Err(io::Error::from_raw_os_error(error as i32)),
            }
        })();
        LocalFree(descriptor);
        result
    }
}

/// Whether `path` is owned by SYSTEM or Administrators, i.e. not pre-created ("squatted") by a normal user.
pub fn owner_is_trusted(path: &Path) -> io::Result<bool> {
    let path = wide(path.as_os_str());
    let mut owner: PSID = null_mut();
    let mut descriptor: PSECURITY_DESCRIPTOR = null_mut();
    // SAFETY: valid NUL-terminated string and out pointers; `descriptor` is freed with LocalFree.
    unsafe {
        let error = GetNamedSecurityInfoW(
            path.as_ptr(),
            SE_FILE_OBJECT,
            OWNER_SECURITY_INFORMATION,
            &mut owner,
            null_mut(),
            null_mut(),
            null_mut(),
            &mut descriptor,
        );
        if error != ERROR_SUCCESS {
            return Err(io::Error::from_raw_os_error(error as i32));
        }
        let trusted = IsWellKnownSid(owner, WinBuiltinAdministratorsSid) != 0
            || IsWellKnownSid(owner, WinLocalSystemSid) != 0;
        LocalFree(descriptor);
        Ok(trusted)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn owner_check() {
        // Owned by SYSTEM.
        assert!(owner_is_trusted(Path::new("C:\\ProgramData")).unwrap());
        // Owned by the current (normal) user.
        let dir = tempfile::tempdir().unwrap();
        if !crate::win::is_elevated().unwrap() {
            assert!(!owner_is_trusted(dir.path()).unwrap());
        }
    }

    #[test]
    fn secured_dir_is_read_only_for_normal_users() {
        if crate::win::is_elevated().unwrap() {
            return; // admins keep full access
        }
        let dir = tempfile::tempdir().unwrap();
        std::fs::create_dir(dir.path().join("sub")).unwrap();
        std::fs::write(dir.path().join("sub").join("x"), b"x").unwrap();
        secure_dir(dir.path(), false).unwrap();
        assert!(std::fs::write(dir.path().join("sub").join("y"), b"y").is_err());
        assert_eq!(
            std::fs::read(dir.path().join("sub").join("x")).unwrap(),
            b"x"
        );
        // Restore inheritance so the temp dir can be removed (the owner may always change the DACL).
        let status = std::process::Command::new("icacls")
            .arg(dir.path())
            .args(["/reset", "/T", "/Q"])
            .status()
            .unwrap();
        assert!(status.success());
    }
}
