#pragma once

#include "../shared/vcpp/AfxAddr.h"

AFXADDR_DECL(cs2_engine_CRenderService_OnClientOutput)

AFXADDR_DECL(cs2_scenesystem_SceneSystem_WaitForRenderingToComplete_vtableofs)
AFXADDR_DECL(cs2_scenesystem_SceneSystem_FrameUpdate_vtableofs)

void Addresses_InitEngine2Dll(AfxAddr engine2Dll);

void Addresses_InitSceneSystemDll(AfxAddr sceneSystemDll);

void Addresses_InitClientDll(AfxAddr clientDll);

