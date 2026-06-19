#include "stdafx.h"

#include "ClientEntitySystem.h"
#include "DeathMsg.h"
#include "WrpConsole.h"
#include "Globals.h"
#include "MirvTime.h"

#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"

#include "../shared/AfxConsole.h"
#include "../shared/binutils.h"
#include "../shared/AfxDetours.h"
#include "../shared/FFITools.h"
#include "../shared/StringTools.h"

#include "AfxHookSource2Rs.h"
#include "SchemaSystem.h"

#define WIN32_LEAN_AND_MEAN
#include "../deps/release/Detours/src/detours.h"

#include <map>
#include <algorithm>
#include <intrin.h>
#include <limits.h>

#pragma intrinsic(_ReturnAddress)

void ** g_pEntityList = nullptr;
GetHighestEntityIndex_t  g_GetHighestEntityIndex = nullptr;
GetEntityFromIndex_t g_GetEntityFromIndex = nullptr;

typedef CEntityInstance * (__fastcall * ClientDll_GetSplitScreenPlayer_t)(int slot);
static ClientDll_GetSplitScreenPlayer_t g_ClientDll_GetSplitScreenPlayer = nullptr;
extern ClientDll_GetSplitScreenPlayer_t g_Org_ClientDll_GetSplitScreenPlayer;

static int g_FakePovRadarControllerIndex = 0;
static bool g_MirvPovAutoSync = false;
static bool g_MirvPovEnabled = false;

struct FakePovRadarFrameContextState {
    bool active = false;
    CEntityInstance * realController = nullptr;
};

struct SpottedRestoreEntry {
    int pawnEntryIndex;
    uint8_t originalSpotted;
    uint32_t originalMask[2];
};

static constexpr int kMaxSpottedRestoreEntries = 64;

static FakePovRadarFrameContextState g_FakePovRadarFrameContextState;
static SpottedRestoreEntry g_SpottedRestoreEntries[kMaxSpottedRestoreEntries];
static int g_SpottedRestoreCount = 0;
static int g_FakePovRadarLastDemoTick = 0;
static bool g_FakePovRadarIsBackwardJump = false;
static bool g_FakePovRadarFrameContextWasActive = false;

static int g_IsLocalPlayerHLTV_SuppressFrames = 0;
static int g_IsLocalPlayerHLTV_LastDemoTick = -1;

static constexpr int kMirvPovIsPlayingDemoVtableIndex = 42;
static constexpr int kMirvPovVoiceSeekThresholdTicks = 16;
static constexpr float kMirvPovSyntheticSpeakingSeconds = 0.65f;

static int g_MirvPovVoiceLastDemoTick = INT_MIN;
static int g_MirvPovVoiceLastControllerIndex = -1;
static int g_MirvPovVoiceLastTeam = -1;
static float g_MirvPovSyntheticSpeakingUntil[64] = {};
static size_t g_MirvPovShowSpeakerRetAddr = 0;
static size_t g_MirvPovServerVoiceDataAddr = 0;
static size_t g_MirvPovVoiceStatusGetAddr = 0;
static size_t g_MirvPovVoiceStatusUpdateSpeakerStatusAddr = 0;

typedef bool (*MirvPov_IsPlayingDemo_t)(void * This);
static MirvPov_IsPlayingDemo_t g_Org_MirvPov_IsPlayingDemo = nullptr;
static bool g_bMirvPovIsPlayingDemoHooked = false;

typedef __int64 (__fastcall * MirvPov_ServerVoiceData_t)(__int64 This, __int64 msg);
static MirvPov_ServerVoiceData_t g_Org_MirvPov_ServerVoiceData = nullptr;
static bool g_bMirvPovServerVoiceDataHooked = false;

typedef __int64 (__fastcall * MirvPov_VoiceStatus_Get_t)();
typedef __int64 (__fastcall * MirvPov_VoiceStatus_UpdateSpeakerStatus_t)(__int64 voiceStatus, unsigned int playerSlot, int localSlot, unsigned __int8 talking);

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
	return **(const char***)(*(unsigned char**)(*(unsigned char**)((unsigned char*)this + 0x10) + 0x8)+0x50);
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
    // GetClientClass function.
    // find it by searching for 4th full-ptr ref to "C_PlantedC4" subtract sizeof(void*) (0x8) and search function that references this struct.
    // you need to search for raw bytes, GiHidra doesn't seem to find the reference.
    void * pClientClass = ((void * (__fastcall *)(void *)) (*(void***)this)[43]) (this);

    if(pClientClass) {
        return *(const char**)((unsigned char*)pClientClass + 0x10);
    }
    return nullptr;
}

// Retrieved from script function.
// GetEntityHandle ...

bool CEntityInstance::IsPlayerPawn() {
	// See cl_ent_text drawing function.
	return ((bool (__fastcall *)(void *)) (*(void***)this)[153]) (this);
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetPlayerPawnHandle() {
	// See cl_ent_text drawing function.
	if(!IsPlayerController())  return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
	return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(unsigned int *)((unsigned char *)this + g_clientDllOffsets.CBasePlayerController.m_hPawn));
}

bool CEntityInstance::IsPlayerController() {
	// See cl_ent_text drawing function. Near "Pawn: (%d) Name: %s".
	return ((bool (__fastcall *)(void *)) (*(void***)this)[154]) (this);    
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
	// GetRenderEyeAngles vtable offset minus 1
	((void (__fastcall *)(void *,float outOrigin[3])) (*(void***)this)[168]) (this,outOrigin);
}

void CEntityInstance::GetRenderEyeAngles(float outAngles[3]) {
	// See cl_track_render_eye_angles. Near "Render eye angles: %.7f, %.7f, %.7f\n".
	((void (__fastcall *)(void *,float outAngles[3])) (*(void***)this)[169]) (this,outAngles);
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
    return *(const char **)((u_char*)(this) + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName);
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

bool CEntityInstance::GetSpottedState(bool & spotted, uint32_t & mask0, uint32_t & mask1) {
	spotted = false;
	mask0 = 0;
	mask1 = 0;

	// Temporarily disabled - requires offsets not yet in Rust code
	// TODO: Add C_CSPlayerPawnBase.m_entitySpottedState and EntitySpottedState_t offsets
	return false;

	/*
	if (!IsPlayerPawn()) return false;
	if (0 == g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState) return false;

	auto spottedState = (unsigned char*)this + g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState;
	spotted = 0 != *(uint8_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpotted);
	auto maskPtr = (uint32_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpottedByMask);
	mask0 = maskPtr[0];
	mask1 = maskPtr[1];
	return true;
	*/
}

SOURCESDK::CS2::CBaseHandle CEntityInstance::GetHandle() {
	if (auto pEntityIdentity = *(u_char**)((u_char*)this + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
		return SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(uint32_t*)(pEntityIdentity + 0x10));
	}

	return SOURCESDK::CS2::CEntityHandle::CEntityHandle();
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
        // https://github.com/bruhmoment21/cs2-sdk
        void ** vtable = **(void****)g_pEntityList;
        g_Org_OnAddEntity = (OnAddEntity_t)vtable[15];
        g_Org_OnRemoveEntity = (OnRemoveEntity_t)vtable[16];
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Org_OnAddEntity, New_OnAddEntity);
        DetourAttach(&(PVOID&)g_Org_OnRemoveEntity, New_OnRemoveEntity);
        firstResult = NO_ERROR == DetourTransactionCommit();
    }

    return firstResult;    
}

void Hook_ClientEntitySystem3(HMODULE clientDll) {
	// these two called one after each other
	// there is only one placed where they are called together
	//
	//                             LAB_1802066cd                                   XREF[1]:     18020660b (j)   
    // 1802066cd 8b  4f  6c       MOV        ECX ,dword ptr [RDI  + 0x6c ]
    // 1802066d0 83  e9  01       SUB        ECX ,0x1
    // 1802066d3 0f  84  3d       JZ         LAB_180206816
    //           01  00  00
    // 1802066d9 83  f9  01       CMP        ECX ,0x1
    // 1802066dc 75  4c           JNZ        LAB_18020672a
    // 1802066de 4c  8b  47  30    MOV        R8,qword ptr [RDI  + 0x30 ]
    // 1802066e2 48  8d  95       LEA        RDX =>Stack [0x28 ],[RBP  + 0xc0 ]
    //           c0  00  00  00
    // 1802066e9 48  8b  ce       MOV        RCX ,RSI
    // 1802066ec e8  9f  bf       CALL       FUN_1808c2690                                    undefined FUN_1808c2690()
    //           6b  00
    // 1802066f1 0f  b6  95       MOVZX      EDX ,byte ptr [RBP  + Stack [0x28 ]]
    //           c0  00  00  00
    // 1802066f8 84  d2           TEST       DL,DL
    // 1802066fa 74  29           JZ         LAB_180206725
    // 1802066fc 4c  8d  45  b0    LEA        R8=>local_e8 ,[RBP  + -0x50 ]
    // 180206700 48  8b  ce       MOV        RCX ,RSI
    // 180206703 e8  78  ca       CALL       FUN_1808b3180                                    undefined FUN_1808b3180()
	//
	// to find this place find function with 5 arguments near "Unable to create non-precached breakable%s\n"
	// then in that function find place like this
	//
	//   else if (((*(int *)((longlong)param_4 + 0x6c) == 2) &&
    //          (FUN_1808c2690(param_3,&param_5,param_4[6]), (char)param_5 != '\0')) &&
    //         (FUN_1808b3180(param_3,(char)param_5,&local_e8), cVar2 != '\0')) {
    //   FUN_1815f22b0(param_2,(char)param_5,local_158,&local_108);
    // }
	//
	// first function can be found near "attachment_point" or called with "muzzle_flash" as last arg
	// second function in some places can be found with offset to m_nAttachmentIndex of CEffectData as 2nd arg

	if (auto startAddr = getAddress(clientDll, "E8 ?? ?? ?? ?? 0F B6 95 ?? ?? ?? ?? 84 D2 74 29 4C 8D 45 B0 48 8B CE E8 ?? ?? ?? ??")) {
		org_LookupAttachment = (org_LookupAttachment_t)(startAddr + 5 + *(int32_t*)(startAddr + 1));
		org_GetAttachment = (org_GetAttachment_t)(startAddr + 23 + 5 + *(int32_t*)(startAddr + 23 + 1));
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));
}

