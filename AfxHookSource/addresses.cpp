#include "stdafx.h"

#include "addresses.h"

#include <shared/binutils.h>

#include <cstdio>
#include <vector>

SourceSdkVer g_SourceSdkVer = SourceSdkVer_Unknonw;

using namespace Afx::BinUtils;

//AFXADDR_DEF(csgo_CPredictionCopy_TransferData)
//AFXADDR_DEF(csgo_CPredictionCopy_TransferData_DSZ)
//AFXADDR_DEF(csgo_C_BaseEntity_IClientEntity_vtable)
//AFXADDR_DEF(csgo_C_BaseEntity_ofs_m_nModelIndex)
//AFXADDR_DEF(csgo_C_BaseEntity_ofs_m_iWorldModelIndex)
//AFXADDR_DEF(csgo_C_BaseAnimating_IClientEntity_vtable)
//AFXADDR_DEF(csgo_C_BaseCombatWeapon_IClientEntity_vtable)
AFXADDR_DEF(csgo_CStaticProp_IClientEntity_vtable)
AFXADDR_DEF(csgo_C_BaseAnimating_vtable)
AFXADDR_DEF(csgo_DT_Animationlayer_m_flCycle_fn)
//AFXADDR_DEF(csgo_DT_Animationlayer_m_flPrevCycle_fn)
//AFXADDR_DEF(csgo_mystique_animation)
AFXADDR_DEF(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel)
AFXADDR_DEF(csgo_C_BaseCombatWeapon_m_iState)
//AFXADDR_DEF(csgo_C_BaseEntity_ToolRecordEnties)
//AFXADDR_DEF(csgo_C_BaseEntity_ToolRecordEnties_DSZ)
AFXADDR_DEF(csgo_C_BasePlayer_OFS_m_skybox3d_scale)
AFXADDR_DEF(csgo_C_BasePlayer_RecvProxy_ObserverTarget)
AFXADDR_DEF(csgo_CCSViewRender_vtable)
AFXADDR_DEF(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha)
AFXADDR_DEF(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw)
AFXADDR_DEF(csgo_CGlowOverlay_Draw)
AFXADDR_DEF(csgo_CGlowOverlay_Draw_DSZ)
AFXADDR_DEF(csgo_CCSGO_HudDeathNotice_FireGameEvent)
AFXADDR_DEF(csgo_CUnknown_GetPlayerName)
AFXADDR_DEF(csgo_CCSGameMovement_vtable)
AFXADDR_DEF(csgo_CSkyboxView_Draw)
AFXADDR_DEF(csgo_CSkyboxView_Draw_DSZ)
AFXADDR_DEF(csgo_CViewRender_RenderView_VGui_DrawHud_In)
AFXADDR_DEF(csgo_CViewRender_RenderView_VGui_DrawHud_Out)
AFXADDR_DEF(csgo_CViewRender_ShouldForceNoVis_vtable_index)
AFXADDR_DEF(csgo_engine_S_MixAsync)
AFXADDR_DEF(csgo_engine_WaveAppendTmpFile)
AFXADDR_DEF(csgo_engine_cl_movieinfo_moviename)
AFXADDR_DEF(csgo_engine_CL_StartMovie)
AFXADDR_DEF(csgo_engine_CVideoMode_Common_vtable)
AFXADDR_DEF(csgo_engine_CEngineVGui_vtable)
AFXADDR_DEF(csgo_SplineRope_CShader_vtable)
AFXADDR_DEF(csgo_Spritecard_CShader_vtable)
AFXADDR_DEF(csgo_UnlitGeneric_CShader_vtable)
AFXADDR_DEF(csgo_Unknown_GetTeamsSwappedOnScreen)
AFXADDR_DEF(csgo_VertexLitGeneric_CShader_vtable)
AFXADDR_DEF(csgo_S_StartSound_StringConversion)
AFXADDR_DEF(csgo_gpGlobals_OFS_curtime)
AFXADDR_DEF(csgo_gpGlobals_OFS_interpolation_amount)
AFXADDR_DEF(csgo_gpGlobals_OFS_interval_per_tick)
AFXADDR_DEF(csgo_snd_mix_timescale_patch)
AFXADDR_DEF(csgo_snd_mix_timescale_patch_DSZ)
AFXADDR_DEF(csgo_view)
AFXADDR_DEF(cstrike_gpGlobals_OFS_absoluteframetime)
AFXADDR_DEF(cstrike_gpGlobals_OFS_curtime)
AFXADDR_DEF(cstrike_gpGlobals_OFS_interpolation_amount)
AFXADDR_DEF(cstrike_gpGlobals_OFS_interval_per_tick)
AFXADDR_DEF(csgo_CClientState_ProcessVoiceData)
AFXADDR_DEF(csgo_CClientState_ProcessVoiceData_DSZ)
AFXADDR_DEF(csgo_CVoiceWriter_AddDecompressedData)
AFXADDR_DEF(csgo_CVoiceWriter_AddDecompressedData_DSZ)
AFXADDR_DEF(csgo_panorama_AVCUIPanel_UnkSetFloatProp)
AFXADDR_DEF(csgo_panorama_CZip_UnkLoadFiles)
AFXADDR_DEF(csgo_panorama_CUIEngine_RunFrame_Plat_FloatTime_Address)
AFXADDR_DEF(csgo_C_CSPlayer_IClientNetworkable_entindex)
AFXADDR_DEF(csgo_CShaderAPIDx8_UnkCreateTexture)
AFXADDR_DEF(csgo_CRendering3dView_DrawTranslucentRenderables)
AFXADDR_DEF(csgo_CGameEventManger_FireEventIntern)
AFXADDR_DEF(csgo_dynamic_cast)
AFXADDR_DEF(csgo_RTTI_CGameEvent)
AFXADDR_DEF(csgo_RTTI_IGameEvent)
//AFXADDR_DEF(csgo_client_dynamic_cast);
//AFXADDR_DEF(csgo_client_RTTI_IClientRenderable);
AFXADDR_DEF(csgo_GlowCurrentPlayer_JMPS)
AFXADDR_DEF(csgo_C_CSPlayer_UpdateClientSideAnimation)
AFXADDR_DEF(csgo_CNetChan_ProcessMessages)
AFXADDR_DEF(csgo_C_CSPlayer_vtable)
AFXADDR_DEF(csgo_C_CSPlayer_ofs_m_angEyeAngles)
AFXADDR_DEF(csgo_crosshair_localplayer_check)
AFXADDR_DEF(csgo_DamageIndicator_MessageFunc)
AFXADDR_DEF(csgo_C_BasePlayer_SetAsLocalPlayer)
AFXADDR_DEF(csgo_C_BasePlayer_GetToolRecordingState)
AFXADDR_DEF(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer)
AFXADDR_DEF(csgo_C_BaseViewModel_ofs_m_nAnimationParity)
AFXADDR_DEF(csgo_C_BaseViewModel_ofs_m_nOldAnimationParity)
AFXADDR_DEF(csgo_C_BaseEntity_ShouldInterpolate)
AFXADDR_DEF(csgo_C_CS_PlayerResource_IGameResources_vtable)
AFXADDR_DEF(csgo_C_Team_vtable)
AFXADDR_DEF(csgo_engine_CModelLoader_vtable)
AFXADDR_DEF(csgo_engine_CModelLoader_GetModel_vtable_index)
AFXADDR_DEF(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ)
AFXADDR_DEF(csgo_engine_Do_CCLCMsg_FileCRCCheck)
AFXADDR_DEF(csgo_C_BaseViewModel_FireEvent)
AFXADDR_DEF(csgo_client_AdjustInterpolationAmount)
//AFXADDR_DEF(csgo_C_BaseEntity_ofs_m_bPredictable)
AFXADDR_DEF(csgo_engine_Cmd_ExecuteCommand)
AFXADDR_DEF(tf2_engine_Cmd_ExecuteCommand)
AFXADDR_DEF(csgo_client_C_CSPlayer_EyeAngles_vtable_index)
AFXADDR_DEF(csgo_client_C_CS_Player_GetFOV_vtable_index)
AFXADDR_DEF(csgo_client_C_CSPlayer_UpdateOnRemove_vtable_index)
AFXADDR_DEF(csgo_client_CPlayerResource_Dtor_vtable_index)
AFXADDR_DEF(csgo_client_CPlayerResource_GetPing_vtable_index)
AFXADDR_DEF(csgo_materialsystem_Material_InterlockedDecrement_vtable_index)
AFXADDR_DEF(csgo_client_C_Team_Get_ClanName_vtable_index)
AFXADDR_DEF(csgo_client_C_Team_Get_FlagImageString_vtable_index)
AFXADDR_DEF(csgo_client_C_Team_Get_LogoImageString_vtable_index)
AFXADDR_DEF(csgo_client_CPlayerResource_GetPlayerName_vtable_index)
AFXADDR_DEF(csgo_client_CCSViewRender_RenderView_vtable_index)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_1_OFS)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_1_LEN)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_2_OFS)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_2_LEN)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_3_OFS)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_3_LEN)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_4_OFS)
AFXADDR_DEF(csgo_client_CanSeeSpectatorOnlyTools_4_LEN)
AFXADDR_DEF(csgo_client_CCSGO_MapOverview_CanShowOverview)
AFXADDR_DEF(csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr)
AFXADDR_DEF(csgo_client_CCSGO_MapOverview_FireGameEvent)
AFXADDR_DEF(csgo_client_CCSGO_MapOverview_ProcessInput)
AFXADDR_DEF(csgo_client_g_bEngineIsHLTV)
AFXADDR_DEF(csgo_client_C_BaseAnimating_RecordBones)
AFXADDR_DEF(css_client_C_BaseAnimating_RecordBones)
AFXADDR_DEF(cssv34_client_C_BaseAnimating_RecordBones)
AFXADDR_DEF(cssv34_client_C_BaseAnimating_m_BoneAccessor_m_pBones)
AFXADDR_DEF(tf2_client_C_BaseAnimating_RecordBones)
AFXADDR_DEF(csgo_client_CModelRenderSystem_SetupBones)
AFXADDR_DEF(csgo_client_s_HLTVCamera)
AFXADDR_DEF(csgo_client_CHLTVCamera_SpecCameraGotoPos)
AFXADDR_DEF(csgo_engine_CBoundedCvar_CmdRate_vtable)
AFXADDR_DEF(csgo_engine_CBoundedCvar_UpdateRate_vtable)
AFXADDR_DEF(csgo_engine_CBoundedCvar_Rate_vtable)
AFXADDR_DEF(csgo_engine_LocalClientClientState_ishltv)
AFXADDR_DEF(csgo_engine_m_SplitScreenPlayers)
AFXADDR_DEF(csgo_engine_m_SplitScreenPlayers_ishltv_ofs)
AFXADDR_DEF(materialsystem_GetRenderCallQueue)
AFXADDR_DEF(materialsystem_CFunctor_vtable_size)
AFXADDR_DEF(materialsystem_CMaterialSystem_SwapBuffers)
AFXADDR_DEF(materialsystem_CMatCallQueue_QueueFunctor)
AFXADDR_DEF(engine_CVideoMode_Common_WriteMovieFrame)
AFXADDR_DEF(engine_HostError)

