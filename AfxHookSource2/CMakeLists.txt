cmake_minimum_required (VERSION 3.24)

project ("AfxHookSource2")

################################################################################

set(AFXHOOKSOURCE2_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs_5_0.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs_5_0.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps_5_0.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps_5_0.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_vs_5_0.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_vs_5_0.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps_5_0.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps_5_0.h"
)

set(AFXHOOKSOURCE2_SHADERS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_vs_5_0.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_ps_5_0.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_vs_5_0.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_ps_5_0.fxc"
)

add_custom_command(
    WORKING_DIRECTORY ${shaderbuilder_BINARY_DIR}
    OUTPUT ${AFXHOOKSOURCE2_SHADERS}
    DEPENDS shaderbuilder ${AFXHOOKSOURCE2_SHADERS_SOURCES}
    COMMAND ShaderBuilder.exe --profile vs_5_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_vs_5_0.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs_5_0"
	COMMAND ShaderBuilder.exe --profile ps_5_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_ps_5_0.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps_5_0"
    COMMAND ShaderBuilder.exe --profile vs_5_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_vs_5_0.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_vs_5_0"
	COMMAND ShaderBuilder.exe --profile ps_5_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_ps_5_0.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps_5_0"
)

add_custom_target(AfxHookSource2Shaders
    DEPENDS ${AFXHOOKSOURCE2_SHADERS}
)


################################################################################

add_library(${PROJECT_NAME} MODULE)

add_dependencies(${PROJECT_NAME}
    ilmbase_build
    openexr_build
    AfxHookSource2Shaders
)

add_definitions(-DGAME_CS2)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../AfxHookSource2Rs/Cargo.toml)

if(${CMAKE_BUILD_TYPE_DEBUG})
    #TODO: Not quite fixed yet: https://corrosion-rs.github.io/corrosion/common_issues.html
    corrosion_set_env_vars(afx_hook_source2_rs "CFLAGS=-MDd" "CXXFLAGS=-MDd")
endif()

#set_target_properties(
#    ${PROJECT_NAME}
#    PROPERTIES 
#        OUTPUT_NAME "${PROJECT_NAME}_IF_YOU_USE_THIS_TO_JOIN_GOTV_OR_VAC_SERVERS_YOU_WILL_GET_VAC_BANNED"
#)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

# - generate PDB in Release too
# - enable Hot Reload in Debug
target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Zi>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/ZI>
	)
target_link_options(${PROJECT_NAME} PRIVATE
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/DEBUG /OPT:REF /OPT:ICF>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/INCREMENTAL>
	)

