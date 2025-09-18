#pragma once

#include <cstdint>

#include <Windows.h>
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

bool Hook_ClientEntitySystem( void* pEntityList, void * pFnGetHighestEntityIterator, void * pFnGetEntityFromIndex );

bool Hook_ClientEntitySystem2();

bool Hook_GetSplitScreenPlayer( void* pAddr);

class CAfxEntityInstanceRef;

class CEntityInstance {
public:
    const char * GetName();

    const char * GetDebugName();

    const char * GetClassName();

    const char * GetClientClassName();

    bool IsPlayerPawn();

    SOURCESDK::CS2::CBaseHandle GetPlayerPawnHandle();

    bool IsPlayerController();

    SOURCESDK::CS2::CBaseHandle GetPlayerControllerHandle();

    unsigned int GetHealth();

    int GetTeam();
	
    /**
     * @remarks FLOAT_MAX if invalid
     */
    void GetOrigin(float & x, float & y, float & z);

    void GetRenderEyeOrigin(float outOrigin[3]);

    void GetRenderEyeAngles(float outAngles[3]);

    SOURCESDK::CS2::CBaseHandle GetViewEntityHandle();

    SOURCESDK::CS2::CBaseHandle GetActiveWeaponHandle();

    const char * GetPlayerName();

    uint64_t GetSteamId();

    const char * GetSanitizedPlayerName();

    uint8_t GetObserverMode();
    SOURCESDK::CS2::CBaseHandle GetObserverTarget();

    SOURCESDK::CS2::CBaseHandle GetHandle();
};

typedef int (__fastcall * GetHighestEntityIndex_t)(void * pEntityList, bool bUnknown);
typedef void * (__fastcall * GetEntityFromIndex_t)(void * pEntityList, int index);

extern GetHighestEntityIndex_t  g_GetHighestEntityIndex;
extern GetEntityFromIndex_t g_GetEntityFromIndex;

extern void ** g_pEntityList;

int GetHighestEntityIndex();