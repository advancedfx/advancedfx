# Building advancedfx

For an overview over the source code see [CONTENTS.md](CONTENTS.md).

## Standard build

### Install dependencies

- Download and install GIT Bash of https://git-scm.com/ to be able to obtain
  the HLAE source code and also other source code during the build process.

- Install Node.js 24 LTS from https://nodejs.org/en/download/prebuilt-installer .  
  We used v24.14.0 (LTS).

- Download and install Microsoft Visual Studio Community 2022, which you can
  obtain for free from https://www.visualstudio.com/downloads/ .
  - Select Workloads - Desktop & Mobile - .NET Desktop Development
  - Select Workloads - Desktop & Mobile - Desktop development with C++
  - Select Individual Components - .NET Framework 4.6.2 targeting pack

- Obtain Python 3 from https://www.python.org/downloads/windows/ .  
	We used python-3.13.7-amd64.exe, but any 3.8 or newer 3 should do.  
	You will need to do the custom installation  
	with pip and py launcher.
	
-	Obtain GNU gettext Windows binaries from https://www.gnu.org/software/gettext/ .  
	We used gettext0.26-iconv1.17-shared-64.exe .

- Obtain and install Rust and Cargo:  
	https://doc.rust-lang.org/cargo/getting-started/installation.html  
	Then execute the following line in command prompt of the user you will compile HLAE with:  
	`rustup target add --toolchain stable-x86_64-pc-windows-msvc i686-pc-windows-msvc`

### Install source code

Obtain the HLAE source code from https://github.com/advancedfx/advancedfx into a
folder you like, we'll call it SOURCE-DIR from now on (by that I mean
extract it so that you end up with the files like advancedfx.sln and directories
in C:\SOURCE-DIR\advancedfx folder).

We recommend using the GIT Bash to obtain the source code, so you can
easily fetch and merge updates from the advancedfx repository:
```bash
cd /c/SOURCE-DIR
git clone --recurse-submodules https://github.com/advancedfx/advancedfx.git
```

### Build source code

Open Visual Studio 2022 Developer Command Prompt:

```batch
cd C:\SOURCE-DIR\advancedfx\
mkdir build
cd build
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -T "v143" -A "Win32" ../..
cmake --build . --config Release -v -- -r
cmake --install . --config Release -v
```

(For Debug builds replace Release with Debug in the instructions above.)

After that the installer and the zip can be found in "C:\SOURCE-DIR\advancedfx\build\Release".

## Releasing a new version

Things you should do before releasing a new version:

- Update version in hlae/CMakeLists.txt
- Update GUIDs in hlae/UpdateCheck.cs
- If releasing installer, all these need to be updated, because otherwise users can end up with hash collisions if you
  don't manage to provide the exact same package binary again for same version number:
  - Increase package version (e.g. 2nd field (minor), at least 3rd field) in installer/HlaeCore/Package.wxs
  - Increase package version (at least 3rd field) in installer/HlaeFfmpeg/Package.wxs
  - Increase bundle version (e.g. 2nd field (minor), at least 3rd field) in installer/setup/Bundle.wxs
- If you updated Rust dependencies run to see if misc/THIRDPARTY.yml needs updating:
  `cargo install cargo-bundle-licenses && cargo bundle-licenses --format yaml --output misc/CI.yml --previous misc/THIRDPARTY.yml --check-previous && echo DONE: Okay.`
- Update credits file with `py -3 make_credits.py`

## Note on Rust builds and new dependencies

By default `cargo build` is invoked by us with `--locked` from CMake.

If you want to update Rust dependencies / Cargo.lock, then you need to turn this off in the CMake configure command by defining `AFX_RUST_CARGO_LOCKED=FALSE` and continuing from there, example:

```batch
cmake -DCMAKE_BUILD_TYPE=Release -DAFX_RUST_CARGO_LOCKED=FALSE -G "Visual Studio 17 2022" -T "v143" -A "Win32" ../..
```

This change will last until you delete CMakeCache.txt or until you define it as TRUE again.