#pragma once

#include "../shared/vcpp/AfxAddr.h"

AFXADDR_DECL(cs2_engine_HostStateRequest_Start)
AFXADDR_DECL(cs2_engine_CRenderService_OnClientOutput)

AFXADDR_DECL(cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx)
AFXADDR_DECL(cs2_SceneSystem_FrameUpdate_vtable_idx)

AFXADDR_DECL(cs2_deathmsg_lifetime_offset)
AFXADDR_DECL(cs2_deathmsg_lifetimemod_offset)

void Addresses_InitEngine2Dll(AfxAddr engine2Dll);

void Addresses_InitSceneSystemDll(AfxAddr sceneSystemDll);

void Addresses_InitClientDll(AfxAddr clientDll);

