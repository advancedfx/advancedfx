cmake_minimum_required (VERSION 3.20)

project ("AfxHookSource")

set(AFXHOOKSOURCE_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_vs20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_vs20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_ps20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_depth_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_depth_ps20.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps20.acs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps20.h"
)

set(AFXHOOKSOURCE_SHADERS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_vs20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_ps20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_vs20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_ps20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_depth_ps20.fxc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_ps20.fxc"
)

add_custom_command(
    WORKING_DIRECTORY ${shaderbuilder_BINARY_DIR}
    OUTPUT ${AFXHOOKSOURCE_SHADERS}
    DEPENDS shaderbuilder ${AFXHOOKSOURCE_SHADERS_SOURCES}
    COMMAND ShaderBuilder.exe --profile vs_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_vs20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_vs20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_line_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_line_ps20"
	COMMAND ShaderBuilder.exe --profile vs_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_vs20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_vs20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_pgldraw_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_pgldraw_ps20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_depth_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_depth_ps20"
	COMMAND ShaderBuilder.exe --profile ps_2_0 --in "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/afx_drawtexture_ps20.fxc" --outPrefix "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/build/afx_drawtexture_ps20"	
)

add_custom_target(AfxHookSourceShaders
    DEPENDS ${AFXHOOKSOURCE_SHADERS}
)

set(AFXHOOKSOURCE_PROTOBUFS
	"${CMAKE_CURRENT_SOURCE_DIR}/../build/protobuf/csgo/netmessages.pb.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/../build/protobuf/csgo/netmessages.pb.h"
)

set(AFXHOOKSOURCE_PROTOBUF_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../deps/release/prop/protobuf/csgo/netmessages.proto"
)

add_custom_command(
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build"
    OUTPUT ${AFXHOOKSOURCE_PROTOBUFS}
    DEPENDS protobuf_build ${AFXHOOKSOURCE_PROTOBUF_SOURCES}
	COMMAND "${CMAKE_COMMAND}" -E make_directory "protobuf/csgo"
    COMMAND "${protobuf_BINARY_DIR}/${CMAKE_CFG_INTDIR}/protoc.exe" "--proto_path=${protobuf_SOURCE_DIR}/src" "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../deps/release/prop/protobuf/csgo" "--cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/../build/protobuf/csgo" "${CMAKE_CURRENT_SOURCE_DIR}/../deps/release/prop/protobuf/csgo/netmessages.proto"
)

add_custom_target(AfxHookSourceProtobufs
    DEPENDS ${AFXHOOKSOURCE_PROTOBUFS}
)

add_library(AfxHookSource MODULE)

add_dependencies(AfxHookSource
    ilmbase_build
    openexr_build
    AfxHookSourceShaders
    AfxHookSourceProtobufs
)

target_compile_definitions(AfxHookSource PRIVATE
    AFX_INTEROP
    AFX_MIRV_PGL
    _CRT_SECURE_NO_WARNINGS
)

# generate PDB in release too:
target_compile_options(AfxHookSource PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RELEASE>>:/Zi>)
target_link_options(AfxHookSource PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RELEASE>>:/DEBUG /DEBUG /OPT:REF /OPT:ICF>)

target_include_directories(AfxHookSource PRIVATE
    ./
    ../deps/release/prop/AfxHookSource
    ../
    ../deps/release/prop
    ${OPENEXR_BUILD_DIR}/include/OpenEXR
    ${ILMBASE_BUILD_DIR}/include/OpenEXR
    ${protobuf_SOURCE_DIR}/src
)

target_link_directories(AfxHookSource PRIVATE
    ${OPENEXR_BUILD_DIR}/lib
    ${protobuf_BINARY_DIR}/${CMAKE_CFG_INTDIR}
)

target_link_libraries(AfxHookSource PRIVATE
    IlmImf-2_5$<$<CONFIG:Debug>:_d>.lib
    libprotobuf$<$<CONFIG:Debug>:d>.lib
)

