cmake_minimum_required (VERSION 3.24)

# default DOWNLOAD_EXTRACT_TIMESTAMP to true
cmake_policy(SET CMP0135 NEW)

project ("zlib")

#
# zlib
# 

FetchContent_Declare(
	zlib
	URL http://zlib.net/zlib-1.2.13.tar.gz
	URL_HASH SHA256=b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30
)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)
endif()

set(zlib_SOURCE_DIR ${zlib_SOURCE_DIR} PARENT_SCOPE)

add_custom_command(
    WORKING_DIRECTORY "${zlib_SOURCE_DIR}"
	OUTPUT "${zlib_SOURCE_DIR}/zlib1.dll"
    COMMAND "${VS_INSTALLPATH}\\VC\\Auxiliary\\Build\\vcvarsall.bat" "x86" "&&" "nmake" "-f" "win32/Makefile.msc" "zlib1.dll"
	USES_TERMINAL
	VERBATIM
    COMMAND ${CMAKE_COMMAND} -E touch "${zlib_SOURCE_DIR}/zlib.stamp"
    BYPRODUCTS "${zlib_SOURCE_DIR}/zlib.stamp"
)

add_custom_target(zlib_build
	DEPENDS "${zlib_SOURCE_DIR}/zlib1.dll"
)
