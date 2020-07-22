cmake_minimum_required (VERSION 3.8)

project ("afxhookgoldsrc")

add_custom_command(
    WORKING_DIRECTORY ${afxhookgoldsrc_BINARY_DIR}
    OUTPUT "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.dll"
    DEPENDS ilmbase_build openexr_build
    COMMAND ${VS16_MSBUILD} "${CMAKE_CURRENT_SOURCE_DIR}/AfxHookGoldSrc.vcxproj" "/p:Configuration=Release" "/p:Platform=Win32" "/p:OutDir=${afxhookgoldsrc_BINARY_DIR}/" "/p:ILMBASE_BUILD_DIR=${ILMBASE_BUILD_DIR};OPENEXR_BUILD_DIR=${OPENEXR_BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.stamp"
    BYPRODUCTS "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.stamp"
)

add_custom_target(afxhookgoldsrc
    DEPENDS "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.dll"
)
