cmake_minimum_required (VERSION 3.20)

project ("AfxHookGoldSrc")

add_library(${PROJECT_NAME} MODULE)

add_dependencies(${PROJECT_NAME}
    ilmbase_build
    openexr_build
)

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
    ../
    ../deps/release/prop
    ../deps/release/halflife/common
    ../deps/release/halflife/engine
    ../deps/release/halflife/pm_shared
    ../deps/release/halflife/public
    ${OPENEXR_BUILD_DIR}/include/OpenEXR
    ${ILMBASE_BUILD_DIR}/include/OpenEXR    
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${OPENEXR_BUILD_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    IlmImf-2_5$<$<CONFIG:Debug>:_d>.lib
)

target_sources(${PROJECT_NAME} PRIVATE
    hooks/client/cstrike/CrossHairFix.cpp
    hooks/client/cstrike/CrossHairFix.h
    hooks/client/cstrike/SpectatorFix.cpp
    hooks/client/cstrike/SpectatorFix.h
    hooks/client/cstrike/ViewmodelAnimationFix.cpp
    hooks/client/cstrike/ViewmodelAnimationFix.h

    hooks/client/HookClient.cpp
    hooks/client/HookClient.h

    hooks/demoplayer/DemoPlayer.cpp
    hooks/demoplayer/DemoPlayer.h

    hooks/hw/CL_Disconnect.cpp
    hooks/hw/CL_Disconnect.h
    hooks/hw/ClientFunctions.cpp
    hooks/hw/ClientFunctions.h
    hooks/hw/Host_Frame.cpp
    hooks/hw/Host_Frame.h
    hooks/hw/Host_Init.cpp
    hooks/hw/Host_Init.h
    hooks/hw/Mod_LeafPvs.cpp
    hooks/hw/Mod_LeafPvs.h
    hooks/hw/R_DrawEntitiesOnList.cpp
    hooks/hw/R_DrawEntitiesOnList.h
    hooks/hw/R_DrawParticles.cpp
    hooks/hw/R_DrawParticles.h
    hooks/hw/R_DrawSkyBoxEx.cpp
    hooks/hw/R_DrawSkyBoxEx.h
    hooks/hw/R_DrawViewModel.cpp
    hooks/hw/R_DrawViewModel.h
    hooks/hw/R_PolyBlend.cpp
    hooks/hw/R_PolyBlend.h
    hooks/hw/R_RenderView.cpp
    hooks/hw/R_RenderView.h
    hooks/hw/UnkDrawHud.cpp
    hooks/hw/UnkDrawHud.h

    hooks/svdll/GiveFnptrsToDll.cpp
    hooks/svdll/GiveFnptrsToDll.h
    hooks/svdll/Server_GetBlendingInterface.cpp
    hooks/svdll/Server_GetBlendingInterface.h

    hooks/gdi32Hooks.cpp
    hooks/gdi32Hooks.h
    hooks/HookGameLoaded.cpp
    hooks/HookGameLoaded.h
    hooks/HookHl.cpp
    hooks/HookHl.h
    hooks/HookHw.cpp
    hooks/HookHw.h
    hooks/interface.cpp
    hooks/interface.h
    hooks/OpenGl32Hooks.cpp
    hooks/OpenGl32Hooks.h
    hooks/user32Hooks.cpp
    hooks/user32Hooks.h

    AfxGlImage.cpp
    AfxGlImage.h
    AfxImageUtils.cpp
    AfxImageUtils.h
    AfxMemory.cpp
    AfxMemory.h
    AfxSettings.cpp
    AfxSettings.h
    aiming.cpp
    aiming.h
    camexport.cpp
    camexport.h
    camimport.cpp
    camimport.h
    CampathDrawer.cpp
    CampathDrawer.h
    cmd_tools.cpp
    cmd_tools.h
    cmdregister.h
    cstrike_ch.cpp
    cstrike_deathmsg.cpp
    cstrike_smoke.cpp
    decals_hook.cpp
    demo_tools.cpp
    demozoom.cpp
    disable_spec_menu.cpp
    film_sound.cpp
    film_sound.h
    filming.cpp
    filming.h
    forcePlayersSolid.cpp
    fx_noblend.cpp
    GameRecord.cpp
    GameRecord.h
    GlPrimMods.cpp
    GlPrimMods.h
    hl_addresses.cpp
    hl_addresses.h
    hlaeFolder.cpp
    hlaeFolder.h
    hlsdk.h
    HltvFix.cpp
    HltvFix.h
    main.cpp
    mdt_media.cpp
    mdt_media.h
    mirv_campath.cpp
    mirv_commands.cpp
    mirv_commands.h
    mirv_glext.cpp
    mirv_glext.h
    mirv_info.cpp
    mirv_input.cpp
    mirv_input.h
    mirv_time.cpp
    mirv_time.h
    MirvCommandArgs.cpp
    MirvCommandArgs.h
    move_to.cpp
    newsky.cpp
    stdafx.cpp
    stdafx.h
    Store.cpp
    Store.h
    supportrender.cpp
    supportrender.h
    sv_hitboxes.cpp
    sv_hitboxes.h
    temporary_dominik_0005.cpp
    variousCommands.cpp
    zooming.cpp
    zooming.h

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

    ../deps/release/halflife/utils/common/mathlib.c

    ../deps/release/rapidxml/rapidxml.hpp
    ../deps/release/rapidxml/rapidxml_iterators.hpp
    ../deps/release/rapidxml/rapidxml_print.hpp
    ../deps/release/rapidxml/rapidxml_utils.hpp

    ../shared/hldemo/hldemo.h
    ../shared/hooks/gameOverlayRenderer.cpp
    ../shared/hooks/gameOverlayRenderer.h
    ../shared/vcpp/AfxAddr.cpp
    ../shared/vcpp/AfxAddr.h
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
    ../shared/EasySampler.cpp
    ../shared/EasySampler.h
    ../shared/FileTools.cpp
    ../shared/FileTools.h
    ../shared/MirvCampath.cpp
    ../shared/MirvCampath.h
    ../shared/MirvInput.cpp
    ../shared/MirvInput.h
    ../shared/OpenExrOutput.cpp
    ../shared/OpenExrOutput.h
    ../shared/RawOutput.cpp
    ../shared/RawOutput.h
    ../shared/StringTools.cpp
    ../shared/StringTools.h
)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_FLAGS  "/Yustdafx.h /FIstdafx.h")
    set_source_files_properties(
        stdafx.cpp
        TARGET_DIRECTORY ${PROJECT_NAME}
        PROPERTIES COMPILE_FLAGS "/Ycstdafx.h"
    )