int GetHighestEntityIndex() {
    return 2048; // Hardcoded for now, because the function we have is the count, not the index and we need to change mirv-script API to support that better.
    //return g_pEntityList && g_GetHighestEntityIndex ? g_GetHighestEntityIndex(*g_pEntityList, false) : -1;
}

static uint8_t * GetTeamFieldPtr(CEntityInstance * entity) {
    if(nullptr == entity) return nullptr;
    return (uint8_t *)((unsigned char *)entity + g_clientDllOffsets.C_BaseEntity.m_iTeamNum);
}

static unsigned int * GetControllerPawnHandleFieldPtr(CEntityInstance * controller) {
    if(nullptr == controller || !controller->IsPlayerController()) return nullptr;
    return (unsigned int *)((unsigned char *)controller + g_clientDllOffsets.CBasePlayerController.m_hPawn);
}

static unsigned int * GetControllerPlayerPawnHandleFieldPtr(CEntityInstance * controller) {
    // Temporarily disabled - requires m_hPlayerPawn offset
    return nullptr;
    /*
    if(nullptr == controller || !controller->IsPlayerController()) return nullptr;
    if(0 == g_clientDllOffsets.CCSPlayerController.m_hPlayerPawn) return nullptr;
    return (unsigned int *)((unsigned char *)controller + g_clientDllOffsets.CCSPlayerController.m_hPlayerPawn);
    */
}

static unsigned int * GetControllerObserverPawnHandleFieldPtr(CEntityInstance * controller) {
    // Temporarily disabled - requires m_hObserverPawn offset
    return nullptr;
    /*
    if(nullptr == controller || !controller->IsPlayerController()) return nullptr;
    if(0 == g_clientDllOffsets.CCSPlayerController.m_hObserverPawn) return nullptr;
    return (unsigned int *)((unsigned char *)controller + g_clientDllOffsets.CCSPlayerController.m_hObserverPawn);
    */
}

static void * GetObserverServicesPtr(CEntityInstance * pawn) {
    if(nullptr == pawn || !pawn->IsPlayerPawn()) return nullptr;
    return *(void **)((unsigned char *)pawn + g_clientDllOffsets.C_BasePlayerPawn.m_pObserverServices);
}

static void * GetObserverServicesPtrUnchecked(CEntityInstance * pawn) {
    if(nullptr == pawn || 0 == g_clientDllOffsets.C_BasePlayerPawn.m_pObserverServices) return nullptr;
    return *(void **)((unsigned char *)pawn + g_clientDllOffsets.C_BasePlayerPawn.m_pObserverServices);
}

static void * GetCameraServicesPtr(CEntityInstance * pawn) {
    if(nullptr == pawn || !pawn->IsPlayerPawn()) return nullptr;
    return *(void **)((unsigned char *)pawn + g_clientDllOffsets.C_BasePlayerPawn.m_pCameraServices);
}

static uint8_t * GetObserverModeFieldPtr(CEntityInstance * pawn) {
    if(void * pObserverServices = GetObserverServicesPtr(pawn)) {
        return (uint8_t *)((unsigned char *)pObserverServices + g_clientDllOffsets.CPlayer_ObserverServices.m_iObserverMode);
    }
    return nullptr;
}

static unsigned int * GetObserverTargetFieldPtr(CEntityInstance * pawn) {
    if(void * pObserverServices = GetObserverServicesPtr(pawn)) {
        return (unsigned int *)((unsigned char *)pObserverServices + g_clientDllOffsets.CPlayer_ObserverServices.m_hObserverTarget);
    }
    return nullptr;
}

static uint8_t * GetObserverModeFieldPtrUnchecked(CEntityInstance * pawn) {
    if(void * pObserverServices = GetObserverServicesPtrUnchecked(pawn)) {
        return (uint8_t *)((unsigned char *)pObserverServices + g_clientDllOffsets.CPlayer_ObserverServices.m_iObserverMode);
    }
    return nullptr;
}

static unsigned int * GetObserverTargetFieldPtrUnchecked(CEntityInstance * pawn) {
    if(void * pObserverServices = GetObserverServicesPtrUnchecked(pawn)) {
        return (unsigned int *)((unsigned char *)pObserverServices + g_clientDllOffsets.CPlayer_ObserverServices.m_hObserverTarget);
    }
    return nullptr;
}

static unsigned int * GetViewEntityFieldPtr(CEntityInstance * pawn) {
    if(void * pCameraServices = GetCameraServicesPtr(pawn)) {
        return (unsigned int *)((unsigned char *)pCameraServices + g_clientDllOffsets.CPlayer_CameraServices.m_hViewEntity);
    }
    return nullptr;
}

CEntityInstance * GetEntityFromIndex(int index) {
    if(index < 0 || nullptr == g_pEntityList || nullptr == *g_pEntityList || nullptr == g_GetEntityFromIndex) return nullptr;
    return (CEntityInstance *)g_GetEntityFromIndex(*g_pEntityList, index);
}

CEntityInstance * GetRealSplitScreenPlayer(int slot) {
    if(nullptr != g_Org_ClientDll_GetSplitScreenPlayer) {
        return g_Org_ClientDll_GetSplitScreenPlayer(slot);
    }

    if(nullptr == g_ClientDll_GetSplitScreenPlayer) return nullptr;
    return g_ClientDll_GetSplitScreenPlayer(slot);
}

static int g_AutoSyncDebugStep = 0; // 0=no debug, set to step# where it fails

CEntityInstance * GetFakePovRadarController() {
    if(g_MirvPovAutoSync) {
        CEntityInstance * realController = GetRealSplitScreenPlayer(0);
        if(nullptr == realController) { g_AutoSyncDebugStep = 1; return nullptr; }
        auto pawnHandle = realController->GetPlayerPawnHandle();
        if(!pawnHandle.IsValid()) { g_AutoSyncDebugStep = 2; return nullptr; }
        CEntityInstance * realPawn = GetEntityFromIndex(pawnHandle.GetEntryIndex());
        if(nullptr == realPawn) { g_AutoSyncDebugStep = 3; return nullptr; }

        uint8_t * pObsMode = GetObserverModeFieldPtrUnchecked(realPawn);
        if(nullptr == pObsMode) { g_AutoSyncDebugStep = 4; return nullptr; }
        if(0 == *pObsMode) { g_AutoSyncDebugStep = 5; return nullptr; }

        unsigned int * pObsTarget = GetObserverTargetFieldPtrUnchecked(realPawn);
        if(nullptr == pObsTarget) { g_AutoSyncDebugStep = 6; return nullptr; }
        SOURCESDK::CS2::CBaseHandle targetHandle(*pObsTarget);
        if(!targetHandle.IsValid()) { g_AutoSyncDebugStep = 7; return nullptr; }

        CEntityInstance * targetPawn = GetEntityFromIndex(targetHandle.GetEntryIndex());
        if(nullptr == targetPawn) { g_AutoSyncDebugStep = 8; return nullptr; }

        auto controllerHandle = targetPawn->GetPlayerControllerHandle();
        if(!controllerHandle.IsValid()) { g_AutoSyncDebugStep = 9; return nullptr; }
        CEntityInstance * targetController = GetEntityFromIndex(controllerHandle.GetEntryIndex());
        if(nullptr == targetController) { g_AutoSyncDebugStep = 10; return nullptr; }
        if(!targetController->IsPlayerController()) { g_AutoSyncDebugStep = 11; return nullptr; }
        g_AutoSyncDebugStep = 0;
        return targetController;
    }

    if(g_FakePovRadarControllerIndex <= 0) return nullptr;
    return GetEntityFromIndex(g_FakePovRadarControllerIndex);
}

int GetAutoSyncDebugStep() { return g_AutoSyncDebugStep; }

CEntityInstance * GetEffectiveSplitScreenPlayer(int slot) {
    if(0 == slot) {
        if(CEntityInstance * fake = GetFakePovRadarController()) {
            return fake;
        }
    }
    return GetRealSplitScreenPlayer(slot);
}

bool IsFakePovRadarFrameContextActive() {
    return g_FakePovRadarFrameContextState.active;
}

bool ConsumeFakePovRadarFrameContextWasActive() {
    bool result = g_FakePovRadarFrameContextWasActive;
    g_FakePovRadarFrameContextWasActive = false;
    return result;
}

void SetFakePovRadarControllerIndex(int index) {
    g_FakePovRadarControllerIndex = 0 < index ? index : 0;
    g_MirvPovAutoSync = false;
}

void SetFakePovRadarAutoSync(bool enabled) {
    g_MirvPovAutoSync = enabled;
    if(enabled) g_FakePovRadarControllerIndex = -1;
}

bool GetFakePovRadarAutoSync() {
    return g_MirvPovAutoSync;
}

int GetFakePovRadarControllerIndex() {
    return g_FakePovRadarControllerIndex;
}

bool MirvPov_IsEnabled() {
    return g_MirvPovEnabled;
}

static void MirvPov_UpdateVoiceTeam() {
    if(!g_MirvPovEnabled || !g_pEngineToClient) return;

    CEntityInstance * controller = GetFakePovRadarController();
    int controllerIndex = GetFakePovRadarControllerIndex();
    if(nullptr == controller || !controller->IsPlayerController()) return;
    if(controllerIndex <= 0) controllerIndex = controller->GetHandle().GetEntryIndex();

    if(g_MirvPovVoiceLastControllerIndex == controllerIndex) return;
    g_MirvPovVoiceLastControllerIndex = controllerIndex;

    int team = controller->GetTeam();
    if(team != 2 && team != 3) return;
    if(g_MirvPovVoiceLastTeam == team) return;

    g_pEngineToClient->ExecuteClientCmd(0, 2 == team ? "mirv_script_voice t" : "mirv_script_voice ct", true);
    g_MirvPovVoiceLastTeam = team;
}

static bool MirvPov_IsVoiceHudReady() {
    return g_MirvPovVoiceStatusGetAddr && g_MirvPovVoiceStatusUpdateSpeakerStatusAddr;
}

