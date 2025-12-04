#include "stdafx.h"

#include "RenderServiceHooks.h"

#include "RenderSystemDX11Hooks.h"

#include "../shared/AfxDetours.h"

#include "addresses.h"

#include <stdint.h>

extern void * g_pSceneSystem;
extern void * g_pRenderDevice;

extern void ErrorBox(char const * messageText);
extern void ErrorBox();

typedef void (__fastcall * Tier0_EventListener_t)(void * pUnk0, void * pUnk1);

Tier0_EventListener_t g_Engine2_RenderService_OnClientOutput = nullptr;

void __fastcall My_Engine2_RenderService_OnClientOutput(void * pUnk0, void * pUnk1) {

    RenderSystemDX11_EngineThread_Prepare();

    while(RenderSystemDX11_EngineThread_HasNextRenderPass()) {
        RenderSystemDX11_EngineThread_BeginNextRenderPass();

        if(g_pSceneSystem) {
            g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);

            void ** vtable = *(void***)g_pSceneSystem;

            void (__fastcall * WaitForRenderingToComplete)(void *) = (void (__fastcall *)(void *))(vtable[26]);
            WaitForRenderingToComplete(g_pSceneSystem);

            void (__fastcall * FrameUpdate)(void *, const char *) = (void (__fastcall *)(void *, const char *))(vtable[73]);
            FrameUpdate(g_pSceneSystem, "advancedfx");
    
/*
            0 != *((unsigned char *)g_pSceneSystem +  0x2a72) // Begin has been called. "Begin has already been called without an intervening render"
*//*
            void (__fastcall * BeginRendering)(void *, void *)  = (void (__fastcall *)(void *, void *))(vtable[20]);
            BeginRendering(g_pSceneSystem,g_pRenderDevice);

            void (__fastcall * FinishRenderingViews)(void *)  = (void (__fastcall *)(void *))(vtable[25]);
            FinishRenderingViews(g_pSceneSystem);
*//*        
            void (__fastcall * OnClientPostOutput)(void *, void *)  = (void (__fastcall *)(void *, void *))AFXADDR_GET(cs2_client_CLoopModeGame_OnClientPostOutput);
            OnClientPostOutput(nullptr,(void*)AFXADDR_GET(cs2_client_CLoopModeGame_OnClientPostOutput_arg1));

            void (__fastcall * OnClientPreOutput)(void *, void *)  = (void (__fastcall *)(void *, void *))AFXADDR_GET(cs2_client_CLoopModeGame_OnClientPreOutput);
            OnClientPreOutput(nullptr,(void*)AFXADDR_GET(cs2_client_CLoopModeGame_OnClientPreOutput_arg1));
*/                
        }

        RenderSystemDX11_EngineThread_EndNextRenderPass();
    }

    RenderSystemDX11_EngineThread_BeginMainRenderPass();

    g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);

    RenderSystemDX11_EngineThread_EndMainRenderPass();
}

bool Hook_Engine_RenderService() {
    static bool bFistRun = true;
    static bool bFirsResult = false;

    if(bFistRun) {
        bFistRun = false;

        if(AFXADDR_GET(cs2_engine_CRenderService_OnClientOutput)) {

            g_Engine2_RenderService_OnClientOutput = (Tier0_EventListener_t)AFXADDR_GET(cs2_engine_CRenderService_OnClientOutput);

    		DetourTransactionBegin();
	    	DetourUpdateThread(GetCurrentThread());
		
		    DetourAttach(&(PVOID&)g_Engine2_RenderService_OnClientOutput, My_Engine2_RenderService_OnClientOutput);

            bFirsResult = NO_ERROR == DetourTransactionCommit();
		
		    if(!bFirsResult) ErrorBox("Hook_Engine_RenderService failed.");            
        }        
    }
    return bFirsResult;
}

