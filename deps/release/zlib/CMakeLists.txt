cmake_minimum_required (VERSION 3.8)

project ("zlib")

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
endif()

set(zlib_SOURCE_DIR ${zlib_SOURCE_DIR} PARENT_SCOPE)

add_custom_command(
    WORKING_DIRECTORY "${zlib_SOURCE_DIR}"
	OUTPUT "${zlib_SOURCE_DIR}/zlib1.dll"
    COMMAND "${VS16_INSTALLPATH}\\VC\\Auxiliary\\Build\\vcvarsall.bat" "x86" "&&" "nmake" "-f" "win32/Makefile.msc" "zlib1.dll"
	USES_TERMINAL
	VERBATIM
    COMMAND ${CMAKE_COMMAND} -E touch "${zlib_SOURCE_DIR}/zlib.stamp"
    BYPRODUCTS "${zlib_SOURCE_DIR}/zlib.stamp"
)

add_custom_target(zlib_build
	DEPENDS "${zlib_SOURCE_DIR}/zlib1.dll"
)