static void MirvPov_SetSyntheticSpeaking(unsigned int playerSlot, bool speaking) {
    if(64 <= playerSlot || !MirvPov_IsVoiceHudReady()) return;

    __int64 voiceStatus = ((MirvPov_VoiceStatus_Get_t)g_MirvPovVoiceStatusGetAddr)();
    if(!voiceStatus) return;

    ((MirvPov_VoiceStatus_UpdateSpeakerStatus_t)g_MirvPovVoiceStatusUpdateSpeakerStatusAddr)(voiceStatus, playerSlot, -1, speaking ? 1 : 0);
    g_MirvPovSyntheticSpeakingUntil[playerSlot] = speaking ? g_MirvTime.curtime_get() + kMirvPovSyntheticSpeakingSeconds : 0.0f;
}

static bool MirvPov_IsVoicePlayerSlotOnWatchedTeam(unsigned int playerSlot) {
    if(64 <= playerSlot) return false;

    int watchedTeam = g_MirvPovVoiceLastTeam;
    if(watchedTeam != 2 && watchedTeam != 3) {
        CEntityInstance * controller = GetFakePovRadarController();
        if(nullptr == controller || !controller->IsPlayerController()) return false;
        watchedTeam = controller->GetTeam();
    }
    if(watchedTeam != 2 && watchedTeam != 3) return false;

    CEntityInstance * voiceController = GetEntityFromIndex((int)playerSlot + 1);
    if(nullptr == voiceController || !voiceController->IsPlayerController()) return false;

    return voiceController->GetTeam() == watchedTeam;
}

static void MirvPov_ClearSyntheticSpeaking() {
    for(int i = 0; i < 64; ++i) {
        if(0.0f < g_MirvPovSyntheticSpeakingUntil[i]) {
            MirvPov_SetSyntheticSpeaking(i, false);
        }
        g_MirvPovSyntheticSpeakingUntil[i] = 0.0f;
    }
}

static void MirvPov_UpdateSyntheticSpeakingExpiry() {
    float curTime = g_MirvTime.curtime_get();
    for(int i = 0; i < 64; ++i) {
        if(0.0f < g_MirvPovSyntheticSpeakingUntil[i] && g_MirvPovSyntheticSpeakingUntil[i] <= curTime) {
            MirvPov_SetSyntheticSpeaking(i, false);
        }
    }
}

static bool New_MirvPov_IsPlayingDemo(void * This) {
    void * ret = _ReturnAddress();
    bool result = g_Org_MirvPov_IsPlayingDemo(This);
    if(g_MirvPovEnabled && g_MirvPovShowSpeakerRetAddr && (size_t)ret == g_MirvPovShowSpeakerRetAddr) {
        return false;
    }
    return result;
}

static __int64 __fastcall New_MirvPov_ServerVoiceData(__int64 This, __int64 msg) {
    unsigned int playerSlot = msg ? *(unsigned int *)(msg + 104) : 0xFFFFFFFF;
    __int64 result = g_Org_MirvPov_ServerVoiceData(This, msg);
    if(g_MirvPovEnabled && MirvPov_IsVoicePlayerSlotOnWatchedTeam(playerSlot)) {
        MirvPov_SetSyntheticSpeaking(playerSlot, true);
    }
    return result;
}

static bool MirvPov_ResolveVoiceHud(HMODULE clientDll) {
    if(!clientDll) return false;

    if(!g_MirvPovShowSpeakerRetAddr) {
        size_t matchAddr = getAddress(clientDll, "48 63 C3 48 8D 0D ?? ?? ?? ?? C6 84 08 ?? ?? ?? ?? 01 48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 90 ?? ?? ?? ?? 84 C0 0F 85");
        if(matchAddr) g_MirvPovShowSpeakerRetAddr = matchAddr + 0x22;
    }
    if(!g_MirvPovServerVoiceDataAddr) {
        g_MirvPovServerVoiceDataAddr = getAddress(clientDll, "48 89 4C 24 ?? 53 55 56 57 41 54 41 55 41 57 48 81 EC");
    }
    if(!g_MirvPovVoiceStatusGetAddr) {
        g_MirvPovVoiceStatusGetAddr = getAddress(clientDll, "48 8B 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 8D 05");
    }
    if(!g_MirvPovVoiceStatusUpdateSpeakerStatusAddr) {
        g_MirvPovVoiceStatusUpdateSpeakerStatusAddr = getAddress(clientDll, "44 88 4C 24 ?? 44 89 44 24 ?? 89 54 24");
    }

    return g_MirvPovShowSpeakerRetAddr && g_MirvPovServerVoiceDataAddr && MirvPov_IsVoiceHudReady();
}

static bool MirvPov_HookVoiceHud(HMODULE clientDll) {
    if(!MirvPov_ResolveVoiceHud(clientDll)) {
        advancedfx::Message("[mirv_pov_voice_hud] voice HUD patterns not found\n");
        return false;
    }

    if(!g_bMirvPovIsPlayingDemoHooked) {
        if(!g_pEngineToClient) return false;
        void ** vtable = *(void ***)g_pEngineToClient;
        if(!vtable) return false;
        if(!AfxDetourPtr((PVOID*)&(vtable[kMirvPovIsPlayingDemoVtableIndex]), New_MirvPov_IsPlayingDemo, (PVOID*)&g_Org_MirvPov_IsPlayingDemo)) return false;
        g_bMirvPovIsPlayingDemoHooked = true;
    }

    if(!g_bMirvPovServerVoiceDataHooked) {
        g_Org_MirvPov_ServerVoiceData = (MirvPov_ServerVoiceData_t)g_MirvPovServerVoiceDataAddr;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Org_MirvPov_ServerVoiceData, New_MirvPov_ServerVoiceData);
        if(NO_ERROR != DetourTransactionCommit()) return false;
        g_bMirvPovServerVoiceDataHooked = true;
    }

    return true;
}

static void MirvPov_UpdateVoiceHud() {
    if(!MirvPov_IsEnabled() || !g_pEngineToClient) return;

    SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile();
    if(!pDemoFile) {
        g_MirvPovVoiceLastDemoTick = INT_MIN;
        MirvPov_ClearSyntheticSpeaking();
        return;
    }

    int curTick = pDemoFile->GetDemoTick();
    if(g_MirvPovVoiceLastDemoTick != INT_MIN) {
        int delta = curTick - g_MirvPovVoiceLastDemoTick;
        if(delta < 0 || delta > kMirvPovVoiceSeekThresholdTicks) {
            g_pEngineToClient->ExecuteClientCmd(0, "servervoice_clear", true);
            MirvPov_ClearSyntheticSpeaking();
        }
    }
    g_MirvPovVoiceLastDemoTick = curTick;
    MirvPov_UpdateVoiceTeam();
    MirvPov_UpdateSyntheticSpeakingExpiry();
}

void MirvPov_RestorePersistentIdentity() {
}

void MirvPov_UpdateSeekDetection() {
    if(!MirvPov_IsEnabled()) return;
    MirvPov_UpdateVoiceHud();
    if(!g_pEngineToClient) return;
    SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile();
    if(!pDemoFile) return;
    int curTick = pDemoFile->GetDemoTick();
    if(g_IsLocalPlayerHLTV_LastDemoTick >= 0) {
        int delta = curTick - g_IsLocalPlayerHLTV_LastDemoTick;
        if(delta < 0) delta = -delta;
        if(delta > 2) {
            g_IsLocalPlayerHLTV_SuppressFrames = 16;
        }
    }
    g_IsLocalPlayerHLTV_LastDemoTick = curTick;
    if(g_IsLocalPlayerHLTV_SuppressFrames > 0) {
        g_IsLocalPlayerHLTV_SuppressFrames--;
    }
}

void MirvPov_UpdatePersistentIdentity() {
}

int CEntityInstance_GetCompTeammateColor(CEntityInstance * controller) {
    return -1;
}

void MirvPov_BeginFrame() {
    // Temporarily disabled - requires EntitySpottedState_t offsets
    return;
    /*
    if(g_FakePovRadarFrameContextState.active) return;
    if(g_IsLocalPlayerHLTV_SuppressFrames > 0) return;
    if(!MirvPov_IsEnabled()) return;

    CEntityInstance * fakeController = GetFakePovRadarController();
    CEntityInstance * realController = GetRealSplitScreenPlayer(0);
    if(nullptr == fakeController || nullptr == realController || fakeController == realController) return;

    g_FakePovRadarFrameContextState.active = true;
    g_FakePovRadarFrameContextWasActive = true;
    g_FakePovRadarFrameContextState.realController = realController;

    g_SpottedRestoreCount = 0;
    int fakeTeam = fakeController->GetTeam();
    if(fakeTeam == 2 || fakeTeam == 3) {
        int highestIndex = GetHighestEntityIndex();
        for(int i = 0; i < highestIndex + 1; ++i) {
            CEntityInstance * controller = GetEntityFromIndex(i);
            if(nullptr == controller || !controller->IsPlayerController()) continue;
            if(controller->GetTeam() != fakeTeam) continue;
            if(controller == fakeController) continue;

            auto pawnHandle = controller->GetPlayerPawnHandle();
            if(!pawnHandle.IsValid()) continue;
            CEntityInstance * pawn = GetEntityFromIndex(pawnHandle.GetEntryIndex());
            if(nullptr == pawn || !pawn->IsPlayerPawn()) continue;
            if(0 == g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState) continue;

            auto spottedState = (unsigned char*)pawn + g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState;
            uint8_t * pSpotted = (uint8_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpotted);
            uint32_t * pMask = (uint32_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpottedByMask);

            if(g_SpottedRestoreCount < kMaxSpottedRestoreEntries) {
                g_SpottedRestoreEntries[g_SpottedRestoreCount].pawnEntryIndex = pawnHandle.GetEntryIndex();
                g_SpottedRestoreEntries[g_SpottedRestoreCount].originalSpotted = *pSpotted;
                g_SpottedRestoreEntries[g_SpottedRestoreCount].originalMask[0] = pMask[0];
                g_SpottedRestoreEntries[g_SpottedRestoreCount].originalMask[1] = pMask[1];
                g_SpottedRestoreCount++;
            }

            *pSpotted = 1;
            pMask[0] = 0xFFFFFFFF;
            pMask[1] = 0xFFFFFFFF;
        }
    }
    */
}

