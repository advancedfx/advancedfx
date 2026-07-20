cmake_minimum_required (VERSION 3.24)

project ("hlae")

set(hlae_VERSION_MAJOR 2)
set(hlae_VERSION_MINOR 191)
set(hlae_VERSION_PATCH 1)
set(hlae_VERSION_REVISON 0)

file(GLOB_RECURSE HLAE_INPUTS CONFIGURE_DEPENDS
    "${hlae_SOURCE_DIR}/*.cs"
    "${hlae_SOURCE_DIR}/*.csproj"
    "${hlae_SOURCE_DIR}/*.resx"
    "${hlae_SOURCE_DIR}/*.config"
    "${hlae_SOURCE_DIR}/*.ico"
    "${hlae_SOURCE_DIR}/*.settings"
)
list(FILTER HLAE_INPUTS EXCLUDE REGEX "/${hlae_SOURCE_DIR}\/(\.vs|bin|obj)/")

add_custom_command(
    OUTPUT "${hlae_BINARY_DIR}/HLAE.exe"
    WORKING_DIRECTORY ${hlae_BINARY_DIR}
    COMMAND ${VS_MSBUILD} "${hlae_SOURCE_DIR}/hlae.csproj" -r "/p:Configuration=${VS_CONFIGURATION}" "-property:Platform=x86" "/p:OutDir=${hlae_BINARY_DIR}/" "-property:Version=${hlae_VERSION_MAJOR}.${hlae_VERSION_MINOR}.${hlae_VERSION_PATCH}.${hlae_VERSION_REVISON}"
    DEPENDS afxcppcli AfxHookGoldSrc AfxHookSource injector ${HLAE_INPUTS}
    VERBATIM
)

add_custom_target(hlae
    DEPENDS "${hlae_BINARY_DIR}/HLAE.exe"
)
