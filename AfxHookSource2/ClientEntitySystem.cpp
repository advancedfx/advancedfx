#include "stdafx.h"

#include "ClientEntitySystem.h"
#include "WrpConsole.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

#include "../shared/AfxConsole.h"
//#include "../shared/binutils.h"

#include "AfxHookSource2Rs.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"

#include <map>

struct EntityListIterator{
	int index = -1;

	bool IsValid() const {
		return 0 < index;
	}

    int GetIndex() const {
        return index;
    }
};

typedef EntityListIterator * (__fastcall * GetHighestEntityIterator_t)(void * entityList, EntityListIterator * it);
typedef void * (__fastcall * GetEntityFromIndex_t)(void * pEntityList, int index);

void ** g_pEntityList = nullptr;
GetHighestEntityIterator_t  g_GetHighestEntityIterator = nullptr;
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
    // can return nullptr!
    const char * GetDebugName() {
        const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x18);
        if(pszName) return pszName;
        return **(const char***)(*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x8)+0x28);
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
        return ((bool (__fastcall *)(void *)) (*(void***)this)[149]) (this);
    }

    SOURCESDK::CS2::CBaseHandle GetPlayerPawnHandle() {
        // See cl_ent_text drawing function. Or Schema system.
        if(!IsPlayerController())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
        return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + 0x5fc));
    }

    bool IsPlayerController() {
        // See cl_ent_text drawing function. Near "Pawn: (%d) Name: %s".
        return ((bool (__fastcall *)(void *)) (*(void***)this)[150]) (this);    
    }

    SOURCESDK::CS2::CBaseHandle GetPlayerControllerHandle() {
        // See cl_ent_text drawing function. Or Schema system.
        if(!IsPlayerPawn())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
        return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + 0x128c));
    }

    unsigned int GetHealth() {
        // See cl_ent_text drawing function. Near "Health: %d\n".
        return ((unsigned int (__fastcall *)(void *)) (*(void***)this)[162]) (this); 
    }

    /**
     * @remarks FLOAT_MAX if invalid
     */
    void GetOrigin(float & x, float & y, float & z) {
        // See cl_ent_text drawing function. Near "Position: %0.3f, %0.3f, %0.3f\n" or cl_ent_viewoffset related function.
        x =  (*(float *)((unsigned char *)this + 0x144));
        y =  (*(float *)((unsigned char *)this + 0x148));
        z =  (*(float *)((unsigned char *)this + 0x14c));
    }

    void GetRenderEyeOrigin(float outOrigin[3]) {
        // GetRenderEyeAngles vtable offset minus 1
        ((void (__fastcall *)(void *,float outOrigin[3])) (*(void***)this)[166]) (this,outOrigin);
    }

    void GetRenderEyeAngles(float outAngles[3]) {
        // See cl_track_render_eye_angles. Near "Render eye angles: %.7f, %.7f, %.7f\n".
        ((void (__fastcall *)(void *,float outAngles[3])) (*(void***)this)[167]) (this,outAngles);
    }

    SOURCESDK::CS2::CBaseHandle GetViewEntityHandle() {
        // Schema system.
        //if (!IsPlayerPawn())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
        return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int*)((unsigned char*)this + 0x9c));
    }
};

class CAfxEntityInstanceRef {
public:
    static CAfxEntityInstanceRef * Aquire(CEntityInstance * pInstance) {
        CAfxEntityInstanceRef * pRef;
        auto it = m_Map.find(pInstance);
        if(it != m_Map.end()) {    
            pRef = it->second;
        } else {
            pRef = new CAfxEntityInstanceRef(pInstance);
            m_Map[pInstance] = pRef;
        }
        pRef->AddRef();
        return pRef;
    }

    static void Invalidate(CEntityInstance * pInstance) {
        if(m_Map.empty()) return;
        auto it = m_Map.find(pInstance);
        if(it != m_Map.end()) {
            auto & pInstance = it->second;
            pInstance->m_pInstance = nullptr;
            m_Map.erase(it);
        }        
    }

    CEntityInstance * GetInstance() {
        return m_pInstance;
    }

    bool IsValid() {
        return nullptr != m_pInstance;
    }

    void AddRef() {
        m_RefCount++;
    }

    void Release() {
        m_RefCount--;
        if(0 == m_RefCount) {
            delete this;
        }
    }

protected:
    CAfxEntityInstanceRef(class CEntityInstance * pInstance)
    : m_pInstance(pInstance)
    {
    }

    ~CAfxEntityInstanceRef() {
        m_Map.erase(m_pInstance);
    }

private:
    int m_RefCount = 0;
    class CEntityInstance * m_pInstance;
    static std::map<CEntityInstance *,CAfxEntityInstanceRef *> m_Map;
};

