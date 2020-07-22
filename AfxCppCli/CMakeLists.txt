cmake_minimum_required (VERSION 3.8)

project ("afxcppcli")

add_custom_command(
    WORKING_DIRECTORY ${afxcppcli_BINARY_DIR}
    OUTPUT "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
    COMMAND ${VS16_MSBUILD} "${afxcppcli_SOURCE_DIR}/AfxCppCli.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32" "/p:OutDir=${afxcppcli_BINARY_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E touch "${afxcppcli_BINARY_DIR}/AfxCppCli.stamp"
    BYPRODUCTS "${afxcppcli_BINARY_DIR}/AfxCppCli.stamp"
)

add_custom_target(afxcppcli
    DEPENDS "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
)

