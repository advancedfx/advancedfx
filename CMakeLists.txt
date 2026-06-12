cmake_minimum_required (VERSION 3.24)

project ("advancedfx")

# Define CMAKE_CXX_STANDARD otherwise e.g. libprotobuf will have linking problems against ABSL when compiling protobuf:
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
	set(CMAKE_BUILD_TYPE_DEBUG 1)
endif()

set(VS_CONFIGURATION $<IF:$<CONFIG:Debug>,Debug,Release>)

#
# Rust / Corrosion related
#

if (NOT DEFINED AFX_RUST_CARGO_LOCKED)
    set(AFX_RUST_CARGO_LOCKED TRUE)
endif()

message(STATUS "AFX_RUST_CARGO_LOCKED=${AFX_RUST_CARGO_LOCKED}")

#
# Get Microsoft Visual Studio related paths:
#

execute_process(
    COMMAND "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" "-latest" "-version" "[17.0,18.0)" "-property" "installationPath"
    OUTPUT_VARIABLE VS_INSTALLPATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\\" "/" VS_INSTALLPATH ${VS_INSTALLPATH})

execute_process(
    COMMAND "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" "-latest" "-version" "[17.0,18.0)" "-requires" "Microsoft.Component.MSBuild" "-find" "MSBuild\\**\\**\\Bin\\MSBuild.exe"
    OUTPUT_VARIABLE VS_MSBUILD
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\\" "/" VS_MSBUILD ${VS_MSBUILD})

################################################################################

# set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

if(${AFX_RUST_CARGO_LOCKED})
    set(AFX_RUST_CARGO_LOCKED "LOCKED")
else()
    set(AFX_RUST_CARGO_LOCKED "")
endif()

#
# Include dependencies:
#

include(FetchContent)
include(ExternalProject)

if(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")

#
# Setup "corrosion" for including Rust code in our projects:
#

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG 1499b14e4906a2890f5cee1547c8848db261753d # v0.6.1
)
FetchContent_MakeAvailable(Corrosion)

#
# Import our main Rust workspace:
#

if(${CMAKE_BUILD_TYPE_DEBUG})
    set(AFX_RUST_CARGO_PROFILE_HOOK)
    set(AFX_RUST_CARGO_PROFILE_TOOL)
else()
    set(AFX_RUST_CARGO_PROFILE_HOOK PROFILE "release-hook")
    set(AFX_RUST_CARGO_PROFILE_TOOL PROFILE "release-tool")
endif()

corrosion_import_crate(
    MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
    ${AFX_RUST_CARGO_LOCKED}
    ${AFX_RUST_CARGO_PROFILE_HOOK}
    CRATES
        afx_hook_source2
)

#corrosion_import_crate(
#    MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
#    ${AFX_RUST_CARGO_LOCKED}
#    ${AFX_RUST_CARGO_PROFILE_TOOL}
#    IMPORTED_CRATES AFX_RUST_TOOL_CRATES
#    CRATES
#        file_size_calc
#)

if(${CMAKE_BUILD_TYPE_DEBUG})
    #TODO: Not quite fixed yet: https://corrosion-rs.github.io/corrosion/common_issues.html
    corrosion_set_env_vars(afx_hook_source2 "CFLAGS=-MDd" "CXXFLAGS=-MDd")
endif()

endif()

#
# Project setup
#

if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    add_subdirectory("deps/release/protobuf")
    add_subdirectory("deps/release/openexr")
    add_subdirectory("deps/release/injector_bin")

    add_subdirectory("AfxCppCli")
    add_subdirectory("ShaderBuilder")
    add_subdirectory("AfxHookGoldSrc")
    add_subdirectory("AfxHookSource")
    add_subdirectory("hlae")

    add_custom_target(advancedfx ALL
	    DEPENDS hlae
    )
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")

    FetchContent_Declare(
	    nasm
	    URL https://github.com/advancedfx/nasm/releases/download/nasm-2.15.05/nasm-2.15.05-win64.zip https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-win64.zip
	    URL_HASH SHA512=0d0469f3d475f3d192b9b5b7ee74662cbcb9e02efa0d51839c12cbf6f5de5cd58b32e1d12c9ee4381a63e99a4956e03df7decaf1369e598e0edff19a12629073
    )
    FetchContent_MakeAvailable(nasm)

    add_subdirectory("deps/release/openexr")
    add_subdirectory("ShaderBuilder")
    add_subdirectory("AfxHookSource")
    add_subdirectory("AfxHookSource2")
    add_subdirectory("misc/mirv-script/src/snippets")

    add_custom_target(advancedfx ALL
	    DEPENDS AfxHookSource AfxHookSource2
    )
endif()

#
# Install
#

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
if(${CMAKE_BUILD_TYPE_DEBUG})
	set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
include(InstallRequiredSystemLibraries)
if(${CMAKE_BUILD_TYPE_DEBUG})
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "${MSVC_CRT_DIR}/vccorlib${v}d.dll" "${MSVC_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.${MSVC_REDIST_NAME}.CRT/vccorlib${v}.dll")
else()
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "${MSVC_CRT_DIR}/vccorlib${v}.dll" )
endif()