void MirvPov_EndFrame() {
    if(!g_FakePovRadarFrameContextState.active) return;
    g_FakePovRadarFrameContextState = FakePovRadarFrameContextState{};
}

void MirvPov_RestoreSpotted() {
    // Temporarily disabled - requires EntitySpottedState_t offsets
    return;
    /*
    if(g_SpottedRestoreCount == 0) return;
    for(int i = 0; i < g_SpottedRestoreCount; ++i) {
        auto & entry = g_SpottedRestoreEntries[i];
        CEntityInstance * pawn = GetEntityFromIndex(entry.pawnEntryIndex);
        if(nullptr == pawn || !pawn->IsPlayerPawn()) continue;
        if(0 == g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState) continue;
        auto spottedState = (unsigned char*)pawn + g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState;
        uint8_t * pSpotted = (uint8_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpotted);
        uint32_t * pMask = (uint32_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpottedByMask);
        *pSpotted = entry.originalSpotted;
        pMask[0] = entry.originalMask[0];
        pMask[1] = entry.originalMask[1];
    }
    g_SpottedRestoreCount = 0;
    */
}

void MirvPov_ReWriteSpotted() {
    // Temporarily disabled - requires EntitySpottedState_t offsets
    return;
    /*
    if(g_SpottedRestoreCount == 0) return;
    for(int i = 0; i < g_SpottedRestoreCount; ++i) {
        auto & entry = g_SpottedRestoreEntries[i];
        CEntityInstance * pawn = GetEntityFromIndex(entry.pawnEntryIndex);
        if(nullptr == pawn || !pawn->IsPlayerPawn()) continue;
        if(0 == g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState) continue;
        auto spottedState = (unsigned char*)pawn + g_clientDllOffsets.C_CSPlayerPawnBase.m_entitySpottedState;
        uint8_t * pSpotted = (uint8_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpotted);
        uint32_t * pMask = (uint32_t*)(spottedState + g_clientDllOffsets.EntitySpottedState_t.m_bSpottedByMask);
        *pSpotted = 1;
        pMask[0] = 0xFFFFFFFF;
        pMask[1] = 0xFFFFFFFF;
    }
    */
}

void MirvPov_RepairTeamSpotted() {
}

void MirvPov_SyncObserverPawnPosition() {
    if(!MirvPov_IsEnabled()) return;

    CEntityInstance * fakeController = GetFakePovRadarController();
    if(nullptr == fakeController) return;

    CEntityInstance * realController = GetRealSplitScreenPlayer(0);
    if(nullptr == realController || realController == fakeController) return;

    auto fakePawnHandle = fakeController->GetPlayerPawnHandle();
    if(!fakePawnHandle.IsValid()) return;
    CEntityInstance * fakePawn = GetEntityFromIndex(fakePawnHandle.GetEntryIndex());
    if(nullptr == fakePawn) return;

    if(0 == g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode || 0 == g_clientDllOffsets.CGameSceneNode.m_vecAbsOrigin) return;

    auto fakeSceneNode = *(u_char**)((u_char*)fakePawn + g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode);
    if(nullptr == fakeSceneNode) return;
    float * fakePos = (float*)(fakeSceneNode + g_clientDllOffsets.CGameSceneNode.m_vecAbsOrigin);

    auto realControllerSceneNode = *(u_char**)((u_char*)realController + g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode);
    if(nullptr != realControllerSceneNode) {
        float * realControllerPos = (float*)(realControllerSceneNode + g_clientDllOffsets.CGameSceneNode.m_vecAbsOrigin);
        realControllerPos[0] = fakePos[0];
        realControllerPos[1] = fakePos[1];
        realControllerPos[2] = fakePos[2];
    }

    auto realPawnHandle = realController->GetPlayerPawnHandle();
    if(realPawnHandle.IsValid()) {
        CEntityInstance * realPawn = GetEntityFromIndex(realPawnHandle.GetEntryIndex());
        if(nullptr != realPawn) {
            auto realPawnSceneNode = *(u_char**)((u_char*)realPawn + g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode);
            if(nullptr != realPawnSceneNode) {
                float * realPawnPos = (float*)(realPawnSceneNode + g_clientDllOffsets.CGameSceneNode.m_vecAbsOrigin);
                realPawnPos[0] = fakePos[0];
                realPawnPos[1] = fakePos[1];
                realPawnPos[2] = fakePos[2];
            }
        }
    }
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

// ============================================================================
// Approach C: Hook GetLocalPlayerController + byte-patch spectator mode
// ============================================================================

// Hook sub_180AD5580 (GetObserverMode) — returns observer mode for slot 0.
// This function calls sub_1808E0E70(0) directly, bypassing our
// GetLocalPlayerController hook. The radar uses it to decide whether to
// show spectator-mode behavior. By returning 0 (OBS_MODE_NONE) when our
// POV radar is active, the radar treats us as the observed player directly.
typedef int (__fastcall * GetObserverMode_t)();
static GetObserverMode_t g_Org_GetObserverMode = nullptr;
static bool g_bGetObserverModeHooked = false;

static int __fastcall New_GetObserverMode() {
    return g_Org_GetObserverMode();
}

// Hook sub_180AD55C0 (GetObserverTarget) — returns observer target handle for slot 0.
// Like GetObserverMode, this calls sub_1808E0E70(0) directly, bypassing our
// GetLocalPlayerController hook. By returning INVALID_EHANDLE during frame context,
// the radar won't try to use a spectator target position.
typedef unsigned int (__fastcall * GetObserverTarget_fn_t)(void* thisPtr);
static GetObserverTarget_fn_t g_Org_GetObserverTarget_fn = nullptr;
static bool g_bGetObserverTargetHooked = false;

static unsigned int __fastcall New_GetObserverTarget_fn(void* thisPtr) {
    return g_Org_GetObserverTarget_fn(thisPtr);
}

// Hook GameStateAPI::IsLocalPlayerHLTV (sub_180EFF830) — Panorama bridge callback.
// The radar JS calls this to decide spectator vs player color mode.
// Return original behavior on the stable baseline.
typedef bool (__fastcall * IsLocalPlayerHLTV_t)();
static IsLocalPlayerHLTV_t g_Org_IsLocalPlayerHLTV = nullptr;
static bool g_bIsLocalPlayerHLTVHooked = false;

static bool __fastcall New_IsLocalPlayerHLTV() {
    return g_Org_IsLocalPlayerHLTV();
}

// Hook GameStateAPI::IsDemoOrHltv (sub_180EFEEE0) — Panorama bridge callback.
// Stable baseline keeps original demo/HLTV behavior.
typedef bool (__fastcall * IsDemoOrHltv_t)();
static IsDemoOrHltv_t g_Org_IsDemoOrHltv = nullptr;
static bool g_bIsDemoOrHltvHooked = false;

static bool __fastcall New_IsDemoOrHltv() {
    return g_Org_IsDemoOrHltv();
}

// Hook sub_180BD7830 (GetEffectiveLocalPlayer for HUD) — this function is
// used by the HUD to determine spectator state. It calls sub_1808E0E70(0)
// directly, bypassing our GetLocalPlayerController hook.
// Instead of hooking the function (which crashes during demo transitions),
// we patch the HUD's spectator check: cmp byte ptr [rax+3EBh], 1 → 0xFF
static uint8_t * g_pHudSpectatorCheckPatchAddr = nullptr;
static uint8_t g_HudSpectatorCheckOrigByte = 0;
static bool g_bHudSpectatorCheckPatched = false;

// Patch 1: Force [rbx+174F0h] = 0 (not spectating any target)
//   Original: test al, al / jz short +0A  (84 C0 74 0A)
//   Patched:  test al, al / jmp short +0A (84 C0 EB 0A)
static uint8_t * g_pRadarSpectatorTargetPatchAddr = nullptr;
static uint8_t g_RadarSpectatorTargetOrigByte = 0;
static bool g_bRadarSpectatorTargetPatched = false;

// Patch 4: Hide spectator player panel (HudSpecplayerRoot--visible always false)
//   Original: mov sil, 1  (40 B6 01)
//   Patched:  xor sil, sil (40 32 F6)
//   Pattern context: test r14,r14 / jz +9 / test bl,bl / jnz +5 / [PATCH HERE] / jmp +3 / xor sil,sil
static uint8_t * g_pHudSpecPanelPatchAddr = nullptr;
static uint8_t g_HudSpecPanelOrigBytes[3] = {0};
static bool g_bHudSpecPanelPatched = false;

// Patch 2: NOP "or byte ptr [rbx+17760h], 1" (prevent show-all flag)
static uint8_t * g_pRadarShowAllPatchAddr = nullptr;
static uint8_t g_RadarShowAllOriginalBytes[7] = {0};
static bool g_bRadarShowAllPatched = false;

// Patch 8: Hook radar spot visibility check (inspired by MulNX_CS2)
// Pattern: "38 5C 24 ?? 0F 84 ?? ?? ?? ?? 48 8B 0D"
static uint8_t * g_pRadarSpotCheckPatchAddr = nullptr;
static uint8_t g_RadarSpotCheckOrigBytes[16] = {0};
static uint8_t * g_pRadarSpotCheckTrampoline = nullptr;
static size_t g_RadarSpotCheckPatchSize = 0;
static bool g_bRadarSpotCheckPatched = false;

// Patch 10 (enemy-red, ported from MulNX_CS2 Pos_WriteMaybeEnumToChangeRadarPlayerDraw):
// mid-function hook at the instruction that writes the radar blip color enum (in rbx)
// to radar-entry +0x16C. Enum values: 9=draw-as-CT, 13=draw-as-T, 17=draw-as-enemy,
// 21=local. In a demo the observer has no team so opponents render as their own team
// color instead of red. We rewrite the enum to 17 when the blip's team color differs
// from the observed player's team. Does NOT depend on the v7/competitive path.
// Pattern: "48 8B 6C 24 ?? 41 39 9E 6C 01 00 00" at 0x180e087d3.
static uint8_t * g_pRadarColorPatchAddr = nullptr;
static uint8_t g_RadarColorOrigBytes[16] = {0};
static uint8_t * g_pRadarColorTrampoline = nullptr;
static size_t g_RadarColorPatchSize = 0;
static bool g_bRadarColorPatched = false;

// Patch 11 (teammate competitive colors): in sub_180E15350 (the match radar color
// applier) force the spectator/team-counter branch off so the match color path runs.
// The demo competitive-mode predicates in sub_180826B30 and sub_18080E180 are scoped
// to the radar tick so valid m_iCompTeammateColor data can drive teammate colors.
static bool MirvPov_ShouldForceRadarSpot(CEntityInstance * targetPawn) {
    if(!g_MirvPovEnabled) return false;
    if(nullptr == targetPawn) return false;

    __try {
        if(!targetPawn->IsPlayerPawn()) return false;
        CEntityInstance * fakeController = GetFakePovRadarController();
        if(nullptr == fakeController) return false;
        int fakeTeam = fakeController->GetTeam();
        if(fakeTeam != 2 && fakeTeam != 3) return false;
        return targetPawn->GetTeam() == fakeTeam;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void EmitU8(uint8_t * code, size_t & pos, uint8_t value) {
    code[pos++] = value;
}

static void EmitU32(uint8_t * code, size_t & pos, uint32_t value) {
    memcpy(code + pos, &value, sizeof(value));
    pos += sizeof(value);
}

static void EmitU64(uint8_t * code, size_t & pos, uint64_t value) {
    memcpy(code + pos, &value, sizeof(value));
    pos += sizeof(value);
}

static bool EmitRel32Jump(uint8_t * code, size_t & pos, uint8_t * target) {
    uint8_t * next = code + pos + 5;
    intptr_t rel = target - next;
    if(rel < INT32_MIN || rel > INT32_MAX) return false;
    EmitU8(code, pos, 0xE9);
    EmitU32(code, pos, (uint32_t)(int32_t)rel);
    return true;
}

static bool EmitRel32Jcc(uint8_t * code, size_t & pos, uint8_t conditionOpcode, uint8_t * target) {
    uint8_t * next = code + pos + 6;
    intptr_t rel = target - next;
    if(rel < INT32_MIN || rel > INT32_MAX) return false;
    EmitU8(code, pos, 0x0F);
    EmitU8(code, pos, conditionOpcode);
    EmitU32(code, pos, (uint32_t)(int32_t)rel);
    return true;
}

static uint8_t * MirvPov_AllocNear(uint8_t * target, size_t size) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    size_t granularity = systemInfo.dwAllocationGranularity;
    uintptr_t targetAddr = (uintptr_t)target;
    uintptr_t minAddr = targetAddr > 0x7fff0000 ? targetAddr - 0x7fff0000 : 0;
    uintptr_t maxAddr = targetAddr + 0x7fff0000;

    for(uintptr_t offset = 0; offset < 0x7fff0000; offset += granularity) {
        if(targetAddr >= offset + granularity) {
            uintptr_t addr = (targetAddr - offset) & ~(granularity - 1);
            if(addr >= minAddr) {
                if(void * result = VirtualAlloc((void*)addr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) {
                    return (uint8_t*)result;
                }
            }
        }

        uintptr_t addr = (targetAddr + offset + granularity - 1) & ~(granularity - 1);
        if(addr <= maxAddr) {
            if(void * result = VirtualAlloc((void*)addr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) {
                return (uint8_t*)result;
            }
        }
    }

    return nullptr;
}


// MulNX latest color-path patches inside sub_180E15350:
// 1) Pos_CmpToSetColor  @ 0x180e15412: force bl=0 so the match-color branch is taken.
// 2) Pos_CmpToSetCTColor @ 0x180e15540: force eax=3 (CT) in the CT compare branch.
// 3) Pos_CmpToSetTColor  @ 0x180e155d4: force eax=2 (T)  in the T compare branch.
// Implemented as local code patches / trampolines (not direct detours) so we can
// keep control over the exact register state at each mid-function site.
static uint8_t * g_pRadarMulColorPatchAddr = nullptr;   // site 1
static uint8_t g_RadarMulColorOrigBytes[16] = {0};
static uint8_t * g_pRadarMulColorTrampoline = nullptr;
static size_t g_RadarMulColorPatchSize = 0;
static bool g_bRadarMulColorPatched = false;

static uint8_t * g_pRadarMulCTPatchAddr = nullptr;      // site 2
static uint8_t g_RadarMulCTOrigBytes[16] = {0};
static uint8_t * g_pRadarMulCTTrampoline = nullptr;
static size_t g_RadarMulCTPatchSize = 0;
static bool g_bRadarMulCTPatched = false;

static uint8_t * g_pRadarMulTPatchAddr = nullptr;       // site 3
static uint8_t g_RadarMulTOrigBytes[16] = {0};
static uint8_t * g_pRadarMulTTrampoline = nullptr;
static size_t g_RadarMulTPatchSize = 0;
static bool g_bRadarMulTPatched = false;

static bool MirvPov_PatchRadarMulColor(HMODULE clientDll) {
    if(g_bRadarMulColorPatched) return true;

    size_t matchAddr = getAddress(clientDll, "4C 89 6C 24 ?? 84 DB 0F 84");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] MulColor path pattern not found\n");
        return false;
    }

    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 13;
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarMulColorOrigBytes, patchAddr, patchSize);

    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 96);
    if(nullptr == trampoline) {
        advancedfx::Message("[mirv_pov_radar_patch] Failed to allocate MulColor trampoline\n");
        return false;
    }

    int32_t jccRel = *(int32_t *)(patchAddr + 9);
    uint8_t * matchColorAddr = patchAddr + patchSize + jccRel;

    size_t pos = 0;
    EmitU8(trampoline, pos, 0x4C); EmitU8(trampoline, pos, 0x89); EmitU8(trampoline, pos, 0x6C); EmitU8(trampoline, pos, 0x24); EmitU8(trampoline, pos, patchAddr[4]);
    EmitU8(trampoline, pos, 0x30); EmitU8(trampoline, pos, 0xDB);
    if(!EmitRel32Jump(trampoline, pos, matchColorAddr)) { VirtualFree(trampoline, 0, MEM_RELEASE); return false; }

    DWORD oldProtect;
    if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5);
        patchAddr[0] = 0xE9; *(int32_t *)(patchAddr + 1) = (int32_t)rel; memset(patchAddr + 5, 0x90, patchSize - 5);
        FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize);
        DWORD dummy; VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarMulColorPatchAddr = patchAddr; g_pRadarMulColorTrampoline = trampoline; g_RadarMulColorPatchSize = patchSize; g_bRadarMulColorPatched = true;
        return true;
    }

    VirtualFree(trampoline, 0, MEM_RELEASE);
    return false;
}

