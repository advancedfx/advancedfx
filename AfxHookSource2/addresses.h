#pragma once

#include "../shared/vcpp/AfxAddr.h"

// Naming: cs2_<dll>_<Class>_<Member>[_vtable_idx|_offset]
//
// Function and global addresses are 0 when not set.
// Offsets (_offset) and vtable indices (_vtable_idx) are AFXADDR_INVALID when not set.
//
// Addresses that can not be found are reported by the Addresses_Init*Dll functions,
// code using them only needs to check if they are set.

AFXADDR_DECL(cs2_engine_HostStateRequest_Start)
AFXADDR_DECL(cs2_engine_CRenderService_OnClientOutput)

AFXADDR_DECL(cs2_scenesystem_CSceneSystem_WaitForRenderingToComplete_vtable_idx)
AFXADDR_DECL(cs2_scenesystem_CSceneSystem_FrameUpdate_vtable_idx)

AFXADDR_DECL(cs2_client_CCSGO_HudDeathNotice_lifetime_offset)
AFXADDR_DECL(cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset)

AFXADDR_DECL(cs2_client_RenderInitScopeOverlay)

void Addresses_InitEngine2Dll(AfxAddr engine2Dll);

void Addresses_InitSceneSystemDll(AfxAddr sceneSystemDll);

void Addresses_InitClientDll(AfxAddr clientDll);
