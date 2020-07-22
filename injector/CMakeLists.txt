cmake_minimum_required (VERSION 3.8)

project ("injector")


add_custom_command(
    WORKING_DIRECTORY ${injector_BINARY_DIR}
    OUTPUT "${injector_BINARY_DIR}/injector.exe"
    COMMAND ${VS16_MSBUILD} "${injector_SOURCE_DIR}/injector.csproj" "-property:Configuration=Release" "-property:Platform=x86" "/p:OutDir=${injector_BINARY_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E touch "${injector_BINARY_DIR}/injector.stamp"
    BYPRODUCTS "${injector_BINARY_DIR}/injector.stamp"
)

add_custom_target(injector
    DEPENDS "${injector_BINARY_DIR}/injector.exe"
)


add_custom_target(injector_x64_dir
    WORKING_DIRECTORY ${injector_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory x64
)

add_custom_command(
    DEPENDS injector_x64_dir
    WORKING_DIRECTORY "${injector_BINARY_DIR}/x64"
    OUTPUT "${injector_BINARY_DIR}/x64/injector.exe"
    COMMAND ${VS16_MSBUILD} "${injector_SOURCE_DIR}/injector.csproj" "-property:Configuration=Release" "-property:Platform=x64" "/p:OutDir=${injector_BINARY_DIR}/x64/"
    COMMAND ${CMAKE_COMMAND} -E touch "${injector_BINARY_DIR}/x64/injector.stamp"
    BYPRODUCTS "${injector_BINARY_DIR}/x64/injector.stamp"
)

add_custom_target(injector_x64
    DEPENDS "${injector_BINARY_DIR}/x64/injector.exe"
)
