cmake_minimum_required (VERSION 3.24)

project ("AfxHookSource2")

add_library(${PROJECT_NAME} MODULE)

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
    ../deps/release/prop/cs2/sdk_src/public/icvar.h    
    ../deps/release/prop/AfxHookSource/SourceSdkShared.cpp
    ../deps/release/prop/AfxHookSource/SourceSdkShared.h
    ../deps/release/prop/AfxHookSource/SourceInterfaces.h
    
    ../shared/AfxConsole.cpp
    ../shared/AfxConsole.h
    ../shared/AfxDetours.cpp
    ../shared/AfxDetours.h
    ../shared/StringTools.cpp
    ../shared/StringTools.h

    ../AfxHookSource/AfxCommandLine.cpp
    ../AfxHookSource/AfxCommandLine.h

    main.cpp
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
