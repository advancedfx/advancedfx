#include "stdafx.h"

#include "ClientEntitySystem.h"
#include "DeathMsg.h"
#include "WrpConsole.h"
#include "Globals.h"
#include "addresses.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"

#include "../shared/AfxConsole.h"
//#include "../shared/binutils.h"
#include "../shared/FFITools.h"
#include "../shared/StringTools.h"

#include "AfxHookSource2Rs.h"
#include "SchemaSystem.h"

#define WIN32_LEAN_AND_MEAN
#include "../deps/release/Detours/src/detours.h"

#include <map>
#include <unordered_set>
#include <algorithm>

void ** g_pEntityList = nullptr;
GetHighestEntityIndex_t  g_GetHighestEntityIndex = nullptr;
GetEntityFromIndex_t g_GetEntityFromIndex = nullptr;

/*
cl_track_render_eye_angles 1
cl_ent_absbox 192
cl_ent_viewoffset 192
*/

// CEntityInstance: Root class for all entities
// Retrieved from script function.
const char * CEntityInstance::GetName() {
    /*
        undefined8 * FUN_1814beac0(void) {
            puVar6[2] = "CEntityInstance: Root class for all entities";
            ...
            puVar4[2] = "Get the entity name";
            ...
            *puVar4 = "GetName";
            ...
            puVar4[8] = FUN_18094f290; // <-  VSCRIPT entity.GetName function.
            ...            
        }        
    */
	const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x18);
	if(pszName) return pszName;
	return "";
}

// Retrieved from script function.
// can return nullptr!
const char * CEntityInstance::GetDebugName() {
    /*
        undefined8 * FUN_1814beac0(void) {
            puVar6[2] = "CEntityInstance: Root class for all entities";
            ...
            puVar4[2] = "Get the entity name w/help if not defined (i.e. classname/etc)";
            ...
            *puVar4 = "GetDebugName";
            ...
           puVar4[8] = &LAB_1814c1b90; // <-  VSCRIPT entity.GetDebugName function.
            ...            
        }        
    */    
	const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x18);
	if(pszName) return pszName;
	return **(const char***)(*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x8)+0x58);
}

// Retrieved from script function.
const char * CEntityInstance::GetClassName() {
    /*
        undefined8 * FUN_1814beac0(void) {
            puVar6[2] = "CEntityInstance: Root class for all entities";
            ...
            *puVar4 = "GetClassname";
            ...
            puVar4[8] = &LAB_1814c1b60; // <-  VSCRIPT entity.GetClassName function.
            ...            
        }        
    */     
	const char * pszName = (const char*)*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x20);
	if(pszName) return pszName;
	return "";
}

extern HMODULE g_H_ClientDll;

// Retrieved from script function.
const char * CEntityInstance::GetClientClassName() {
    void * pClientClass = ((void * (__fastcall *)(void *)) (*(void***)this)[AFXADDR_GET(cs2_client_CEntityInstance_GetClientClass_vtable_idx)]) (this);

    if(pClientClass) {
        return *(const char**)((unsigned char*)pClientClass + 0x10);
    }
    return nullptr;
}

// Retrieved from script function.
// GetEntityHandle ...

bool CEntityInstance::IsPlayerPawn() {
	return ((bool (__fastcall *)(void *)) (*(void***)this)[AFXADDR_GET(cs2_client_CEntityInstance_IsPlayerPawn_vtable_idx)]) (this);
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetPlayerPawnHandle() {
	// See cl_ent_text drawing function.
	if(!IsPlayerController())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
	return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + g_clientDllOffsets.CBasePlayerController.m_hPawn));
}

bool CEntityInstance::IsPlayerController() {
	return ((bool (__fastcall *)(void *)) (*(void***)this)[AFXADDR_GET(cs2_client_CEntityInstance_IsPlayerController_vtable_idx)]) (this);
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetPlayerControllerHandle() {
	// See cl_ent_text drawing function.
	if(!IsPlayerPawn())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
	return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + g_clientDllOffsets.C_BasePlayerPawn.m_hController));
}

unsigned int CEntityInstance::GetHealth() {
	// See cl_ent_text drawing function. Near "Health: %d\n".
	return *(unsigned int *)((unsigned char *)this + g_clientDllOffsets.C_BaseEntity.m_iHealth);
}

int CEntityInstance::GetTeam() {
    return *(int*)((u_char*)(this) + g_clientDllOffsets.C_BaseEntity.m_iTeamNum);
}


