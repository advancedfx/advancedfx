cmake_minimum_required (VERSION 3.8)

project ("injector")


add_custom_target(injector
    WORKING_DIRECTORY ${injector_BINARY_DIR}
    BYPRODUCTS "${injector_BINARY_DIR}/injector.exe"
    COMMAND ${VS16_MSBUILD} "${injector_SOURCE_DIR}/injector.csproj" "-property:Configuration=Release" "-property:Platform=x86" "/p:OutDir=${injector_BINARY_DIR}/"
)

add_custom_target(injector_x64_dir
    WORKING_DIRECTORY ${injector_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory x64
)

add_custom_target(injector_x64
    DEPENDS injector_x64_dir
    WORKING_DIRECTORY "${injector_BINARY_DIR}/x64"
    BYPRODUCTS "${injector_BINARY_DIR}/x64/injector.exe"
    COMMAND ${VS16_MSBUILD} "${injector_SOURCE_DIR}/injector.csproj" "-property:Configuration=Release" "-property:Platform=x64" "/p:OutDir=${injector_BINARY_DIR}/x64/"
)
