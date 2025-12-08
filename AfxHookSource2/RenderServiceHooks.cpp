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

bool g_bHad_ClientOutput = false;
bool g_bLastPassWasExtra = false;

typedef void (__fastcall * SceneSystem_WaitForRenderingToComplete_t)(void * pThis);
SceneSystem_WaitForRenderingToComplete_t g_Old_SceneSystem_WaitForRenderingToComplete = nullptr;
void __fastcall My_SceneSystem_WaitForRenderingToComplete(void * pThis) {
    g_Old_SceneSystem_WaitForRenderingToComplete(pThis);

    if(!g_bHad_ClientOutput) return;
    g_bHad_ClientOutput = false;

    if(g_bLastPassWasExtra) {
        g_bLastPassWasExtra = false;
        RenderSystemDX11_EngineThread_EndNextRenderPass();
    }
    else RenderSystemDX11_EngineThread_EndMainRenderPass();

    RenderSystemDX11_EngineThread_Finish();
}

void __fastcall My_Engine2_RenderService_OnClientOutput(void * pUnk0, void * pUnk1) {

    g_bHad_ClientOutput = true;

    RenderSystemDX11_EngineThread_Prepare();

    // We need to do the a normal / main render pass first, since it's used to generate the UI background texture
    // and it won't be re-generated in subsequent passes.

    RenderSystemDX11_EngineThread_BeginMainRenderPass();

    g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);

    if(RenderSystemDX11_EngineThread_HasNextRenderPass()) {

        bool bFirstExtraPass = true;
        g_bLastPassWasExtra = true;

        bool bHooksAvailable = g_pSceneSystem && g_Old_SceneSystem_WaitForRenderingToComplete && AFXADDR_GET(cs2_scenesystem_SceneSystem_FrameUpdate_vtableofs);

        while(RenderSystemDX11_EngineThread_HasNextRenderPass()) {

            if(bHooksAvailable) {
                void ** vtable = *(void***)g_pSceneSystem;

                g_Old_SceneSystem_WaitForRenderingToComplete(g_pSceneSystem);

                void (__fastcall * FrameUpdate)(void *, unsigned char) = (void (__fastcall *)(void *, unsigned char))(vtable[AFXADDR_GET(cs2_scenesystem_SceneSystem_FrameUpdate_vtableofs)]);
                FrameUpdate(g_pSceneSystem, 1);

                // Note:
                // We are wasteful here, since we always wait for the render to finish and begin a new render,
                // even if not needed after the last pass.
                // But this way we need less hooks and logic (otherwise we would need to put RenderSystemDX11_EngineThread_EndNextRenderPass elsewhere).
                // This can be optimized in future I guess.
            }

            if(bFirstExtraPass){
                bFirstExtraPass = false;
                RenderSystemDX11_EngineThread_EndMainRenderPass();
            } else RenderSystemDX11_EngineThread_EndNextRenderPass();

            RenderSystemDX11_EngineThread_BeginNextRenderPass();

            if(bHooksAvailable) {
                g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);
            }
        }
    }
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

bool Hook_SceneSystem_WaitForRenderingToComplete(void * g_pSceneSystem) {
    static bool bFistRun = true;
    static bool bFirsResult = false;

    if(bFistRun) {
        bFistRun = false;

        if(g_pSceneSystem && AFXADDR_GET(cs2_scenesystem_SceneSystem_WaitForRenderingToComplete_vtableofs)) {
            void ** vtable = *(void***)g_pSceneSystem;

            g_Old_SceneSystem_WaitForRenderingToComplete = (SceneSystem_WaitForRenderingToComplete_t)(vtable[AFXADDR_GET(cs2_scenesystem_SceneSystem_WaitForRenderingToComplete_vtableofs)]);

    		DetourTransactionBegin();
	    	DetourUpdateThread(GetCurrentThread());
		
		    DetourAttach(&(PVOID&)g_Old_SceneSystem_WaitForRenderingToComplete, My_SceneSystem_WaitForRenderingToComplete);

            bFirsResult = NO_ERROR == DetourTransactionCommit();
		
		    if(!bFirsResult) ErrorBox("Hook_SceneSystem_WaitForRenderingToComplete failed.");            
        }        
    }
    return bFirsResult;
}
