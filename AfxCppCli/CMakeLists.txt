cmake_minimum_required (VERSION 3.24)

project ("afxcppcli")

file(GLOB_RECURSE AFXCPPCLI_INPUTS CONFIGURE_DEPENDS
    "${afxcppcli_SOURCE_DIR}/*.cpp"
    "${afxcppcli_SOURCE_DIR}/*.h"
    "${afxcppcli_SOURCE_DIR}/*.resx"
    "${afxcppcli_SOURCE_DIR}/*.vcxproj"
    "${afxcppcli_SOURCE_DIR}/*.vcxproj.filters"
)
list(FILTER AFXCPPCLI_INPUTS EXCLUDE REGEX "/${afxcppcli_SOURCE_DIR}\/(\.vs|bin|obj)/")

add_custom_command(
    OUTPUT "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
    WORKING_DIRECTORY ${afxcppcli_BINARY_DIR}
    COMMAND ${VS_MSBUILD} "${afxcppcli_SOURCE_DIR}/AfxCppCli.vcxproj" "/p:Configuration=${VS_CONFIGURATION}" "/p:Platform=Win32" "/p:OutDir=${afxcppcli_BINARY_DIR}/"
    DEPENDS ${AFXCPPCLI_INPUTS}
    VERBATIM
)

add_custom_target(afxcppcli
    DEPENDS "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
)