static bool MirvPov_PatchRadarMulCT(HMODULE clientDll) {
    if(g_bRadarMulCTPatched) return true;
    size_t matchAddr = getAddress(clientDll, "E8 ?? ?? ?? ?? 83 F8 03 75 ?? 8B D3");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] MulCT path pattern not found\n");
        return false;
    }
    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 5;
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarMulCTOrigBytes, patchAddr, patchSize);
    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 64);
    if(nullptr == trampoline) return false;

    size_t pos = 0;
    EmitU8(trampoline, pos, 0xB8); EmitU32(trampoline, pos, 3);
    if(!EmitRel32Jump(trampoline, pos, returnAddr)) { VirtualFree(trampoline,0,MEM_RELEASE); return false; }

    DWORD oldProtect; if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5); patchAddr[0]=0xE9; *(int32_t*)(patchAddr+1)=(int32_t)rel; FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize); DWORD dummy; VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarMulCTPatchAddr = patchAddr; g_pRadarMulCTTrampoline = trampoline; g_RadarMulCTPatchSize = patchSize; g_bRadarMulCTPatched = true; return true;
    }
    VirtualFree(trampoline,0,MEM_RELEASE); return false;
}

static bool MirvPov_PatchRadarMulT(HMODULE clientDll) {
    if(g_bRadarMulTPatched) return true;
    size_t matchAddr = getAddress(clientDll, "E8 ?? ?? ?? ?? 41 3B C5 0F 85 ?? ?? ?? ?? F6 86");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] MulT path pattern not found\n");
        return false;
    }
    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 5;
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarMulTOrigBytes, patchAddr, patchSize);
    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 64);
    if(nullptr == trampoline) return false;

    size_t pos = 0;
    EmitU8(trampoline, pos, 0xB8); EmitU32(trampoline, pos, 2);
    if(!EmitRel32Jump(trampoline, pos, returnAddr)) { VirtualFree(trampoline,0,MEM_RELEASE); return false; }

    DWORD oldProtect; if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5); patchAddr[0]=0xE9; *(int32_t*)(patchAddr+1)=(int32_t)rel; FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize); DWORD dummy; VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarMulTPatchAddr = patchAddr; g_pRadarMulTTrampoline = trampoline; g_RadarMulTPatchSize = patchSize; g_bRadarMulTPatched = true; return true;
    }
    VirtualFree(trampoline,0,MEM_RELEASE); return false;
}

static void MirvPov_RestoreRadarMulColor() {
    if(g_bRadarMulColorPatched && g_pRadarMulColorPatchAddr) {
        DWORD oldProtect; if(VirtualProtect(g_pRadarMulColorPatchAddr, g_RadarMulColorPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pRadarMulColorPatchAddr, g_RadarMulColorOrigBytes, g_RadarMulColorPatchSize); FlushInstructionCache(GetCurrentProcess(), g_pRadarMulColorPatchAddr, g_RadarMulColorPatchSize); DWORD dummy; VirtualProtect(g_pRadarMulColorPatchAddr, g_RadarMulColorPatchSize, oldProtect, &dummy);
        }
        if(g_pRadarMulColorTrampoline) VirtualFree(g_pRadarMulColorTrampoline,0,MEM_RELEASE);
        g_bRadarMulColorPatched=false; g_pRadarMulColorPatchAddr=nullptr; g_pRadarMulColorTrampoline=nullptr; g_RadarMulColorPatchSize=0;
    }
}

