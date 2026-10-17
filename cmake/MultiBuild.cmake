cmake_minimum_required(VERSION 3.24)

get_filename_component(AFX_SCRIPT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/AfxBuildScript.cmake")

if(NOT DEFINED AFX_MULTIBUILD_STAGING)
    set(AFX_MULTIBUILD_STAGING OFF)
endif()
if(AFX_MULTIBUILD_STAGING)
    set(AFX_MULTIBUILD_LABEL Staging)
    if(NOT DEFINED AFX_MULTIBUILD_STAGING_X64)
        set(AFX_MULTIBUILD_STAGING_X64 all)
    endif()
else()
    set(AFX_MULTIBUILD_LABEL Package)
    set(AFX_MULTIBUILD_STAGING_X64 all)
endif()
string(TOLOWER "${AFX_MULTIBUILD_STAGING_X64}" AFX_MULTIBUILD_STAGING_X64_LOWER)

if(NOT DEFINED AFX_MULTIBUILD_CONFIG)
    set(AFX_MULTIBUILD_CONFIG Release)
endif()
string(TOLOWER "${AFX_MULTIBUILD_CONFIG}" AFX_MULTIBUILD_CONFIG_LOWER)

if(NOT DEFINED AFX_MULTIBUILD_CONFIGURE)
    set(AFX_MULTIBUILD_CONFIGURE AUTO)
endif()

if(NOT DEFINED AFX_MULTIBUILD_PREFIX)
    if(NOT AFX_MULTIBUILD_STAGING)
        set(AFX_MULTIBUILD_PREFIX "${AFX_SCRIPT_SOURCE_DIR}/build/package-${AFX_MULTIBUILD_CONFIG_LOWER}")
    else()
        set(AFX_MULTIBUILD_PREFIX "${AFX_SCRIPT_SOURCE_DIR}/build/staging-${AFX_MULTIBUILD_CONFIG_LOWER}")
    endif()
endif()
get_filename_component(AFX_MULTIBUILD_PREFIX "${AFX_MULTIBUILD_PREFIX}" ABSOLUTE BASE_DIR "${AFX_SCRIPT_SOURCE_DIR}")

if(NOT DEFINED AFX_MULTIBUILD_INSTALLER)
    if(NOT AFX_MULTIBUILD_STAGING)
        set(AFX_MULTIBUILD_INSTALLER ON)
    endif()
endif()

set(AFX_MULTIBUILD_WIN32_BUILD_DIR "${AFX_SCRIPT_SOURCE_DIR}/build/win32-${AFX_MULTIBUILD_CONFIG_LOWER}")
set(AFX_MULTIBUILD_X64_BUILD_DIR "${AFX_SCRIPT_SOURCE_DIR}/build/x64-${AFX_MULTIBUILD_CONFIG_LOWER}")

message(STATUS "${AFX_MULTIBUILD_LABEL} config: ${AFX_MULTIBUILD_CONFIG}")
message(STATUS "${AFX_MULTIBUILD_LABEL} prefix: ${AFX_MULTIBUILD_PREFIX}")
message(STATUS "${AFX_MULTIBUILD_LABEL} configure mode: ${AFX_MULTIBUILD_CONFIGURE}")
if(AFX_MULTIBUILD_STAGING)
    message(STATUS "${AFX_MULTIBUILD_LABEL} x64 mode: ${AFX_STAGE_X64_LOWER}")
else()
    message(STATUS "${AFX_MULTIBUILD_LABEL} installer: ${AFX_MULTIBUILD_INSTALLER}")
endif()

afx_stage_architectures(
    "${AFX_MULTIBUILD_CONFIG}"
    "${AFX_MULTIBUILD_PREFIX}"
    "${AFX_MULTIBUILD_CONFIGURE}"
    "${AFX_MULTIBUILD_STAGING_X64_LOWER}"
)

afx_script_run("${AFX_SCRIPT_SOURCE_DIR}/copy_resources_release.bat" "${AFX_MULTIBUILD_PREFIX}/bin")

afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/corrosion")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_X64_BUILD_DIR}/_deps/corrosion-src/LICENSE" "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/corrosion")

afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/deflate")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_WIN32_BUILD_DIR}/_deps/deflate-src/COPYING" "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/deflate")
afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/imath")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_WIN32_BUILD_DIR}/_deps/imath-src/LICENSE.md" "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/imath")
afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/openexr")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_WIN32_BUILD_DIR}/_deps/openexr-src/LICENSE.md" "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/openexr")

afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/absl")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_WIN32_BUILD_DIR}/_deps/absl-src/LICENSE" "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/absl")
afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/protobuf")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_WIN32_BUILD_DIR}/_deps/protobuf-src/LICENSE" "${AFX_MULTIBUILD_PREFIX}/bin/LICENSES/protobuf")

afx_script_run("${AFX_SCRIPT_SOURCE_DIR}/l10n_update_to_source.bat" "${AFX_MULTIBUILD_PREFIX}/bin")

if(NOT AFX_MULTIBUILD_STAGING)
    afx_script_run_in("${AFX_MULTIBUILD_PREFIX}/bin" "${CMAKE_COMMAND}" -E tar cfv "${AFX_MULTIBUILD_PREFIX}/hlae.zip" --format=zip -- .)
    afx_script_run_in("${AFX_MULTIBUILD_PREFIX}/pdb" "${CMAKE_COMMAND}" -E tar cfv "${AFX_MULTIBUILD_PREFIX}/hlae_pdb.zip" --format=zip -- .)

    if(AFX_MULTIBUILD_INSTALLER)
        # HLAE_Setup.exe (installer2, see installer2/NOTES.md 6.1): embeds bin/ as its payload (the
        # packer skips ffmpeg/), version from hlae/CMakeLists.txt. Built with cargo directly, not via
        # Corrosion: the payload only exists now, after both architectures are staged.
        file(STRINGS "${AFX_SCRIPT_SOURCE_DIR}/hlae/CMakeLists.txt" AFX_MULTIBUILD_HLAE_VERSION_LINES REGEX "^set\\(hlae_VERSION_")
        foreach(AFX_MULTIBUILD_PART MAJOR MINOR PATCH REVISON)
            if(NOT AFX_MULTIBUILD_HLAE_VERSION_LINES MATCHES "set\\(hlae_VERSION_${AFX_MULTIBUILD_PART} ([0-9]+)\\)")
                message(FATAL_ERROR "hlae_VERSION_${AFX_MULTIBUILD_PART} not found in hlae/CMakeLists.txt")
            endif()
            set(AFX_MULTIBUILD_HLAE_VERSION_${AFX_MULTIBUILD_PART} "${CMAKE_MATCH_1}")
        endforeach()
        set(AFX_MULTIBUILD_HLAE_VERSION "${AFX_MULTIBUILD_HLAE_VERSION_MAJOR}.${AFX_MULTIBUILD_HLAE_VERSION_MINOR}.${AFX_MULTIBUILD_HLAE_VERSION_PATCH}")
        if(NOT AFX_MULTIBUILD_HLAE_VERSION_REVISON STREQUAL "0")
            string(APPEND AFX_MULTIBUILD_HLAE_VERSION ".${AFX_MULTIBUILD_HLAE_VERSION_REVISON}")
        endif()
        message(STATUS "${AFX_MULTIBUILD_LABEL} installer version: ${AFX_MULTIBUILD_HLAE_VERSION}")

        if(NOT DEFINED AFX_RUST_CARGO_LOCKED OR AFX_RUST_CARGO_LOCKED)
            set(AFX_MULTIBUILD_CARGO_LOCKED --locked)
        else()
            set(AFX_MULTIBUILD_CARGO_LOCKED)
        endif()
        # Kept between builds: later builds only redo what changed (payload, our crates, LTO).
        set(AFX_MULTIBUILD_INSTALLER_TARGET_DIR "${AFX_SCRIPT_SOURCE_DIR}/build/cargo-installer")
        set(ENV{HLAE_PAYLOAD_DIR} "${AFX_MULTIBUILD_PREFIX}/bin")
        set(ENV{HLAE_VERSION} "${AFX_MULTIBUILD_HLAE_VERSION}")
        # Always the release profile, also for a Debug package: debug builds have test-only switches.
        afx_script_run(cargo build ${AFX_MULTIBUILD_CARGO_LOCKED} --profile release-installer -p hlae-installer --target-dir "${AFX_MULTIBUILD_INSTALLER_TARGET_DIR}")
        afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_MULTIBUILD_INSTALLER_TARGET_DIR}/release-installer/HLAE_Setup.exe" "${AFX_MULTIBUILD_PREFIX}/HLAE_Setup.exe")
    endif()
endif()

message(STATUS "${AFX_MULTIBUILD_LABEL} ready: ${AFX_MULTIBUILD_PREFIX}")