std::map<CEntityInstance *,CAfxEntityInstanceRef *> CAfxEntityInstanceRef::m_Map;


typedef void* (__fastcall * OnAddEntity_t)(void* This, CEntityInstance* pInstance, SOURCESDK::uint32 handle);
OnAddEntity_t g_Org_OnAddEntity = nullptr;


void* __fastcall New_OnAddEntity(void* This, CEntityInstance* pInstance, SOURCESDK::uint32 handle) {

    void * result =  g_Org_OnAddEntity(This,pInstance,handle);

    if(g_b_on_add_entity && pInstance) {
        auto pRef = CAfxEntityInstanceRef::Aquire(pInstance);
        AfxHookSource2Rs_Engine_OnAddEntity(pRef,handle);
        pRef->Release();
    }

    return result;
}

typedef void* (__fastcall * OnRemoveEntity_t)(void* This, CEntityInstance* inst, SOURCESDK::uint32 handle);
OnRemoveEntity_t g_Org_OnRemoveEntity = nullptr;

void* __fastcall New_OnRemoveEntity(void* This, CEntityInstance* pInstance, SOURCESDK::uint32 handle) {

    if(g_b_on_remove_entity && pInstance) {
        auto pRef = CAfxEntityInstanceRef::Aquire(pInstance);
        AfxHookSource2Rs_Engine_OnRemoveEntity(pRef,handle);
        pRef->Release();
    }

    CAfxEntityInstanceRef::Invalidate(pInstance);

    void * result =  g_Org_OnRemoveEntity(This,pInstance,handle);
    return result;
}

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)
extern void ErrorBox(char const * messageText);

bool Hook_ClientEntitySystem( void* pEntityList, void * pFnGetHighestEntityIterator, void * pFnGetEntityFromIndex ) {
    static bool firstResult = false;
    static bool firstRun = true;

    if(firstRun) {
        firstRun = false;
        g_pEntityList = (void**)pEntityList;
        g_GetHighestEntityIterator = (GetHighestEntityIterator_t)pFnGetHighestEntityIterator;
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
    EntityListIterator it;
    int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();
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

int afx_hook_source2_get_highest_entity_index() {
    EntityListIterator it;
    if(g_GetHighestEntityIterator) return g_GetHighestEntityIterator(*g_pEntityList,&it)->GetIndex();
    return -1;
}

void * afx_hook_source2_get_entity_ref_from_index(int index) {
    if(CEntityInstance * result = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,index)) {
        return CAfxEntityInstanceRef::Aquire(result);
    }
    return nullptr;
}

void afx_hook_source2_add_ref_entity_ref(void * pRef) {
    ((CAfxEntityInstanceRef *)pRef)->AddRef();
}

void afx_hook_source2_release_entity_ref(void * pRef) {
    ((CAfxEntityInstanceRef *)pRef)->Release();
}

bool afx_hook_source2_getEntityRefIsValid(void * pRef) {
    return ((CAfxEntityInstanceRef *)pRef)->IsValid();
}

const char * afx_hook_source2_getEntityRefName(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetName();
    }
    return "";
}

const char * afx_hook_source2_getEntityRefDebugName(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetDebugName();
    }
    return nullptr;
}

const char * afx_hook_source2_getEntityRefClassName(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetClassName();
    }
    return "";
}

bool afx_hook_source2_getEntityRefIsPlayerPawn(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->IsPlayerPawn();
    }
    return false;
}

int afx_hook_source2_getEntityRefPlayerPawnHandle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetPlayerPawnHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;    
}

bool afx_hook_source2_getEntityRefIsPlayerController(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->IsPlayerController();
    }
    return false;    
}

int afx_hook_source2_getEntityRefPlayerControllerHandle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetPlayerControllerHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;  
}

int afx_hook_source2_getEntityRefHealth(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetHealth();
    }
    return 0;    
}

void afx_hook_source2_getEntityRefOrigin(void * pRef, float & x, float & y, float & z) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       pInstance->GetOrigin(x,y,z);
    }    
}

void afx_hook_source2_getEntityRefRenderEyeOrigin(void * pRef, float & x, float & y, float & z) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        float tmp[3];
       pInstance->GetRenderEyeOrigin(tmp);
       x = tmp[0];
       y = tmp[1];
       z = tmp[2];
    }    
}

void afx_hook_source2_getEntityRefRenderEyeAngles(void * pRef, float & x, float & y, float & z) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        float tmp[3];
       pInstance->GetRenderEyeAngles(tmp);
       x = tmp[0];
       y = tmp[1];
       z = tmp[2];
    }    
}

int afx_hook_source2_getEntityRefViewEntityHandle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetViewEntityHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
}
