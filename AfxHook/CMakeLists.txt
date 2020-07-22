cmake_minimum_required (VERSION 3.8)

project ("afxhook")

#
# NASM
#

include(FetchContent)

FetchContent_Declare(
	nasm
	URL https://github.com/advancedfx/nasm/releases/download/v2.14.02/nasm-2.14.02-win64.zip https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/win64/nasm-2.14.02-win64.zip
	URL_HASH SHA512=D1C691DF380F53E5D620CC82E607BF78427817BE588B90617268B6D7253CA06CFA98A893F5BCA4C6D325C1A44B51C208719F8113DCF25207DBD51F33098D7051
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
