cmake_minimum_required (VERSION 3.8)

project ("openexr")

#
# OpenEXR
#

FetchContent_Declare(
	openexr
	GIT_REPOSITORY https://github.com/advancedfx/openexr.git
    GIT_TAG v2.5.2
)

FetchContent_GetProperties(openexr)
if(NOT openexr_POPULATED)
  FetchContent_Populate(openexr)
  add_custom_target(openexr)
endif()

set(ILMBASE_TMP_DIR "${openexr_BINARY_DIR}/tmp-ilmbase")
set(OPENEXR_TMP_DIR "${openexr_BINARY_DIR}/tmp-openexr")

set(ILMBASE_BUILD_DIR "${openexr_BINARY_DIR}/ilmbase-build")
set(OPENEXR_BUILD_DIR "${openexr_BINARY_DIR}/openexr-build")

set(ILMBASE_BUILD_DIR ${ILMBASE_BUILD_DIR} PARENT_SCOPE)
set(OPENEXR_BUILD_DIR ${OPENEXR_BUILD_DIR} PARENT_SCOPE)


add_custom_target(openexr_build_dirs
    DEPENDS openexr
    WORKING_DIRECTORY ${openexr_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ILMBASE_TMP_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OPENEXR_TMP_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ILMBASE_BUILD_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OPENEXR_BUILD_DIR}
)

set(ILMBASE_BUILD_BINARIES
	"${ILMBASE_BUILD_DIR}/bin/Half-2_5.dll"
	"${ILMBASE_BUILD_DIR}/bin/Iex-2_5.dll"
	"${ILMBASE_BUILD_DIR}/bin/IexMath-2_5.dll"
	"${ILMBASE_BUILD_DIR}/bin/IlmThread-2_5.dll"
	"${ILMBASE_BUILD_DIR}/bin/Imath-2_5.dll"
)


add_custom_command(
    WORKING_DIRECTORY ${ILMBASE_TMP_DIR}
    OUTPUT ${ILMBASE_BUILD_BINARIES}
    DEPENDS openexr openexr_build_dirs
    COMMAND ${CMAKE_COMMAND} "-DBUILD_SHARED_LIBS=1" "-DCMAKE_INSTALL_PREFIX=${ILMBASE_BUILD_DIR}" -G "Visual Studio 16 2019" -T "v142" -A "Win32" "${openexr_SOURCE_DIR}/IlmBase"
    COMMAND ${VS16_MSBUILD} "ALL_BUILD.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32"
    COMMAND ${VS16_MSBUILD} "INSTALL.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32"
)

add_custom_target(ilmbase_build
    DEPENDS ${ILMBASE_BUILD_BINARIES}
)

set(OPENEXR_BUILD_BINARIES
    "${OPENEXR_BUILD_DIR}/bin/IlmImf-2_5.dll"
)

add_custom_command(
    WORKING_DIRECTORY ${OPENEXR_TMP_DIR}
    OUTPUT ${OPENEXR_BUILD_BINARIES}
    DEPENDS openexr openexr_build_dirs ilmbase_build zlib_build
    COMMAND ${CMAKE_COMMAND} "-DBUILD_SHARED_LIBS=1" "-DZLIB_ROOT=${zlib_SOURCE_DIR}" "-DCMAKE_PREFIX_PATH=${ILMBASE_BUILD_DIR}" "-DCMAKE_SYSTEM_PREFIX=${ILMBASE_BUILD_DIR}" "-DCMAKE_INSTALL_PREFIX=${OPENEXR_BUILD_DIR}" -G "Visual Studio 16 2019" -T "v142" -A "Win32" "${openexr_SOURCE_DIR}/OpenEXR"
    COMMAND ${VS16_MSBUILD} "ALL_BUILD.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32"
    COMMAND ${VS16_MSBUILD} "INSTALL.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32"
)

add_custom_target(openexr_build
    DEPENDS ${OPENEXR_BUILD_BINARIES}
)