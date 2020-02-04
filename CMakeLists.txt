# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("advancedfx")

include(FetchContent)

add_subdirectory("AfxHook")

#
# Get MSBUILD path:
#

execute_process(
    COMMAND "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" "-latest" "-version" "[16.0,17.0)" "-requires" "Microsoft.Component.MSBuild" "-find" "MSBuild\\**\\**\\Bin\\MSBuild.exe"
    OUTPUT_VARIABLE VS16_MSBUILD
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#
# Update translation files
#

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/l10n.stamp
    COMMAND ${CMAKE_SOURCE_DIR}/l10n_update_to_source.bat
    COMMAND ${CMAKE_COMMAND} -E touch l10n.stamp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(l10n_update
    DEPENDS ${CMAKE_BINARY_DIR}/l10n.stamp
)

#
# zlib
# 

FetchContent_Declare(
	zlib
	URL http://zlib.net/zlib-1.2.11.tar.gz
	URL_HASH SHA256=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)
  add_custom_target(zlib)
endif()

#
# OpenEXR
#

FetchContent_Declare(
	openexr
	GIT https://github.com/advancedfx/openexr.git
    GIT_TAG v2.4.0
)

FetchContent_GetProperties(openexr)
if(NOT openexr_POPULATED)
  FetchContent_Populate(openexr)
  add_custom_command(
     OUTPUT 
  )

  add_subdirectory(${openexr_SOURCE_DIR} ${openexr_BINARY_DIR})
endif()


#
# HLAE Project
#

add_custom_command(
    DEPENDS l10n_update
    OUTPUT ${CMAKE_BINARY_DIR}/HLAE.exe
    COMMAND ${VS16_MSBUILD} "hlae.csproj" "-property:Configuration=Release" "/p:OutputPath=${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hlae
)

add_custom_target(hlae
    DEPENDS ${CMAKE_BINARY_DIR}/HLAE.exe
)

#
# advancedfx
#

add_custom_target(advancedfx
	DEPENDS afxhook
)

# Build / copy resources:

add_custom_command(
    TARGET advancedfx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/AfxHook/AfxHook_586.dat ${CMAKE_CURRENT_BINARY_DIR}/AfxHook.dat
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/AfxHook/AfxHook_x64.dat ${CMAKE_CURRENT_BINARY_DIR}/x64/AfxHook.dat
    COMMAND ${CMAKE_SOURCE_DIR}/copy_resources_release.bat
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