void ErrorBox(char const * messageText);

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

/*
void Addresses_InitShaderApiDll(AfxAddr shaderApiDll, SourceSdkVer sourceSdkVer)
{
	AFXADDR_SET(csgo_CShaderAPIDx8_vtable, 0);

	if (SourceSdkVer_CSGO == sourceSdkVer)
	{
		{
			ImageSectionsReader sections((HMODULE)shaderApiDll);
			if (!sections.Eof())
			{
				// Now in .text (1)

				sections.Next();

				if (!sections.Eof())
				{
					// Now in .idata / .rdata (2)
					
					MemRange section2 = sections.GetMemRange();

					sections.Next();

					if (!sections.Eof())
					{
						// Now in .data (3).

						MemRange result = FindCString(sections.GetMemRange(), ".?AVCShaderAPIDx8@@");
						if (!result.IsEmpty())
						{
							DWORD tmpAddr = result.Start - 0x8;

							result.End = section2.Start;

							for (int i = 0; i < 2; ++i)
							{
								result = FindBytes(MemRange(result.End, section2.End), (char const *)&tmpAddr, sizeof(tmpAddr));
							}

							if (!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start - 0xC;

								result.End = section2.Start;

								for (int i = 0; i < 1; ++i)
								{
									result = FindBytes(MemRange(result.End, section2.End), (char const *)&tmpAddr, sizeof(tmpAddr));
								}

								if (!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start + (DWORD)sizeof(void *);

									AFXADDR_SET(csgo_CShaderAPIDx8_vtable, tmpAddr);
								}
								else ErrorBox(MkErrStr(__FILE__, __LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
	}
}
*/

