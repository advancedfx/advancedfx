cmake_minimum_required (VERSION 3.24)

project ("advancedfx")

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
            ${CMAKE_COMMAND} --build . --config $<CONFIG> -- -r
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
        execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/installer/setup\" COMMAND \"${VS_MSBUILD}\" \"${CMAKE_SOURCE_DIR}/installer/setup/setup.wixproj\" \"-property:Configuration=${VS_CONFIGURATION}\" \"-property:Platform=x86\" \"-property:OutputPath=${CMAKE_INSTALL_PREFIX}/installer/build\")
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E copy installer/build/HLAE_Setup.exe ./HLAE_Setup.exe)
        execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E copy installer/build/HLAE_Setup.wixpdb ./HLAE_Setup.wixpdb)
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
# Project setup
#

if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    add_subdirectory("deps/release/zlib")
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
    add_subdirectory("AfxHookSource2")

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
            "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.dll"
            "$<TARGET_FILE:AfxHookSource>"
            "${hlae_BINARY_DIR}/HLAE.exe"
            "${hlae_BINARY_DIR}/NGettext.dll"
            "${injector_BINARY_DIR}/injector.exe"

            "${zlib_SOURCE_DIR}/zlib1.dll"

            ${ILMBASE_BUILD_BINARIES}
            ${OPENEXR_BUILD_BINARIES}

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
            "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.pdb"
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

            "${AfxHookSource2_BINARY_DIR}/$<CONFIG>/AfxHookSource2.dll"
        DESTINATION "bin/x64"
    )    

    install(
        FILES
            "${AfxHookSource2_BINARY_DIR}/$<CONFIG>/AfxHookSource2.pdb"
        DESTINATION "pdb/x64"
    )    
endif()

