cmake_minimum_required(VERSION 3.24)

get_filename_component(AFX_SCRIPT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/AfxBuildScript.cmake")

if(NOT DEFINED AFX_PACKAGE_CONFIG)
    set(AFX_PACKAGE_CONFIG Release)
endif()

if(NOT DEFINED AFX_PACKAGE_CONFIGURE)
    set(AFX_PACKAGE_CONFIGURE AUTO)
endif()

if(NOT DEFINED AFX_PACKAGE_PREFIX)
    string(TOLOWER "${AFX_PACKAGE_CONFIG}" AFX_PACKAGE_CONFIG_LOWER)
    set(AFX_PACKAGE_PREFIX "${AFX_SCRIPT_SOURCE_DIR}/build/package-${AFX_PACKAGE_CONFIG_LOWER}")
endif()
get_filename_component(AFX_PACKAGE_PREFIX "${AFX_PACKAGE_PREFIX}" ABSOLUTE BASE_DIR "${AFX_SCRIPT_SOURCE_DIR}")

if(NOT DEFINED AFX_PACKAGE_INSTALLER)
    set(AFX_PACKAGE_INSTALLER ON)
endif()

string(TOLOWER "${AFX_PACKAGE_CONFIG}" AFX_PACKAGE_CONFIG_LOWER)
set(AFX_PACKAGE_WIN32_BUILD_DIR "${AFX_SCRIPT_SOURCE_DIR}/build/win32-${AFX_PACKAGE_CONFIG_LOWER}")
set(AFX_PACKAGE_X64_BUILD_DIR "${AFX_SCRIPT_SOURCE_DIR}/build/x64-${AFX_PACKAGE_CONFIG_LOWER}")

message(STATUS "Package config: ${AFX_PACKAGE_CONFIG}")
message(STATUS "Package prefix: ${AFX_PACKAGE_PREFIX}")
message(STATUS "Package configure mode: ${AFX_PACKAGE_CONFIGURE}")
message(STATUS "Package installer: ${AFX_PACKAGE_INSTALLER}")

afx_stage_architectures(
    "${AFX_PACKAGE_CONFIG}"
    "${AFX_PACKAGE_PREFIX}"
    "${AFX_PACKAGE_CONFIGURE}"
    all
)

afx_script_run("${AFX_SCRIPT_SOURCE_DIR}/copy_resources_release.bat" "${AFX_PACKAGE_PREFIX}/bin")

afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/bin/LICENSES/corrosion")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_PACKAGE_X64_BUILD_DIR}/_deps/corrosion-src/LICENSE" "${AFX_PACKAGE_PREFIX}/bin/LICENSES/corrosion")

afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/bin/LICENSES/deflate")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_PACKAGE_WIN32_BUILD_DIR}/_deps/deflate-src/COPYING" "${AFX_PACKAGE_PREFIX}/bin/LICENSES/deflate")
afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/bin/LICENSES/imath")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_PACKAGE_WIN32_BUILD_DIR}/_deps/imath-src/LICENSE.md" "${AFX_PACKAGE_PREFIX}/bin/LICENSES/imath")
afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/bin/LICENSES/openexr")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_PACKAGE_WIN32_BUILD_DIR}/_deps/openexr-src/LICENSE.md" "${AFX_PACKAGE_PREFIX}/bin/LICENSES/openexr")

afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/bin/LICENSES/absl")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_PACKAGE_WIN32_BUILD_DIR}/_deps/absl-src/LICENSE" "${AFX_PACKAGE_PREFIX}/bin/LICENSES/absl")
afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/bin/LICENSES/protobuf")
afx_script_run("${CMAKE_COMMAND}" -E copy "${AFX_PACKAGE_WIN32_BUILD_DIR}/_deps/protobuf-src/LICENSE" "${AFX_PACKAGE_PREFIX}/bin/LICENSES/protobuf")

afx_script_run("${AFX_SCRIPT_SOURCE_DIR}/l10n_update_to_source.bat" "${AFX_PACKAGE_PREFIX}/bin")

afx_script_run_in("${AFX_PACKAGE_PREFIX}/bin" "${CMAKE_COMMAND}" -E tar cfv "${AFX_PACKAGE_PREFIX}/hlae.zip" --format=zip -- .)
afx_script_run_in("${AFX_PACKAGE_PREFIX}/pdb" "${CMAKE_COMMAND}" -E tar cfv "${AFX_PACKAGE_PREFIX}/hlae_pdb.zip" --format=zip -- .)

if(AFX_PACKAGE_INSTALLER)
    execute_process(
        COMMAND "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" "-latest" "-version" "[17.0,18.0)" "-requires" "Microsoft.Component.MSBuild" "-find" "MSBuild\\**\\**\\Bin\\MSBuild.exe"
        OUTPUT_VARIABLE AFX_PACKAGE_VS_MSBUILD
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\\" "/" AFX_PACKAGE_VS_MSBUILD "${AFX_PACKAGE_VS_MSBUILD}")

    afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/installer")
    afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/installer/hlae")
    afx_script_run("${CMAKE_COMMAND}" -E copy_directory "${AFX_PACKAGE_PREFIX}/bin" "${AFX_PACKAGE_PREFIX}/installer/hlae")
    afx_script_run("${CMAKE_COMMAND}" -E rm -Rf "${AFX_PACKAGE_PREFIX}/installer/hlae/ffmpeg")
    afx_script_run("${CMAKE_COMMAND}" -E rm -Rf "${AFX_PACKAGE_PREFIX}/installer/hlae-opt")
    afx_script_run("${CMAKE_COMMAND}" -E make_directory "${AFX_PACKAGE_PREFIX}/installer/hlae-opt")
    afx_script_run("${CMAKE_COMMAND}" -E rename "${AFX_PACKAGE_PREFIX}/installer/hlae/locales" "${AFX_PACKAGE_PREFIX}/installer/hlae-opt/locales")
    afx_script_run("${CMAKE_COMMAND}" -E rename "${AFX_PACKAGE_PREFIX}/installer/hlae/HLAE.exe" "${AFX_PACKAGE_PREFIX}/installer/hlae-opt/HLAE.exe")

    set(AFX_SCRIPT_SOURCE_DIR "${AFX_SCRIPT_SOURCE_DIR}/installer")
    afx_script_run("build_installer.bat" "${AFX_PACKAGE_PREFIX}/installer/build" "${AFX_PACKAGE_PREFIX}" "${AFX_PACKAGE_VS_MSBUILD}" "${AFX_PACKAGE_CONFIG}")
endif()

message(STATUS "Package ready: ${AFX_PACKAGE_PREFIX}")
