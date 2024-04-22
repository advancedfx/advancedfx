#include "stdafx.h"

#include "ClientEntitySystem.h"
#include "WrpConsole.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

#include "../shared/AfxConsole.h"
//#include "../shared/binutils.h"

//#include "AfxHookSource2Rs.h"

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include "../deps/release/Detours/src/detours.h"


class CGameEntitySystem {
    

    //:014 - OnAddEntity
    //:015 - OnRemoveEntity
};


struct EntityListIterator{
	SOURCESDK::CS2::CBaseHandle handle = SOURCESDK_CS2_INVALID_EHANDLE_INDEX;

	bool IsValid() const {
		return handle != SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
	}

	SOURCESDK::CS2::CBaseHandle GetHandle() const {
		return handle;
	}

    int GetEntryIndex() const {
        return handle.GetEntryIndex();
    }
};

typedef EntityListIterator & (__fastcall * GetHighestEntityHandle_t)(void * entityList);
typedef void * (__fastcall * GetEntityFromIndex_t)(void * pEntityList, int index);

void ** g_pEntityList = nullptr;
GetHighestEntityHandle_t  g_GetHighestEntityHandle = nullptr;
GetEntityFromIndex_t g_GetEntityFromIndex = nullptr;

// CEntityInstance: Root class for all entities
class CEntityInstance {
public:
    // Retrieved from script function.
    const char * GetName() {
        const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x18);
        if(pszName) return pszName;
        return "";
    }

    // Retrieved from script function.
    const char * GetDebugName() {
        const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x18);
        if(pszName) return pszName;
        else {
            return **(const char***)(*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x8)+0x28);
        }
        return "";
    }

    // Retrieved from script function.
    const char * GetClassName() {
        const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x20);
        if(pszName) return pszName;
        return "";
    }

    // 0x508 GetRenderEyeAngles

    // Retrieved from script function.
    // GetEntityHandle ...
};

bool Hook_ClientEntitySystem( void* pEntityList, void * pFnGetHighestEntityHandle, void * pFnGetEntityFromIndex ) {
    static bool firstResult = false;
    static bool firstRun = true;

    if(firstRun) {
        firstRun = false;
        g_pEntityList = (void**)pEntityList;
        g_GetHighestEntityHandle = (GetHighestEntityHandle_t)pFnGetHighestEntityHandle;
        g_GetEntityFromIndex = (GetEntityFromIndex_t)pFnGetEntityFromIndex;
        firstResult = true;
    }

    return firstResult;
}

CON_COMMAND(__mirv_listentities, "") {
    int highestIndex = g_GetHighestEntityHandle(*g_pEntityList).GetEntryIndex();
    for(int i = 0; i < highestIndex + 1; i++) {
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
            advancedfx::Message("%i: %s / %s / %s\n", i, ent->GetName(), ent->GetDebugName(), ent->GetClassName());
        }
    }
}
