cmake_minimum_required (VERSION 3.8)

project ("protobuf")

#
# protobuf
#

FetchContent_Declare(
	protobuf
	GIT_REPOSITORY https://github.com/advancedfx/protobuf.git
    GIT_TAG v3.13.0
)

FetchContent_GetProperties(protobuf)
if(NOT openexr_POPULATED)
  FetchContent_Populate(protobuf)
  add_custom_target(protobuf)
endif()

set(PROTOBUF_BUILD_BINARIES
	"${protobuf_BINARY_DIR}/${CMAKE_CFG_INTDIR}/libprotobuf$<$<CONFIG:Debug>:d>.lib"
	"${protobuf_BINARY_DIR}/${CMAKE_CFG_INTDIR}/protoc.exe"
)

set(protobuf_BINARY_DIR ${protobuf_BINARY_DIR} PARENT_SCOPE)
set(protobuf_SOURCE_DIR ${protobuf_SOURCE_DIR} PARENT_SCOPE)

add_custom_command(
    WORKING_DIRECTORY ${protobuf_BINARY_DIR}
	OUTPUT ${PROTOBUF_BUILD_BINARIES}
    DEPENDS protobuf
    COMMAND ${CMAKE_COMMAND} "-DCMAKE_INSTALL_PREFIX=${protobuf_BINARY_DIR}/install" -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF -G "Visual Studio 16 2019" -T "v142" -A "Win32"  "${protobuf_SOURCE_DIR}/cmake"
    COMMAND ${VS16_MSBUILD} "libprotobuf.vcxproj" "/p:Configuration=${VS16_CONFIGURATION}" "/p:Platform=Win32"
    COMMAND ${VS16_MSBUILD} "protoc.vcxproj" "/p:Configuration=${VS16_CONFIGURATION}" "/p:Platform=Win32"
)

add_custom_target(protobuf_build
    DEPENDS ${PROTOBUF_BUILD_BINARIES}
)
