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

AFXADDR_DECL(cs2_client_CCSGO_HudDeathNotice_HandlePlayerDeath)
AFXADDR_DECL(cs2_client_HashString)
AFXADDR_DECL(cs2_client_GetLocalSteamId)
AFXADDR_DECL(cs2_client_CPanel2D_GetClientClass_vtable_idx)
AFXADDR_DECL(cs2_client_CUIPanel_GetAttributeString_vtable_idx)
AFXADDR_DECL(cs2_client_CUIPanel_SetAttributeString_vtable_idx)
AFXADDR_DECL(cs2_client_CUIEngine_MakeSymbol_vtable_idx)
AFXADDR_DECL(cs2_client_g_pHudPanel)
AFXADDR_DECL(cs2_client_g_pUIEngine)

AFXADDR_DECL(cs2_client_CGlowProperty_SetGlowColor)
AFXADDR_DECL(cs2_client_ApplySmokeProps)
AFXADDR_DECL(cs2_client_RenderSmokes)
AFXADDR_DECL(cs2_client_GetParticleManager)
AFXADDR_DECL(cs2_client_DrawStuff)
AFXADDR_DECL(cs2_client_CreateParticle)
AFXADDR_DECL(cs2_client_UpdateParticle)

AFXADDR_DECL(cs2_client_OnFlashMaxAlphaChanged)
AFXADDR_DECL(cs2_client_EOM)
AFXADDR_DECL(cs2_client_SetGlowProps)
AFXADDR_DECL(cs2_client_CGlowProperty_ShouldGlow)
AFXADDR_DECL(cs2_client_C_EnvSky_ForceUpdateSkybox)
AFXADDR_DECL(cs2_client_C_EnvSky_UnkPtr_offset)

AFXADDR_DECL(cs2_panorama_CLayoutFile_LoadFromFile)
AFXADDR_DECL(cs2_panorama_CStylePropertyForegroundColor_Parse)
AFXADDR_DECL(cs2_panorama_CStylePropertyBackgroundColor_Parse)
AFXADDR_DECL(cs2_panorama_CStylePropertyBorder_Parse)
AFXADDR_DECL(cs2_panorama_CStylePropertyWashColor_Clone)
AFXADDR_DECL(cs2_panorama_CStylePropertyWashColor_Parse)
AFXADDR_DECL(cs2_panorama_CStylePropertyOpacity_vtable)
AFXADDR_DECL(cs2_panorama_CStyleSymbol_Resolve)
AFXADDR_DECL(cs2_panorama_g_StylePropertySymbols)
AFXADDR_DECL(cs2_panorama_CPanelStyle_SetStyleProperty)

void Addresses_InitEngine2Dll(AfxAddr engine2Dll);

void Addresses_InitSceneSystemDll(AfxAddr sceneSystemDll);

void Addresses_InitClientDll(AfxAddr clientDll);

void Addresses_InitPanoramaDll(AfxAddr panoramaDll);
