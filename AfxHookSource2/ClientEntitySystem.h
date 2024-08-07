#pragma once

#include <Windows.h>
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

bool Hook_ClientEntitySystem( void* pEntityList, void * pFnGetHighestEntityIterator, void * pFnGetEntityFromIndex );

bool Hook_ClientEntitySystem2();

class CAfxEntityInstanceRef;

struct EntityListIterator{
	int index = -1;

	bool IsValid() const {
		return 0 < index;
	}

    int GetIndex() const {
        return index;
    }
};

class CEntityInstance {
public:
    const char * GetName();

    const char * GetDebugName();

    const char * GetClassName();

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
};

typedef EntityListIterator * (__fastcall * GetHighestEntityIterator_t)(void * entityList, EntityListIterator * it);
typedef void * (__fastcall * GetEntityFromIndex_t)(void * pEntityList, int index);

extern GetHighestEntityIterator_t  g_GetHighestEntityIterator;
extern GetEntityFromIndex_t g_GetEntityFromIndex;

extern void ** g_pEntityList;