static void MirvPov_RestoreRadarMulCT() {
    if(g_bRadarMulCTPatched && g_pRadarMulCTPatchAddr) {
        DWORD oldProtect; if(VirtualProtect(g_pRadarMulCTPatchAddr, g_RadarMulCTPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pRadarMulCTPatchAddr, g_RadarMulCTOrigBytes, g_RadarMulCTPatchSize); FlushInstructionCache(GetCurrentProcess(), g_pRadarMulCTPatchAddr, g_RadarMulCTPatchSize); DWORD dummy; VirtualProtect(g_pRadarMulCTPatchAddr, g_RadarMulCTPatchSize, oldProtect, &dummy);
        }
        if(g_pRadarMulCTTrampoline) VirtualFree(g_pRadarMulCTTrampoline,0,MEM_RELEASE);
        g_bRadarMulCTPatched=false; g_pRadarMulCTPatchAddr=nullptr; g_pRadarMulCTTrampoline=nullptr; g_RadarMulCTPatchSize=0;
    }
}

static void MirvPov_RestoreRadarMulT() {
    if(g_bRadarMulTPatched && g_pRadarMulTPatchAddr) {
        DWORD oldProtect; if(VirtualProtect(g_pRadarMulTPatchAddr, g_RadarMulTPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pRadarMulTPatchAddr, g_RadarMulTOrigBytes, g_RadarMulTPatchSize); FlushInstructionCache(GetCurrentProcess(), g_pRadarMulTPatchAddr, g_RadarMulTPatchSize); DWORD dummy; VirtualProtect(g_pRadarMulTPatchAddr, g_RadarMulTPatchSize, oldProtect, &dummy);
        }
        if(g_pRadarMulTTrampoline) VirtualFree(g_pRadarMulTTrampoline,0,MEM_RELEASE);
        g_bRadarMulTPatched=false; g_pRadarMulTPatchAddr=nullptr; g_pRadarMulTTrampoline=nullptr; g_RadarMulTPatchSize=0;
    }
}

static bool MirvPov_PatchRadarSpotCheck(HMODULE clientDll) {
    if(g_bRadarSpotCheckPatched) return true;

    size_t matchAddr = getAddress(clientDll, "38 5C 24 ?? 0F 84 ?? ?? ?? ?? 48 8B 0D");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] Radar spot check pattern not found\n");
        return false;
    }

    uint8_t * patchAddr = (uint8_t *)matchAddr;
    uint8_t * renderAddr = patchAddr + 10;
    int32_t originalSkipRel = *(int32_t *)(patchAddr + 6);
    uint8_t * skipAddr = patchAddr + 10 + originalSkipRel;
    size_t patchSize = 10;
    memcpy(g_RadarSpotCheckOrigBytes, patchAddr, patchSize);

    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 256);
    if(nullptr == trampoline) {
        advancedfx::Message("[mirv_pov_radar_patch] Failed to allocate nearby radar spot trampoline\n");
        return false;
    }

    size_t pos = 0;
    uint8_t stackOffset = patchAddr[3];

    uint8_t pushRegs[] = {
        0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57,
        0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,
        0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57
    };
    memcpy(trampoline + pos, pushRegs, sizeof(pushRegs));
    pos += sizeof(pushRegs);

    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xEC); EmitU8(trampoline, pos, 0x28);
    EmitU8(trampoline, pos, 0x4C); EmitU8(trampoline, pos, 0x89); EmitU8(trampoline, pos, 0xF9);
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0xB8); EmitU64(trampoline, pos, (uint64_t)&MirvPov_ShouldForceRadarSpot);
    EmitU8(trampoline, pos, 0xFF); EmitU8(trampoline, pos, 0xD0);
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xC4); EmitU8(trampoline, pos, 0x28);
    EmitU8(trampoline, pos, 0x84); EmitU8(trampoline, pos, 0xC0);

    uint8_t * jnzPos = trampoline + pos;
    pos += 6;

    uint8_t popRegs[] = {
        0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C,
        0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58,
        0x5F, 0x5E, 0x5D, 0x5B, 0x5A, 0x59, 0x58
    };
    memcpy(trampoline + pos, popRegs, sizeof(popRegs));
    pos += sizeof(popRegs);

    EmitU8(trampoline, pos, 0x38); EmitU8(trampoline, pos, 0x5C); EmitU8(trampoline, pos, 0x24); EmitU8(trampoline, pos, stackOffset);
    if(!EmitRel32Jcc(trampoline, pos, 0x84, skipAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }
    if(!EmitRel32Jump(trampoline, pos, renderAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    uint8_t * forceRenderAddr = trampoline + pos;
    memcpy(trampoline + pos, popRegs, sizeof(popRegs));
    pos += sizeof(popRegs);
    if(!EmitRel32Jump(trampoline, pos, renderAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    intptr_t forceRel = forceRenderAddr - (jnzPos + 6);
    if(forceRel < INT32_MIN || forceRel > INT32_MAX) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }
    jnzPos[0] = 0x0F;
    jnzPos[1] = 0x85;
    *(int32_t *)(jnzPos + 2) = (int32_t)forceRel;

    DWORD oldProtect;
    if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5);
        if(rel < INT32_MIN || rel > INT32_MAX) {
            DWORD dummy;
            VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            return false;
        }

        patchAddr[0] = 0xE9;
        *(int32_t *)(patchAddr + 1) = (int32_t)rel;
        memset(patchAddr + 5, 0x90, patchSize - 5);
        FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize);

        DWORD dummy;
        VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarSpotCheckPatchAddr = patchAddr;
        g_pRadarSpotCheckTrampoline = trampoline;
        g_RadarSpotCheckPatchSize = patchSize;
        g_bRadarSpotCheckPatched = true;
        return true;
    }

    VirtualFree(trampoline, 0, MEM_RELEASE);
    advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for radar spot check (error %lu)\n", GetLastError());
    return false;
}

static void MirvPov_RestoreRadarSpotCheck() {
    if(!g_bRadarSpotCheckPatched || !g_pRadarSpotCheckPatchAddr) return;

    DWORD oldProtect;
    if(VirtualProtect(g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckOrigBytes, g_RadarSpotCheckPatchSize);
        FlushInstructionCache(GetCurrentProcess(), g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckPatchSize);
        DWORD dummy;
        VirtualProtect(g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckPatchSize, oldProtect, &dummy);
    }
    if(g_pRadarSpotCheckTrampoline) {
        VirtualFree(g_pRadarSpotCheckTrampoline, 0, MEM_RELEASE);
    }
    g_bRadarSpotCheckPatched = false;
    g_pRadarSpotCheckPatchAddr = nullptr;
    g_pRadarSpotCheckTrampoline = nullptr;
    g_RadarSpotCheckPatchSize = 0;
}

// Helper for Patch 10: given the radar blip color enum (9=CT,13=T,17=enemy,21=local),
// return 17 (enemy red) when the blip's team color is opposite to the observed
// player's team. Mirrors MulNX's enemy-red logic.
static int __fastcall MirvPov_AdjustRadarColor(int enumVal) {
    if(!g_MirvPovEnabled) return enumVal;
    __try {
        CEntityInstance * fakeController = GetFakePovRadarController();
        if(nullptr == fakeController) return enumVal;
        int obsTeam = fakeController->GetTeam(); // 2=T, 3=CT
        if(obsTeam == 3 && enumVal == 13) return 17; // observing CT: a T-colored blip is enemy
        if(obsTeam == 2 && enumVal == 9)  return 17; // observing T:  a CT-colored blip is enemy
        return enumVal;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return enumVal;
    }
}

static bool MirvPov_PatchRadarColor(HMODULE clientDll) {
    if(g_bRadarColorPatched) return true;

    size_t matchAddr = getAddress(clientDll, "48 8B 6C 24 ?? 41 39 9E 6C 01 00 00");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] Radar color enum pattern not found\n");
        return false;
    }

    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 12; // two instrs: mov rbp,[rsp+48] (5) + cmp [r14+16C],ebx (7)
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarColorOrigBytes, patchAddr, patchSize);

    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 256);
    if(nullptr == trampoline) {
        advancedfx::Message("[mirv_pov_radar_patch] Failed to allocate nearby radar color trampoline\n");
        return false;
    }

    size_t pos = 0;
    uint8_t pushRegs[] = {
        0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57,
        0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,
        0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57
    };
    memcpy(trampoline + pos, pushRegs, sizeof(pushRegs));
    pos += sizeof(pushRegs);

    EmitU8(trampoline, pos, 0x8B); EmitU8(trampoline, pos, 0xCB);                       // mov ecx, ebx
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xEC); EmitU8(trampoline, pos, 0x28); // sub rsp,0x28
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0xB8); EmitU64(trampoline, pos, (uint64_t)&MirvPov_AdjustRadarColor); // mov rax,&helper
    EmitU8(trampoline, pos, 0xFF); EmitU8(trampoline, pos, 0xD0);                       // call rax
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xC4); EmitU8(trampoline, pos, 0x28); // add rsp,0x28
    EmitU8(trampoline, pos, 0x89); EmitU8(trampoline, pos, 0x84); EmitU8(trampoline, pos, 0x24); EmitU32(trampoline, pos, 0x58); // mov [rsp+0x58],eax (rbx slot)

    uint8_t popRegs[] = {
        0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C,
        0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58,
        0x5F, 0x5E, 0x5D, 0x5B, 0x5A, 0x59, 0x58
    };
    memcpy(trampoline + pos, popRegs, sizeof(popRegs));
    pos += sizeof(popRegs);

    // relocated original instructions
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x8B); EmitU8(trampoline, pos, 0x6C); EmitU8(trampoline, pos, 0x24); EmitU8(trampoline, pos, 0x48); // mov rbp,[rsp+48]
    EmitU8(trampoline, pos, 0x41); EmitU8(trampoline, pos, 0x39); EmitU8(trampoline, pos, 0x9E); EmitU32(trampoline, pos, 0x16C); // cmp [r14+16C],ebx
    if(!EmitRel32Jump(trampoline, pos, returnAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    DWORD oldProtect;
    if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5);
        if(rel < INT32_MIN || rel > INT32_MAX) {
            DWORD dummy;
            VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            return false;
        }
        patchAddr[0] = 0xE9;
        *(int32_t *)(patchAddr + 1) = (int32_t)rel;
        memset(patchAddr + 5, 0x90, patchSize - 5);
        FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize);
        DWORD dummy;
        VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarColorPatchAddr = patchAddr;
        g_pRadarColorTrampoline = trampoline;
        g_RadarColorPatchSize = patchSize;
        g_bRadarColorPatched = true;
        return true;
    }

    VirtualFree(trampoline, 0, MEM_RELEASE);
    advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for radar color enum (error %lu)\n", GetLastError());
    return false;
}

