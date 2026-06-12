cmake_minimum_required (VERSION 3.24)

project ("shaderbuilder")

file(GLOB_RECURSE SHADERBUILDER_INPUTS CONFIGURE_DEPENDS
    "${shaderbuilder_SOURCE_DIR}/*.cs"
    "${shaderbuilder_SOURCE_DIR}/*.csproj"
    "${shaderbuilder_SOURCE_DIR}/*.config"
)
list(FILTER SHADERBUILDER_INPUTS EXCLUDE REGEX "/${shaderbuilder_SOURCE_DIR}\/(bin|obj)/")

add_custom_command(
    OUTPUT "${shaderbuilder_BINARY_DIR}/ShaderBuilder.exe"
    WORKING_DIRECTORY ${shaderbuilder_BINARY_DIR}
    COMMAND ${VS_MSBUILD} "${shaderbuilder_SOURCE_DIR}/shaderbuilder.csproj" -r "/p:Configuration=${VS_CONFIGURATION}" "-property:Platform=x64" "/p:OutDir=${shaderbuilder_BINARY_DIR}/"
    DEPENDS ${SHADERBUILDER_INPUTS}
    VERBATIM
)

add_custom_target(shaderbuilder
    DEPENDS "${shaderbuilder_BINARY_DIR}/ShaderBuilder.exe"
)
