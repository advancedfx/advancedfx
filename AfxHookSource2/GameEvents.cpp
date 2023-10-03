#include "stdafx.h"

#include "GameEvents.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameevents.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/KeyValues.h"

#include "../shared/AfxConsole.h"
#include "../shared/binutils.h"

#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"

enum CS2GameEventKeyType
{
	CS2GameEventKeyType_Local = 0,
	CS2GameEventKeyType_CString = 1,
	CS2GameEventKeyType_Float = 2,
	CS2GameEventKeyType_Long = 3,
	CS2GameEventKeyType_Short = 4,
	CS2GameEventKeyType_Byte = 5,
	CS2GameEventKeyType_Bool = 6,
	CS2GameEventKeyType_Uint64 = 7
};


typedef SOURCESDK::CS2::CGameEvent * (*CGameEventManager_CreateEvent_t)( void * This, const char *name, bool bForce /*= false*/, int *pCookie /*= NULL*/ ); //:006
typedef bool (*CGameEventManager_FireEvent_t)( void * This, SOURCESDK::CS2::CGameEvent *event, bool bDontBroadcast /*= false*/ ); //:007
typedef bool (*CGameEventManager_FireEventClientSide_t)( void * This, SOURCESDK::CS2::CGameEvent *event ); //:008
typedef void (*CGameEventManager_FreeEvent_t)( void * This, SOURCESDK::CS2::CGameEvent *event ); //:010

void * g_pGameEventManager = nullptr;

CGameEventManager_CreateEvent_t g_CGameEventManager_CreateEvent = nullptr;
CGameEventManager_FreeEvent_t g_CGameEventManager_FreeEvent = nullptr;

CGameEventManager_FireEvent_t g_Old_CGameEventManager_FireEvent = nullptr;
CGameEventManager_FireEventClientSide_t g_Old_CGameEventManager_FireEventClientSide = nullptr;

void DumpGameEvent(SOURCESDK::CS2::CGameEvent *event) {
    advancedfx::Message("Event: \"%s\" (%i)", event->GetName(), event->GetID());

    if (SOURCESDK::CS2::CGameEventDescriptor * descriptor = event->m_pDescriptor)
	{
		int eventId = descriptor->eventid;

        advancedfx::Message(" (%i) {\n",eventId);

        if (descriptor->keys)
        {
            if (SOURCESDK::CS2::KeyValues *key = descriptor->keys)
            {
                while (key)
                {
                    int type = key->GetInt();

                    advancedfx::Message("\t(%i) %s;\n",key->GetInt(),key->GetName());

                    key = key->GetNextKey();
                }
            }
        }

        advancedfx::Message("}"); 
    }

    advancedfx::Message("\n"); 
}

bool New_CGameEventManager_FireEvent( void * This, SOURCESDK::CS2::CGameEvent *event, bool bDontBroadcast /*= false*/ ) {
    g_pGameEventManager = This;

    //advancedfx::Message("Server Event: %s\n", event->GetName());

    return g_Old_CGameEventManager_FireEvent(This, event, bDontBroadcast);
}

bool g_bDebugGameClientEvents = false;

bool New_CGameEventManager_FireEventClientSide( void * This, SOURCESDK::CS2::CGameEvent *event ) {
    g_pGameEventManager = This;

    if(g_bDebugGameClientEvents) DumpGameEvent(event);

    return g_Old_CGameEventManager_FireEventClientSide(This, event);
}

bool Hook_CGameEventManager(void* addrClientDll) {
    static bool firstResult = false;
    static bool firstRun = true;

    if(firstRun) {
        firstRun = false;

        if(size_t arddrVtable = Afx::BinUtils::FindClassVtable((HMODULE)addrClientDll,".?AVCGameEventManager@@",0,0)) {
            void ** vtable = (void**)arddrVtable;
            g_CGameEventManager_CreateEvent = (CGameEventManager_CreateEvent_t)vtable[6];
            g_Old_CGameEventManager_FireEvent = (CGameEventManager_FireEvent_t)vtable[7];
            g_Old_CGameEventManager_FireEventClientSide = (CGameEventManager_FireEventClientSide_t)vtable[8];
            g_CGameEventManager_FreeEvent = (CGameEventManager_FreeEvent_t)vtable[10];
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Old_CGameEventManager_FireEvent, New_CGameEventManager_FireEvent);
            DetourAttach(&(PVOID&)g_Old_CGameEventManager_FireEventClientSide, New_CGameEventManager_FireEventClientSide);
            firstResult = NO_ERROR == DetourTransactionCommit();
        }
    }

    return firstResult;
}