/**
 * @remarks FLOAT_MAX if invalid
 */
void CEntityInstance::GetOrigin(float & x, float & y, float & z) {
    auto ptr = *(u_char**)((u_char*)this + g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode);
	// See cl_ent_text drawing function. Near "Position: %0.3f, %0.3f, %0.3f\n" or cl_ent_viewoffset related function.
	auto vector = (float*)(ptr + g_clientDllOffsets.CGameSceneNode.m_vecAbsOrigin);
	x =  vector[0];
	y =  vector[1];
	z =  vector[2];
}

void CEntityInstance::GetRenderEyeOrigin(float outOrigin[3]) {
	((void (__fastcall *)(void *,float outOrigin[3])) (*(void***)this)[AFXADDR_GET(cs2_client_CEntityInstance_GetRenderEyeOrigin_vtable_idx)]) (this,outOrigin);
}

void CEntityInstance::GetRenderEyeAngles(float outAngles[3]) {
	((void (__fastcall *)(void *,float outAngles[3])) (*(void***)this)[AFXADDR_GET(cs2_client_CEntityInstance_GetRenderEyeAngles_vtable_idx)]) (this,outAngles);
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetViewEntityHandle() {
	if (!IsPlayerPawn())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
    void * pCameraServices = *(void**)((unsigned char*)this + g_clientDllOffsets.C_BasePlayerPawn.m_pCameraServices);
    if(nullptr == pCameraServices) return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
	return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int*)((unsigned char*)pCameraServices + g_clientDllOffsets.CPlayer_CameraServices.m_hViewEntity));
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetActiveWeaponHandle() {
	if (!IsPlayerPawn())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
    void * pWeaponServices = *(void**)((unsigned char*)this + g_clientDllOffsets.C_BasePlayerPawn.m_pWeaponServices);
    if(nullptr == pWeaponServices) return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
	return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int*)((unsigned char*)pWeaponServices + g_clientDllOffsets.CPlayer_WeaponServices.m_hActiveWeapon));
}

const char * CEntityInstance::GetPlayerName(){
    if (!IsPlayerController()) return nullptr;
    return (const char *)((u_char*)(this) + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName);
}

uint64_t CEntityInstance::GetSteamId(){
    if (!IsPlayerController())  return 0;
    return *(uint64_t*)((u_char*)(this) + g_clientDllOffsets.CBasePlayerController.m_steamID);
}

const char * CEntityInstance::GetSanitizedPlayerName() {
   if (!IsPlayerController()) return nullptr;
    return *(const char **)((u_char*)(this) + g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName);

}

uint8_t CEntityInstance::GetObserverMode() {
	if (!IsPlayerPawn()) return 0;
    void * pObserverServices = *(void**)((unsigned char*)this + g_clientDllOffsets.C_BasePlayerPawn.m_pObserverServices);
    if(nullptr == pObserverServices) return 0;
	return *(uint8_t*)((unsigned char*)pObserverServices + g_clientDllOffsets.CPlayer_ObserverServices.m_iObserverMode);    
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetObserverTarget() {
	if (!IsPlayerPawn())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
    void * pObserverServices = *(void**)((unsigned char*)this + g_clientDllOffsets.C_BasePlayerPawn.m_pObserverServices);
    if(nullptr == pObserverServices) return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
	return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int*)((unsigned char*)pObserverServices + g_clientDllOffsets.CPlayer_ObserverServices.m_hObserverTarget));    
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetHandle() {
	if (auto pEntityIdentity = *(u_char**)((u_char*)this + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
		return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(uint32_t*)(pEntityIdentity + 0x10));
	}

	return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetParentHandle() {
	auto defaultHandle = SOURCESDK::CS2::CEntityHandle::CEntityHandle();

	auto pGameSceneNode = *(u_char**)((u_char*)this + g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode);
	if (!pGameSceneNode) return defaultHandle;

	auto pParent = *(u_char**)(pGameSceneNode + g_clientDllOffsets.CGameSceneNode.m_pParent);
	if (!pParent) return defaultHandle;

	auto pOwner = *(CEntityInstance**)(pParent + g_clientDllOffsets.CGameSceneNode.m_pOwner);
	if (!pOwner) return defaultHandle;

	return pOwner->GetHandle();
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetRootHandle() {
	auto lastHandle = this->GetHandle();
	auto lastEntity = this;
	if (!lastHandle.IsValid()) return lastHandle;

	std::unordered_set<CEntityInstance*> entities {this};

	while (true) {
		auto parentHandle = lastEntity->GetParentHandle();
		if (!parentHandle.IsValid()) break;

		auto parentEntity = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,parentHandle.GetEntryIndex());
		if (!parentEntity) break;
		if (parentEntity->GetHandle() != parentHandle) break;

		if (!entities.insert(parentEntity).second) break;

		lastEntity = parentEntity;
		lastHandle = parentHandle;
	}

	return lastHandle;
}

typedef	void (__fastcall * org_LookupAttachment_t)(void* This, uint8_t& outIdx, const char* attachmentName);
org_LookupAttachment_t org_LookupAttachment = nullptr;

typedef	bool (__fastcall * org_GetAttachment_t)(void* This, uint8_t idx, void* out);
org_GetAttachment_t org_GetAttachment = nullptr;

uint8_t CEntityInstance::LookupAttachment(const char* attachmentName) {
	uint8_t idx = 0;
	org_LookupAttachment(this, idx, attachmentName);
	return idx;
}

bool CEntityInstance::GetAttachment(uint8_t idx, SOURCESDK::Vector &origin, SOURCESDK::Quaternion &angles) {
	alignas(16) float resData[8] = {0};

	if(org_GetAttachment(this, idx, resData)) {
		origin.x = resData[0];
		origin.y = resData[1];
		origin.z = resData[2];

		angles.x = resData[4];
		angles.y = resData[5];
		angles.z = resData[6];
		angles.w = resData[7];

		return true;
	}

	return false;
}

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
        g_GetHighestEntityIndex = (GetHighestEntityIndex_t)pFnGetHighestEntityIterator;
        g_GetEntityFromIndex = (GetEntityFromIndex_t)pFnGetEntityFromIndex;
        firstResult = true;
    }

    return firstResult;
}