if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    install(
        PROGRAMS
            ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}

            "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
            "$<TARGET_FILE:AfxHookGoldSrc>"
            "$<TARGET_FILE:AfxHookSource>"
            "${hlae_BINARY_DIR}/HLAE.exe"
            "${hlae_BINARY_DIR}/NGettext.dll"
            "${injector_BINARY_DIR}/injector.exe"

            ${AFX_OPENEXR_INSTALL_BINARIES}

        DESTINATION "bin"
    )
    install(
        FILES
            "${injector_BINARY_DIR}/AfxHook.dat"
        DESTINATION "bin"
    )

    install(
        FILES
            "${afxcppcli_BINARY_DIR}/AfxCppCli.pdb"
            "$<TARGET_PDB_FILE:AfxHookGoldSrc>"
            "$<TARGET_PDB_FILE:AfxHookSource>"
            "${hlae_BINARY_DIR}/HLAE.pdb"
            "${injector_BINARY_DIR}/injector.pdb"
        DESTINATION "pdb"
    )

    install(
        PROGRAMS
            "${injector_BINARY_DIR}/x64/injector.exe"
        DESTINATION "bin/x64"
    )
    install(
        FILES
            "${injector_BINARY_DIR}/x64/AfxHook.dat"
        DESTINATION "bin/x64"
    )
    install(
        FILES
            "${injector_BINARY_DIR}/x64/injector.pdb"
        DESTINATION "pdb/x64"
    )
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
    install(
        PROGRAMS
            ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            ${AFX_OPENEXR_INSTALL_BINARIES}
        DESTINATION "bin/x64"
        COMPONENT x64-runtime
    )

    install(
        PROGRAMS
            "$<TARGET_FILE:AfxHookSource>"
        DESTINATION "bin/x64"
        COMPONENT x64-source1
    )

    install(
        PROGRAMS
            "$<TARGET_FILE:AfxHookSource2>"
        DESTINATION "bin/x64"
        COMPONENT x64-source2
    )

    install(
        FILES
            "$<TARGET_PDB_FILE:AfxHookSource>"
        DESTINATION "pdb/x64"
        COMPONENT x64-source1
    )

    install(
        FILES
            "$<TARGET_PDB_FILE:AfxHookSource2>"
        DESTINATION "pdb/x64"
        COMPONENT x64-source2
    )

    install(
		DIRECTORY
            "${AfxHookSource2Snippets_BINARY_DIR}/dist/"
        DESTINATION "bin/resources/AfxHookSource2/snippets"
        COMPONENT x64-source2
    )

#    corrosion_install(
#        TARGETS
#            ${AFX_RUST_TOOL_CRATES}
#        RUNTIME
#            DESTINATION "bin/x64"
#    )
endif()