target_include_directories(${PROJECT_NAME} PRIVATE
    ./
    ${OPENEXR_BUILD_DIR}/include/OpenEXR
    ${ILMBASE_BUILD_DIR}/include/OpenEXR    
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${OPENEXR_BUILD_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    IlmImf-2_5$<$<CONFIG:Debug>:_d>.lib
    afx_hook_source2_rs
    bcrypt.lib # corrosion fails to detect this
)

target_sources(${PROJECT_NAME} PRIVATE
    ../deps/release/Detours/src/detours.cpp
    ../deps/release/Detours/src/detours.h
    ../deps/release/Detours/src/detver.h
    ../deps/release/Detours/src/disasm.cpp
    ../deps/release/Detours/src/disolarm.cpp
    ../deps/release/Detours/src/disolarm64.cpp
    ../deps/release/Detours/src/disolia64.cpp
    ../deps/release/Detours/src/disolx64.cpp
    ../deps/release/Detours/src/disolx86.cpp
    ../deps/release/Detours/src/image.cpp
    ../deps/release/Detours/src/modules.cpp

    ../deps/release/prop/cs2/sdk_src/public/appframework/IAppSystem.h
    ../deps/release/prop/cs2/sdk_src/public/interfaces/interfaces.h
    ../deps/release/prop/cs2/sdk_src/public/tier0/memalloc.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/convar.cpp
    ../deps/release/prop/cs2/sdk_src/public/tier1/convar.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/iconvar.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/KeyValues.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/strtools.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/utlmemory.cpp
    ../deps/release/prop/cs2/sdk_src/public/tier1/utlmemory.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/utlstring.cpp
    ../deps/release/prop/cs2/sdk_src/public/tier1/utlstring.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/utlvector.h
    ../deps/release/prop/cs2/sdk_src/public/vstdlib/IKeyValuesSystem.h
    ../deps/release/prop/cs2/sdk_src/public/cdll_int.h    
    ../deps/release/prop/cs2/sdk_src/public/const.h    
    ../deps/release/prop/cs2/sdk_src/public/entityhandle.h    
    ../deps/release/prop/cs2/sdk_src/public/icvar.h    
    ../deps/release/prop/cs2/sdk_src/public/igameevents.h    
    ../deps/release/prop/cs2/sdk_src/public/igameuiservice.h    
    ../deps/release/prop/cs2/sdk_src/tier1/KeyValues.cpp
    ../deps/release/prop/AfxHookSource/SourceSdkShared.cpp
    ../deps/release/prop/AfxHookSource/SourceSdkShared.h
    ../deps/release/prop/AfxHookSource/SourceInterfaces.h
    
    ../shared/AfxCommandLine.cpp
    ../shared/AfxCommandLine.h
    ../shared/AfxConsole.cpp
    ../shared/AfxConsole.h
    ../shared/AfxDetours.cpp
    ../shared/AfxDetours.h
    ../shared/AfxMath.cpp
    ../shared/AfxMath.h
    ../shared/AfxImageBuffer.h    
    ../shared/AfxOutStreams.cpp
    ../shared/AfxOutStreams.h    
    ../shared/CamIO.cpp
    ../shared/CamIO.h
    ../shared/CamPath.cpp
    ../shared/CamPath.h
    ../shared/CamPathRs.cpp
    ../shared/CommandSystem.cpp
    ../shared/CommandSystem.h
    ../shared/ConsolePrinter.h
    ../shared/EasySampler.cpp
    ../shared/EasySampler.h
    ../shared/FFITools.h
    ../shared/FileTools.cpp
    ../shared/FileTools.h
    ../shared/FovScaling.cpp
    ../shared/FovScaling.h
    ../shared/ImageBufferPoolThreadSafe.h
    ../shared/ImageTransformer.cpp
    ../shared/ImageTransformer.h    
    ../shared/OutVideoStreamCreators.cpp
    ../shared/OutVideoStreamCreators.h
    ../shared/OpenExrOutput.cpp
    ../shared/OpenExrOutput.h    
    ../shared/RawOutput.cpp
    ../shared/RawOutput.h    
    ../shared/RecordingSettings.cpp
    ../shared/RecordingSettings.h    
    ../shared/StringTools.cpp
    ../shared/StringTools.h
    ../shared/ThreadPool.h
    ../shared/binutils.cpp
    ../shared/binutils.h
    ../shared/MirvCamIO.cpp
    ../shared/MirvCamIO.h
    ../shared/MirvCampath.cpp
    ../shared/MirvCampath.h
	../shared/MirvDeathMsgFilter.cpp
	../shared/MirvDeathMsgFilter.h
    ../shared/MirvInput.cpp
    ../shared/MirvInput.h
    ../shared/MirvSkip.cpp
    ../shared/MirvSkip.h

    AfxHookSource2Rs.cpp
    AfxHookSource2Rs.h

    AfxShaders.cpp
    AfxShaders.h

	CamIO.cpp
	CamIO.h

    CampathDrawer.cpp
    CampathDrawer.h

    ClientEntitySystem.cpp
    ClientEntitySystem.h

    ConsoleRs.cpp

	DeathMsg.cpp
	DeathMsg.h

    GameEvents.cpp
    GameEvents.h

	Globals.cpp
	Globals.h

    hlaeFolder.cpp
    hlaeFolder.h
	
	ViewModel.cpp
	ViewModel.h

    main.cpp

    RenderSystemDX11Hooks.cpp
    RenderSystemDX11Hooks.h

    ReShadeAdvancedfx.cpp
    ReShadeAdvancedfx.h

    WrpConsole.cpp
    WrpConsole.h
)

# This is far form proper, but it will do for now:
source_group("deps/release/Detours" REGULAR_EXPRESSION "/deps/release/Detours/")
source_group("deps/release/Detours/src" REGULAR_EXPRESSION "/deps/release/Detours/src/")
source_group("deps/release/prop" REGULAR_EXPRESSION "/deps/release/prop/")
source_group("deps/release/prop/AfxHookSource" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource")
source_group("deps/release/prop/cs2" REGULAR_EXPRESSION "/deps/release/prop/cs2/")
source_group("deps/release/prop/cs2/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/")
source_group("deps/release/prop/cs2/sdk_src/public/appframework" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/appframework/")
source_group("deps/release/prop/cs2/sdk_src/public/interfaces" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/interfaces/")
source_group("deps/release/prop/cs2/sdk_src/public/tier0" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/tier0/")
source_group("deps/release/prop/cs2/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/tier1/")


source_group("shared" REGULAR_EXPRESSION "/shared/")
source_group("shared/hooks" REGULAR_EXPRESSION "/shared/hooks/")
source_group("shared/vcpp" REGULAR_EXPRESSION "/shared/vcpp/")
source_group("AfxHookSource2" REGULAR_EXPRESSION "/AfxHookSource2/")