bool Hook_ClientEntitySystem2() {
    static bool firstResult = false;
    static bool firstRun = true;

    if(g_pEntityList && *g_pEntityList) {
        void ** vtable = **(void****)g_pEntityList;
        g_Org_OnAddEntity = (OnAddEntity_t)vtable[AFXADDR_GET(cs2_client_CGameEntitySystem_OnAddEntity_vtable_idx)];
        g_Org_OnRemoveEntity = (OnRemoveEntity_t)vtable[AFXADDR_GET(cs2_client_CGameEntitySystem_OnRemoveEntity_vtable_idx)];
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Org_OnAddEntity, New_OnAddEntity);
        DetourAttach(&(PVOID&)g_Org_OnRemoveEntity, New_OnRemoveEntity);
        firstResult = NO_ERROR == DetourTransactionCommit();
    }

    return firstResult;    
}

void Hook_ClientEntitySystem3(HMODULE clientDll) {
	if (AFXADDR_GET(cs2_client_LookupAttachment) && AFXADDR_GET(cs2_client_GetAttachment)) {
		org_LookupAttachment = (org_LookupAttachment_t)AFXADDR_GET(cs2_client_LookupAttachment);
		org_GetAttachment = (org_GetAttachment_t)AFXADDR_GET(cs2_client_GetAttachment);
	}
}

int GetHighestEntityIndex() {
    return 0x7FFF; // Hardcoded for now, because the function we have is the count, not the index and we need to change mirv-script API to support that better.
    //return g_pEntityList && g_GetHighestEntityIndex ? g_GetHighestEntityIndex(*g_pEntityList, false) : -1;
}

struct MirvEntityEntry {
	int entryIndex;
	int handle;
	std::string debugName;
	std::string className;
	std::string clientClassName;
	SOURCESDK::Vector origin;
	SOURCESDK::QAngle angles;
};

