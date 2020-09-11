cmake_minimum_required (VERSION 3.8)

project ("afxhook")

#
# NASM
#

include(FetchContent)

FetchContent_Declare(
	nasm
	URL https://github.com/advancedfx/nasm-mirror/releases/download/nasm-2.15.05/nasm-2.15.05-win64.zip https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-win64.zip
	URL_HASH SHA512=0d0469f3d475f3d192b9b5b7ee74662cbcb9e02efa0d51839c12cbf6f5de5cd58b32e1d12c9ee4381a63e99a4956e03df7decaf1369e598e0edff19a12629073
)

FetchContent_GetProperties(nasm)
if(NOT nasm_POPULATED)
  FetchContent_Populate(nasm)
  add_custom_target(nasm)
endif()

#
# AfxHook
#

add_custom_command(
	WORKING_DIRECTORY ${afxhook_BINARY_DIR}
	OUTPUT "${afxhook_BINARY_DIR}/AfxHook.dat"
	COMMAND ${nasm_SOURCE_DIR}/nasm.exe  -f bin -o "${afxhook_BINARY_DIR}/AfxHook.dat" "${afxhook_SOURCE_DIR}/AfxHook_586.asm"
	DEPENDS nasm
		"${afxhook_SOURCE_DIR}/AfxHook_586.asm"
)

add_custom_target(afxhook
	DEPENDS "${afxhook_BINARY_DIR}/AfxHook.dat"
)


add_custom_target(afxhook_x64_dir
    WORKING_DIRECTORY ${afxhook_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory x64
)

add_custom_command(
	WORKING_DIRECTORY "${afxhook_BINARY_DIR}/x64"
	OUTPUT "${afxhook_BINARY_DIR}/x64/AfxHook.dat"
	COMMAND ${nasm_SOURCE_DIR}/nasm.exe  -f bin -o "${afxhook_BINARY_DIR}/x64/AfxHook.dat" "${afxhook_SOURCE_DIR}/AfxHook_x64.asm"
	DEPENDS
		nasm afxhook_x64_dir
		"${afxhook_SOURCE_DIR}/AfxHook_x64.asm"
)

add_custom_target(afxhook_x64
	DEPENDS "${afxhook_BINARY_DIR}/x64/AfxHook.dat"
)
