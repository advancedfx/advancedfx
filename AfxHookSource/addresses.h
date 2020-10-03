#pragma once

#include <shared/vcpp/AfxAddr.h>

enum SourceSdkVer
{
	SourceSdkVer_Unknonw,
	SourceSdkVer_CSGO,
	SourceSdkVer_CSSV34,
	SourceSdkVer_TF2,
	SourceSdkVer_CSS,
	SourceSdkVer_SWARM,
	SourceSdkVer_L4D2,
	SourceSdkVer_BM,
	SourceSdkVer_Insurgency2,
	SourceSdkVer_Garrysmod,
	SourceSdkVer_Momentum
};

extern SourceSdkVer g_SourceSdkVer;

//AFXADDR_DECL(csgo_CPredictionCopy_TransferData)
//AFXADDR_DECL(csgo_CPredictionCopy_TransferData_DSZ)
//AFXADDR_DECL(csgo_C_BaseEntity_IClientEntity_vtable)
//AFXADDR_DECL(csgo_C_BaseEntity_ofs_m_nModelIndex)
//AFXADDR_DECL(csgo_C_BaseEntity_ofs_m_iWorldModelIndex)
//AFXADDR_DECL(csgo_C_BaseAnimating_IClientEntity_vtable)
//AFXADDR_DECL(csgo_C_BaseCombatWeapon_IClientEntity_vtable)
//AFXADDR_DECL(csgo_CStaticProp_IClientEntity_vtable)
AFXADDR_DECL(csgo_C_BaseAnimating_vtable)
AFXADDR_DECL(csgo_DT_Animationlayer_m_flCycle_fn)
//AFXADDR_DECL(csgo_DT_Animationlayer_m_flPrevCycle_fn)
//AFXADDR_DECL(csgo_mystique_animation)
AFXADDR_DECL(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel)
AFXADDR_DECL(csgo_C_BaseCombatWeapon_m_iState)
//AFXADDR_DECL(csgo_C_BasePlayer_OFS_m_bDucked)
//AFXADDR_DECL(csgo_C_BasePlayer_OFS_m_bDucking)
//AFXADDR_DECL(csgo_C_BasePlayer_OFS_m_flDuckAmount)
AFXADDR_DECL(csgo_C_BasePlayer_OFS_m_skybox3d_scale)
AFXADDR_DECL(csgo_C_BasePlayer_RecvProxy_ObserverTarget)
AFXADDR_DECL(csgo_CCSViewRender_vtable)
AFXADDR_DECL(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha)
AFXADDR_DECL(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw)
AFXADDR_DECL(csgo_CGlowOverlay_Draw)
AFXADDR_DECL(csgo_CGlowOverlay_Draw_DSZ)
AFXADDR_DECL(csgo_CCSGO_HudDeathNotice_FireGameEvent)
AFXADDR_DECL(csgo_CUnknown_GetPlayerName)
AFXADDR_DECL(csgo_CUnknown_GetPlayerName_DSZ)
AFXADDR_DECL(csgo_CCSGameMovement_vtable)
AFXADDR_DECL(csgo_CSkyboxView_Draw)
AFXADDR_DECL(csgo_CSkyboxView_Draw_DSZ)
AFXADDR_DECL(csgo_CViewRender_RenderView_VGui_DrawHud_In)
AFXADDR_DECL(csgo_CViewRender_RenderView_VGui_DrawHud_Out)
AFXADDR_DECL(csgo_g_AudioDevice2)
AFXADDR_DECL(csgo_MIX_PaintChannels)
AFXADDR_DECL(csgo_MIX_PaintChannels_DSZ)
AFXADDR_DECL(csgo_SplineRope_CShader_vtable)
AFXADDR_DECL(csgo_Spritecard_CShader_vtable)
AFXADDR_DECL(csgo_UnlitGeneric_CShader_vtable)
AFXADDR_DECL(csgo_Unknown_GetTeamsSwappedOnScreen)
AFXADDR_DECL(csgo_VertexLitGeneric_CShader_vtable)
AFXADDR_DECL(csgo_S_StartSound_StringConversion)
AFXADDR_DECL(csgo_gpGlobals_OFS_curtime)
AFXADDR_DECL(csgo_gpGlobals_OFS_interpolation_amount)
AFXADDR_DECL(csgo_gpGlobals_OFS_interval_per_tick)
AFXADDR_DECL(csgo_pLocalPlayer)
AFXADDR_DECL(csgo_snd_mix_timescale_patch)
AFXADDR_DECL(csgo_snd_mix_timescale_patch_DSZ)
AFXADDR_DECL(csgo_view)
AFXADDR_DECL(cstrike_gpGlobals_OFS_absoluteframetime)
AFXADDR_DECL(cstrike_gpGlobals_OFS_curtime)
AFXADDR_DECL(cstrike_gpGlobals_OFS_interpolation_amount)
AFXADDR_DECL(cstrike_gpGlobals_OFS_interval_per_tick)
AFXADDR_DECL(csgo_CClientState_ProcessVoiceData)
AFXADDR_DECL(csgo_CClientState_ProcessVoiceData_DSZ)
AFXADDR_DECL(csgo_CVoiceWriter_AddDecompressedData)
AFXADDR_DECL(csgo_CVoiceWriter_AddDecompressedData_DSZ)
AFXADDR_DECL(csgo_panorama_AVCUIPanel_UnkSetFloatProp)
AFXADDR_DECL(csgo_panorama_CZip_UnkLoadFiles)
AFXADDR_DECL(csgo_C_CSPlayer_IClientNetworkable_entindex)
AFXADDR_DECL(csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall)
AFXADDR_DECL(csgo_CShaderAPIDx8_UnkCreateTexture)
AFXADDR_DECL(csgo_CRendering3dView_DrawTranslucentRenderables)
AFXADDR_DECL(csgo_CGameEventManger_FireEventIntern)
AFXADDR_DECL(csgo_dynamic_cast)
AFXADDR_DECL(csgo_RTTI_CGameEvent)
AFXADDR_DECL(csgo_RTTI_IGameEvent)
//AFXADDR_DECL(csgo_client_dynamic_cast)
//AFXADDR_DECL(csgo_client_RTTI_IClientRenderable)
AFXADDR_DECL(csgo_GlowCurrentPlayer_JMPS)
AFXADDR_DECL(csgo_C_CSPlayer_UpdateClientSideAnimation)
AFXADDR_DECL(csgo_CNetChan_ProcessMessages)
AFXADDR_DECL(csgo_C_CSPlayer_vtable)
AFXADDR_DECL(csgo_C_CSPlayer_ofs_m_angEyeAngles)
AFXADDR_DECL(csgo_crosshair_localplayer_check)
AFXADDR_DECL(csgo_DamageIndicator_MessageFunc)
AFXADDR_DECL(csgo_C_BasePlayer_SetAsLocalPlayer)
AFXADDR_DECL(csgo_C_BasePlayer_GetToolRecordingState)

void Addresses_InitEngineDll(AfxAddr engineDll, SourceSdkVer sourceSdkVer);
void Addresses_InitPanoramaDll(AfxAddr panoramaDll, SourceSdkVer sourceSdkVer);
void Addresses_InitClientDll(AfxAddr clientDll, SourceSdkVer sourceSdkVer);
//void Addresses_InitStdshader_dx9Dll(AfxAddr stdshader_dx9Dll, bool isCsgo);