CON_COMMAND(mirv_listentities, "List entities.")
{
	auto argC = args->ArgC();
	auto arg0 = args->ArgV(0);

	bool filterPlayers = false;
	bool sortByDistance = false;
	int printCount = -1;

	if (2 <= argC && 0 == _stricmp(args->ArgV(1), "help")) {
		advancedfx::Message(
			"%s help - Print this help.\n"
			"%s <option1> <option2> ... - Customize printed output with options.\n"
			"Where <option> is (you don't have to use all):\n"
			"\t\"isPlayer=1\" - Show only player related entities. Unless you need handles, the \"mirv_deathmsg help players\" might be more useful.\n"
			"\t\"sort=distance\" - Sort entities by distance relative to current position, from closest to most distant.\n"
			"\t\"limit=<i>\" - Limit number of printed entries.\n"
			"Example:\n"
			"%s sort=distance limit=10\n" 
			, arg0, arg0, arg0
		);
		return;
	} else {
		for (int i = 1; i < argC; i++) {
			const char * argI = args->ArgV(i);
			if (StringIBeginsWith(argI, "limit=")) {
				printCount = atoi(argI + strlen("limit="));
			} 
			else if (StringIBeginsWith(argI, "sort=")) {
				if (0 == _stricmp(argI + strlen("sort="), "distance")) sortByDistance = true;
			}
			else if (0 == _stricmp(argI, "isPlayer=1")) {
				filterPlayers = true;
			}
		}
	}

	std::vector<MirvEntityEntry> entries;

    int highestIndex = GetHighestEntityIndex();
    for(int i = 0; i < highestIndex + 1; i++) {
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
			if (filterPlayers && !ent->IsPlayerController() && !ent->IsPlayerPawn()) continue;
			
            float render_origin[3];
            float render_angles[3];
            ent->GetRenderEyeOrigin(render_origin);
            ent->GetRenderEyeAngles(render_angles);

			auto debugName = ent->GetDebugName();
			auto className = ent->GetClassName();
			auto clientClassName = ent->GetClientClassName();

			entries.emplace_back(
				MirvEntityEntry {
					i, ent->GetHandle().ToInt(), 
					debugName ? debugName : "", className ? className : "", clientClassName ? clientClassName : "",
					SOURCESDK::Vector {render_origin[0], render_origin[1], render_origin[2]},
					SOURCESDK::QAngle {render_angles[0], render_angles[1], render_angles[2]} 
				}
			);

        }
    }

	if (sortByDistance) {
		SOURCESDK::Vector curPos = {(float)g_CurrentGameCamera.origin[0], (float)g_CurrentGameCamera.origin[1], (float)g_CurrentGameCamera.origin[2]};

		std::sort(entries.begin(), entries.end(), [&](MirvEntityEntry & a, MirvEntityEntry & b) {
			auto distA = (curPos - a.origin).LengthSqr();
			auto distB = (curPos - b.origin).LengthSqr();
			return distA < distB;
		});
	}

	advancedfx::Message("entryIndex / handle / debugName / className / clientClassName / [ x , y , z , rX , rY , rZ ]\n");
	if (printCount == -1) printCount = entries.size();
	for (int i = 0; i < printCount; i++) {
		auto e = entries[i];
		advancedfx::Message("%i / %i / %s / %s / %s / [ %f , %f , %f , %f , %f , %f ]\n"
			, e.entryIndex, e.handle
			, e.debugName.c_str(), e.className.c_str(), e.clientClassName.c_str()
			, e.origin.x, e.origin.y, e.origin.z 
			, e.angles.x, e.angles.y, e.angles.z
		);
	}
}

extern "C" int afx_hook_source2_get_highest_entity_index() {
    int highestIndex = GetHighestEntityIndex();
    return highestIndex;
}

extern "C" void * afx_hook_source2_get_entity_ref_from_index(int index) {
    if(CEntityInstance * result = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,index)) {
        return CAfxEntityInstanceRef::Aquire(result);
    }
    return nullptr;
}

extern "C" void afx_hook_source2_add_ref_entity_ref(void * pRef) {
    ((CAfxEntityInstanceRef *)pRef)->AddRef();
}

extern "C" void afx_hook_source2_release_entity_ref(void * pRef) {
    ((CAfxEntityInstanceRef *)pRef)->Release();
}

extern "C" FFIBool afx_hook_source2_get_entity_ref_is_valid(void * pRef) {
    return BOOL_TO_FFIBOOL(((CAfxEntityInstanceRef *)pRef)->IsValid());
}

extern "C" const char * afx_hook_source2_get_entity_ref_name(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetName();
    }
    return "";
}

extern "C" const char * afx_hook_source2_get_entity_ref_debug_name(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetDebugName();
    }
    return nullptr;
}

extern "C" const char * afx_hook_source2_get_entity_ref_class_name(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetClassName();
    }
    return "";
}

extern "C" const char * afx_hook_source2_get_entity_ref_client_class_name(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetClientClassName();
    }
    return "";
}

extern "C" FFIBool afx_hook_source2_get_entity_ref_is_player_pawn(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return BOOL_TO_FFIBOOL(pInstance->IsPlayerPawn());
    }
    return FFIBOOL_FALSE;
}

extern "C" int afx_hook_source2_get_entity_ref_player_pawn_handle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetPlayerPawnHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;    
}

