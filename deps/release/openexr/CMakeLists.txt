cmake_minimum_required (VERSION 3.24)

project ("openexr")

#
# OpenEXR
#

set(IMATH_INSTALL Off) # this one is important, otherwise it OpenEXR Imath build shits us a DLL into installer /bin folder that will overwrite our 32 bit DLL when merging in install step.
set(OPENEXR_INSTALL Off)
set(OPENEXR_BUILD_LIBS On)
set(OPENEXR_BUILD_TOOLS Off)
set(OPENEXR_INSTALL_TOOLS Off)
set(OPENEXR_BUILD_EXAMPLES Off)
set(OPENEXR_TEST_LIBRARIES On)
set(OPENEXR_TEST_TOOLS Off)
set(OPENEXR_TEST_PYTHON Off)
set(BUILD_SHARED_LIBS On)

# (We add Imath outselves to work around a bug in OpenEXR x64 Debug builds.)
FetchContent_Declare(Imath
	GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/Imath.git
	GIT_TAG c0396a055a01bc537d32f435aee11a9b7ed6f0b5 #v3.1.12
	GIT_SHALLOW ON
)

FetchContent_Declare(
	openexr
	GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openexr.git
    GIT_TAG 6661cb40c6fa8d332f142e660e25ac96675c15bd #v3.3.5
)

FetchContent_MakeAvailable(Imath)
FetchContent_MakeAvailable(openexr)

set(AFX_OPENEXR_INCLUDE_DIRECTORIES
    ${Imath_SOURCE_DIR}/src/Imath
    ${Imath_BINARY_DIR}/config
    ${OpenEXR_SOURCE_DIR}/src/lib/Iex
    ${OpenEXR_SOURCE_DIR}/src/lib/OpenEXR
    ${OpenEXR_BINARY_DIR}/cmake
)
set(AFX_OPENEXR_INCLUDE_DIRECTORIES ${AFX_OPENEXR_INCLUDE_DIRECTORIES} PARENT_SCOPE)

set(AFX_OPENEXR_LINK_DIRECTORIES
	${OpenEXR_BINARY_DIR}/src/lib
)
set(AFX_OPENEXR_LINK_DIRECTORIES ${AFX_OPENEXR_LINK_DIRECTORIES} PARENT_SCOPE)

set(AFX_OPENEXR_LINK_LIBRARIES
	${OpenEXR_BINARY_DIR}/src/lib/OpenEXR/$<CONFIG>/OpenEXR-3_3$<$<CONFIG:Debug>:_d>.lib
)
set(AFX_OPENEXR_LINK_LIBRARIES ${AFX_OPENEXR_LINK_LIBRARIES} PARENT_SCOPE)

set(AFX_OPENEXR_INSTALL_BINARIES
	${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Iex-3_3$<$<CONFIG:Debug>:_d>.dll
	${CMAKE_BINARY_DIR}/bin/$<CONFIG>/IlmThread-3_3$<$<CONFIG:Debug>:_d>.dll
	${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Imath-3_1$<$<CONFIG:Debug>:_d>.dll
	${CMAKE_BINARY_DIR}/bin/$<CONFIG>/OpenEXR-3_3$<$<CONFIG:Debug>:_d>.dll
	${CMAKE_BINARY_DIR}/bin/$<CONFIG>/OpenEXRCore-3_3$<$<CONFIG:Debug>:_d>.dll
)
set(AFX_OPENEXR_INSTALL_BINARIES ${AFX_OPENEXR_INSTALL_BINARIES} PARENT_SCOPE)