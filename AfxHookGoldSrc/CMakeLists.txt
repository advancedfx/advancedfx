cmake_minimum_required (VERSION 3.20)

project ("afxhookgoldsrc")

add_custom_target(afxhookgoldsrc
    WORKING_DIRECTORY ${afxhookgoldsrc_BINARY_DIR}
    BYPRODUCTS "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.dll"
    DEPENDS ilmbase_build openexr_build
    COMMAND ${VS16_MSBUILD} "${CMAKE_CURRENT_SOURCE_DIR}/AfxHookGoldSrc.vcxproj" "/p:Configuration=${VS16_CONFIGURATION}" "/p:Platform=Win32" "/p:OutDir=${afxhookgoldsrc_BINARY_DIR}/" "/p:ILMBASE_BUILD_DIR=${ILMBASE_BUILD_DIR};OPENEXR_BUILD_DIR=${OPENEXR_BUILD_DIR}"
)