endif(MSVC)

source_group("AfxHookGoldSrc" REGULAR_EXPRESSION "/AfxHookGoldSrc/")
source_group("AfxHookGoldSrc/hooks" REGULAR_EXPRESSION "/AfxHookGoldSrc/hooks/")
source_group("AfxHookGoldSrc/hooks/client/" REGULAR_EXPRESSION "/AfxHookGoldSrc/hooks/client/")
source_group("AfxHookGoldSrc/hooks/client/cstrike" REGULAR_EXPRESSION "/AfxHookGoldSrc/hooks/client/cstrike")
source_group("AfxHookGoldSrc/hooks/demoplayer" REGULAR_EXPRESSION "/AfxHookGoldSrc/hooks/demoplayer/")
source_group("AfxHookGoldSrc/hooks/hw" REGULAR_EXPRESSION "/AfxHookGoldSrc/hooks/hw/")
source_group("AfxHookGoldSrc/hooks/svdll" REGULAR_EXPRESSION "/AfxHookGoldSrc/hooks/svdll/")
source_group("deps" REGULAR_EXPRESSION "/deps/")
source_group("deps/release" REGULAR_EXPRESSION "/deps/release/")
source_group("deps/release/Detours" REGULAR_EXPRESSION "/deps/release/Detours/")
source_group("deps/release/Detours/src" REGULAR_EXPRESSION "/deps/release/Detours/src/")
source_group("deps/release/halflife" REGULAR_EXPRESSION "/deps/release/halflife/")
source_group("deps/release/halflife/utils" REGULAR_EXPRESSION "/deps/release/halflife/utils/")
source_group("deps/release/halflife/utils/common" REGULAR_EXPRESSION "/deps/release/halflife/utils/common/")
source_group("deps/release/rapidxml" REGULAR_EXPRESSION "/deps/release/rapidxml/")
source_group("shared" REGULAR_EXPRESSION "/shared/")
source_group("shared/hldemo" REGULAR_EXPRESSION "/shared/hldemo/")
source_group("shared/hooks" REGULAR_EXPRESSION "/shared/hooks/")
source_group("shared/vcpp" REGULAR_EXPRESSION "/shared/vcpp/")