target_sources(AfxHookSource PRIVATE
    csgo/hooks/client/csgo_spectatortools.cpp
    csgo/hooks/engine/cmd.cpp
    csgo/hooks/engine/cmd.h
    csgo/ClientToolsCsgo.cpp
    csgo/ClientToolsCsgo.h
    csgo/Panorama.cpp
    csgo/Panorama.h
    
    css/ClientToolsCss.cpp
    css/ClientToolsCss.h
    
    cssV34/ClientToolsCssV34.cpp
    cssV34/ClientToolsCssV34.h
    
    momentum/ClientToolsMom.cpp
    momentum/ClientToolsMom.h
    
    tf2/ClientToolsTf2.cpp
    tf2/ClientToolsTf2.h

    addresses.cpp
    addresses.h
    AfxClasses.cpp
    AfxClasses.h
    AfxInterfaces.h
    AfxInterop.cpp
    AfxInterop.h
    AfxShaders.cpp
    AfxShaders.h
    AfxStreams.cpp
    AfxStreams.h
    AfxWriteFileLimiter.cpp
    AfxWriteFileLimiter.h
    aiming.cpp
    aiming.h
    asmClassTools.h
    CamIO.cpp
    CamIO.h
    CampathDrawer.cpp
    CampathDrawer.h
    ClientTools.cpp
    ClientTools.h
    csgo_Audio.cpp
    csgo_Audio.h
    csgo_c_baseanimatingoverlay.cpp
    csgo_c_baseanimatingoverlay.h
    csgo_CBasePlayer.cpp
    csgo_CBasePlayer.h
    csgo_CCSGameMovement.cpp
    csgo_CCSGameMovement.h
    csgo_CHudDeathNotice.cpp
    csgo_CHudDeathNotice.h
    csgo_CRendering3dView.cpp
    csgo_CRendering3dView.h
    csgo_CSkyBoxView.cpp
    csgo_CSkyBoxView.h
    csgo_CViewRender.cpp
    csgo_CViewRender.h
    csgo_GameEvents.cpp
    csgo_GameEvents.h
    csgo_models_replace.cpp
    csgo_models_replace.h
    csgo_net_chan.cpp
    csgo_net_chan.h
    csgo_S_StartSound.cpp
    csgo_S_StartSound.h
    csgo_SndMixTimeScalePatch.cpp
    csgo_SndMixTimeScalePatch.h
    csgo_Stdshader_dx9_Hooks.cpp
    csgo_Stdshader_dx9_Hooks.h
    csgo_view.cpp
    csgo_view.h
    csgo_vphysics.cpp
    csgo_vphysics.h
    d3d9Hooks.cpp
    d3d9Hooks.h
    D3D9ImageBuffer.h
    FovScaling.cpp
    FovScaling.h
    Gui.cpp
    Gui.h
    hlaeFolder.cpp
    hlaeFolder.h
    main.cpp
    MaterialSystemHooks.cpp
    MaterialSystemHooks.h
    MatRenderContextHook.cpp
    MatRenderContextHook.h
    mirv_voice.cpp
    mirv_voice.h
    MirvCalcs.cpp
    MirvCalcs.h
    MirvCam.cpp
    MirvCam.h
    MirvCommands.cpp
    MirvPgl.cpp
    MirvPgl.h
    MirvTime.cpp
    MirvTime.h
    MirvWav.cpp
    MirvWav.h
    RenderView.cpp
    RenderView.h
    ReShadeAdvancedfx.cpp
    ReShadeAdvancedfx.h
    stdafx.h
    WrpConsole.cpp
    WrpConsole.h
    WrpGlobals.cpp
    WrpGlobals.h
    WrpVEngineClient.cpp
    WrpVEngineClient.h

    ../build/protobuf/csgo/netmessages.pb.cc
    ../build/protobuf/csgo/netmessages.pb.h

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

    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/appframework/IAppSystem.h
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/interfaces/interfaces.h
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/tier0/memalloc.h
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/tier1/convar.cpp
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/tier1/convar.h
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/tier1/iconvar.h
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/cdll_int.h
    ../deps/release/prop/AfxHookSource/bm/sdk_src/public/icvar.h

    ../deps/release/prop/AfxHookSource/csgo/bitbuf/demofilebitbuf.cpp
    ../deps/release/prop/AfxHookSource/csgo/bitbuf/demofilebitbuf.h
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/appframework/IAppSystem.h
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/interfaces/interfaces.h
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier0/memalloc.h
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier1/convar.cpp
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier1/convar.h
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier1/iconvar.h
    ../deps/release/prop/AfxHookSource/csgo/sdk_src/public/icvar.h

    ../deps/release/prop/AfxHookSource/css/sdk_src/public/engine/ishadowmgr.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/tier1/KeyValues.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/toolframework/itoolentity.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/toolframework/itoolframework.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/vstdlib/IKeyValuesSystem.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/basehandle.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/const.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/iclientrenderable.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/public/ihandleentity.h
    ../deps/release/prop/AfxHookSource/css/sdk_src/tier1/KeyValues.cpp

    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/engine/ishadowmgr.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tier1/KeyValues.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/toolframework/itoolentity.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/toolframework/itoolframework.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tools/bonelist.cpp
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tools/bonelist.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/vstdlib/ikeyvaluessystem.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/basehandle.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/const.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/iclientrenderable.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/public/ihandleentity.h
    ../deps/release/prop/AfxHookSource/cssV34/sdk_src/tier1/KeyValues.cpp

    ../deps/release/prop/AfxHookSource/insurgency2/public/cdll_int.h

    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/appframework/IAppSystem.h
    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/interfaces/interfaces.h
    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier0/memalloc.h
    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier1/convar.cpp
    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier1/convar.h
    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier1/iconvar.h
    ../deps/release/prop/AfxHookSource/l4d2/sdk_src/public/icvar.h

    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/appframework/IAppSystem.h
    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/interfaces/interfaces.h
    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier0/memalloc.h
    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier1/convar.cpp
    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier1/convar.h
    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier1/iconvar.h
    ../deps/release/prop/AfxHookSource/swarm/sdk_src/public/icvar.h

    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/engine/ishadowmgr.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/tier1/KeyValues.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/toolframework/itoolentity.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/toolframework/itoolframework.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/tools/bonelist.cpp
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/tools/bonelist.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/vstdlib/IKeyValuesSystem.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/basehandle.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/const.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/iclientrenderable.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/public/ihandleentity.h
    ../deps/release/prop/AfxHookSource/tf2/sdk_src/tier1/KeyValues.cpp

    ../deps/release/prop/AfxHookSource/SourceInterfaces.cpp
    ../deps/release/prop/AfxHookSource/SourceInterfaces.h
    ../deps/release/prop/AfxHookSource/SourceSdkShared.cpp
    ../deps/release/prop/AfxHookSource/SourceSdkShared.h

    ../deps/release/rapidxml/rapidxml.hpp
    ../deps/release/rapidxml/rapidxml_iterators.hpp
    ../deps/release/rapidxml/rapidxml_print.hpp
    ../deps/release/rapidxml/rapidxml_utils.hpp

    ../shared/hooks/gameOverlayRenderer.cpp
    ../shared/hooks/gameOverlayRenderer.h
    ../shared/vcpp/AfxAddr.cpp
    ../shared/vcpp/AfxAddr.h
    ../shared/AfxColorLut.cpp
    ../shared/AfxColorLut.h
    ../shared/AfxCommandLine.cpp
    ../shared/AfxCommandLine.h
    ../shared/AfxConsole.cpp
    ../shared/AfxConsole.h
    ../shared/AfxDetours.cpp
    ../shared/AfxDetours.h
    ../shared/AfxGameRecord.cpp
    ../shared/AfxGameRecord.h
    ../shared/AfxImageBuffer.h
    ../shared/AfxMath.cpp
    ../shared/AfxMath.h
    ../shared/AfxOutStreams.cpp
    ../shared/AfxOutStreams.h
    ../shared/AfxRefCounted.h
    ../shared/binutils.cpp
    ../shared/binutils.h
    ../shared/bvhexport.cpp
    ../shared/bvhexport.h
    ../shared/bvhimport.cpp
    ../shared/bvhimport.h
    ../shared/CamPath.cpp
    ../shared/CamPath.h
    ../shared/CommandSystem.cpp
    ../shared/CommandSystem.h
    ../shared/EasySampler.cpp
    ../shared/EasySampler.h
    ../shared/FileTools.cpp
    ../shared/FileTools.h
    ../shared/ImageBufferPoolThreadSafe.h
    ../shared/MirvCampath.cpp
    ../shared/MirvCampath.h
    ../shared/MirvInput.cpp
    ../shared/MirvInput.h
    ../shared/OpenExrOutput.cpp
    ../shared/OpenExrOutput.h
    ../shared/RawOutput.cpp
    ../shared/RawOutput.h
    ../shared/RefCountedThreadSafe.h
    ../shared/StringTools.cpp
    ../shared/StringTools.h
    ../shared/ThreadPool.h
)

