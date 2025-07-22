cmake_minimum_required (VERSION 3.24)

project ("shaderbuilder")

add_custom_target(shaderbuilder
    WORKING_DIRECTORY ${shaderbuilder_BINARY_DIR}
    BYPRODUCTS "${shaderbuilder_BINARY_DIR}/shaderbuilder.exe"
    COMMAND ${VS_MSBUILD} "${shaderbuilder_SOURCE_DIR}/shaderbuilder.csproj" -r "/p:Configuration=${VS_CONFIGURATION}" "-property:Platform=x64" "/p:OutDir=${shaderbuilder_BINARY_DIR}/"
)