void Addresses_InitEngineDll(AfxAddr engineDll, SourceSdkVer sourceSdkVer)
{
	if (SourceSdkVer_CSGO == sourceSdkVer)
	{
		// csgo_snd_mix_timescale_patch: // Last checked 2022-10-22
		{
			ImageSectionsReader sections((HMODULE)engineDll);
			MemRange textRange = sections.GetMemRange();

			// MIX_MixChannelsToPaintbuffer // Last checked 2022-10-22
			// Second function to reference "snd_pause_all" cvar this (ecx).
			MemRange result = FindPatternString(textRange, "55 8B EC 83 EC 14 89 55 F4 B8 44 AC 00 00 99");
			if (!result.IsEmpty()) {

				DWORD tempAddr = result.Start + 0xB3;

				MemRange result = FindPatternString(MemRange(tempAddr - 2, tempAddr - 2 + 2), "FF D0");
				if (!result.IsEmpty())
				{
					// After call VEnglineClient013::GetTimeScale now.
					AFXADDR_SET(csgo_snd_mix_timescale_patch, tempAddr);
					AFXADDR_SET(csgo_snd_mix_timescale_patch_DSZ, 0x8);
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else
				ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_S_StartSound_StringConversion: // Checked 2017-05-13.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)engineDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "Starting sound '%s' while system disabled.\n");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)engineDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					addr = result.Start - 0x1d;

					// check for pattern to see if it is the right address:
					unsigned char pattern[14] = { 0x8B, 0x01, 0x8D, 0x54, 0x24, 0x78, 0x68, 0x04, 0x01, 0x00, 0x00, 0x52, 0xFF, 0x10 };

					DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
					MemRange patternRange(addr, addr + patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if (result.Start != patternRange.Start || result.End != patternRange.End)
					{
						addr = 0;
						ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (addr)
			{
				AFXADDR_SET(csgo_S_StartSound_StringConversion, addr);
			}
			else
			{
				AFXADDR_SET(csgo_S_StartSound_StringConversion, 0x0);
			}
		}

		// csgo_engine_S_MixAsync // Checked 2022-12-01.
		{
			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();
				sections.Next(); // skip .text
				if (!sections.Eof())
				{
					MemRange firstDataRange = sections.GetMemRange();

					MemRange result = FindCString(sections.GetMemRange(), "snd_mix_async_frequency");
					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start;

						result = FindBytes(textRange, (char const*)&tmpAddr, sizeof(tmpAddr));
						if (!result.IsEmpty())
						{
							result = FindPatternString(MemRange(result.Start - 0x1, result.Start - 0x1 + 10).And(textRange), "68 ?? ?? ?? ?? B9 ?? ?? ?? ??");
							if (!result.IsEmpty()) {
								DWORD cvarThisAddr = *(DWORD *)(result.Start + 6);
								// this is the first match of this cvar in current code, we look for the second one:
								result = FindBytes(MemRange(result.Start - 0x1 + 10, textRange.End), (const char*)&cvarThisAddr, sizeof(cvarThisAddr));
								if(!result.IsEmpty()) {
									result = FindPatternString(MemRange(result.Start - 0xE, result.Start - 0xE + 3).And(textRange), "55 8B EC");
									if(!result.IsEmpty())
										AFXADDR_SET(csgo_engine_S_MixAsync, result.Start);
									else ErrorBox(MkErrStr(__FILE__, __LINE__));
								} else ErrorBox(MkErrStr(__FILE__, __LINE__));
							} else ErrorBox(MkErrStr(__FILE__, __LINE__));
						} else ErrorBox(MkErrStr(__FILE__, __LINE__));
					} else ErrorBox(MkErrStr(__FILE__, __LINE__));
				} else ErrorBox(MkErrStr(__FILE__, __LINE__));
			} else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_engine_WaveAppendTmpFile: // Checked 2022-08-02.
		{
			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();
				sections.Next(); // skip .text
				if (!sections.Eof())
				{
					MemRange firstDataRange = sections.GetMemRange();

					MemRange result = FindCString(sections.GetMemRange(), ".WAV");
					if (!result.IsEmpty())
					{
						DWORD strAddr = result.Start;
						MemRange result = textRange;

						for (int i = 0; i < 2 && !(result = FindBytes(result, (const char*)&strAddr, sizeof(strAddr))).IsEmpty(); ++i)
						{
							if (i < 1)
							{
								result.Start = result.End;
								result.End = textRange.End;
							}
						}

						if (!result.IsEmpty()) {
							result = FindPatternString(MemRange(result.Start - 0x28, result.Start - 0x28 + 3).And(textRange), "55 8B EC");
							if (!result.IsEmpty())
							{
								AFXADDR_SET(csgo_engine_WaveAppendTmpFile, result.Start);
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}
		}


		// csgo_engine_cl_movieinfo_moviename // Checked 2022-10-22.
		{
			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();
				sections.Next(); // skip .text
				if (!sections.Eof())
				{
					MemRange firstDataRange = sections.GetMemRange();

					MemRange result = FindCString(sections.GetMemRange(), "CEngineClient::Sound_ExtraUpdate()");
					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start;

						result = FindBytes(textRange, (char const*)&tmpAddr, sizeof(tmpAddr));
						if (!result.IsEmpty())
						{
							result = FindPatternString(MemRange(result.Start - 0xC9, result.Start - 0xC9 + 3).And(textRange), "55 8B EC");
							if (!result.IsEmpty())
							{
								//AFXADDR_SET(csgo_engine_S_ExtraUpdate, result.Start);

								MemRange result2 = FindPatternString(MemRange(result.Start + 0xA6, result.Start + 0xA6 + 6).And(textRange), "38 0D ?? ?? ?? ??");
								if (!result2.IsEmpty()) {
									DWORD addr_cl_movieinfo_moviename = *(DWORD*)(result2.Start + 2);
									AFXADDR_SET(csgo_engine_cl_movieinfo_moviename, addr_cl_movieinfo_moviename);
								}
								else ErrorBox(MkErrStr(__FILE__, __LINE__));

								//result2 = FindPatternString(MemRange(result.Start + 0x244, result.Start + 0x244 + 5).And(textRange), "E8 ?? ?? ?? ??");
								//if (!result2.IsEmpty()) {
								//	DWORD addr_S_Update_ = result2.Start + 5 + *(DWORD*)(result2.Start + 1); // decode call address.
								//	AFXADDR_SET(csgo_engine_S_Update_, addr_S_Update_);
								//}
								//else ErrorBox(MkErrStr(__FILE__, __LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_engine_CL_StartMovie: // Checked 2022-08-04.
		{
			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();
				sections.Next(); // skip .text
				if (!sections.Eof())
				{
					MemRange firstDataRange = sections.GetMemRange();

					MemRange result = FindCString(sections.GetMemRange(), "Started recording movie, frames will record after console is cleared...\n");
					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start;

						result = FindBytes(textRange, (char const*)&tmpAddr, sizeof(tmpAddr));
						if (!result.IsEmpty())
						{
							result = FindPatternString(MemRange(result.Start - 0x6, result.Start - 0x6 + 5).And(textRange), "E8 ?? ?? ?? ??");
							if (!result.IsEmpty())
							{
								DWORD addrFn = result.Start + 5 + *(DWORD*)(result.Start + 1); // decode call address.
								AFXADDR_SET(csgo_engine_CL_StartMovie, addrFn);
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		//csgo_engine_CVideoMode_Common_vtable // Checked 2022-08-04
		AFXADDR_SET(csgo_engine_CVideoMode_Common_vtable, FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0));
		if (!AFXADDR_GET(csgo_engine_CVideoMode_Common_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		//csgo_engine_CEngineVGui_vtable // Checked 2022-08-02
		AFXADDR_SET(csgo_engine_CEngineVGui_vtable, FindClassVtable((HMODULE)engineDll, ".?AVCEngineVGui@@", 0, 0x0));
		if (!AFXADDR_GET(csgo_engine_CEngineVGui_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_CClientState_ProcessVoiceData: // Checked 2017-09-21.
		{
			DWORD addr = 0;
			{
				ImageSectionsReader sections((HMODULE)engineDll);
				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						MemRange result = FindCString(sections.GetMemRange(), "Received voice from: %d\n");
						if (!result.IsEmpty())
						{
							DWORD tmpAddr = result.Start;

							result = FindBytes(textRange, (char const *)&tmpAddr, sizeof(tmpAddr));
							if (!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x2B;

								result = FindPatternString(MemRange(tmpAddr, tmpAddr + 0x6), "55 8B EC 83 E4 F8");
								if (!result.IsEmpty())
								{
									addr = tmpAddr;
								}
								else ErrorBox(MkErrStr(__FILE__, __LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_CClientState_ProcessVoiceData, addr);
		}

		// csgo_CVoiceWriter_AddDecompressedData: // Checked 2017-09-21.
		{
			DWORD addr = 0;
			{
				ImageSectionsReader sections((HMODULE)engineDll);
				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						MemRange result = FindCString(sections.GetMemRange(), "Voice channel %d circular buffer overflow!\n");
						if (!result.IsEmpty())
						{
							DWORD tmpAddr = result.Start;

							result = FindBytes(textRange, (char const *)&tmpAddr, sizeof(tmpAddr));
							if (!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x26;

								result = FindPatternString(MemRange(tmpAddr - 0x1, tmpAddr - 0x1 + 0x5), "E8 ?? ?? ?? ??");
								if (!result.IsEmpty())
								{
									addr = tmpAddr;
									addr = addr + 4 + *(DWORD *)addr; // get CALL address
								}
								else ErrorBox(MkErrStr(__FILE__, __LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_CVoiceWriter_AddDecompressedData, addr);
		}

		// csgo_CGameEventManger_FireEventIntern: // Last checked 2022-10-22
		//
		// (References "Game event \"%s\", Tick %i:\n".)
		{
			AFXADDR_SET(csgo_CGameEventManger_FireEventIntern, 0);
			AFXADDR_SET(csgo_dynamic_cast, 0);
			AFXADDR_SET(csgo_RTTI_CGameEvent, 0);
			AFXADDR_SET(csgo_RTTI_IGameEvent, 0);

			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F8 83 EC 0C 8B C1 53 56 57 8D B0 98 00 00 00 89 44 24 10 89 74 24 14 FF 15 ?? ?? ?? ??");

				if (!result.IsEmpty())
				{
					AFXADDR_SET(csgo_CGameEventManger_FireEventIntern, result.Start);

					result = FindPatternString(MemRange(result.Start, result.Start + 310).And(textRange), "6A 00 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? 6A 00 53 E8 ?? ?? ?? ??");

					if (!result.IsEmpty())
					{
						AFXADDR_SET(csgo_RTTI_CGameEvent, *(DWORD *)(result.Start + 3));
						AFXADDR_SET(csgo_RTTI_IGameEvent, *(DWORD *)(result.Start + 8));
						AFXADDR_SET(csgo_dynamic_cast, *(DWORD *)(result.Start + 16) + result.Start + 16 + 4);
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_CStaticProp_IClientEntity_vtable // Checked 2019-08-24.
		AFXADDR_SET(csgo_CStaticProp_IClientEntity_vtable, FindClassVtable((HMODULE)engineDll, ".?AVCStaticProp@@", 0, 0x4));
		if (!AFXADDR_GET(csgo_CStaticProp_IClientEntity_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_CNetChan_ProcessMessages // Checked 2020-09-20.
		// This function references string "ProcessMessages %s: incoming buffer overflow!\n".
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F0 81 EC 88 00 00 00 56 57 8B F9 B9 ?? ?? ?? ?? C6 47 16 00 F7 05 ?? ?? ?? ?? 00 10 00 00");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_CNetChan_ProcessMessages, addr);
		}

		// csgo_engine_CModelLoader_vtable	
		AFXADDR_SET(csgo_engine_CModelLoader_vtable, FindClassVtable((HMODULE)engineDll, ".?AVCModelLoader@@", 0, 0x0));
		if (!AFXADDR_GET(csgo_engine_CModelLoader_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));
		AFXADDR_SET(csgo_engine_CModelLoader_GetModel_vtable_index, 7);

		// csgo_engine_Do_CCLCMsg_FileCRCCheck
		// This function is called right after a function that references the string "CheckUpdatingSteamResources"
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)engineDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 81 EC ?? ?? ?? ?? 53 8B D9 89 5D F8 80 BB ?? ?? ?? ?? 00 0F 84 ?? ?? ?? ?? 83 BB 00 01 00 00 06 0F 85 ?? ?? ?? ?? FF 15 ?? ?? ?? ??");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_engine_Do_CCLCMsg_FileCRCCheck, addr);
		}
	}

	if (SourceSdkVer_CSGO == sourceSdkVer || SourceSdkVer_TF2 == sourceSdkVer)
	{
		// csgo_engine_Cmd_ExecuteCommand: // Checked 2019-11-11.
		// tf2_engine_Cmd_ExecuteCommand: // Checked 2024-08-03.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)engineDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "WARNING: INVALID EXECUTION MARKER.\n");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)engineDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					addr = result.Start - (SourceSdkVer_CSGO == sourceSdkVer ? 0x79 : 0x69);

					// check for pattern to see if it is the right address:
					unsigned char pattern[3] = { 0x55, 0x8B, 0xEC};

					DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
					MemRange patternRange(addr, addr + patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if (result.Start != patternRange.Start || result.End != patternRange.End)
					{
						addr = 0;
						ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (addr)
			{
				if(SourceSdkVer_CSGO == sourceSdkVer) {
					AFXADDR_SET(csgo_engine_Cmd_ExecuteCommand, addr);
				} else {
					AFXADDR_SET(tf2_engine_Cmd_ExecuteCommand, addr);
				}
			}
		}
	}

	if (SourceSdkVer_CSGO == sourceSdkVer)
	{
		// mirv_pov related
		// csgo_engine_CBoundedCvar_CmdRate_vtable // Checked 2023-03-24
		{
			AFXADDR_SET(csgo_engine_CBoundedCvar_CmdRate_vtable,FindClassVtable((HMODULE)engineDll, ".?AVCBoundedCvar_CmdRate@@", 0, 0x0));
			if(!AFXADDR_GET(csgo_engine_CBoundedCvar_CmdRate_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		// csgo_engine_CBoundedCvar_UpradteRate_vtable // Checked 2023-03-24
		{
			AFXADDR_SET(csgo_engine_CBoundedCvar_UpdateRate_vtable,FindClassVtable((HMODULE)engineDll, ".?AVCBoundedCvar_UpdateRate@@", 0, 0x0));
			if(!AFXADDR_GET(csgo_engine_CBoundedCvar_UpdateRate_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));
			else {
				void **vtable = (void **)AFXADDR_GET(csgo_engine_CBoundedCvar_UpdateRate_vtable);
				auto result = FindPatternString(MemRange((size_t)vtable[12],(size_t)vtable[12]+0x1c),"55 8B EC 83 EC 0C E8 ?? ?? ?? ?? A1 ?? ?? ?? ?? F3 0F 11 45 F4 80 B8 ?? ?? ?? ?? 00");
				if(result.IsEmpty()) ErrorBox(MkErrStr(__FILE__, __LINE__));
				else {
					AFXADDR_SET(csgo_engine_m_SplitScreenPlayers, *(size_t*)(result.Start + 12));
					AFXADDR_SET(csgo_engine_m_SplitScreenPlayers_ishltv_ofs, *(size_t*)(result.Start + 23));
				}
			}
		}
		// csgo_engine_CBoundedCvar_Rate_vtable // Checked 2023-03-24
		{
			AFXADDR_SET(csgo_engine_CBoundedCvar_Rate_vtable,FindClassVtable((HMODULE)engineDll, ".?AVCBoundedCvar_Rate@@", 0, 0x0));
			if(!AFXADDR_GET(csgo_engine_CBoundedCvar_Rate_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
	}
	else
	{
		AFXADDR_SET(csgo_S_StartSound_StringConversion, 0x0);
		AFXADDR_SET(csgo_CClientState_ProcessVoiceData, 0x0);
		AFXADDR_SET(csgo_CVoiceWriter_AddDecompressedData, 0x0);
		AFXADDR_SET(csgo_CGameEventManger_FireEventIntern, 0);
		AFXADDR_SET(csgo_dynamic_cast, 0);
		AFXADDR_SET(csgo_RTTI_CGameEvent, 0);
		AFXADDR_SET(csgo_RTTI_IGameEvent, 0);
		AFXADDR_SET(csgo_CStaticProp_IClientEntity_vtable, 0x0);
		AFXADDR_SET(csgo_CNetChan_ProcessMessages, 0x0);
		AFXADDR_SET(csgo_engine_CModelLoader_vtable, 0x0);
		AFXADDR_SET(csgo_engine_Do_CCLCMsg_FileCRCCheck, 0x0);
	}
	AFXADDR_SET(csgo_CClientState_ProcessVoiceData_DSZ, 0x6);
	AFXADDR_SET(csgo_CVoiceWriter_AddDecompressedData_DSZ, 0x8);

	// engine_CVideoMode_Common_WriteMovieFrame
	//
	// To find the virtual function search for string such as:
	// - "endmovie\n"
	// - "Tried to write movie buffer with no filename set!\n" <--
	// - "Couldn't allocate bitmap header to snapshot.\n"
	// - "%s%04d.tga"
	// - "%s%04d.jpg"
	{
		MemRange::FromEmpty();
		switch(sourceSdkVer) {
		case SourceSdkVer_SWARM: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[23]));
		} break;
		case SourceSdkVer_CSGO: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[23]));
		} break;
		case SourceSdkVer_TF2: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[26]));
		} break;
		case SourceSdkVer_CSS: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[26]));
		} break;
		case SourceSdkVer_CSSV34: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[22]));
		} break;
		case SourceSdkVer_Insurgency2: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[23]));
		} break;
		case SourceSdkVer_L4D2: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[23]));
		} break;
		case SourceSdkVer_HL2MP: {
			// Checked 2024-01-05.
			void **vtable = (void **)FindClassVtable((HMODULE)engineDll, ".?AVCVideoMode_Common@@", 0, 0x0);
			if(vtable) AFXADDR_SET(engine_CVideoMode_Common_WriteMovieFrame, (size_t)(vtable[26]));
		} break;
		}
	}

	// engine_CallHostError_CL_PreserveExistingEntity
	{
		switch(sourceSdkVer) {
			case SourceSdkVer_CSS:
			case SourceSdkVer_TF2: { // Checked 2024-03-22
				ImageSectionsReader sections((HMODULE)engineDll);
				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						MemRange result = FindCString(sections.GetMemRange(), "CL_PreserveExistingEntity: missing client entity %d.\n");
						if (!result.IsEmpty())
						{
							DWORD tmpAddr = result.Start;

							result = FindBytes(textRange, (char const*)&tmpAddr, sizeof(tmpAddr));
							if (!result.IsEmpty())
							{
								result = FindPatternString(MemRange(result.Start + 0x4, result.Start + 0x4 + 5).And(textRange), "E8 ?? ?? ?? ??");
								if (!result.IsEmpty()) {
									AFXADDR_SET(engine_HostError, result.Start + 5 + *(size_t*)(result.Start + 1));
								} else ErrorBox(MkErrStr(__FILE__, __LINE__));
							} else ErrorBox(MkErrStr(__FILE__, __LINE__));
						} else ErrorBox(MkErrStr(__FILE__, __LINE__));
					} else ErrorBox(MkErrStr(__FILE__, __LINE__));
				} else ErrorBox(MkErrStr(__FILE__, __LINE__));			
			} break;
		}		
	}
}

void Addresses_InitPanoramaDll(AfxAddr panoramaDll, SourceSdkVer sourceSdkVer)
{
	if(SourceSdkVer_CSGO == sourceSdkVer)
	{
		// csgo_panorama_AVCUIPanel_UnkSetFloatProp // Checked 2018-08-03.
		{
			DWORD addr = 0;
			DWORD tmpAddr = FindClassVtable((HMODULE)panoramaDll, ".?AVCUIPanel@panorama@@", 0, 0x0);
			if (tmpAddr) {
				addr = ((DWORD *)tmpAddr)[283];
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_panorama_AVCUIPanel_UnkSetFloatProp, addr);
		}

		// csgo_panorama_CZip_UnkLoadFiles // Checked 2018-08-03.
		{
			DWORD addr = 0;
			DWORD tmpAddr = FindClassVtable((HMODULE)panoramaDll, ".?AVCZip@@", 0, 0x0);
			if (tmpAddr) {
				addr = ((DWORD *)tmpAddr)[14];
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_panorama_CZip_UnkLoadFiles, addr);
		}

		// csgo_panorama_CUIEngine_RunFrame_Plat_FloatTime_Address // Checked 2023-03-10
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)panoramaDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "CUIEngine::RunFrame");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)panoramaDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					addr = result.Start + 31;

					// check for pattern to see if it is the right address:
					unsigned char pattern[2] = { 0xFF, 0x15 };

					DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
					MemRange patternRange(addr, addr + patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if (result.Start != patternRange.Start || result.End != patternRange.End)
					{
						ErrorBox(MkErrStr(__FILE__, __LINE__));
					} else {
						AFXADDR_SET(csgo_panorama_CUIEngine_RunFrame_Plat_FloatTime_Address, addr + 2);
					}
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
		}			
	}
}

void Addresses_InitClientDll(AfxAddr clientDll, SourceSdkVer sourceSdkVer)
{
	if(SourceSdkVer_CSGO == sourceSdkVer)
	{
		// csgo_CCSGO_HudDeathNotice_FireGameEvent // Checked 2018-08-03.
		{
			AFXADDR_SET(csgo_CCSGO_HudDeathNotice_FireGameEvent, 0x0);
			//AFXADDR_SET(csgo_CCSGO_HudDeathNotice_UnkRemoveNotices, 0x0);
			DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVCCSGO_HudDeathNotice@@", 0, 0x14);
			if (tmpAddr) {
				AFXADDR_SET(csgo_CCSGO_HudDeathNotice_FireGameEvent, ((DWORD *)tmpAddr)[1]);
				//AFXADDR_SET(csgo_CCSGO_HudDeathNotice_UnkRemoveNotices, ((DWORD *)tmpAddr)[9]);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));	
		}

		// csgo_CUnknown_GetPlayerName: // Checked 2019-03-07
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "#SFUI_coach_name_ct");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					addr = result.Start - 0x1a1;

					// check for pattern to see if it is the right address:
					unsigned char pattern[3] = { 0x55, 0x8B, 0xEC };

					DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
					MemRange patternRange(addr, addr + patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if (result.Start != patternRange.Start || result.End != patternRange.End)
					{
						addr = 0;
						ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (addr)
			{
				AFXADDR_SET(csgo_CUnknown_GetPlayerName, addr);
			}
			else
			{
				AFXADDR_SET(csgo_CUnknown_GetPlayerName, 0x0);
			}
		}

		// csgo_CViewRender_RenderView_VGui_DrawHud_In: // Checked 2019-02-02.
		// csgo_CViewRender_RenderView_VGui_DrawHud_Out: // Checked 2019-02-02.
		{
			// This is around the reference to "VGui_DrawHud".
			// The hooks using the address need updating if the pattern changes!

			AFXADDR_SET(csgo_CViewRender_RenderView_VGui_DrawHud_In, 0);
			AFXADDR_SET(csgo_CViewRender_RenderView_VGui_DrawHud_Out, 0);

			ImageSectionsReader sections((HMODULE)clientDll);
			
			MemRange baseRange = sections.GetMemRange();
			MemRange result = FindPatternString(baseRange, "0F 84 ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? 8B 81 0C 10 00 00 89 44 24 40 85 C0 74 16 6A 04 6A 00 68 ?? ?? ?? ?? 6A 00 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ?? E8 ?? ?? ?? ??");
			if(!result.IsEmpty())
			{
				DWORD jzAddr = *(DWORD *)(result.Start + 2) + result.Start + 6;
				MemRange result2 = FindPatternString(baseRange.And(MemRange(result.Start, jzAddr)), "83 7C 24 40 00 74 0C");
				if (!result2.IsEmpty())
				{
					AFXADDR_SET(csgo_CViewRender_RenderView_VGui_DrawHud_In, result.Start + 48);
					AFXADDR_SET(csgo_CViewRender_RenderView_VGui_DrawHud_Out, result2.Start);
				}
					
			}
			else ErrorBox(MkErrStr(__FILE__,__LINE__));
		}
		AFXADDR_SET(csgo_CViewRender_ShouldForceNoVis_vtable_index, 42);

		// Smoke overlay related.
		//
		// csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha: // Checked 2020-06-16.
		// csgo_CCSViewRender_RenderSmokeOverlay_OnLoadAlphaBeforeDraw: // Checked 2021-04-28.
		// If these change, multiple ASM snippets and offsets need to be changed in csgo_CViewRender.cpp
		{
			DWORD addrOnLoadOldAlpha = 0;
			DWORD addrOnCompareAlphaBeforeDraw = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "effects/overlaysmoke");
						if(!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			if(strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);
			
				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if(!result.IsEmpty())
				{
					baseRange = MemRange(result.End, baseRange.End);
					result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
					if(!result.IsEmpty())
					{
						DWORD pushStringAddr = result.Start - 0x1;

						{
							DWORD tmpAddr = pushStringAddr - 0x6F;

							// check for pattern nearby to see if it is the right address:
							unsigned char pattern[8] = { 0xF3, 0x0F, 0x10, 0x9A, 0x88, 0x05, 0x00, 0x00 }; 

							DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
							MemRange patternRange(tmpAddr, tmpAddr + patternSize);
							MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
							if (result.Start != patternRange.Start || result.End != patternRange.End)
								ErrorBox(MkErrStr(__FILE__, __LINE__));
							else
								addrOnLoadOldAlpha = tmpAddr;
						}

						{
							DWORD tmpAddr = pushStringAddr - 0x1A;

							// check for pattern nearby to see if it is the right address:
							unsigned char pattern[7] = { 0x0F, 0x2F, 0x82, 0x88, 0x05, 0x00, 0x00 };


							DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
							MemRange patternRange(tmpAddr, tmpAddr + patternSize);
							MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
							if (result.Start != patternRange.Start || result.End != patternRange.End)
								ErrorBox(MkErrStr(__FILE__, __LINE__));
							else
								addrOnCompareAlphaBeforeDraw = tmpAddr;
						}
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha, addrOnLoadOldAlpha);
			AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw, addrOnCompareAlphaBeforeDraw);
		}
		AFXADDR_SET(csgo_client_CCSViewRender_RenderView_vtable_index, 6);


		// csgo_CSkyboxView_Draw: // Checked 2017-05-13.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "CViewRender::Draw3dSkyboxworld");
						if(!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			if(strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);
			
				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if(!result.IsEmpty())
				{
					DWORD tmpAddr = result.Start;
					tmpAddr -= 0x20;

					// check for pattern nearby to see if it is the right address:
					unsigned char pattern[4] = { 0x56, 0x57, 0x8B, 0xF9 };

					DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
					MemRange patternRange(tmpAddr, tmpAddr + patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if (result.Start != patternRange.Start || result.End != patternRange.End)
						ErrorBox(MkErrStr(__FILE__, __LINE__));
					else
						addr = tmpAddr;
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_CSkyboxView_Draw, addr);
		}

		// csgo_view // Checked 2017-05-13.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "CViewRender::SetUpView->OnRenderEnd");
						if(!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			if(strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);
			
				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if(!result.IsEmpty())
				{
					DWORD tmpAddr = result.Start;
					tmpAddr += 0xB;

					if (!FindPatternString(MemRange(tmpAddr -2, tmpAddr + 11 -2), "8B  0D ?? ?? ?? ?? 8B 01 FF 50 10").IsEmpty())
					{
						addr = *(DWORD *)tmpAddr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_view, addr);
		}

		// csgo_C_BasePlayer_OFS_m_skybox3d_scale // Checked 2017-05-13.
		//
		{
			// this basically uses the RecvProp* functions in c_baseplayer.cpp to get the offsets of the fields.

			DWORD strAddr_m_Local = 0;
			DWORD strAddr_m_skybox3d_scale = 0;
			//DWORD strAddr_m_bDucked = 0;
			//DWORD strAddr_m_bDucking = 0;
			//DWORD strAddr_m_flDuckAmount = 0;
			DWORD ofs_m_Local_m_skybox3d_scale = -1;
			//DWORD ofs_m_Local_m_bDucked = -1;
			//DWORD ofs_m_Local_m_bDucking = -1;
			//DWORD ofs_m_Local_m_flDuckAmount = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_Local");
							if(!result.IsEmpty())
							{
								strAddr_m_Local = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__,__LINE__));
						}
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_skybox3d.scale");
							if(!result.IsEmpty())
							{
								strAddr_m_skybox3d_scale = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__,__LINE__));
						}
						/*
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_bDucked");
							if (!result.IsEmpty())
							{
								strAddr_m_bDucked = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_bDucking");
							if (!result.IsEmpty())
							{
								strAddr_m_bDucking = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_flDuckAmount");
							if (!result.IsEmpty())
							{
								strAddr_m_flDuckAmount = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						*/
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}

			if(strAddr_m_Local)
			{
				bool has_m_Local = false;
				DWORD ofs_m_Local = 0;

				{
					ImageSectionsReader sections((HMODULE)clientDll);
			
					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_Local, sizeof(strAddr_m_Local));
					if(!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 4+2+4;
						addr = *(DWORD *)addr;
						ofs_m_Local = addr;
						has_m_Local = true;
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));				
				}

				if (has_m_Local)
				{
					if (strAddr_m_skybox3d_scale)
					{
						ImageSectionsReader sections((HMODULE)clientDll);

						MemRange baseRange = sections.GetMemRange();
						MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_skybox3d_scale, sizeof(strAddr_m_skybox3d_scale));
						if (!result.IsEmpty())
						{
							DWORD addr = result.Start;
							addr += 4 + 2 + 4;
							addr = *(DWORD *)addr;
							ofs_m_Local_m_skybox3d_scale = ofs_m_Local + addr;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
				}
			}

			AFXADDR_SET(csgo_C_BasePlayer_OFS_m_skybox3d_scale, ofs_m_Local_m_skybox3d_scale);
		}

		// csgo_C_BaseCombatWeapon_m_hWeaponWorldModel
		//
		{
			// this basically uses the RecvProp* functions in c_baseplayer.cpp to get the offsets of the fields.

			DWORD strAddr_m_hWeaponWorldModel = 0;
			DWORD ofs_m_hWeaponWorldModel = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_hWeaponWorldModel");
							if (!result.IsEmpty())
							{
								strAddr_m_hWeaponWorldModel = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr_m_hWeaponWorldModel)
			{
				{
					ImageSectionsReader sections((HMODULE)clientDll);

					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_hWeaponWorldModel, sizeof(strAddr_m_hWeaponWorldModel));
					if (!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 4 + 2 + 4;
						addr = *(DWORD *)addr;
						ofs_m_hWeaponWorldModel = addr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}

			AFXADDR_SET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel, ofs_m_hWeaponWorldModel);
		}


		// csgo_C_BaseCombatWeapon_m_iState
		//
		{
			// this basically uses the RecvProp* functions in c_baseplayer.cpp to get the offsets of the fields.

			DWORD strAddr_m_iState = 0;
			DWORD ofs_m_iState = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_iState");
							if (!result.IsEmpty())
							{
								strAddr_m_iState = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr_m_iState)
			{
				{
					ImageSectionsReader sections((HMODULE)clientDll);

					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_iState, sizeof(strAddr_m_iState));
					if (!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 4 + 2 + 4;
						addr = *(DWORD *)addr;
						ofs_m_iState = addr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}

			AFXADDR_SET(csgo_C_BaseCombatWeapon_m_iState, ofs_m_iState);
		}

		// csgo_C_BaseEntity_ToolRecordEnties: // Checked 2017-05-13.
		/*
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "C_BaseEntity::ToolRecordEnties");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					DWORD targetAddr = result.Start - 0x1D;

					// check for pattern nearby to see if it is the right address:
					unsigned char pattern[2] = { 0x8B, 0x0D };

					DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
					MemRange patternRange(targetAddr, targetAddr + patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if (result.Start != patternRange.Start || result.End != patternRange.End)
						ErrorBox(MkErrStr(__FILE__, __LINE__));
					else
						addr = targetAddr;
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			AFXADDR_SET(csgo_C_BaseEntity_ToolRecordEnties, addr);
		}
		*/

		// csgo_C_BasePlayer_RecvProxy_ObserverTarget: // Fixed 2017-08-18.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "m_hObserverTarget");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();

					MemRange result = FindBytes(textRange, (const char *)&strAddr, sizeof(strAddr));

					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start - 0xf;

						result = FindPatternString(MemRange(tmpAddr, result.End), "68 ?? ?? ?? ?? 6A 00 6A 04 68 ?? ?? ?? ??");

						if (!result.IsEmpty() && result.Start == tmpAddr)
						{
							addr = *(DWORD *)(tmpAddr + 0x1);
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_C_BasePlayer_RecvProxy_ObserverTarget, addr);
		}

		// csgo_DT_Animationlayer_m_flCycle_fn: // Checked 2017-05-19.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "m_flCycle");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();

					MemRange result = textRange;

					for (int i = 0; i < 2 && !(result = FindBytes(result, (const char *)&strAddr, sizeof(strAddr))).IsEmpty(); ++i)
					{
						if (i < 1)
						{
							result.Start = result.End;
							result.End = textRange.End;
						}
					}

					// may check instruction opcode here, hm.

					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start + 0x24;

						result = FindPatternString(MemRange(tmpAddr -0x2, tmpAddr +0xC -0x2), "C7 05 ?? ?? ?? ?? ?? ?? ?? ?? C7 05");

						if (!result.IsEmpty() && result.Start == tmpAddr - 2)
						{
							addr = tmpAddr;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_DT_Animationlayer_m_flCycle_fn, addr);
		}

		/*
		// csgo_DT_Animationlayer_m_flPrevCycle_fn
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "m_flPrevCycle");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();

					MemRange result = textRange;

					for (int i = 0; i < 1 && !(result = FindBytes(result, (const char *)&strAddr, sizeof(strAddr))).IsEmpty(); ++i)
					{
						if (i < 0)
						{
							result.Start = result.End;
							result.End = textRange.End;
						}
					}

					// may check instruction opcode here, hm.

					if (!result.IsEmpty())
					{
						addr = result.Start + 0x24;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_DT_Animationlayer_m_flPrevCycle_fn, addr);
		}
		*/

		// csgo_CCSViewRender_vtable // Checked 2018-08-03.
		AFXADDR_SET(csgo_CCSViewRender_vtable, FindClassVtable((HMODULE)clientDll, ".?AVCCSViewRender@@", 0, 0x0));
		if(!AFXADDR_GET(csgo_CCSViewRender_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_C_CSPlayer_IClientNetworkable_entindex // Checked 2018-08-03.
		{
			DWORD addr = 0;
			DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVC_CSPlayer@@", 0, 0x08);
			if (tmpAddr) {
				addr = ((DWORD *)tmpAddr)[10];
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_C_CSPlayer_IClientNetworkable_entindex, addr);
		}

		/*
		// csgo_CPredictionCopy_TransferData:
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "C_BaseEntity::SaveData");
						if (!result.IsEmpty())
						{
							strAddr = result.Start;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();

					MemRange result = FindBytes(textRange, (const char *)&strAddr, sizeof(strAddr));

					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start + 0x8;

						tmpAddr = *(DWORD *)(tmpAddr +0x1) + tmpAddr + 0x5; // read call target of copyHelper.TransferData in C_BaseEntity::SaveData

						if (textRange.Start <= tmpAddr
							&& tmpAddr < textRange.End
							&& (result = FindPatternString(MemRange(tmpAddr, tmpAddr+0x3), "55 8B EC"), !result.IsEmpty()))
						{
							addr = tmpAddr;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_CPredictionCopy_TransferData, addr);
		}
		*/

		// csgo_CCSGameMovement_vtable // Checked 2018-08-03.
		AFXADDR_SET(csgo_CCSGameMovement_vtable, FindClassVtable((HMODULE)clientDll, ".?AVCCSGameMovement@@", 0, 0x0));
		if (!AFXADDR_GET(csgo_CCSGameMovement_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_C_BaseEntity_IClientEntity_vtable // Checked 2019-08-24.
		//AFXADDR_SET(csgo_C_BaseEntity_IClientEntity_vtable, FindClassVtable((HMODULE)clientDll, ".?AVC_BaseEntity@@", 0, 0x4));
		//if (!AFXADDR_GET(csgo_C_BaseEntity_IClientEntity_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_C_BaseAnimating_vtable // Checked 2018-08-03.
		AFXADDR_SET(csgo_C_BaseAnimating_vtable, FindClassVtable((HMODULE)clientDll, ".?AVC_BaseAnimating@@", 0, 0x0));
		if (!AFXADDR_GET(csgo_C_BaseAnimating_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_C_BaseAnimating_IClientEntity_vtable // Checked 2019-08-31.
		//AFXADDR_SET(csgo_C_BaseAnimating_IClientEntity_vtable, FindClassVtable((HMODULE)clientDll, ".?AVC_BaseAnimating@@", 0, 0x4));
		//if (!AFXADDR_GET(csgo_C_BaseAnimating_IClientEntity_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_C_BaseCombatWeapon_IClientEntity_vtable // Checked 2019-08-31.
		//AFXADDR_SET(csgo_C_BaseCombatWeapon_IClientEntity_vtable, FindClassVtable((HMODULE)clientDll, ".?AVC_BaseCombatWeapon@@", 0, 0x4));
		//if (!AFXADDR_GET(csgo_C_BaseCombatWeapon_IClientEntity_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_CGlowOverlay_Destructor, csgo_CGlowOverlay_Draw:
		{
			DWORD addrDraw = 0;
			/*{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();

					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						sections.Next(); // skip first .data
						if (!sections.Eof())
						{
							MemRange result = FindCString(sections.GetMemRange(), ".?AVCGlowOverlay@@");
							if (!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x8;

								result.Start = firstDataRange.Start;
								result.End = firstDataRange.Start;

								for (int i = 0; i < 2; ++i)
								{
									result = FindBytes(MemRange(result.End, firstDataRange.End), (char const *)&tmpAddr, sizeof(tmpAddr));
								}

								if (!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start;
									tmpAddr -= 0xC;

									result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
									if (!result.IsEmpty())
									{
										DWORD vtableAddr = result.Start;
										vtableAddr += (1) * 4;

										DWORD tmpDrawAddr = *(DWORD *)( 4*4 +vtableAddr);
										if (textRange.Start <= tmpDrawAddr && tmpDrawAddr < textRange.End - 0xc)
										{
											if (!FindPatternString(MemRange(tmpDrawAddr, tmpDrawAddr + 0xc), "55 8B EC 83 E4 F0 81 EC 78 04 00 00").IsEmpty())
											{
												addrDraw = tmpDrawAddr;
											}
											else ErrorBox(MkErrStr(__FILE__, __LINE__));
										}
										else ErrorBox(MkErrStr(__FILE__, __LINE__));
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));
								}
								else ErrorBox(MkErrStr(__FILE__, __LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}*/
			AFXADDR_SET(csgo_CGlowOverlay_Draw, addrDraw);			
		}

		/*
		// csgo_mystique_animation:
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 EC 1C 56 57 8B F9 F3 0F 11 55 F8 F3 0F 11 4D F4 8B 4F 60 85 C9");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_mystique_animation, addr);
		}
		*/

		
		// csgo_Unknown_GetTeamsSwappedOnScreen:
		//
		// This function holds the second and third reference to global class pointer of "cl_spec_swapplayersides".
		// It's most reliable to find it by pattern matching I think :/
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 8B 0D ?? ?? ?? ?? 53 56 E8 ?? ?? ?? ?? 8B 4D 04");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_Unknown_GetTeamsSwappedOnScreen, addr);
		}

		// csgo_CRendering3dView_DrawTranslucentRenderables:
		// (CRendering3dView::DrawTranslucentRenderables)
		//
		// This function holds the only two refrercnes to the string "DrawTranslucentEntities".
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 81 EC ?? ?? ?? ?? 83 3D ?? ?? ?? ?? 00 53 56 8B D9 57 89 5D ?? 74 ??");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_CRendering3dView_DrawTranslucentRenderables, addr);
		}

		/*
		// client dynamic_cast // Checked 2019-08-28
		//
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "6A 18 68 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 7D 08 85 FF 75 08 33 C0 E8 ?? ?? ?? ?? C3");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_client_dynamic_cast, addr);
		}

		// RTTI_IClientRenderable  // Checked 2019-08-28
		//
		{
			DWORD addr = 0;

			ImageSectionsReader imageSectionReader((HMODULE)clientDll);

			if (!imageSectionReader.Eof())
			{

				imageSectionReader.Next();

				MemRange data2Range = imageSectionReader.GetMemRange();

				if (!imageSectionReader.Eof())
				{

					imageSectionReader.Next();

					MemRange data3Range = imageSectionReader.GetMemRange();

					MemRange rangeName = FindCString(data3Range, ".?AVIClientRenderable@@");

					if (!rangeName.IsEmpty())
					{
						MemRange rangeRttiTypeDescriptor = data3Range.And(MemRange::FromSize((DWORD)(rangeName.Start - 0x8), (DWORD)sizeof(DWORD)));

						if (!rangeRttiTypeDescriptor.IsEmpty())
						{
							addr = rangeRttiTypeDescriptor.Start;
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_client_RTTI_IClientRenderable, addr);
		}
		*/

		// When this changes mirv_fix selectedPlayerGlow needs an update as well!
		// This is above spec_glow_silent_factor reference, where it makes the selected player glow
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "75 05 38 45 FF 74 16 C7 06 00 00 80 3F C7 46 04 00 00 80 3F C7 46 08 00 00 80 3F EB ??");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_GlowCurrentPlayer_JMPS, addr);
		}

		/*
		// csgo_C_BaseEntity_ofs_m_nModelIndex
		//
		{
			// this basically uses the RecvProp* functions in c_baseplayer.cpp to get the offsets of the fields.

			DWORD strAddr_m_nModelIndex = 0;
			DWORD ofs_m_nModelIndex = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_nModelIndex");
							if (!result.IsEmpty())
							{
								strAddr_m_nModelIndex = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr_m_nModelIndex)
			{
				{
					ImageSectionsReader sections((HMODULE)clientDll);

					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_nModelIndex, sizeof(strAddr_m_nModelIndex));
					if (!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 4 + 2 + 4;
						addr = *(DWORD *)addr;
						ofs_m_nModelIndex = addr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}

			AFXADDR_SET(csgo_C_BaseEntity_ofs_m_nModelIndex, ofs_m_nModelIndex);
		}

		// csgo_C_BaseEntity_ofs_m_iWorldModelIndex
		//
		{
			// this basically uses the RecvProp* functions in c_baseplayer.cpp to get the offsets of the fields.

			DWORD strAddr_m_iWorldModelIndex = 0;
			DWORD ofs_m_iWorldModelIndex = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_iWorldModelIndex");
							if (!result.IsEmpty())
							{
								strAddr_m_iWorldModelIndex = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr_m_iWorldModelIndex)
			{
				{
					ImageSectionsReader sections((HMODULE)clientDll);

					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const*)&strAddr_m_iWorldModelIndex, sizeof(strAddr_m_iWorldModelIndex));
					if (!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 4 + 2 + 4;
						addr = *(DWORD*)addr;
						ofs_m_iWorldModelIndex = addr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}

			AFXADDR_SET(csgo_C_BaseEntity_ofs_m_iWorldModelIndex, ofs_m_iWorldModelIndex);
		}
		*/

		// csgo_C_CSPlayer_UpdateClientSideAnimation // Checked 2020-08-22.
		// csgo_C_CSPlayer_EyeAngles // Checked 2020-09-21.
		{
			AFXADDR_SET(csgo_C_CSPlayer_vtable, 0x0);
			AFXADDR_SET(csgo_C_BasePlayer_GetToolRecordingState, 0x0);
			AFXADDR_SET(csgo_C_CSPlayer_UpdateClientSideAnimation, 0x0);
			DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVC_CSPlayer@@", 0, 0x0);
			if (tmpAddr) {
				AFXADDR_SET(csgo_C_CSPlayer_vtable, tmpAddr);
				AFXADDR_SET(csgo_C_BasePlayer_GetToolRecordingState, ((DWORD*)tmpAddr)[103]);
				AFXADDR_SET(csgo_C_CSPlayer_UpdateClientSideAnimation, ((DWORD*)tmpAddr)[224]);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_C_CSPlayer_ofs_m_angEyeAngles
		{
			DWORD strAddr = 0;
			DWORD ofs = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_angEyeAngles");
							if (!result.IsEmpty())
							{
								strAddr = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr)
			{
				{
					ImageSectionsReader sections((HMODULE)clientDll);

					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
					if (!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 0x0a;
						addr = *(DWORD *)addr;
						ofs = addr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}

			AFXADDR_SET(csgo_C_CSPlayer_ofs_m_angEyeAngles, ofs);
		}

		// csgo_crosshair_localplayer_check // Last checked: 2022-10-22
		// Hook needs updating as well!
		//
		// (Right after the first XOR of cvar cl_show_observer_crosshair value.)
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "8B 01 FF 50 34 8B D0 85 D2 74 82 ?? ?? ?? ?? ?? ?? ?? ?? 8B 3D ?? ?? ?? ?? 3B F7 0F 84 ?? ?? ?? ??");

				if (!result.IsEmpty())
					AFXADDR_SET(csgo_crosshair_localplayer_check, result.Start + 27);				
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

		}
		
		// mirv_pov related
		//
		AFXADDR_SET(csgo_client_C_CSPlayer_EyeAngles_vtable_index, 170);
		AFXADDR_SET(csgo_client_C_CS_Player_GetFOV_vtable_index, 332);
		AFXADDR_SET(csgo_client_C_CSPlayer_UpdateOnRemove_vtable_index, 127);
		AFXADDR_SET(csgo_client_CPlayerResource_GetPing_vtable_index, 10);

		// mirv_pov related
		//
		// Called by function that references string "CCSGO_HudDamageIndicator".
		// 
		// If this changes MYcsgo_DamageIndicator_MessageFunc asm might need adjustments.
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F8 81 EC 94 00 00 00 80 3D ?? ?? ?? ?? 00 53 56 57 8B D9");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_DamageIndicator_MessageFunc, addr);
		}

		// csgo_C_BasePlayer_SetAsLocalPlayer // Last checked: 2022-09-22
		// Also check: csgo_C_BasePlayer_ofs_m_bIsLocalPlayer!
		// References "snd_soundmixer" and is called by function that references "Setting fallback player %s as local player\n" right before the call.
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "C6 81 34 36 00 00 01 C7 81 2C 36 00 00 00 00 00 00 89 0D ?? ?? ?? ?? C7 81 30 36 00 00 FF FF FF FF B9 ?? ?? ?? ?? E8 ?? ?? ?? ??");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_C_BasePlayer_SetAsLocalPlayer, addr);
		}

		// csgo_C_BasePlayer_ofs_m_bIsLocalPlayer // Last checked: 2022-09-22
		// Set to true in csgo_C_BasePlayer_SetAsLocalPlayer.
		AFXADDR_SET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer, 0x3634);

		{
			AFXADDR_SET(csgo_C_BaseViewModel_ofs_m_nAnimationParity, -1);
			AFXADDR_SET(csgo_C_BaseViewModel_ofs_m_nOldAnimationParity, -1);

			DWORD strAddr = 0;
			DWORD ofs = -1;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "m_nAnimationParity");
							if (!result.IsEmpty())
							{
								strAddr = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const*)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					DWORD addr = result.Start;
					addr += 0x0a;
					addr = *(DWORD*)addr;
					AFXADDR_SET(csgo_C_BaseViewModel_ofs_m_nAnimationParity, addr);
					AFXADDR_SET(csgo_C_BaseViewModel_ofs_m_nOldAnimationParity, addr+ 0x1c);
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
		}

		{
			AFXADDR_SET(csgo_C_BaseEntity_ShouldInterpolate, 0x0);
			DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVC_BaseViewModel@@", 0, 0x0);
			if (tmpAddr) {
				AFXADDR_SET(csgo_C_BaseEntity_ShouldInterpolate, ((DWORD*)tmpAddr)[179]);
				AFXADDR_SET(csgo_C_BaseViewModel_FireEvent, ((DWORD*)tmpAddr)[200]);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		{
			AFXADDR_SET(csgo_C_CS_PlayerResource_IGameResources_vtable,FindClassVtable((HMODULE)clientDll, ".?AVC_CS_PlayerResource@@", 0, 0x9D8));
			if(!AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		{
			AFXADDR_SET(csgo_C_Team_vtable,FindClassVtable((HMODULE)clientDll, ".?AVC_Team@@", 0, 0x0));
			if(!AFXADDR_GET(csgo_C_Team_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ // Last Checked: 2021-09-22
		//
		// This is for the the JNZ C_TEPlayerAnimEvent::PostDataUpdate if not new model animations.
		// The compiler changed class layout a bit, IClientNetworkable is at 0x4 instead of 0x8 (IClientRenderable missing).
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "8B 57 10 38 86 14 9B 00 00 75 14 83 FA 07 74 0F 8B 8E 5C 99 00 00 FF 77 14 52 8B 01 FF 50 18");

				if (!result.IsEmpty())
					addr = result.Start + 9;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ, addr);
		}

		// csgo_client_CanSeeSpectatorOnlyTools
		//
		{
			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();
				sections.Next(); // skip .text
				if (!sections.Eof())
				{
					MemRange firstDataRange = sections.GetMemRange();

					MemRange result = FindCString(sections.GetMemRange(), "Enable to force the server to show 5v5 scoreboards and allows spectators to see characters through walls.");
					if (!result.IsEmpty())
					{
						DWORD tmpAddr = result.Start;

						result = FindBytes(textRange, (char const *)&tmpAddr, sizeof(tmpAddr));
						if (!result.IsEmpty())
						{
							result = FindPatternString(MemRange(result.Start - 0x08, result.Start - 0x08 + 7).And(textRange), "B9 ?? ?? ?? ?? 6A 00");
							if (!result.IsEmpty())
							{
								DWORD csgo_client_sv_competitive_official_5v5_cvar = *(DWORD *)(result.Start + 1);
								{
									const char * fmt = "E8 ?? ?? ?? ?? 84 C0 74 31 A1 %02x %02x %02x %02x B9 %02x %02x %02x %02x FF 50 34";
									int sz = snprintf(nullptr, 0, fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);
									std::vector<char> buf(sz + 1);
									snprintf(&buf[0], buf.size(), fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);

									result = FindPatternString(MemRange(result.End,textRange.End), &buf[0]);
									if (!result.IsEmpty()) {
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS, result.Start);
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_1_LEN, (sz+1) / 3);
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));
								}
								if(!result.IsEmpty())
								{
									const char * fmt = "E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? A1 %02x %02x %02x %02x B9 %02x %02x %02x %02x 8B 40 34 FF D0";
									int sz = snprintf(nullptr, 0, fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);
									std::vector<char> buf(sz + 1);
									snprintf(&buf[0], buf.size(), fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);

									result = FindPatternString(MemRange(result.End,textRange.End), &buf[0]);
									if (!result.IsEmpty()) {
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS, result.Start);
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_2_LEN, (sz+1) / 3);
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));
								}
								if(!result.IsEmpty())
								{
									const char * fmt = "E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? A1 %02x %02x %02x %02x B9 %02x %02x %02x %02x 8B 40 34 FF D0";
									int sz = snprintf(nullptr, 0, fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);
									std::vector<char> buf(sz + 1);
									snprintf(&buf[0], buf.size(), fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);

									result = FindPatternString(MemRange(result.End,textRange.End), &buf[0]);
									if (!result.IsEmpty()) {
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS, result.Start);
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_3_LEN, (sz+1) / 3);
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));
								}
								if(!result.IsEmpty())
								{
									const char * fmt = "E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? A1 %02x %02x %02x %02x B9 %02x %02x %02x %02x 8B 40 34 FF D0";
									int sz = snprintf(nullptr, 0, fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);
									std::vector<char> buf(sz + 1);
									snprintf(&buf[0], buf.size(), fmt,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 0) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 8) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 16) &0xff,
										(csgo_client_sv_competitive_official_5v5_cvar >> 24) &0xff
									);

									result = FindPatternString(MemRange(result.End,textRange.End), &buf[0]);
									if (!result.IsEmpty()) {	
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS, result.Start);
										AFXADDR_SET(csgo_client_CanSeeSpectatorOnlyTools_4_LEN, (sz+1) / 3);
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));
								}
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}		

		// csgo_client_CCSGO_MapOverview__CanShowOverview
		// References "#Hint_Survival_MapOverview"
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F8 8B 4D 04 83 EC 28 56 57 e8 ?? ?? ?? ?? 8B 35 ?? ?? ?? ?? 85 F6 74 38 8B 06 8B CE FF 90 98 04 00 00");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_client_CCSGO_MapOverview_CanShowOverview, addr);
		}

		// csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr
		// References "armrace-scoreboard"
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F8 83 EC 08 56 8B F1 8B 0D ?? ?? ?? ?? 57 8B 01 8B 80 48 01 00 00 FF D0 84 C0 74 0D 80 3D ?? ?? ?? ?? 00 0F 84 AE 01 00 00");

				if (!result.IsEmpty())
					addr = result.Start + 42;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr, addr);
		}	

		{
			DWORD addr = 0;
			DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVCHLClient@@", 0, 0x0);
			if (tmpAddr) {
				tmpAddr = ((DWORD *)tmpAddr)[37]; // ::FrameStageNotify
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					MemRange textRange = sections.GetMemRange();
					MemRange result = FindPatternString(MemRange(tmpAddr+0x13, tmpAddr+0x13+5), "A2 ?? ?? ?? ??");
					if(!result.IsEmpty()) {
						AFXADDR_SET(csgo_client_g_bEngineIsHLTV, *(DWORD *)(result.Start+1));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		{
			DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVCCSGO_MapOverview@@", 0, 0x14);
			if (tmpAddr) {
				AFXADDR_SET(csgo_client_CCSGO_MapOverview_FireGameEvent, ((DWORD *)tmpAddr)[1]);
				AFXADDR_SET(csgo_client_CCSGO_MapOverview_ProcessInput, ((DWORD *)tmpAddr)[9]);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));	
		}

		// csgo_client_AdjustInterpolationAmount // Last checked: 2022-10-22
		// mirv_pov related
		// My_csgo_client_AdjustInterpolationAmount asm needs update if this changes
		//
		// This static function is the 2nd function taht references the "cl_interp_npcs" cvar.
		{
			DWORD addr = 0;

			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 EC 08 8B 15 ?? ?? ?? ?? F3 0F 11 4D F8 56 8B F1 81 FA ?? ?? ?? ?? 75 71 F3 0F 10 05 ?? ?? ?? ?? F3 0F 10 15 ?? ?? ?? ?? 0F 2E C2 9F");

				if (!result.IsEmpty())
					AFXADDR_SET(csgo_client_AdjustInterpolationAmount, result.Start);
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_C_BaseEntity_ofs_m_bPredictable // Last checked: 2022-10-24
		// referenced by Second function that uses cl_interp_all cvar (meaning CheckInitPredictable).
		//AFXADDR_SET(csgo_C_BaseEntity_ofs_m_bPredictable, 0x2ee);

		// Deathnotice releated:
		//
		AFXADDR_SET(csgo_client_C_Team_Get_ClanName_vtable_index, 188);
		AFXADDR_SET(csgo_client_C_Team_Get_FlagImageString_vtable_index, 189);
		AFXADDR_SET(csgo_client_C_Team_Get_LogoImageString_vtable_index, 190);
		AFXADDR_SET(csgo_client_CPlayerResource_GetPlayerName_vtable_index, 8);

		// csgo_client_CModelRenderSystem_SetupBones
		//
		// This is referenced before
		//   cmp     eax, 2 
		// in two functions that reference "Fast Path Model Rendering" string.
		{
			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F0 B8 38 30 00 00 E8 ?? ?? ?? ?? 83 7D 08 00");

				if (!result.IsEmpty())
					AFXADDR_SET(csgo_client_CModelRenderSystem_SetupBones, result.Start);
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// csgo_client_s_HLTVCamera // 2022-09-17.
		{
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)clientDll);
				if (!sections.Eof())
				{
					sections.Next(); // skip .text
					if (!sections.Eof())
					{
						{
							MemRange result = FindCString(sections.GetMemRange(), "items_gifted");
							if (!result.IsEmpty())
							{
								strAddr = result.Start;
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}

			if (strAddr)
			{
				ImageSectionsReader sections((HMODULE)clientDll);

				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const*)&strAddr, sizeof(strAddr));
				if (!result.IsEmpty())
				{
					result = FindPatternString(MemRange(result.Start + 0x0a, result.Start + 0x0a +5), "B9 ?? ?? ?? ??");
					if(!result.IsEmpty()) {
						AFXADDR_SET(csgo_client_s_HLTVCamera, *(DWORD*)((DWORD)result.Start + 0x1));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
		}

		// C_HLTVCamera::SpecCameraGotoPos // 2022
		// Second ref to "spec_target_updated".
		{
			ImageSectionsReader sections((HMODULE)clientDll);
			if (!sections.Eof())
			{
				MemRange textRange = sections.GetMemRange();

				MemRange result = FindPatternString(textRange, "55 8B EC 83 E4 F8 8B 45 20 81 EC 70 01 00 00 56 8B F1 57 C7 46 10 00 00 00 00 85 C0");

				if (!result.IsEmpty())
					AFXADDR_SET(csgo_client_CHLTVCamera_SpecCameraGotoPos, result.Start);
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}		
	}
	else
	{
		//AFXADDR_SET(csgo_CPredictionCopy_TransferData, 0x0);
		//AFXADDR_SET(csgo_C_BaseEntity_IClientEntity_vtable, 0x0);
		//AFXADDR_SET(csgo_C_BaseAnimating_IClientEntity_vtable, 0x0);
		AFXADDR_SET(csgo_C_BaseAnimating_vtable, 0x0);
		//AFXADDR_SET(csgo_C_BaseCombatWeapon_IClientEntity_vtable, 0x0);
		AFXADDR_SET(csgo_DT_Animationlayer_m_flCycle_fn, 0x0);
		//AFXADDR_SET(csgo_DT_Animationlayer_m_flPrevCycle_fn, 0x0);
		AFXADDR_SET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel, -1);
		AFXADDR_SET(csgo_C_BaseCombatWeapon_m_iState, -1);
		//AFXADDR_SET(csgo_C_BaseEntity_ToolRecordEnties, 0x0);
		AFXADDR_SET(csgo_C_BasePlayer_OFS_m_skybox3d_scale, (AfxAddr)-1);
		AFXADDR_SET(csgo_C_BasePlayer_RecvProxy_ObserverTarget, 0x0);
		AFXADDR_SET(csgo_CGlowOverlay_Draw, 0x0);
		AFXADDR_SET(csgo_CCSGO_HudDeathNotice_FireGameEvent, 0x0);
		AFXADDR_SET(csgo_CCSGameMovement_vtable, 0x0);
		AFXADDR_SET(csgo_CSkyboxView_Draw, 0x0);
		//AFXADDR_SET(csgo_CViewRender_Render, 0x0);
		AFXADDR_SET(csgo_CViewRender_RenderView_VGui_DrawHud_In, 0x0);
		AFXADDR_SET(csgo_CViewRender_RenderView_VGui_DrawHud_Out, 0x0);
		AFXADDR_SET(csgo_view, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_vtable, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw, 0x0);
		//AFXADDR_SET(csgo_mystique_animation, 0x0);
		AFXADDR_SET(csgo_Unknown_GetTeamsSwappedOnScreen, 0x0);
		AFXADDR_SET(csgo_C_CSPlayer_IClientNetworkable_entindex, 0x0);
		AFXADDR_SET(csgo_CRendering3dView_DrawTranslucentRenderables, 0x0);
		//AFXADDR_SET(csgo_client_dynamic_cast, 0x0);
		//AFXADDR_SET(csgo_client_RTTI_IClientRenderable, 0x0);
		AFXADDR_SET(csgo_GlowCurrentPlayer_JMPS, 0x0);
		//AFXADDR_SET(csgo_C_BaseEntity_ofs_m_nModelIndex, -1);
		//AFXADDR_SET(csgo_C_BaseEntity_ofs_m_iWorldModelIndex, -1);
		AFXADDR_SET(csgo_C_CSPlayer_vtable, 0x0);
		AFXADDR_SET(csgo_C_CSPlayer_UpdateClientSideAnimation, 0x0);
		AFXADDR_SET(csgo_DamageIndicator_MessageFunc, 0x0);
		AFXADDR_SET(csgo_C_BasePlayer_SetAsLocalPlayer, 0x0);
		AFXADDR_SET(csgo_C_BasePlayer_GetToolRecordingState, 0x0);
		AFXADDR_SET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer, 0x0);
		AFXADDR_SET(csgo_C_BaseViewModel_ofs_m_nAnimationParity, -1);
		AFXADDR_SET(csgo_C_BaseViewModel_ofs_m_nOldAnimationParity, -1);
		AFXADDR_SET(csgo_C_BaseEntity_ShouldInterpolate, 0x0);
		AFXADDR_SET(csgo_C_CS_PlayerResource_IGameResources_vtable, 0x0);
		AFXADDR_SET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ, 0x0);
		AFXADDR_SET(csgo_C_BaseViewModel_FireEvent, 0x0);
		//AFXADDR_SET(csgo_C_BaseEntity_ofs_m_bPredictable, 0x0);
	}

	//AFXADDR_SET(csgo_CPredictionCopy_TransferData_DSZ, 0x0a);
	//AFXADDR_SET(csgo_C_BaseEntity_ToolRecordEnties_DSZ, 0xd);
	AFXADDR_SET(csgo_CGlowOverlay_Draw_DSZ, 0xc);
	AFXADDR_SET(csgo_CSkyboxView_Draw_DSZ, 0x0a);
	AFXADDR_SET(csgo_gpGlobals_OFS_curtime, 4*4);
	AFXADDR_SET(csgo_gpGlobals_OFS_interpolation_amount, 9*4);
	AFXADDR_SET(csgo_gpGlobals_OFS_interval_per_tick, 8*4);
	//AFXADDR_SET(csgo_CViewRender_Render_DSZ, 0x0c);
	AFXADDR_SET(cstrike_gpGlobals_OFS_curtime, 3*4);
	AFXADDR_SET(cstrike_gpGlobals_OFS_absoluteframetime, 2*4);
	AFXADDR_SET(cstrike_gpGlobals_OFS_interpolation_amount, 8*4);
	AFXADDR_SET(cstrike_gpGlobals_OFS_interval_per_tick, 7*4);

	// C_BaseAnimating_RecordBones
	{
		ImageSectionsReader sections((HMODULE)clientDll);
		if (!sections.Eof())
		{
			MemRange textRange = sections.GetMemRange();
			sections.Next(); // skip .text
			if (!sections.Eof())
			{
				{
					MemRange result = FindCString(sections.GetMemRange(), "C_BaseAnimating::RecordBones");
					if (!result.IsEmpty())
					{
						DWORD strAddr = result.Start;
						result = FindBytes(textRange, (char const *)&strAddr, sizeof(strAddr));
						if(!result.IsEmpty()) {
							DWORD refStrAddr = result.Start;

							switch(sourceSdkVer) {
							case SourceSdkVer_CSGO:
								{
									MemRange result = FindPatternString(textRange.And(MemRange(refStrAddr - 0x4e, refStrAddr -0x4e + 3)), "55 8B EC");
									if(!result.IsEmpty())
										AFXADDR_SET(csgo_client_C_BaseAnimating_RecordBones, result.Start);
									else ErrorBox(MkErrStr(__FILE__, __LINE__));	
								}
								break;
							case SourceSdkVer_CSS:
								{
									MemRange result = FindPatternString(textRange.And(MemRange(refStrAddr - 0x3a, refStrAddr -0x3a + 3)), "55 8B EC");
									if(!result.IsEmpty())
										AFXADDR_SET(css_client_C_BaseAnimating_RecordBones, result.Start);
									else ErrorBox(MkErrStr(__FILE__, __LINE__));	
								}
								break;
							case SourceSdkVer_CSSV34:
								{
									MemRange result = FindPatternString(textRange.And(MemRange(refStrAddr - 0x1e, refStrAddr -0x1e + 6)), "81 EC 9C 00 00 00");
									if(!result.IsEmpty()) {
										AFXADDR_SET(cssv34_client_C_BaseAnimating_RecordBones, result.Start);
										AFXADDR_SET(cssv34_client_C_BaseAnimating_m_BoneAccessor_m_pBones, 0x4A8);
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));	
								}
								break;
							case SourceSdkVer_TF2:
								{
									MemRange result = FindPatternString(textRange.And(MemRange(refStrAddr - 0x3b, refStrAddr -0x3b + 3)), "55 8B EC");
									if(!result.IsEmpty())
										AFXADDR_SET(tf2_client_C_BaseAnimating_RecordBones, result.Start);
									else ErrorBox(MkErrStr(__FILE__, __LINE__));	
								}
								break;
							}	
						}					
						else ErrorBox(MkErrStr(__FILE__, __LINE__));	
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));	
	}
}

/*
void Addresses_InitStdshader_dx9Dll(AfxAddr stdshader_dx9Dll, bool isCsgo)
{
	if(isCsgo)
	{
		// csgo_SplineRope_CShader_vtable:
		{
			DWORD addr = 0;
			{
				ImageSectionsReader sections((HMODULE)stdshader_dx9Dll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						sections.Next(); // skip first .data
						if(!sections.Eof())
						{
							MemRange result = FindCString(sections.GetMemRange(), ".?AVCShader@SplineRope@@");
							if(!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x8;

								result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
								if(!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start;
									tmpAddr -= 0xC;

									result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
									if(!result.IsEmpty())
									{
										DWORD tmpAddr = result.Start;
										tmpAddr += (1)*4;

										addr = tmpAddr;
									}
									else ErrorBox(MkErrStr(__FILE__,__LINE__));
								}
								else ErrorBox(MkErrStr(__FILE__,__LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__,__LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_SplineRope_CShader_vtable, addr);
		}

		// csgo_Spritecard_CShader_vtable:
		{
			DWORD addr = 0;
			{
				ImageSectionsReader sections((HMODULE)stdshader_dx9Dll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						sections.Next(); // skip first .data
						if(!sections.Eof())
						{
							MemRange result = FindCString(sections.GetMemRange(), ".?AVCShader@Spritecard@@");
							if(!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x8;

								result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
								if(!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start;
									tmpAddr -= 0xC;

									result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
									if(!result.IsEmpty())
									{
										DWORD tmpAddr = result.Start;
										tmpAddr += (1)*4;

										addr = tmpAddr;
									}
									else ErrorBox(MkErrStr(__FILE__,__LINE__));
								}
								else ErrorBox(MkErrStr(__FILE__,__LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__,__LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_Spritecard_CShader_vtable, addr);
		}

		// csgo_UnlitGeneric_CShader_vtable:
		{
			DWORD addr = 0;
			{
				ImageSectionsReader sections((HMODULE)stdshader_dx9Dll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						sections.Next(); // skip first .data
						if(!sections.Eof())
						{
							MemRange result = FindCString(sections.GetMemRange(), ".?AVCShader@UnlitGeneric@@");
							if(!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x8;

								result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
								if(!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start;
									tmpAddr -= 0xC;

									result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
									if(!result.IsEmpty())
									{
										DWORD tmpAddr = result.Start;
										tmpAddr += (1)*4;

										addr = tmpAddr;
									}
									else ErrorBox(MkErrStr(__FILE__,__LINE__));
								}
								else ErrorBox(MkErrStr(__FILE__,__LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__,__LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_UnlitGeneric_CShader_vtable, addr);
		}

		// csgo_VertexLitGeneric_CShader_vtable:
		{
			DWORD addr = 0;
			{
				ImageSectionsReader sections((HMODULE)stdshader_dx9Dll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange firstDataRange = sections.GetMemRange();

						sections.Next(); // skip first .data
						if(!sections.Eof())
						{
							MemRange result = FindCString(sections.GetMemRange(), ".?AVCShader@VertexLitGeneric@@");
							if(!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;
								tmpAddr -= 0x8;

								result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
								if(!result.IsEmpty())
								{
									result = FindBytes(MemRange(result.End,firstDataRange.End), (char const *)&tmpAddr, sizeof(tmpAddr));
									if(!result.IsEmpty())
									{
										DWORD tmpAddr = result.Start;
										tmpAddr -= 0xC;

										result = FindBytes(firstDataRange, (char const *)&tmpAddr, sizeof(tmpAddr));
										if(!result.IsEmpty())
										{
											DWORD tmpAddr = result.Start;
											tmpAddr += (1)*4;

											addr = tmpAddr;
										}
										else ErrorBox(MkErrStr(__FILE__,__LINE__));
									}
									else ErrorBox(MkErrStr(__FILE__,__LINE__));
								}
								else ErrorBox(MkErrStr(__FILE__,__LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__,__LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_VertexLitGeneric_CShader_vtable, addr);
		}
	}
	else
	{
		AFXADDR_SET(csgo_Spritecard_CShader_vtable, 0x0);
		AFXADDR_SET(csgo_UnlitGeneric_CShader_vtable, 0x0);
		AFXADDR_SET(csgo_VertexLitGeneric_CShader_vtable, 0x0);
	}
}
*/

void Addresses_InitMaterialsystemDll(AfxAddr materialsystemDll, SourceSdkVer sourceSdkVer) {

	MemRange textRange;
	MemRange firstDataRange;
	ImageSectionsReader sections((HMODULE)materialsystemDll);
	if(!sections.Eof()) {
	 	textRange = sections.GetMemRange();
		sections.Next();
		if(!sections.Eof()) {
			firstDataRange = sections.GetMemRange();
		}
	}

	if (sourceSdkVer == SourceSdkVer_CSGO) {

		AFXADDR_SET(csgo_materialsystem_Material_InterlockedDecrement_vtable_index, 13);
	}

	// Queued rendering related hooks.
	//
	// Dependencies:
	// - materialsystem_CMaterialSystem_SwapBuffers address.
	// - mirv_campath draw (in future, not yet).
	// - mirv_streams record screen.
	//
	int materialsystem_GetRenderCallQueue_vtable_offset = -1;
	{
		switch(sourceSdkVer) {
		case SourceSdkVer_SWARM:
			// Checked 2023-09-16.
			materialsystem_GetRenderCallQueue_vtable_offset = 141;
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 3);
			break;
		case SourceSdkVer_CSGO:
			// Checked 2023-09-16.
			materialsystem_GetRenderCallQueue_vtable_offset = 179; // 3rd last
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 4);
			break;
		case SourceSdkVer_TF2:
			// Checked 2023-09-16.
			materialsystem_GetRenderCallQueue_vtable_offset = 147;
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 4);
			break;
		case SourceSdkVer_CSS:
			// Checked 2023-09-16.
			materialsystem_GetRenderCallQueue_vtable_offset = 143;
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 4);
			break;
		case SourceSdkVer_CSSV34:
			// Checked 2023-09-16.
			// this game is non-queued.
			materialsystem_GetRenderCallQueue_vtable_offset = -1;
			break;
		case SourceSdkVer_Insurgency2:
			// Checked 2023-09-16.
			materialsystem_GetRenderCallQueue_vtable_offset = 171;
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 4);
			break;
		case SourceSdkVer_L4D2:
			// Checked 2023-12-30.
			materialsystem_GetRenderCallQueue_vtable_offset = 137; // 3rd last
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 4);
			break;
		case SourceSdkVer_HL2MP:
			materialsystem_GetRenderCallQueue_vtable_offset = 143; // 4th last
			AFXADDR_SET(materialsystem_CFunctor_vtable_size, 4);
			break;
		}

		if(0 <= materialsystem_GetRenderCallQueue_vtable_offset) {
			// materialsystem_GetRenderCallQueue
			{
				// CMaterialSystem inherhits from IMaterialSystemInternal inherits from IMaterialSystem, so
				// we guess that GetRenderCallQueue is often the 3rd last function.
				AfxAddr addr_CMaterialSystem_vtable = FindClassVtable((HMODULE)materialsystemDll, ".?AVCMaterialSystem@@", 0, 0x0);
				if (!addr_CMaterialSystem_vtable) ErrorBox(MkErrStr(__FILE__, __LINE__));
				else {
					AFXADDR_SET(materialsystem_GetRenderCallQueue, (size_t)*(void**)(addr_CMaterialSystem_vtable + sizeof(void*)*materialsystem_GetRenderCallQueue_vtable_offset));
				}
			}


			// materialsystem_CMatCallQueue_QueueFunctor // Checked 2023-09-16.
			//
			// This is called by CCallQueueExternal::QueueFunctorInternal (the only virtual function),
			// so we can get the address that way.
			{
				AfxAddr vtable_addr = FindClassVtable((HMODULE)materialsystemDll, ".?AVCCallQueueExternal@CMatQueuedRenderContext@@", 0, 0x0);
				if (vtable_addr)
				{
					AfxAddr tmpAddr = (size_t)*(void**)vtable_addr;
					/* We search the first call inside the function (ignoring interface calls to AddRef / Release):
		call    sub_10013A70			
					*/
					MemRange result = FindPatternString(MemRange(tmpAddr, tmpAddr + 0x40).And(textRange), "E8 ?? ?? ?? ??");
					if (!result.IsEmpty()) {
						AfxAddr tmpAddr2 = ((size_t)*(void**)(result.Start + 1)) + result.Start + 5;
						if(!MemRange::FromSize(tmpAddr2,sizeof(void*)).And(textRange).IsEmpty()) {
							AFXADDR_SET(materialsystem_CMatCallQueue_QueueFunctor, tmpAddr2);
						} else ErrorBox(MkErrStr(__FILE__, __LINE__));
					} else ErrorBox(MkErrStr(__FILE__, __LINE__));
				} else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}	
		}
	}

	// materialsystem_CMaterialSystem_SwapBuffers // Checked 2023-08-30
	//
	// Dependencies:
	// - mirv_streams record screen.
	//
	if(0 <= materialsystem_GetRenderCallQueue_vtable_offset || g_SourceSdkVer == SourceSdkVer_CSSV34) {
		MemRange result = FindCString(firstDataRange, "CMaterialSystem::SwapBuffers");
		if (!result.IsEmpty())
		{
			AfxAddr tmpAddr = result.Start;

			result = FindBytes(textRange, (char const*)&tmpAddr, sizeof(tmpAddr));
			if (!result.IsEmpty())
			{
				if( g_SourceSdkVer == SourceSdkVer_CSSV34 ) {
					/*
mov     ecx, ds:g_VProfCurrentProfile
push    4
push    0						
					*/
						result = FindPatternString(MemRange(result.Start - 0x12, result.Start).And(textRange), "8B 0D ?? ?? ?? ?? 6A 04  6A 00");
				} else {
					/*
	push    ebp
	mov     ebp, esp
					*/
					unsigned char pattern[3] = {0x55, 0x8B, 0xEC};
					// We have a bigger search range intentionally here, since we want to support several mods.
					result = FindBytesReverse(MemRange(result.Start - 0x50, result.Start).And(textRange), (char *)pattern, sizeof(pattern));
				}
				if (!result.IsEmpty()) {
						AFXADDR_SET(materialsystem_CMaterialSystem_SwapBuffers, result.Start);
				} else ErrorBox(MkErrStr(__FILE__, __LINE__));
			} else ErrorBox(MkErrStr(__FILE__, __LINE__));
		} else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}		
}
