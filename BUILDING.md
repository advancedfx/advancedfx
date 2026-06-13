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

A full HLAE release consists of both, Win32 and x64 architecture parts, as such it is required to build both parts.

The `cmake/MultiBuild.cmake` script takes care of puzzling these parts to gether to a working release.


Open Visual Studio 2022 Developer Command Prompt and run the following commands from the repository root, for example `C:\SOURCE-DIR\advancedfx`.

For a regular release package, including `hlae.zip`, `hlae_pdb.zip`, and the WiX installer:

```batch
cmake -P cmake/MultiBuild.cmake
```

This reuses `build/win32-release` and `build/x64-release` and writes output to:

```text
build/package-release
```

For testing HLAE as an installed release folder without creating zip files or an installer:

```batch
cmake -DAFX_MULTIBUILD_STAGING=ON -P cmake/MultiBuild.cmake
```

This writes output to:

```text
build/staging-release
```

For day-to-day development, you can build/install only the architecture you are working on:

```batch
cmake --preset x64-debug
cmake --build --preset x64-debug
cmake --install build/x64-debug --config Debug --prefix build/dev-dist/debug
```

```batch
cmake --preset win32-debug
cmake --build --preset win32-debug
cmake --install build/win32-debug --config Debug --prefix build/dev-dist/debug
```

Use `x64-debug` for x64 hooks. Use `win32-debug` for the GUI, injector, and Win32 hooks. These build directories are reused across builds, so changing one source file should only rebuild the affected targets and their dependents.

To stage a combined debug layout for testing:

```batch
cmake -DAFX_MULTIBUILD_CONFIG=Debug -DAFX_MULTIBUILD_STAGING=ON -P cmake/MultiBuild.cmake
```

This writes output to:

```text
build/staging-debug
```

Useful options of `cmake/MultiBuild.cmake`:

| Option | Purpose |
| --- | --- |
| `-DAFX_MULTIBUILD_STAGING=ON` | Enable staging folder mode, no ZIP files and no installer is created.
| `-DAFX_MULTIBUILD_CONFIG=Debug` / `Release` | Select build configuration. |
| `-DAFX_MULTIBUILD_CONFIGURE=ON` | Force reconfigure of existing preset build directories. |
| `-DAFX_MULTIBUILD_STAGING_X64=source1` / `source2` | Only valid in staging mode: Build only one x64 hook family plus the shared x64 runtime. |
| `-DAFX_MULTIBUILD_INSTALLER=OFF` | Only valid in non-staging (default) mode: Skip WiX installer creation and create only the package tree and ZIP files. |

Available configure and build presets are `win32-debug`, `x64-debug`, `win32-release`, `x64-release`.

The intended workflow is to use the separate `win32-*` and `x64-*` preset build directories through `cmake/MultiBuild.cmake`.

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
cmake --preset x64-debug -DAFX_RUST_CARGO_LOCKED=FALSE
```

This change will last until you delete CMakeCache.txt or until you define it as TRUE again.
