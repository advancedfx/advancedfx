//! .NET Framework check (NOTES.md 4.1): check + message only, no prerequisite install.

use winreg::RegKey;
use winreg::enums::{HKEY_LOCAL_MACHINE, KEY_READ, KEY_WOW64_64KEY};

/// .NET Framework 4.6.2 (`hlae.csproj` targets `net462`), see
/// https://learn.microsoft.com/dotnet/framework/install/how-to-determine-which-versions-are-installed
pub const MIN_RELEASE: u32 = 394_802;
pub const MIN_VERSION: &str = "4.6.2";

/// `Release` value of the installed .NET Framework 4.x, `None` if not installed.
pub fn framework_release() -> Option<u32> {
    RegKey::predef(HKEY_LOCAL_MACHINE)
        .open_subkey_with_flags(
            r"SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full",
            KEY_READ | KEY_WOW64_64KEY,
        )
        .and_then(|key| key.get_value::<u32, _>("Release"))
        .ok()
}

pub fn framework_ok() -> bool {
    framework_release().is_some_and(|release| release >= MIN_RELEASE)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn this_machine_has_a_recent_framework() {
        // Every supported Windows 10 / 11 has 4.8+.
        assert!(framework_ok(), "{:?}", framework_release());
    }
}
