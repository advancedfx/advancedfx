cmake_minimum_required (VERSION 3.8)

project ("afxhooksource")

set(AFXHOOKSOURCE_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_vs20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_vs20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_ps20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_depth_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_depth_ps20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps20.h"
)

set(AFXHOOKSOURCE_SHADERS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_vs20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_ps20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_vs20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_ps20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_depth_ps20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_ps20.fxc"
)

add_custom_command(
    WORKING_DIRECTORY ${shaderbuilder_BINARY_DIR}
    OUTPUT ${AFXHOOKSOURCE_SHADERS}
    DEPENDS shaderbuilder ${AFXHOOKSOURCE_SHADERS_SOURCES}
    COMMAND ShaderBuilder.exe --profile vs_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_vs20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps20"
	COMMAND ShaderBuilder.exe --profile vs_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_vs20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_vs20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_ps20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_depth_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_depth_ps20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps20"	
)

add_custom_target(afxhooksourceshaders
    DEPENDS ${AFXHOOKSOURCE_SHADERS}
)

set(AFXHOOKSOURCE_PROTOBUFS
	"${CMAKE_CURRENT_SOURCE_DIR}/../build/protobuf/csgo/netmessages.pb.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../build/protobuf/csgo/netmessages.pb.h"
)

set(AFXHOOKSOURCE_PROTOBUF_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../deps/release/prop/protobuf/csgo/netmessages.proto"
)

add_custom_command(
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build"
    OUTPUT ${AFXHOOKSOURCE_PROTOBUFS}
    DEPENDS protobuf_build ${AFXHOOKSOURCE_PROTOBUF_SOURCES}
	COMMAND "${CMAKE_COMMAND}" -E make_directory "protobuf/csgo"
    COMMAND "${protobuf_BINARY_DIR}/Release/protoc.exe" "--proto_path=${protobuf_SOURCE_DIR}/src" "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../deps/release/prop/protobuf/csgo" "--cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/../build/protobuf/csgo" "${CMAKE_CURRENT_SOURCE_DIR}/../deps/release/prop/protobuf/csgo/netmessages.proto"
)

add_custom_target(afxhooksourceprotobufs
    DEPENDS ${AFXHOOKSOURCE_PROTOBUFS}
)

separate_arguments(AFXHOOKSOURCE_ARGS)

add_custom_target(afxhooksource
    DEPENDS ilmbase_build openexr_build afxhooksourceshaders afxhooksourceprotobufs afx
    WORKING_DIRECTORY ${afxhooksource_BINARY_DIR}
    BYPRODUCTS "${afxhooksource_BINARY_DIR}/AfxHookSource.dll"
    COMMAND "${VS16_MSBUILD}" "${afxhooksource_SOURCE_DIR}/AfxHookSource.vcxproj" "/p:Configuration=${VS16_CONFIGURATION}" "/p:Platform=Win32" "/p:OutDir=${afxhooksource_BINARY_DIR}/" "/p:ILMBASE_BUILD_DIR=${ILMBASE_BUILD_DIR}" "/p:OPENEXR_BUILD_DIR=${OPENEXR_BUILD_DIR}" "/p:PROTOBUF_BINARY_DIR=${protobuf_BINARY_DIR}" "/p:PROTOBUF_SOURCE_DIR=${protobuf_SOURCE_DIR}"
	VERBATIM
)