static void MirvPov_RestoreRadarColor() {
    if(!g_bRadarColorPatched || !g_pRadarColorPatchAddr) return;
    DWORD oldProtect;
    if(VirtualProtect(g_pRadarColorPatchAddr, g_RadarColorPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_pRadarColorPatchAddr, g_RadarColorOrigBytes, g_RadarColorPatchSize);
        FlushInstructionCache(GetCurrentProcess(), g_pRadarColorPatchAddr, g_RadarColorPatchSize);
        DWORD dummy;
        VirtualProtect(g_pRadarColorPatchAddr, g_RadarColorPatchSize, oldProtect, &dummy);
    }
    if(g_pRadarColorTrampoline) {
        VirtualFree(g_pRadarColorTrampoline, 0, MEM_RELEASE);
    }
    g_bRadarColorPatched = false;
    g_pRadarColorPatchAddr = nullptr;
    g_pRadarColorTrampoline = nullptr;
    g_RadarColorPatchSize = 0;
}

static bool MirvPov_ApplyPatches(HMODULE clientDll) {
    if(g_bRadarShowAllPatched && g_bHudSpectatorCheckPatched && g_bHudSpecPanelPatched && g_bGetObserverModeHooked && g_bGetObserverTargetHooked && g_bIsLocalPlayerHLTVHooked && g_bIsDemoOrHltvHooked) return true;
    if(nullptr == clientDll) {
        advancedfx::Message("[mirv_pov_radar_patch] No client.dll handle\n");
        return false;
    }

    // --- Hook GetObserverMode (sub_180AD5580) — return OBS_MODE_NONE during frame context ---
    if(!g_bGetObserverModeHooked) {
        size_t funcAddr = getAddress(clientDll, "48 83 EC 28 33 C9 E8 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 88 F8 11 00 00");
        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] GetObserverMode pattern not found\n");
        } else {
            g_Org_GetObserverMode = (GetObserverMode_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_GetObserverMode, New_GetObserverMode);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bGetObserverModeHooked = true;
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] GetObserverMode detour failed\n");
                g_Org_GetObserverMode = nullptr;
            }
        }
    }

    // --- Hook GetObserverTarget (sub_180AD55C0) — return INVALID_EHANDLE during frame context ---
    if(!g_bGetObserverTargetHooked) {
        size_t funcAddr = getAddress(clientDll, "40 53 48 83 EC 20 48 8B D9 33 C9 E8 ?? ?? ?? ?? 48 85 C0 74");
        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] GetObserverTarget pattern not found\n");
        } else {
            g_Org_GetObserverTarget_fn = (GetObserverTarget_fn_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_GetObserverTarget_fn, New_GetObserverTarget_fn);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bGetObserverTargetHooked = true;
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] GetObserverTarget detour failed\n");
                g_Org_GetObserverTarget_fn = nullptr;
            }
        }
    }

    // --- Hook IsLocalPlayerHLTV (Panorama GameStateAPI callback) ---
    // DISABLED: interferes with xray / head markers in demo POV. Kept code for reference.
    if(false) {
        size_t funcAddr = getAddress(clientDll, "48 83 EC ?? 33 C9 E8 ?? ?? ?? ?? 48 85 C0 74 ?? 80 B8");
        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] IsLocalPlayerHLTV pattern not found\n");
        } else {
            g_Org_IsLocalPlayerHLTV = (IsLocalPlayerHLTV_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_IsLocalPlayerHLTV, New_IsLocalPlayerHLTV);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bIsLocalPlayerHLTVHooked = true;
                advancedfx::Message("[mirv_pov_radar_patch] Hooked IsLocalPlayerHLTV at %p\n", (void*)funcAddr);
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] IsLocalPlayerHLTV detour failed\n");
                g_Org_IsLocalPlayerHLTV = nullptr;
            }
        }
    }

    // --- IsDemoOrHltv hook: DISABLED (interferes with xray / head markers in demo POV) ---
    if(false) {
        size_t funcAddr = 0;
        unsigned char * base = (unsigned char *)clientDll;
        IMAGE_DOS_HEADER * dosHeader = (IMAGE_DOS_HEADER *)base;
        IMAGE_NT_HEADERS * ntHeaders = (IMAGE_NT_HEADERS *)(base + dosHeader->e_lfanew);
        size_t size = ntHeaders->OptionalHeader.SizeOfImage;

        const char * searchStr = "IsDemoOrHltv";
        size_t searchLen = strlen(searchStr);
        size_t strAddr = 0;

        for(size_t i = 0; i + searchLen < size; i++) {
            if(0 == memcmp(base + i, searchStr, searchLen + 1)) {
                strAddr = (size_t)(base + i);
                break;
            }
        }

        if(strAddr) {
            // Find LEA instruction referencing this string (RIP-relative: REX.W 8D ModRM[rm=5] disp32)
            for(size_t i = 0; i + 7 < size; i++) {
                unsigned char * p = base + i;
                if((p[0] == 0x48 || p[0] == 0x4C) && p[1] == 0x8D && (p[2] & 0x07) == 0x05) {
                    int32_t disp = *(int32_t *)(p + 3);
                    size_t target = (size_t)(p + 7) + disp;
                    if(target == strAddr) {
                        // Found LEA loading "IsDemoOrHltv". Scan nearby for another LEA (function ptr).
                        for(int delta = -64; delta <= 64; delta++) {
                            if(delta >= -3 && delta <= 6) continue;
                            unsigned char * q = p + delta;
                            if(q < base || q + 7 >= base + size) continue;
                            if((q[0] == 0x48 || q[0] == 0x4C) && q[1] == 0x8D && (q[2] & 0x07) == 0x05) {
                                int32_t disp2 = *(int32_t *)(q + 3);
                                size_t candidate = (size_t)(q + 7) + disp2;
                                if(candidate >= (size_t)base && candidate < (size_t)base + size) {
                                    unsigned char * cand = (unsigned char *)candidate;
                                    // Heuristic: looks like function prologue
                                    if(cand[0] == 0x48 || cand[0] == 0x40 || cand[0] == 0x55 ||
                                       cand[0] == 0x53 || cand[0] == 0x56 || cand[0] == 0x41 ||
                                       cand[0] == 0xB0 || (cand[0] == 0x33 && cand[1] == 0xC0) ||
                                       cand[0] == 0x8B) {
                                        funcAddr = candidate;
                                        advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv: string at %p, LEA at %p, func at %p (bytes: %02X %02X %02X %02X)\n",
                                            (void*)strAddr, (void*)(size_t)p, (void*)funcAddr,
                                            cand[0], cand[1], cand[2], cand[3]);
                                        break;
                                    }
                                }
                            }
                        }
                        if(funcAddr) break;
                    }
                }
            }
        } else {
            advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv: string not found in client.dll\n");
        }

        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv: function not found\n");
            g_bIsDemoOrHltvHooked = true;
        } else {
            g_Org_IsDemoOrHltv = (IsDemoOrHltv_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_IsDemoOrHltv, New_IsDemoOrHltv);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bIsDemoOrHltvHooked = true;
                advancedfx::Message("[mirv_pov_radar_patch] Hooked IsDemoOrHltv at %p\n", (void*)funcAddr);
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv detour failed\n");
                g_Org_IsDemoOrHltv = nullptr;
                g_bIsDemoOrHltvHooked = true;
            }
        }
    }

    // --- Patch 3: HUD spectator check (cmp byte ptr [rax+3EBh], 1 → 0xFF) ---
    // DISABLED: this toggles the Panorama "HUD--localplayer--spectator" CSS class,
    // which also drives spectator head markers / xray overlay. Forcing it off removed
    // those. Bottom spectator bar is still hidden separately by Patch 4.
    if(false) {
        size_t match3 = getAddress(clientDll, "80 B8 EB 03 00 00 01 48 8B 11 41 0F 94 C0");
        if(0 == match3) {
            advancedfx::Message("[mirv_pov_radar_patch] HUD spectator check pattern not found\n");
        } else {
            uint8_t * patchAddr = (uint8_t *)(match3 + 6);
            g_HudSpectatorCheckOrigByte = *patchAddr;

            DWORD oldProtect;
            if(VirtualProtect(patchAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *patchAddr = 0xFF;
                DWORD dummy;
                VirtualProtect(patchAddr, 1, oldProtect, &dummy);
                g_pHudSpectatorCheckPatchAddr = patchAddr;
                g_bHudSpectatorCheckPatched = true;
                advancedfx::Message("[mirv_pov_radar_patch] Patched HUD spectator check at %p (0x01->0xFF)\n", (void*)patchAddr);
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for HUD spectator patch (error %lu)\n", GetLastError());
            }
        }
    }

    // --- Patch 4: Hide spectator player panel (mov sil,1 → xor sil,sil) ---
    if(!g_bHudSpecPanelPatched) {
        size_t match4 = getAddress(clientDll, "4D 85 F6 74 09 84 DB 75 05 40 B6 01 EB 03 40 32 F6");
        if(0 == match4) {
            advancedfx::Message("[mirv_pov_radar_patch] HUD spec panel pattern not found\n");
        } else {
            uint8_t * patchAddr = (uint8_t *)(match4 + 9);
            memcpy(g_HudSpecPanelOrigBytes, patchAddr, 3);

            DWORD oldProtect;
            if(VirtualProtect(patchAddr, 3, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                patchAddr[0] = 0x40;
                patchAddr[1] = 0x32;
                patchAddr[2] = 0xF6;
                DWORD dummy;
                VirtualProtect(patchAddr, 3, oldProtect, &dummy);
                g_pHudSpecPanelPatchAddr = patchAddr;
                g_bHudSpecPanelPatched = true;
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for HUD spec panel patch (error %lu)\n", GetLastError());
            }
        }
    }

    // --- Patch 1: DISABLED — letting radar use spectator target position for correct centering ---
    if(!g_bRadarSpectatorTargetPatched) {
        g_bRadarSpectatorTargetPatched = true;
    }

    // Patch 2 (show-all NOP) is applied separately via MirvPov_ApplyShowAllNOP()
    // when experiment 'b' (ForceSpotted) is active. This allows 'de' to keep all players visible.

    // --- MulNX latest color-path patches (smaller replacement for patchV7 + compColorIndex) ---
    if(!g_bRadarMulColorPatched) MirvPov_PatchRadarMulColor(clientDll);
    if(!g_bRadarMulCTPatched)    MirvPov_PatchRadarMulCT(clientDll);
    if(!g_bRadarMulTPatched)     MirvPov_PatchRadarMulT(clientDll);

    // --- Patch 8: Selectively force smoke-hidden teammate radar spots only. ---
    if(!g_bRadarSpotCheckPatched) {
        MirvPov_PatchRadarSpotCheck(clientDll);
    }

    // --- Patch 10: Enemy-red radar blip color (MulNX-style enum rewrite) ---
    if(!g_bRadarColorPatched) {
        MirvPov_PatchRadarColor(clientDll);
    }

    return g_bRadarSpectatorTargetPatched || g_bRadarShowAllPatched || g_bRadarSpotCheckPatched;
}

static void MirvPov_RemoveShowAllNOP(); // forward declaration

static void MirvPov_RemovePatches() {
    if(g_bGetObserverModeHooked && g_Org_GetObserverMode) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_GetObserverMode, New_GetObserverMode);
        DetourTransactionCommit();
        g_bGetObserverModeHooked = false;
    }

    if(g_bGetObserverTargetHooked && g_Org_GetObserverTarget_fn) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_GetObserverTarget_fn, New_GetObserverTarget_fn);
        DetourTransactionCommit();
        g_bGetObserverTargetHooked = false;
    }

    if(g_bIsLocalPlayerHLTVHooked && g_Org_IsLocalPlayerHLTV) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_IsLocalPlayerHLTV, New_IsLocalPlayerHLTV);
        DetourTransactionCommit();
        g_bIsLocalPlayerHLTVHooked = false;
        g_IsLocalPlayerHLTV_SuppressFrames = 0;
        g_IsLocalPlayerHLTV_LastDemoTick = -1;
        advancedfx::Message("[mirv_pov_radar_patch] Unhooked IsLocalPlayerHLTV\n");
    }

    if(g_bIsDemoOrHltvHooked && g_Org_IsDemoOrHltv) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_IsDemoOrHltv, New_IsDemoOrHltv);
        DetourTransactionCommit();
        g_bIsDemoOrHltvHooked = false;
        advancedfx::Message("[mirv_pov_radar_patch] Unhooked IsDemoOrHltv\n");
    }

    if(g_bHudSpectatorCheckPatched && g_pHudSpectatorCheckPatchAddr) {
        DWORD oldProtect;
        if(VirtualProtect(g_pHudSpectatorCheckPatchAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *g_pHudSpectatorCheckPatchAddr = g_HudSpectatorCheckOrigByte;
            DWORD dummy;
            VirtualProtect(g_pHudSpectatorCheckPatchAddr, 1, oldProtect, &dummy);
        }
        g_bHudSpectatorCheckPatched = false;
        g_pHudSpectatorCheckPatchAddr = nullptr;
        advancedfx::Message("[mirv_pov_radar_patch] Restored HUD spectator check\n");
    }

    if(g_bHudSpecPanelPatched && g_pHudSpecPanelPatchAddr) {
        DWORD oldProtect;
        if(VirtualProtect(g_pHudSpecPanelPatchAddr, 3, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pHudSpecPanelPatchAddr, g_HudSpecPanelOrigBytes, 3);
            DWORD dummy;
            VirtualProtect(g_pHudSpecPanelPatchAddr, 3, oldProtect, &dummy);
        }
        g_bHudSpecPanelPatched = false;
        g_pHudSpecPanelPatchAddr = nullptr;
    }

    if(g_bRadarSpectatorTargetPatched && g_pRadarSpectatorTargetPatchAddr) {
        DWORD oldProtect;
        if(VirtualProtect(g_pRadarSpectatorTargetPatchAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *g_pRadarSpectatorTargetPatchAddr = g_RadarSpectatorTargetOrigByte;
            DWORD dummy;
            VirtualProtect(g_pRadarSpectatorTargetPatchAddr, 1, oldProtect, &dummy);
        }
        g_bRadarSpectatorTargetPatched = false;
        g_pRadarSpectatorTargetPatchAddr = nullptr;
        advancedfx::Message("[mirv_pov_radar_patch] Restored spectator target jz\n");
    }

    if(g_bRadarShowAllPatched) {
        MirvPov_RemoveShowAllNOP();
    }

    if(g_bRadarSpotCheckPatched) {
        MirvPov_RestoreRadarSpotCheck();
    }

    if(g_bRadarColorPatched) {
        MirvPov_RestoreRadarColor();
    }

    if(g_bRadarMulColorPatched) MirvPov_RestoreRadarMulColor();
    if(g_bRadarMulCTPatched)    MirvPov_RestoreRadarMulCT();
    if(g_bRadarMulTPatched)     MirvPov_RestoreRadarMulT();
}

static bool MirvPov_ApplyShowAllNOP(HMODULE clientDll) {
    if(g_bRadarShowAllPatched) return true;
    size_t match2 = getAddress(clientDll, "74 ?? 80 8B 60 77 01 00 01 EB");
    if(0 == match2) {
        advancedfx::Message("[mirv_pov_radar_patch] Spectator show-all instruction not found\n");
        return false;
    }
    uint8_t * patchAddr = (uint8_t *)(match2 + 2);
    memcpy(g_RadarShowAllOriginalBytes, patchAddr, 7);
    DWORD oldProtect;
    if(VirtualProtect(patchAddr, 7, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memset(patchAddr, 0x90, 7);
        DWORD dummy;
        VirtualProtect(patchAddr, 7, oldProtect, &dummy);
        g_pRadarShowAllPatchAddr = patchAddr;
        g_bRadarShowAllPatched = true;
        return true;
    }
    advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for show-all NOP (error %lu)\n", GetLastError());
    return false;
}

static void MirvPov_RemoveShowAllNOP() {
    if(!g_bRadarShowAllPatched || !g_pRadarShowAllPatchAddr) return;
    DWORD oldProtect;
    if(VirtualProtect(g_pRadarShowAllPatchAddr, 7, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_pRadarShowAllPatchAddr, g_RadarShowAllOriginalBytes, 7);
        DWORD dummy;
        VirtualProtect(g_pRadarShowAllPatchAddr, 7, oldProtect, &dummy);
    }
    g_bRadarShowAllPatched = false;
    g_pRadarShowAllPatchAddr = nullptr;
}

void MirvPov_Enable(HMODULE clientDll) {
    if(g_MirvPovEnabled) return;
    g_MirvPovAutoSync = true;
    MirvPov_ApplyPatches(clientDll);
    MirvPov_HookVoiceHud(clientDll);
    g_MirvPovVoiceLastDemoTick = INT_MIN;
    g_MirvPovVoiceLastControllerIndex = -1;
    g_MirvPovVoiceLastTeam = -1;
    g_MirvPovEnabled = true;
    MirvPov_UpdateVoiceTeam();
}

void MirvPov_Disable() {
    if(!g_MirvPovEnabled) return;
    g_MirvPovAutoSync = false;
    MirvPov_RemovePatches();
    MirvPov_RemoveShowAllNOP();
    MirvPov_ClearSyntheticSpeaking();
    g_MirvPovVoiceLastDemoTick = INT_MIN;
    g_MirvPovVoiceLastControllerIndex = -1;
    g_MirvPovVoiceLastTeam = -1;
    g_MirvPovEnabled = false;
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

ClientDll_GetSplitScreenPlayer_t g_Org_ClientDll_GetSplitScreenPlayer = nullptr;

static CEntityInstance * __fastcall New_ClientDll_GetSplitScreenPlayer(int slot) {
    if(nullptr == g_Org_ClientDll_GetSplitScreenPlayer) return nullptr;
    if(0 == slot) {
        if(MirvPov_IsEnabled() && g_FakePovRadarFrameContextState.active) {
            if(CEntityInstance * fakeController = GetFakePovRadarController()) {
                return fakeController;
            }
        }
    }
    return g_Org_ClientDll_GetSplitScreenPlayer(slot);
}

bool Hook_GetSplitScreenPlayer( void* pAddr) {
    g_Org_ClientDll_GetSplitScreenPlayer = (ClientDll_GetSplitScreenPlayer_t)pAddr;
    g_ClientDll_GetSplitScreenPlayer = g_Org_ClientDll_GetSplitScreenPlayer;

    static bool s_Detoured = false;
    if(s_Detoured) return true;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)g_Org_ClientDll_GetSplitScreenPlayer, New_ClientDll_GetSplitScreenPlayer);

    if(NO_ERROR != DetourTransactionCommit()) {
        advancedfx::Message("[mirv_pov_radar_hook] GetSplitScreenPlayer detour failed\n");
        g_ClientDll_GetSplitScreenPlayer = (ClientDll_GetSplitScreenPlayer_t)pAddr;
        g_Org_ClientDll_GetSplitScreenPlayer = (ClientDll_GetSplitScreenPlayer_t)pAddr;
        return false;
    }

    s_Detoured = true;
    return true;
}

extern "C" void * afx_hook_source2_get_entity_ref_from_split_screen_player(int index) {
    if(0 == index) {
        if(CEntityInstance * result = GetRealSplitScreenPlayer(index)) {
            return CAfxEntityInstanceRef::Aquire(result);
        }
    }
    return nullptr;
}

extern "C" void * afx_hook_source2_get_entity_ref_from_effective_split_screen_player(int index) {
    if(0 == index) {
        if(CEntityInstance * result = GetEffectiveSplitScreenPlayer(index)) {
            return CAfxEntityInstanceRef::Aquire(result);
        }
    }
    return nullptr;
}

extern "C" FFIBool afx_hook_source2_is_fake_pov_radar_frame_context_active() {
    return BOOL_TO_FFIBOOL(IsFakePovRadarFrameContextActive());
}

extern "C" FFIBool afx_hook_source2_consume_fake_pov_radar_frame_context_was_active() {
    return BOOL_TO_FFIBOOL(ConsumeFakePovRadarFrameContextWasActive());
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
