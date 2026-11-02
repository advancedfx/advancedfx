#include "stdafx.h"

#include "ClientRenderScopeOverlay.h"

#include "addresses.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../deps/release/Detours/src/detours.h"

#include <atomic>

extern void ErrorBox(char const * messageText);

std::atomic_bool g_bRenderScopeOverlay = true;

void SetRenderScopeOverlay(bool bValue) {
    g_bRenderScopeOverlay = bValue;
}

typedef void (__fastcall * Client_RenderScopeOverlay_t)(void * pLocalPlayerPawn, void * pContextData);
Client_RenderScopeOverlay_t g_Org_Client_RenderScopeOverlay = nullptr;

void __fastcall My_Client_RenderScopeOverlay(void * pLocalPlayerPawn, void * pContextData) {
    if(g_bRenderScopeOverlay) g_Org_Client_RenderScopeOverlay(pLocalPlayerPawn, pContextData);
}

bool Hook_Client_RenderScopeOverlay() {
    static bool bFirstResult = false;
    static bool bFirstRun = true;

    if(bFirstRun) {
        bFirstRun = false;
        if(AFXADDR_GET(cs2_client_RenderInitScopeOverlay)) {
            g_Org_Client_RenderScopeOverlay = (Client_RenderScopeOverlay_t)AFXADDR_GET(cs2_client_RenderInitScopeOverlay);

    		DetourTransactionBegin();
	    	DetourUpdateThread(GetCurrentThread());
		
		    DetourAttach(&(PVOID&)g_Org_Client_RenderScopeOverlay, My_Client_RenderScopeOverlay);

            bFirstResult = NO_ERROR == DetourTransactionCommit();
		
		    if(!bFirstResult) ErrorBox("Hook_Client_RenderScopeOverlay failed.");
        }
    }

    return bFirstResult;
}
