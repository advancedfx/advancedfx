cmake_minimum_required (VERSION 3.8)

project ("hlae")


add_custom_command(
    DEPENDS afxhook afxhook_x64 afxcppcli afxhookgoldsrc afxhooksource injector injector_x64
    WORKING_DIRECTORY ${hlae_BINARY_DIR}
    OUTPUT "${hlae_BINARY_DIR}/HLAE.exe"
    COMMAND ${VS16_MSBUILD} "${hlae_SOURCE_DIR}/hlae.csproj" "-property:Configuration=Release" "-property:Platform=x86" "/p:OutDir=${hlae_BINARY_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E touch "${hlae_BINARY_DIR}/HLAE.stamp"
    BYPRODUCTS "${hlae_BINARY_DIR}/HLAE.stamp"
)

add_custom_target(hlae
    DEPENDS "${hlae_BINARY_DIR}/HLAE.exe"
)
