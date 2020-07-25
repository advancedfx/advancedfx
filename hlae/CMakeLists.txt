cmake_minimum_required (VERSION 3.8)

project ("hlae")

add_custom_target(hlae
    DEPENDS afxhook afxhook_x64 afxcppcli afxhookgoldsrc afxhooksource injector injector_x64
    WORKING_DIRECTORY ${hlae_BINARY_DIR}
    BYPRODUCTS "${hlae_BINARY_DIR}/HLAE.exe"
    COMMAND ${VS16_MSBUILD} "${hlae_SOURCE_DIR}/hlae.csproj" "-property:Configuration=Release" "-property:Platform=x86" "/p:OutDir=${hlae_BINARY_DIR}/"
)
