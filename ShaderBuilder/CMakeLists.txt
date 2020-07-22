cmake_minimum_required (VERSION 3.8)

project ("shaderbuilder")

add_custom_command(
    WORKING_DIRECTORY ${shaderbuilder_BINARY_DIR}
    OUTPUT "${shaderbuilder_BINARY_DIR}/ShaderBuilder.exe"
    COMMAND ${VS16_MSBUILD} "${CMAKE_CURRENT_SOURCE_DIR}/ShaderBuilder.csproj" "-property:Configuration=Release" "-property:Platform=x64" "/p:OutDir=${shaderbuilder_BINARY_DIR}/"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hlae
    COMMAND ${CMAKE_COMMAND} -E touch "${shaderbuilder_BINARY_DIR}/ShaderBuilder.stamp"
    BYPRODUCTS "${shaderbuilder_BINARY_DIR}/HLAE.ShaderBuilder"
)

add_custom_target(shaderbuilder
    DEPENDS "${shaderbuilder_BINARY_DIR}/ShaderBuilder.exe"
)
