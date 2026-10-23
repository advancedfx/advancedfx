//! Admin-only directories for the install state and setup cache (NOTES.md 6.7).

use std::io;
use std::os::windows::ffi::OsStrExt;
use std::os::windows::io::{AsRawHandle, BorrowedHandle};
use std::path::Path;
use std::ptr::null_mut;

use windows_sys::Win32::Foundation::{ERROR_SUCCESS, HANDLE, LocalFree};
use windows_sys::Win32::Security::Authorization::{
    ConvertSecurityDescriptorToStringSecurityDescriptorW,
    ConvertStringSecurityDescriptorToSecurityDescriptorW, GetNamedSecurityInfoW, SDDL_REVISION_1,
    SE_FILE_OBJECT, SetNamedSecurityInfoW,
};
use windows_sys::Win32::Security::{
    ACL, DACL_SECURITY_INFORMATION, GetKernelObjectSecurity, GetSecurityDescriptorDacl,
    GetSecurityDescriptorOwner, IsWellKnownSid, OWNER_SECURITY_INFORMATION,
    PROTECTED_DACL_SECURITY_INFORMATION, PSECURITY_DESCRIPTOR, PSID, SECURITY_ATTRIBUTES,
    SetKernelObjectSecurity, WinBuiltinAdministratorsSid, WinLocalSystemSid,
};
use windows_sys::Win32::Storage::FileSystem::CreateDirectoryW;
use windows_sys::core::PWSTR;

/// SYSTEM + Administrators full control, Users read & execute (the cached setup must be startable
/// from Apps & features), inherited by everything below, nothing inherited from the parent.
pub const ADMIN_ONLY_DACL: &str = "D:P(A;OICI;FA;;;SY)(A;OICI;FA;;;BA)(A;OICI;0x1200a9;;;BU)";
/// What a dir below an [`ADMIN_ONLY_DACL`] dir inherits (no own entries).
pub const INHERITED_DIR_DACL: &str =
    "D:AI(A;OICIID;FA;;;SY)(A;OICIID;FA;;;BA)(A;OICIID;0x1200a9;;;BU)";
/// What a file below an [`ADMIN_ONLY_DACL`] dir inherits (no own entries).
pub const INHERITED_FILE_DACL: &str = "D:AI(A;ID;FA;;;SY)(A;ID;FA;;;BA)(A;ID;0x1200a9;;;BU)";

fn raw(handle: BorrowedHandle<'_>) -> HANDLE {
    handle.as_raw_handle() as HANDLE
}

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

/// Sets the DACL `dacl` (SDDL, see the constants above) on the object `handle` was opened for
/// (needs `WRITE_DAC`, with `set_owner` also `WRITE_OWNER` + admin rights: owner Administrators).
///
/// Unlike `SetNamedSecurityInfoW` this doesn't walk the dirs below to update what they inherit, so
/// it can't be led anywhere by a junction; callers set the children themselves.
pub fn set_handle_security(
    handle: BorrowedHandle<'_>,
    dacl: &str,
    set_owner: bool,
) -> io::Result<()> {
    let sddl = if set_owner {
        format!("O:BA{dacl}")
    } else {
        dacl.to_owned()
    };
    let sddl = wide(sddl.as_ref());
    let mut descriptor: PSECURITY_DESCRIPTOR = null_mut();
    // SAFETY: valid NUL-terminated string and out pointer; `descriptor` is freed with LocalFree.
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
        let mut info = DACL_SECURITY_INFORMATION;
        if set_owner {
            info |= OWNER_SECURITY_INFORMATION;
        }
        let result = if SetKernelObjectSecurity(raw(handle), info, descriptor) == 0 {
            Err(io::Error::last_os_error())
        } else {
            Ok(())
        };
        LocalFree(descriptor);
        result
    }
}

/// The owner + DACL of the object `handle` was opened for as SDDL (needs `READ_CONTROL`).
pub fn handle_sddl(handle: BorrowedHandle<'_>) -> io::Result<String> {
    let info = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
    let mut needed = 0u32;
    // SAFETY: size query, then a buffer of that size; the string is freed with LocalFree.
    unsafe {
        GetKernelObjectSecurity(raw(handle), info, null_mut(), 0, &mut needed);
        let mut buffer = vec![0u8; needed as usize];
        if GetKernelObjectSecurity(
            raw(handle),
            info,
            buffer.as_mut_ptr().cast(),
            needed,
            &mut needed,
        ) == 0
        {
            return Err(io::Error::last_os_error());
        }
        let mut text: PWSTR = null_mut();
        let mut len = 0u32;
        if ConvertSecurityDescriptorToStringSecurityDescriptorW(
            buffer.as_mut_ptr().cast(),
            SDDL_REVISION_1,
            info,
            &mut text,
            &mut len,
        ) == 0
        {
            return Err(io::Error::last_os_error());
        }
        let sddl = String::from_utf16_lossy(std::slice::from_raw_parts(text, len as usize));
        LocalFree(text.cast());
        Ok(sddl.trim_end_matches('\0').to_owned())
    }
}

/// SYSTEM + Administrators only, for working dirs of an elevated process in a shared place.
const PRIVATE_DACL: &str = "D:P(A;OICI;FA;;;SY)(A;OICI;FA;;;BA)";

/// Creates the *new* dir `dir`, accessible for SYSTEM + Administrators only, in one step (the
/// permissions are set by the creation itself, no window without them). Fails if anything exists
/// there already – also a junction someone planted – so pick an unguessable name.
pub fn create_private_dir(dir: &Path) -> io::Result<()> {
    let sddl = wide(PRIVATE_DACL.as_ref());
    let path = wide(dir.as_os_str());
    let mut descriptor: PSECURITY_DESCRIPTOR = null_mut();
    // SAFETY: valid NUL-terminated strings and out pointer; `descriptor` is freed with LocalFree.
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
        let attributes = SECURITY_ATTRIBUTES {
            nLength: size_of::<SECURITY_ATTRIBUTES>() as u32,
            lpSecurityDescriptor: descriptor,
            bInheritHandle: 0,
        };
        let result = if CreateDirectoryW(path.as_ptr(), &attributes) == 0 {
            Err(io::Error::last_os_error())
        } else {
            Ok(())
        };
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
    fn private_dir_is_new_and_closed() {
        let tmp = tempfile::tempdir().unwrap();
        let dir = tmp.path().join("private");
        create_private_dir(&dir).unwrap();
        let again = create_private_dir(&dir).unwrap_err();
        assert_eq!(again.kind(), io::ErrorKind::AlreadyExists);
        if !crate::win::is_elevated().unwrap() {
            // Normal users (also the creator) can't put anything in.
            assert!(std::fs::write(dir.join("x"), b"x").is_err());
        }
        // The owner may always change the DACL back, so the temp dir can be removed.
        let status = std::process::Command::new("icacls")
            .arg(&dir)
            .args(["/reset", "/Q"])
            .status()
            .unwrap();
        assert!(status.success());
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
