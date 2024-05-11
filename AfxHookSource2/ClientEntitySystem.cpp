#include "stdafx.h"

#include "ClientEntitySystem.h"
#include "WrpConsole.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

#include "../shared/AfxConsole.h"
//#include "../shared/binutils.h"

//#include "AfxHookSource2Rs.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"


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

/*
cl_track_render_eye_angles 1
cl_ent_absbox 192
cl_ent_viewoffset 192
*/

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

    // Retrieved from script function.
    // GetEntityHandle ...

    bool IsPlayerPawn() {
        // See cl_ent_text drawing function.
        return ((bool (__fastcall *)(void *)) (*(void***)this)[143]) (this);
    }

    SOURCESDK::CS2::CBaseHandle GetPlayerPawnHandle() {
        // See cl_ent_text drawing function.
        return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + 0x604));
    }

    bool IsPlayerController() {
        // See cl_ent_text drawing function.
        return ((bool (__fastcall *)(void *)) (*(void***)this)[144]) (this);    
    }

    SOURCESDK::CS2::CBaseHandle GetPlayerControllerHandle() {
        // See cl_ent_text drawing function.
        return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + 0x1294));
    }

    unsigned int GetHealth() {
        // See cl_ent_text drawing function.
        return ((unsigned int (__fastcall *)(void *)) (*(void***)this)[156]) (this); 
    }

    /// @return FLOAT_MAX if invalid
    float GetX() {
        // See cl_ent_text drawing function.
        return (*(float *)((unsigned char *)this + 0x138));
    }
    /// @return FLOAT_MAX if invalid
    float GetY() {
        // See cl_ent_text drawing function.
        return (*(float *)((unsigned char *)this + 0x13c));
    }
    /// @return FLOAT_MAX if invalid
    float GetZ() {
        // See cl_ent_text drawing function.
        return (*(float *)((unsigned char *)this + 0x140));
    }

    void GetRenderEyeOrigin(float outAngles[3]) {
        // GetRenderEyeAngles vtable offset minus 1
        ((void (__fastcall *)(void *,float outAngles[3])) (*(void***)this)[160]) (this,outAngles);
    }

    void GetRenderEyeAngles(float outAngles[3]) {
        // See cl_track_render_eye_angles.
        ((void (__fastcall *)(void *,float outAngles[3])) (*(void***)this)[161]) (this,outAngles);
    }

    CEntityInstance * GetViewEntity() {
        // Debug code related to cl_ent_viewoffset.
        return ((CEntityInstance * (__fastcall *)(void *)) (*(void***)this)[99]) (this);
    }
};

typedef void* (__fastcall * OnAddEntity_t)(void* This, CEntityInstance* pInstance, SOURCESDK::uint32 handle);
OnAddEntity_t g_Org_OnAddEntity = nullptr;

void* __fastcall New_OnAddEntity(void* This, CEntityInstance* pInstance, SOURCESDK::uint32 handle) {
    void * result =  g_Org_OnAddEntity(This,pInstance,handle);
    return result;
}

typedef void* (__fastcall * OnRemoveEntity_t)(void* This, CEntityInstance* inst, SOURCESDK::uint32 handle);
OnRemoveEntity_t g_Org_OnRemoveEntity = nullptr;

void* __fastcall New_OnRemoveEntity(void* This, CEntityInstance* pInstance, SOURCESDK::uint32 handle) {
    void * result =  g_Org_OnRemoveEntity(This,pInstance,handle);
    return result;
}

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)
extern void ErrorBox(char const * messageText);

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

bool Hook_ClientEntitySystem2() {
    static bool firstResult = false;
    static bool firstRun = true;

    if(g_pEntityList && *g_pEntityList) {
        // https://github.com/bruhmoment21/cs2-sdk
        void ** vtable = **(void****)g_pEntityList;
        g_Org_OnAddEntity = (OnAddEntity_t)vtable[14];
        g_Org_OnRemoveEntity = (OnRemoveEntity_t)vtable[15];
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Org_OnAddEntity, New_OnAddEntity);
        DetourAttach(&(PVOID&)g_Org_OnRemoveEntity, New_OnRemoveEntity);
        firstResult = NO_ERROR == DetourTransactionCommit();                
    }

    return firstResult;    
}

CON_COMMAND(__mirv_listentities, "") {
    int highestIndex = g_GetHighestEntityHandle(*g_pEntityList).GetEntryIndex();
    for(int i = 0; i < highestIndex + 1; i++) {
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
            float origin[3];
            float angles[3];
            ent->GetRenderEyeOrigin(origin);
            ent->GetRenderEyeAngles(angles);
            advancedfx::Message("%i: %s / %s / %s [%f,%f,%f / %f,%f,%f] %i HP\n", i, ent->GetName(), ent->GetDebugName(), ent->GetClassName(), origin[0], origin[1], origin[2], angles[0], angles[1], angles[2], ent->GetHealth());
        }
        else advancedfx::Message("%i:\n",i);
    }
}
