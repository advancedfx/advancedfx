#include "stdafx.h"

#include "RenderServiceHooks.h"

#include "RenderSystemDX11Hooks.h"

#include "../shared/AfxDetours.h"

#include "addresses.h"

#include <stdint.h>

extern void * g_pSceneSystem;

extern void ErrorBox(char const * messageText);
extern void ErrorBox();

typedef void (__fastcall * Tier0_EventListener_t)(void * pUnk0, void * pUnk1);

Tier0_EventListener_t g_Engine2_RenderService_OnClientOutput = nullptr;

bool g_bUpdateFrame = true;
bool g_bHad_ClientOutput = false;
bool g_bLastPassWasExtra = false;

extern void ClearThreadSceneLayerContexts();

typedef void (__fastcall * SceneSystem_WaitForRenderingToComplete_t)(void * pThis);
SceneSystem_WaitForRenderingToComplete_t g_Old_SceneSystem_WaitForRenderingToComplete = nullptr;
void __fastcall My_SceneSystem_WaitForRenderingToComplete(void * pThis) {
    if(g_bHad_ClientOutput) {
        g_bHad_ClientOutput = false;

        g_Old_SceneSystem_WaitForRenderingToComplete(pThis);

        if(g_bLastPassWasExtra) {
            g_bLastPassWasExtra = false;
            RenderSystemDX11_EngineThread_EndNextRenderPass();
        }
        else RenderSystemDX11_EngineThread_EndMainRenderPass();
    } else {
        g_bUpdateFrame = true;
        g_Old_SceneSystem_WaitForRenderingToComplete(pThis);
    }

    ClearThreadSceneLayerContexts();
}

typedef void (__fastcall * FrameUpdate_t)(void *, unsigned char);
FrameUpdate_t g_Old_FrameUpdate = nullptr;
void __fastcall New_FrameUpdate(void *pThisCSceneSystem, unsigned char ucUnk2) {
    if(g_bUpdateFrame) g_Old_FrameUpdate(pThisCSceneSystem, 1);
}

void __fastcall My_Engine2_RenderService_OnClientOutput(void * pUnk0, void * pUnk1) {

    bool bHooksAvailable = g_pSceneSystem && g_Old_FrameUpdate && g_Old_SceneSystem_WaitForRenderingToComplete && AFXADDR_GET(cs2_SceneSystem_FrameUpdate_vtable_idx);
    if(!bHooksAvailable) {
        g_bHad_ClientOutput = false;
        g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);
        return;
    }

    g_bHad_ClientOutput = true;

    RenderSystemDX11_EngineThread_Prepare();

    // We need to do the a normal / main render pass first, since it's used to generate the UI background texture
    // and it won't be re-generated in subsequent passes.

    g_bUpdateFrame = RenderSystemDX11_EngineThread_BeginMainRenderPass();

    RenderSystemDX11_EngineThread_BeforeRender();

    g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);

    bool bFirstExtraPass = true;

    while(RenderSystemDX11_EngineThread_HasNextRenderPass()) {

        g_Old_SceneSystem_WaitForRenderingToComplete(g_pSceneSystem);
        New_FrameUpdate(g_pSceneSystem, 1);

        if(bFirstExtraPass) {
            bFirstExtraPass = false;
            RenderSystemDX11_EngineThread_EndMainRenderPass();
        } else {
            RenderSystemDX11_EngineThread_EndNextRenderPass();
        }

        ClearThreadSceneLayerContexts();

        g_bLastPassWasExtra = true;

        g_bUpdateFrame = RenderSystemDX11_EngineThread_BeginNextRenderPass();

        RenderSystemDX11_EngineThread_BeforeRender();

        g_Engine2_RenderService_OnClientOutput(pUnk0,pUnk1);
    }
}

bool Hook_Engine_RenderService() {
    static bool bFirstRun = true;
    static bool bFirstResult = false;

    if(bFirstRun) {
        bFirstRun = false;

        if(AFXADDR_GET(cs2_engine_CRenderService_OnClientOutput)) {

            g_Engine2_RenderService_OnClientOutput = (Tier0_EventListener_t)AFXADDR_GET(cs2_engine_CRenderService_OnClientOutput);

    		DetourTransactionBegin();
	    	DetourUpdateThread(GetCurrentThread());
		
		    DetourAttach(&(PVOID&)g_Engine2_RenderService_OnClientOutput, My_Engine2_RenderService_OnClientOutput);

            bFirstResult = NO_ERROR == DetourTransactionCommit();
		
		    if(!bFirstResult) ErrorBox("Hook_Engine_RenderService failed.");            
        }        
    }
    return bFirstResult;
}

bool Hook_SceneSystem_WaitForRenderingToComplete(void * g_pSceneSystem) {
    static bool bFirstRun = true;
    static bool bFirstResult = false;

    if(bFirstRun) {
        bFirstRun = false;

        if(g_pSceneSystem && AFXADDR_GET(cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx)) {
            void ** vtable = *(void***)g_pSceneSystem;

            g_Old_FrameUpdate = (FrameUpdate_t)(vtable[AFXADDR_GET(cs2_SceneSystem_FrameUpdate_vtable_idx)]);
            g_Old_SceneSystem_WaitForRenderingToComplete = (SceneSystem_WaitForRenderingToComplete_t)(vtable[AFXADDR_GET(cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx)]);

    		DetourTransactionBegin();
	    	DetourUpdateThread(GetCurrentThread());
		
		    DetourAttach(&(PVOID&)g_Old_FrameUpdate, New_FrameUpdate);
		    DetourAttach(&(PVOID&)g_Old_SceneSystem_WaitForRenderingToComplete, My_SceneSystem_WaitForRenderingToComplete);

            bFirstResult = NO_ERROR == DetourTransactionCommit();
		
		    if(!bFirstResult) ErrorBox("Hook_SceneSystem_WaitForRenderingToComplete failed.");            
        }        
    }
    return bFirstResult;
}
