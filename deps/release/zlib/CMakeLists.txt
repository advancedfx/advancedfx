cmake_minimum_required (VERSION 3.24)

# default DOWNLOAD_EXTRACT_TIMESTAMP to true
cmake_policy(SET CMP0135 NEW)

project ("zlib")

#
# zlib
# 

FetchContent_Declare(
	zlib
	URL http://www.zlib.net/zlib-1.3.1.tar.gz
	URL_HASH SHA256=9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23
)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)
endif()

set(zlib_SOURCE_DIR ${zlib_SOURCE_DIR} PARENT_SCOPE)

if ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
	set(zlib_VCVARSALL_PLATFORM "x86")
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
	set(zlib_VCVARSALL_PLATFORM "x64")
endif ()

add_custom_command(
    WORKING_DIRECTORY "${zlib_SOURCE_DIR}"
	OUTPUT "${zlib_SOURCE_DIR}/zlib1.dll"
    COMMAND "${VS_INSTALLPATH}\\VC\\Auxiliary\\Build\\vcvarsall.bat" "${zlib_VCVARSALL_PLATFORM}" "&&" "nmake" "-f" "win32/Makefile.msc" "zlib1.dll"
	USES_TERMINAL
	VERBATIM
    COMMAND ${CMAKE_COMMAND} -E touch "${zlib_SOURCE_DIR}/zlib.stamp"
    BYPRODUCTS "${zlib_SOURCE_DIR}/zlib.stamp"
)

add_custom_target(zlib_build
	DEPENDS "${zlib_SOURCE_DIR}/zlib1.dll"
)