# This is far form proper, but it will do for now:
source_group("AfxHookSource" REGULAR_EXPRESSION "/AfxHookSource/")
source_group("AfxHookSource/csgo" REGULAR_EXPRESSION "/AfxHookSource/csgo/")
source_group("AfxHookSource/csgo/hooks" REGULAR_EXPRESSION "/AfxHookSource/csgo/hooks/")
source_group("AfxHookSource/csgo/hooks/client" REGULAR_EXPRESSION "/AfxHookSource/csgo/hooks/client/")
source_group("AfxHookSource/csgo/hooks/engine" REGULAR_EXPRESSION "/AfxHookSource/csgo/hooks/engine/")
source_group("AfxHookSource/css" REGULAR_EXPRESSION "/AfxHookSource/css/")
source_group("AfxHookSource/cssV34" REGULAR_EXPRESSION "/AfxHookSource/cssV34/")
source_group("AfxHookSource/momentum" REGULAR_EXPRESSION "/AfxHookSource/momentum/")
source_group("AfxHookSource/tf2" REGULAR_EXPRESSION "/AfxHookSource/tf2/")
source_group("build" REGULAR_EXPRESSION "/build/")
source_group("build/protobuf" REGULAR_EXPRESSION "/build/protobuf/")
source_group("build/protobuf/csgo" REGULAR_EXPRESSION "/build/protobuf/csgo/")
source_group("deps" REGULAR_EXPRESSION "/deps/")
source_group("deps/release" REGULAR_EXPRESSION "/deps/release/")
source_group("deps/release/Detours" REGULAR_EXPRESSION "/deps/release/Detours/")
source_group("deps/release/Detours/src" REGULAR_EXPRESSION "/deps/release/Detours/src/")
source_group("deps/release/prop" REGULAR_EXPRESSION "/deps/release/prop/")
source_group("deps/release/prop/AfxHookSource" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/")
source_group("deps/release/prop/AfxHookSource/bm" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/")
source_group("deps/release/prop/AfxHookSource/bm/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/sdk_src/")
source_group("deps/release/prop/AfxHookSource/bm/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/bm/sdk_src/public/appframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/sdk_src/public/appframework/")
source_group("deps/release/prop/AfxHookSource/bm/sdk_src/public/interfaces" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/sdk_src/public/interfaces/")
source_group("deps/release/prop/AfxHookSource/bm/sdk_src/public/tier0" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/sdk_src/public/tier0/")
source_group("deps/release/prop/AfxHookSource/bm/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/bm/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/csgo" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/")
source_group("deps/release/prop/AfxHookSource/csgo/bitbuf" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/bitbuf/")
source_group("deps/release/prop/AfxHookSource/csgo/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/sdk_src/")
source_group("deps/release/prop/AfxHookSource/csgo/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/csgo/sdk_src/public/appframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/sdk_src/public/appframework/")
source_group("deps/release/prop/AfxHookSource/csgo/sdk_src/public/interfaces" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/sdk_src/public/interfaces/")
source_group("deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier0" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier0/")
source_group("deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/csgo/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/css" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src/public/engine" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/public/engine/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src/public/toolframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/public/toolframework/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src/public/vstdlib" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/public/vstdlib/")
source_group("deps/release/prop/AfxHookSource/css/sdk_src/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/css/sdk_src/tier1/")
source_group("deps/release/prop/AfxHookSource/cssV34" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/public/engine" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/public/engine/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/public/toolframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/public/toolframework/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tools" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/public/tools/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/public/vstdlib" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/public/vstdlib/")
source_group("deps/release/prop/AfxHookSource/cssV34/sdk_src/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/cssV34/sdk_src/tier1/")
source_group("deps/release/prop/AfxHookSource/insurgency2" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/insurgency2/")
source_group("deps/release/prop/AfxHookSource/insurgency2/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/insurgency2/sdk_src/")
source_group("deps/release/prop/AfxHookSource/insurgency2/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/insurgency2/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/l4d2" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/")
source_group("deps/release/prop/AfxHookSource/l4d2/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/sdk_src/")
source_group("deps/release/prop/AfxHookSource/l4d2/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/l4d2/sdk_src/public/appframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/sdk_src/public/appframework/")
source_group("deps/release/prop/AfxHookSource/l4d2/sdk_src/public/interfaces" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/sdk_src/public/interfaces/")
source_group("deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier0" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier0/")
source_group("deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/l4d2/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/swarm" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/")
source_group("deps/release/prop/AfxHookSource/swarm/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/sdk_src/")
source_group("deps/release/prop/AfxHookSource/swarm/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/swarm/sdk_src/public/appframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/sdk_src/public/appframework/")
source_group("deps/release/prop/AfxHookSource/swarm/sdk_src/public/interfaces" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/sdk_src/public/interfaces/")
source_group("deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier0" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier0/")
source_group("deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/swarm/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/tf2/" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/public/engine" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/engine/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/tier1/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/public/toolframework" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/toolframework/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/public/tools" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/tools/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/public/vstdlib" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/vstdlib/")
source_group("deps/release/prop/AfxHookSource/tf2/sdk_src/tier1" REGULAR_EXPRESSION "/deps/release/prop/AfxHookSource/tf2/sdk_src/public/tier1/")
source_group("deps/release/rapidxml" REGULAR_EXPRESSION "/deps/release/rapidxml/")
source_group("shared" REGULAR_EXPRESSION "/shared/")
source_group("shared/hooks" REGULAR_EXPRESSION "/shared/hooks/")
source_group("shared/vcpp" REGULAR_EXPRESSION "/shared/vcpp/")
