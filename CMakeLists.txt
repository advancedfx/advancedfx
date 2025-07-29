cmake_minimum_required (VERSION 3.24)

project ("advancedfx")

# Define CMAKE_CXX_STANDARD otherwise e.g. libprotobuf will have linking problems against ABSL when compiling protobuf:
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(CMAKE_BUILD_TYPE_DEBUG 1)
endif()

set(VS_CONFIGURATION $<IF:$<CONFIG:Debug>,Debug,Release>)

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
#
# MultiBuild

if (NOT ${PROJECT_NAME}-MultiBuild)
    include(ExternalProject)

    macro (setup_arch arch)
        set(CMAKE_GENERATOR_PLATFORM ${arch})  
        ExternalProject_Add (${PROJECT_NAME}-${arch}
        DEPENDS
        BINARY_DIR "${PROJECT_NAME}-${arch}"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-${arch}-install"
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -D${PROJECT_NAME}-MultiBuild=ON
            -DPROJECT_NAME_MULTI=${PROJECT_NAME}-${arch}
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-${arch}-install"
        BUILD_COMMAND
            ${CMAKE_COMMAND} --build . --config $<CONFIG> -- ${BUILD_TOOL_OPTIONS}
        INSTALL_COMMAND
            ${CMAKE_COMMAND} --install . --config $<CONFIG>
        BUILD_ALWAYS true
        )
    endmacro()

    setup_arch(Win32)
    setup_arch(x64)

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist" CACHE PATH "..." FORCE)
    endif()

    add_custom_target(advancedfx ALL
	    DEPENDS advancedfx-Win32 advancedfx-x64
    )

    install(
        CODE
        "
        # Create directories:
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}\")

        # Copy Win32 (currrent exception: old injector x64 is in here too):
        execute_process( WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32-install\" COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"bin\" \"${CMAKE_INSTALL_PREFIX}/bin\" )
        execute_process( WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32-install\" COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"pdb\" \"${CMAKE_INSTALL_PREFIX}/pdb\" )

        # Copy x64:
        execute_process( WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-x64-install\" COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"bin\" \"${CMAKE_INSTALL_PREFIX}/bin\" )
        execute_process( WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-x64-install\" COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"pdb\" \"${CMAKE_INSTALL_PREFIX}/pdb\" )

        # Copy resources:        
        execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\" COMMAND \"${CMAKE_SOURCE_DIR}/copy_resources_release.bat\" \"${CMAKE_INSTALL_PREFIX}/bin\" )

        # Copy additional licenses related to corrosion:
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/corrosion\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E copy \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32/_deps/corrosion-src/LICENSE\" -t  \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/corrosion\")

        # Copy additional licenses related to OpenEXR:
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/deflate\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E copy \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32/_deps/deflate-src/COPYING\" -t  \"${CMAKE_INSTALL_PREFIX}/bin/deflate/corrosion\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/imath\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E copy \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32/_deps/imath-src/LICENSE.md\" -t  \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/imath\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/openexr\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E copy \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32/_deps/openexr-src/LICENSE.md\" -t  \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/openexr\")

        # Copy additional licenses related to protobuf:
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/absl\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E copy \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32/_deps/absl-src/LICENSE\" -t  \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/absl\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/protobuf\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E copy \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-Win32/_deps/protobuf-src/LICENSE\" -t  \"${CMAKE_INSTALL_PREFIX}/bin/LICENSES/protobuf\")

        # Update translations from l10n repo (e.g. required for installer):
        execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\" COMMAND \"${CMAKE_SOURCE_DIR}/l10n_update_to_source.bat\" \"${CMAKE_INSTALL_PREFIX}/bin\" )
        
        # Create ZIP files:
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E tar cfv  \"${CMAKE_INSTALL_PREFIX}/hlae.zip\" --format=zip -- . )
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/pdb\" COMMAND \"${CMAKE_COMMAND}\" -E tar cfv  \"${CMAKE_INSTALL_PREFIX}/hlae_pdb.zip\" --format=zip -- . )

        # Installer related:
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory installer)
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory installer/hlae)
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E copy_directory bin installer/hlae)
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E rm -Rf installer/hlae/ffmpeg)
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E rm -Rf installer/hlae/locales)
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E rm -f installer/hlae/HLAE.exe)
        execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/installer\" COMMAND \"build_installer.bat\" \"${CMAKE_INSTALL_PREFIX}/installer/build\" \"${CMAKE_INSTALL_PREFIX}\" \"${VS_MSBUILD}\")
        "
    )  
    
    return()
endif()

################################################################################

# set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

#
# Include dependencies:
#

include(FetchContent)
include(ExternalProject)

#
# Setup "corrosion" for including Rust code in our projects:
#

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG b1fab721655c5c4b1b08a083d3cd29f163af75d0 # v0.5.1
)
FetchContent_MakeAvailable(Corrosion)

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
    add_subdirectory("deps/release/openexr")
    add_subdirectory("ShaderBuilder")
    add_subdirectory("AfxHookSource2")
    add_subdirectory("misc/mirv-script/src/snippets")

    add_custom_target(advancedfx ALL
	    DEPENDS AfxHookSource2
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

            "$<TARGET_FILE:AfxHookSource2>"

            ${AFX_OPENEXR_INSTALL_BINARIES}           
        DESTINATION "bin/x64"
    )    

    install(
        FILES
            "$<TARGET_PDB_FILE:AfxHookSource2>"
        DESTINATION "pdb/x64"
    )

    install(
        FILES
            ${AFXHOOKSOURCE2SNIPPETS_OUTPUTS}
        DESTINATION "bin/resources/AfxHookSource2/snippets"
    )
endif()