extern "C" FFIBool afx_hook_source2_get_entity_ref_is_player_controller(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return BOOL_TO_FFIBOOL(pInstance->IsPlayerController());
    }
    return FFIBOOL_FALSE;    
}

extern "C" int afx_hook_source2_get_entity_ref_player_controller_handle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetPlayerControllerHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;  
}

extern "C" int afx_hook_source2_get_entity_ref_health(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetHealth();
    }
    return 0;    
}

extern "C" int afx_hook_source2_get_entity_ref_team(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        return pInstance->GetTeam();
    }
    return 0;    
}


extern "C" void afx_hook_source2_get_entity_ref_origin(void * pRef, float & x, float & y, float & z) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       pInstance->GetOrigin(x,y,z);
    }    
}

extern "C" void afx_hook_source2_get_entity_ref_render_eye_origin(void * pRef, float & x, float & y, float & z) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        float tmp[3];
       pInstance->GetRenderEyeOrigin(tmp);
       x = tmp[0];
       y = tmp[1];
       z = tmp[2];
    }    
}

extern "C" void afx_hook_source2_get_entity_ref_render_eye_angles(void * pRef, float & x, float & y, float & z) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
        float tmp[3];
       pInstance->GetRenderEyeAngles(tmp);
       x = tmp[0];
       y = tmp[1];
       z = tmp[2];
    }    
}

extern "C" int afx_hook_source2_get_entity_ref_view_entity_handle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetViewEntityHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
}

extern "C" int afx_hook_source2_get_entity_ref_active_weapon_handle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetActiveWeaponHandle().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
}

extern "C" const char* afx_hook_source2_get_entity_ref_player_name(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetPlayerName();
    }
    return nullptr;
}

extern "C" uint64_t afx_hook_source2_get_entity_ref_steam_id(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetSteamId();
    }
    return 0;
}

extern "C" const char* afx_hook_source2_get_entity_ref_sanitized_player_name(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetSanitizedPlayerName();
    }
    return nullptr;
}

typedef CEntityInstance *  (__fastcall * ClientDll_GetSplitScreenPlayer_t)(int slot);
ClientDll_GetSplitScreenPlayer_t g_ClientDll_GetSplitScreenPlayer = nullptr;

bool Hook_GetSplitScreenPlayer( void* pAddr) {
    g_ClientDll_GetSplitScreenPlayer = (ClientDll_GetSplitScreenPlayer_t)pAddr;
    return true;
}

extern "C" void * afx_hook_source2_get_entity_ref_from_split_screen_player(int index) {
    if(0 == index && g_ClientDll_GetSplitScreenPlayer) {
        if(CEntityInstance * result = g_ClientDll_GetSplitScreenPlayer(index)) {
            return CAfxEntityInstanceRef::Aquire(result);
        }
    }
    return nullptr;
}

extern "C" uint8_t afx_hook_source2_get_entity_ref_observer_mode(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetObserverMode();
    }
    return 0;
}

extern "C" int afx_hook_source2_get_entity_ref_observer_target_handle(void * pRef) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
       return pInstance->GetObserverTarget().ToInt();
    }
    return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
}

extern "C" int afx_hook_source2_get_entity_ref_parent_handle(void * pRef) {
	if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
		return pInstance->GetParentHandle().ToInt();
	}
	return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
}

extern "C" int afx_hook_source2_get_entity_ref_root_handle(void * pRef) {
	if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
		return pInstance->GetRootHandle().ToInt();
	}
	return SOURCESDK_CS2_INVALID_EHANDLE_INDEX;
}

extern "C" FFIBool afx_hook_source2_get_entity_ref_attachment(void * pRef, const char* attachmentName, double outPosition[3], double outAngles[4]) {
    if(auto pInstance = ((CAfxEntityInstanceRef *)pRef)->GetInstance()) {
		auto idx = pInstance->LookupAttachment(attachmentName);
		if (0 == idx) return FFIBOOL_FALSE;
		
		SOURCESDK::Vector origin;
		SOURCESDK::Quaternion angles;

		if (pInstance->GetAttachment(idx, origin, angles)) {
			outPosition[0] = origin.x;
			outPosition[1] = origin.y;
			outPosition[2] = origin.z;

			outAngles[0] = angles.w;
			outAngles[1] = angles.x;
			outAngles[2] = angles.y;
			outAngles[3] = angles.z;

			return FFIBOOL_TRUE;
		}
    }

    return FFIBOOL_FALSE;
}
