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
AFXADDR_DECL(csgo_C_BasePlayer_OFS_m_skybox3d_scale)
AFXADDR_DECL(csgo_C_BasePlayer_RecvProxy_ObserverTarget)
AFXADDR_DECL(csgo_CCSViewRender_vtable)
AFXADDR_DECL(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha)
AFXADDR_DECL(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw)
AFXADDR_DECL(csgo_CGlowOverlay_Draw)
AFXADDR_DECL(csgo_CGlowOverlay_Draw_DSZ)
AFXADDR_DECL(csgo_CCSGO_HudDeathNotice_FireGameEvent)
AFXADDR_DECL(csgo_CUnknown_GetPlayerName)
AFXADDR_DECL(csgo_CCSGameMovement_vtable)
AFXADDR_DECL(csgo_CSkyboxView_Draw)
AFXADDR_DECL(csgo_CSkyboxView_Draw_DSZ)
AFXADDR_DECL(csgo_CViewRender_RenderView_VGui_DrawHud_In)
AFXADDR_DECL(csgo_CViewRender_RenderView_VGui_DrawHud_Out)
AFXADDR_DECL(csgo_CViewRender_ShouldForceNoVis_vtable_index)
AFXADDR_DECL(csgo_engine_WaveAppendTmpFile)
AFXADDR_DECL(csgo_engine_S_ExtraUpdate)
AFXADDR_DECL(csgo_engine_S_Update_)
AFXADDR_DECL(csgo_engine_cl_movieinfo_moviename)
AFXADDR_DECL(csgo_engine_CL_StartMovie)
AFXADDR_DECL(csgo_engine_CVideoMode_Common_vtable)
AFXADDR_DECL(csgo_engine_CEngineVGui_vtable)
AFXADDR_DECL(csgo_SplineRope_CShader_vtable)
AFXADDR_DECL(csgo_Spritecard_CShader_vtable)
AFXADDR_DECL(csgo_UnlitGeneric_CShader_vtable)
AFXADDR_DECL(csgo_Unknown_GetTeamsSwappedOnScreen)
AFXADDR_DECL(csgo_VertexLitGeneric_CShader_vtable)
AFXADDR_DECL(csgo_S_StartSound_StringConversion)
AFXADDR_DECL(csgo_gpGlobals_OFS_curtime)
AFXADDR_DECL(csgo_gpGlobals_OFS_interpolation_amount)
AFXADDR_DECL(csgo_gpGlobals_OFS_interval_per_tick)
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
AFXADDR_DECL(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer)
AFXADDR_DECL(csgo_C_BaseViewModel_ofs_m_nAnimationParity)
AFXADDR_DECL(csgo_C_BaseViewModel_ofs_m_nOldAnimationParity)
AFXADDR_DECL(csgo_C_BaseEntity_ShouldInterpolate)
AFXADDR_DECL(csgo_C_CS_PlayerResource_IGameResources_vtable)
AFXADDR_DECL(csgo_C_Team_vtable)
AFXADDR_DECL(csgo_engine_CModelLoader_vtable)
AFXADDR_DECL(csgo_engine_CModelLoader_GetModel_vtable_index)
AFXADDR_DECL(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ)
AFXADDR_DECL(csgo_materialsystem_CMaterialSystem_ForceSingleThreaded)
AFXADDR_DECL(csgo_engine_Do_CCLCMsg_FileCRCCheck)
AFXADDR_DECL(csgo_C_BaseViewModel_FireEvent)
AFXADDR_DECL(csgo_client_AdjustInterpolationAmount)
//AFXADDR_DECL(csgo_C_BaseEntity_ofs_m_bPredictable)
AFXADDR_DECL(csgo_engine_Cmd_ExecuteCommand)
AFXADDR_DECL(csgo_client_C_CSPlayer_EyeAngles_vtable_index)
AFXADDR_DECL(csgo_client_C_CS_Player_GetFOV_vtable_index)
AFXADDR_DECL(csgo_client_C_CSPlayer_UpdateOnRemove_vtable_index)
AFXADDR_DECL(csgo_client_CPlayerResource_GetPing_vtable_index)
AFXADDR_DECL(csgo_materialsystem_Material_InterlockedDecrement_vtable_index)
AFXADDR_DECL(csgo_engine_CAudioXAudio2_UnkSupplyAudio_vtable_index)
AFXADDR_DECL(csgo_client_C_Team_Get_ClanName_vtable_index)
AFXADDR_DECL(csgo_client_C_Team_Get_FlagImageString_vtable_index)
AFXADDR_DECL(csgo_client_C_Team_Get_LogoImageString_vtable_index)
AFXADDR_DECL(csgo_client_CPlayerResource_GetPlayerName_vtable_index)
AFXADDR_DECL(csgo_client_CCSViewRender_RenderView_vtable_index)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_1_OFS)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_1_LEN)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_2_OFS)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_2_LEN)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_3_OFS)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_3_LEN)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_4_OFS)
AFXADDR_DECL(csgo_client_CanSeeSpectatorOnlyTools_4_LEN)
AFXADDR_DECL(csgo_client_CCSGO_MapOverview_CanShowOverview)
AFXADDR_DECL(csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr)
AFXADDR_DECL(csgo_client_CCSGO_MapOverview_FireGameEvent)
AFXADDR_DECL(csgo_client_CCSGO_MapOverview_ProcessInput)
AFXADDR_DECL(csgo_client_C_BaseEntity_GetTeamNumber)
AFXADDR_DECL(csgo_client_g_bEngineIsHLTV)
AFXADDR_DECL(csgo_client_C_BaseAnimating_RecordBones)
AFXADDR_DECL(css_client_C_BaseAnimating_RecordBones)
AFXADDR_DECL(cssv34_client_C_BaseAnimating_RecordBones)
AFXADDR_DECL(cssv34_client_C_BaseAnimating_m_BoneAccessor_m_pBones)
AFXADDR_DECL(tf2_client_C_BaseAnimating_RecordBones)
AFXADDR_DECL(csgo_client_CModelRenderSystem_SetupBones)

void Addresses_InitEngineDll(AfxAddr engineDll, SourceSdkVer sourceSdkVer);
void Addresses_InitPanoramaDll(AfxAddr panoramaDll, SourceSdkVer sourceSdkVer);
void Addresses_InitClientDll(AfxAddr clientDll, SourceSdkVer sourceSdkVer);
void Addresses_InitMaterialsystemDll(AfxAddr materialsystemDll, SourceSdkVer sourceSdkVer);

