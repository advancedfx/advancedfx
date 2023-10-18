cmake_minimum_required (VERSION 3.24)

project ("AfxHookSource2")

add_library(${PROJECT_NAME} MODULE)

#set_target_properties(
#    ${PROJECT_NAME}
#    PROPERTIES 
#        OUTPUT_NAME "${PROJECT_NAME}_IF_YOU_USE_THIS_TO_JOIN_GOTV_OR_VAC_SERVERS_YOU_WILL_GET_VAC_BANNED"
#)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

# generate PDB in release too:
target_compile_options(${PROJECT_NAME} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RELEASE>>:/Zi>)
target_link_options(${PROJECT_NAME} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RELEASE>>:/DEBUG /DEBUG /OPT:REF /OPT:ICF>)

target_include_directories(${PROJECT_NAME} PRIVATE
    ./
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
    ../deps/release/prop/cs2/sdk_src/public/tier1/convar.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/iconvar.h
    ../deps/release/prop/cs2/sdk_src/public/tier1/KeyValues.h
    ../deps/release/prop/cs2/sdk_src/public/vstdlib/IKeyValuesSystem.h
    ../deps/release/prop/cs2/sdk_src/public/cdll_int.h    
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
    ../shared/CamIO.cpp
    ../shared/CamIO.h
    ../shared/CamPath.cpp
    ../shared/CamPath.h
    ../shared/CommandSystem.cpp
    ../shared/CommandSystem.h
    ../shared/FovScaling.cpp
    ../shared/FovScaling.h
    ../shared/StringTools.cpp
    ../shared/StringTools.h
    ../shared/binutils.cpp
    ../shared/binutils.h
    ../shared/MirvCamIO.cpp
    ../shared/MirvCamIO.h
    ../shared/MirvCampath.cpp
    ../shared/MirvCampath.h
    ../shared/MirvInput.cpp
    ../shared/MirvInput.h
    ../shared/MirvSkip.cpp
    ../shared/MirvSkip.h

    GameEvents.cpp
    GameEvents.h

    main.cpp

    WrpConsole.cpp
    WrpConsole.h
)

# This is far form proper, but it will do for now:
source_group("deps/release/Detours" REGULAR_EXPRESSION "/deps/release/Detours/")
source_group("deps/release/Detours/src" REGULAR_EXPRESSION "/deps/release/Detours/src/")
source_group("deps/release/prop" REGULAR_EXPRESSION "/deps/release/prop/")
source_group("deps/release/prop/cs2" REGULAR_EXPRESSION "/deps/release/prop/cs2/")
source_group("deps/release/prop/cs2/sdk_src/public" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/")
source_group("deps/release/prop/cs2/sdk_src/public/appframework" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/appframework/")
source_group("deps/release/prop/cs2/sdk_src/public/interfaces" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/interfaces/")
source_group("deps/release/prop/cs2/sdk_src/public/tier0" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/tier0/")
source_group("deps/release/prop/cs2/sdk_src/public/tier1" REGULAR_EXPRESSION "/deps/release/prop/cs2/sdk_src/public/tier1/")


source_group("shared" REGULAR_EXPRESSION "/shared/")
source_group("shared/hooks" REGULAR_EXPRESSION "/shared/hooks/")
source_group("shared/vcpp" REGULAR_EXPRESSION "/shared/vcpp/")
source_group("AfxHookSource" REGULAR_EXPRESSION "/AfxHookSource/")
