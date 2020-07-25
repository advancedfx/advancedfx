cmake_minimum_required (VERSION 3.8)

project ("afxcppcli")

add_custom_target(afxcppcli
    WORKING_DIRECTORY ${afxcppcli_BINARY_DIR}
    BYPRODUCTS "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
    COMMAND ${VS16_MSBUILD} "${afxcppcli_SOURCE_DIR}/AfxCppCli.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32" "/p:OutDir=${afxcppcli_BINARY_DIR}/"
)

