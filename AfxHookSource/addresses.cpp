#include "stdafx.h"

#include "addresses.h"

#include <shared/binutils.h>

SourceSdkVer g_SourceSdkVer = SourceSdkVer_Unknonw;

using namespace Afx::BinUtils;

//AFXADDR_DEF(csgo_CPredictionCopy_TransferData)
//AFXADDR_DEF(csgo_CPredictionCopy_TransferData_DSZ)
AFXADDR_DEF(csgo_C_BaseAnimating_vtable)
AFXADDR_DEF(csgo_DT_Animationlayer_m_flCycle_fn)
//AFXADDR_DEF(csgo_DT_Animationlayer_m_flPrevCycle_fn)
//AFXADDR_DEF(csgo_mystique_animation)
AFXADDR_DEF(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel)
AFXADDR_DEF(csgo_C_BaseCombatWeapon_m_iState)
AFXADDR_DEF(csgo_C_BaseEntity_ToolRecordEnties)
AFXADDR_DEF(csgo_C_BaseEntity_ToolRecordEnties_DSZ)
//AFXADDR_DEF(csgo_C_BasePlayer_OFS_m_bDucked)
//AFXADDR_DEF(csgo_C_BasePlayer_OFS_m_bDucking)
//AFXADDR_DEF(csgo_C_BasePlayer_OFS_m_flDuckAmount)
AFXADDR_DEF(csgo_C_BasePlayer_OFS_m_skybox3d_scale)
AFXADDR_DEF(csgo_C_BasePlayer_RecvProxy_ObserverTarget)
AFXADDR_DEF(csgo_CCSViewRender_vtable)
AFXADDR_DEF(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha)
AFXADDR_DEF(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw)
AFXADDR_DEF(csgo_CCSViewRender_RenderSmokeOverlay_OnBeforeExitFunc)
AFXADDR_DEF(csgo_CGlowOverlay_Draw)
AFXADDR_DEF(csgo_CGlowOverlay_Draw_DSZ)
AFXADDR_DEF(csgo_CCSGO_HudDeathNotice_FireGameEvent)
AFXADDR_DEF(csgo_CUnknown_GetPlayerName)
AFXADDR_DEF(csgo_CUnknown_GetPlayerName_DSZ)
AFXADDR_DEF(csgo_CCSGameMovement_vtable)
AFXADDR_DEF(csgo_CSkyboxView_Draw)
AFXADDR_DEF(csgo_CSkyboxView_Draw_DSZ)
AFXADDR_DEF(csgo_CViewRender_RenderView_AfterVGui_DrawHud)
AFXADDR_DEF(csgo_CAudioXAudio2_vtable)
AFXADDR_DEF(csgo_MIX_PaintChannels)
AFXADDR_DEF(csgo_MIX_PaintChannels_DSZ)
AFXADDR_DEF(csgo_SplineRope_CShader_vtable)
AFXADDR_DEF(csgo_Spritecard_CShader_vtable)
AFXADDR_DEF(csgo_UnlitGeneric_CShader_vtable)
AFXADDR_DEF(csgo_Unknown_GetTeamsSwappedOnScreen)
AFXADDR_DEF(csgo_VertexLitGeneric_CShader_vtable)
AFXADDR_DEF(csgo_S_StartSound_StringConversion)
AFXADDR_DEF(csgo_gpGlobals_OFS_curtime)
AFXADDR_DEF(csgo_gpGlobals_OFS_interpolation_amount)
AFXADDR_DEF(csgo_gpGlobals_OFS_interval_per_tick)
AFXADDR_DEF(csgo_pLocalPlayer)
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
AFXADDR_DEF(csgo_C_CSPlayer_IClientNetworkable_entindex)
AFXADDR_DEF(csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall)

bool g_Adresses_ClientIsPanorama = true;

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
	if(SourceSdkVer_CSGO == sourceSdkVer)
	{
		// csgo_snd_mix_timescale_patch: // Checked 2017-07-08.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)engineDll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "Start profiling MIX_PaintChannels\n");
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
				ImageSectionsReader sections((HMODULE)engineDll);
			
				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if(!result.IsEmpty())
				{
					DWORD tempAddr = result.Start +0xB9;

					MemRange result = FindPatternString(MemRange(tempAddr-3, tempAddr-3+3), "51 52 E8");
					if (!result.IsEmpty())
					{
						tempAddr = tempAddr + 4 + *(DWORD *)tempAddr;
						// in MIX_PaintChannels now.

						tempAddr = tempAddr + 0x273;

						MemRange result = FindPatternString(MemRange(tempAddr - 7, tempAddr - 7 + 7), "8D 8D 50 FE FF FF E8");
						if (!result.IsEmpty())
						{
							tempAddr = tempAddr + 4 + *(DWORD *)tempAddr;
							// In SoundMixFunction2 now.

							tempAddr = tempAddr + 0xEA;

							MemRange result = FindPatternString(MemRange(tempAddr - 5, tempAddr - 5 + 5), "33 D2 8B CE E8");
							if (!result.IsEmpty())
							{
								tempAddr = tempAddr + 4 + *(DWORD *)tempAddr;
								// In SoundMixFunction now.

								tempAddr = tempAddr + 0x66;

								MemRange result = FindPatternString(MemRange(tempAddr - 2, tempAddr - 2 + 2), "FF D0");
								if (!result.IsEmpty())
								{
									// After call VEnglineClient013::GetTimeScale now.

									addr = tempAddr;
								}
								else ErrorBox(MkErrStr(__FILE__, __LINE__));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			if(addr)
			{
				AFXADDR_SET(csgo_snd_mix_timescale_patch, addr);
			}
			else
			{
				AFXADDR_SET(csgo_snd_mix_timescale_patch, 0x0);
			}
		}

		// csgo_S_StartSound_StringConversion: // Checked 2017-05-13.
		{
			DWORD addr = 0;
			DWORD strAddr = 0;
			{
				ImageSectionsReader sections((HMODULE)engineDll);
				if(!sections.Eof())
				{
					sections.Next(); // skip .text
					if(!sections.Eof())
					{
						MemRange result = FindCString(sections.GetMemRange(), "Starting sound '%s' while system disabled.\n");
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
				ImageSectionsReader sections((HMODULE)engineDll);
			
				MemRange baseRange = sections.GetMemRange();
				MemRange result = FindBytes(baseRange, (char const *)&strAddr, sizeof(strAddr));
				if(!result.IsEmpty())
				{
					addr = result.Start -0x1d;

					// check for pattern to see if it is the right address:
					unsigned char pattern[14] = { 0x8B, 0x01, 0x8D, 0x54, 0x24, 0x78, 0x68, 0x04, 0x01, 0x00, 0x00, 0x52, 0xFF, 0x10 };

					DWORD patternSize = sizeof(pattern)/sizeof(pattern[0]);
					MemRange patternRange(addr, addr+patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if(result.Start != patternRange.Start || result.End != patternRange.End)
					{
						addr = 0;
						ErrorBox(MkErrStr(__FILE__,__LINE__));
					}
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			if(addr)
			{
				AFXADDR_SET(csgo_S_StartSound_StringConversion, addr);
			}
			else
			{
				AFXADDR_SET(csgo_S_StartSound_StringConversion, 0x0);
			}
		}

		// csgo_CAudioXAudio2_vtable: // Checked 2018-08-03.
		AFXADDR_SET(csgo_CAudioXAudio2_vtable, FindClassVtable((HMODULE)engineDll, ".?AVCAudioXAudio2@@", 0, 0x0));
		if(!AFXADDR_GET(csgo_CAudioXAudio2_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

		// csgo_MIX_PaintChannels: // Checked 2017-07-18.
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

						MemRange result = FindCString(sections.GetMemRange(), "%d milliseconds \n");
						if (!result.IsEmpty())
						{
							DWORD tmpAddr = result.Start;

							result = FindBytes(textRange, (char const *)&tmpAddr, sizeof(tmpAddr));
							if (!result.IsEmpty())
							{
								result = FindBytes(MemRange(result.End, textRange.End), (char const *)&tmpAddr, sizeof(tmpAddr));
								if (!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start;
									tmpAddr -= 0xA;

									result = FindPatternString(MemRange(tmpAddr - 0x1, tmpAddr - 0x1 + 0x7), "E8 ?? ?? ?? ?? FF D6");
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
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			AFXADDR_SET(csgo_MIX_PaintChannels, addr);
		}

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

			// csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall: // Checked 2018-07-14.
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

							MemRange result = FindCString(sections.GetMemRange(), "ToggleDebugger");
							if (!result.IsEmpty())
							{
								DWORD tmpAddr = result.Start;

								result = FindBytes(textRange, (char const *)&tmpAddr, sizeof(tmpAddr));
								if (!result.IsEmpty())
								{
									DWORD tmpAddr = result.Start;
									tmpAddr += 0x26;

									result = FindPatternString(MemRange(tmpAddr, tmpAddr + 0x6), "FF 90 B8 00 00  00");
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
				AFXADDR_SET(csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall, addr);
			}
		}
	}
	else
	{
		AFXADDR_SET(csgo_snd_mix_timescale_patch, 0x0);
		AFXADDR_SET(csgo_S_StartSound_StringConversion, 0x0);
		AFXADDR_SET(csgo_CAudioXAudio2_vtable, 0x0);
		AFXADDR_SET(csgo_MIX_PaintChannels, 0x0);
		AFXADDR_SET(csgo_CClientState_ProcessVoiceData, 0x0);
		AFXADDR_SET(csgo_CVoiceWriter_AddDecompressedData, 0x0);
		AFXADDR_SET(csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall, 0x0);
	}
	AFXADDR_SET(csgo_snd_mix_timescale_patch_DSZ, 0x08);
	AFXADDR_SET(csgo_MIX_PaintChannels_DSZ, 0x9);
	AFXADDR_SET(csgo_CClientState_ProcessVoiceData_DSZ, 0x6);
	AFXADDR_SET(csgo_CVoiceWriter_AddDecompressedData_DSZ, 0x8);
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
				addr = ((DWORD *)tmpAddr)[13];
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_panorama_CZip_UnkLoadFiles, addr);
		}
	}
	else
	{
		AFXADDR_SET(csgo_panorama_AVCUIPanel_UnkSetFloatProp, 0);
		AFXADDR_SET(csgo_panorama_CZip_UnkLoadFiles, 0);
	}
}

void Addresses_InitClientDll(AfxAddr clientDll, SourceSdkVer sourceSdkVer, bool isPanorama)
{
	if(SourceSdkVer_CSGO == sourceSdkVer)
	{
		g_Adresses_ClientIsPanorama = isPanorama;

		if (isPanorama)
		{
			// csgo_CCSGO_HudDeathNotice_FireGameEvent // Checked 2018-08-03.
			{
				DWORD addr = 0;
				DWORD tmpAddr = FindClassVtable((HMODULE)clientDll, ".?AVCCSGO_HudDeathNotice@@", 0, 0x14);
				if (tmpAddr) {
					addr = ((DWORD *)tmpAddr)[1];
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));

				AFXADDR_SET(csgo_CCSGO_HudDeathNotice_FireGameEvent, addr);
			}
		}
		else {
			// csgo_CCSGO_HudDeathNotice_FireGameEvent: // Checked 2017-05-13.
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
							MemRange result = FindCString(sections.GetMemRange(), "realtime_passthrough");
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
						addr = result.Start + 0x0D;

						// check for pattern to see if it is the right address:
						unsigned char pattern[4] = { 0x56, 0x8B, 0xCF, 0xE8 };

						DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
						MemRange patternRange(addr, addr + patternSize);
						MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
						if (result.Start != patternRange.Start || result.End != patternRange.End)
						{
							addr = 0;
							ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else
						{
							addr += patternSize;
							addr = addr + 4 + *(DWORD *)addr; // get CALL address
						}
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				AFXADDR_SET(csgo_CCSGO_HudDeathNotice_FireGameEvent, addr);
			}

		}

		// csgo_CUnknown_GetPlayerName: // Checked 2018-07-10.
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
						MemRange result = FindCString(sections.GetMemRange(), "#SFUI_bot_controlled_by");
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
					addr = result.Start - 0x396;

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

		// csgo_CViewRender_RenderView_AfterVGui_DrawHud: // Checked 2017-05-13.
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
						MemRange result = FindCString(sections.GetMemRange(), "VGui_DrawHud");
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
					DWORD targetAddr = result.Start +0x0a;

					// check for pattern nearby to see if it is the right address:
					unsigned char pattern[5] = { 0x6a, 0x04, 0x6a, 0x00, 0x68};

					DWORD patternSize = sizeof(pattern)/sizeof(pattern[0]);
					MemRange patternRange(result.Start -0x0C, result.Start -0x0C +patternSize);
					MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
					if(result.Start != patternRange.Start || result.End != patternRange.End)
						ErrorBox(MkErrStr(__FILE__,__LINE__));
					else
						addr = targetAddr;
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_CViewRender_RenderView_AfterVGui_DrawHud, addr);
		}

		// csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha: // Checked 2017-05-13.
		// csgo_CCSViewRender_RenderSmokeOverlay_OnLoadAlphaBeforeDraw: // Checked 2017-05-13.
		// csgo_CCSViewRender_RenderSmokeOverlay_OnBeforeExitFunc: // Checked 2017-05-13.
		{
			DWORD addrOnLoadOldAlpha = 0;
			DWORD addrOnCompareAlphaBeforeDraw = 0;
			DWORD addrOnBeforeExitFunc = 0;
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
							DWORD tmpAddr = pushStringAddr - 0x79;

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
							unsigned char pattern[7] = { 0x0F, 0x2F, 0xAA, 0x88, 0x05, 0x00, 0x00 };


							DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
							MemRange patternRange(tmpAddr, tmpAddr + patternSize);
							MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
							if (result.Start != patternRange.Start || result.End != patternRange.End)
								ErrorBox(MkErrStr(__FILE__, __LINE__));
							else
								addrOnCompareAlphaBeforeDraw = tmpAddr;
						}

						{
							DWORD tmpAddr = pushStringAddr + 0x5E;

							// check for pattern nearby to see if it is the right address:
							unsigned char pattern[8] = { 0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0x00 };

							DWORD patternSize = sizeof(pattern) / sizeof(pattern[0]);
							MemRange patternRange(tmpAddr, tmpAddr + patternSize);
							MemRange result = FindBytes(patternRange, (char *)pattern, patternSize);
							if (result.Start != patternRange.Start || result.End != patternRange.End)
								ErrorBox(MkErrStr(__FILE__, __LINE__));
							else
								addrOnBeforeExitFunc = tmpAddr;
						}
					}
					else ErrorBox(MkErrStr(__FILE__,__LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha, addrOnLoadOldAlpha);
			AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw, addrOnCompareAlphaBeforeDraw);
			AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnBeforeExitFunc, addrOnBeforeExitFunc);
		}

		// csgo_pLocalPlayer: // Checked 2017-05-13.
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
						MemRange result = FindCString(sections.GetMemRange(), "time to initial render = %f\n");
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
					tmpAddr += 0xe;

					if (!FindPatternString(MemRange(tmpAddr-1, tmpAddr + 10 -1), "A1 ?? ?? ?? ?? C6 44 ?? ?? 00").IsEmpty())
					{
						addr = *(DWORD *)tmpAddr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__,__LINE__));
			}
			if(addr)
			{
				AFXADDR_SET(csgo_pLocalPlayer, addr);
			}
			else
			{
				AFXADDR_SET(csgo_pLocalPlayer, 0x0);
			}
		}

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
					/*
					if (strAddr_m_bDucked)
					{
						ImageSectionsReader sections((HMODULE)clientDll);

						MemRange baseRange = sections.GetMemRange();
						MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_bDucked, sizeof(strAddr_m_bDucked));
						if (!result.IsEmpty())
						{
							DWORD addr = result.Start;
							addr += 4 + 2 + 4;
							addr = *(DWORD *)addr;
							ofs_m_Local_m_bDucked = ofs_m_Local + addr;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					if (strAddr_m_bDucking)
					{
						ImageSectionsReader sections((HMODULE)clientDll);

						MemRange baseRange = sections.GetMemRange();
						MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_bDucking, sizeof(strAddr_m_bDucking));
						if (!result.IsEmpty())
						{
							DWORD addr = result.Start;
							addr += 4 + 2 + 4;
							addr = *(DWORD *)addr;
							ofs_m_Local_m_bDucking = ofs_m_Local + addr;
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					*/
				}

				/*
				if (strAddr_m_flDuckAmount)
				{
					ImageSectionsReader sections((HMODULE)clientDll);

					MemRange baseRange = sections.GetMemRange();
					MemRange result = FindBytes(baseRange, (char const *)&strAddr_m_flDuckAmount, sizeof(strAddr_m_flDuckAmount));
					if (!result.IsEmpty())
					{
						DWORD addr = result.Start;
						addr += 4 + 2 + 4;
						addr = *(DWORD *)addr;
						ofs_m_Local_m_flDuckAmount = addr;
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				*/
			}

			AFXADDR_SET(csgo_C_BasePlayer_OFS_m_skybox3d_scale, ofs_m_Local_m_skybox3d_scale);
			//AFXADDR_SET(csgo_C_BasePlayer_OFS_m_bDucked, ofs_m_Local_m_bDucked);
			//AFXADDR_SET(csgo_C_BasePlayer_OFS_m_bDucking, ofs_m_Local_m_bDucking);
			//AFXADDR_SET(csgo_C_BasePlayer_OFS_m_flDuckAmount, ofs_m_Local_m_flDuckAmount);
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

						result = FindPatternString(MemRange(tmpAddr, result.End), "68 ?? ?? ?? ?? 6A 00 6A 04 68 60 33 00 00");

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

		// csgo_C_BaseAnimating_vtable // Checked 2018-08-03.
		AFXADDR_SET(csgo_CCSGameMovement_vtable, FindClassVtable((HMODULE)clientDll, ".?AVC_BaseAnimating@@", 0, 0x10));
		if (!AFXADDR_GET(csgo_CCSGameMovement_vtable)) ErrorBox(MkErrStr(__FILE__, __LINE__));

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

				MemRange result = FindPatternString(textRange, "8B 0D ?? ?? ?? ?? 53 56 E8 ?? ?? ?? ?? 8B 15 ?? ?? ?? ??");

				if (!result.IsEmpty())
					addr = result.Start;
				else
					ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));

			AFXADDR_SET(csgo_Unknown_GetTeamsSwappedOnScreen, addr);
		}

	}
	else
	{
		//AFXADDR_SET(csgo_CPredictionCopy_TransferData, 0x0);
		AFXADDR_SET(csgo_C_BaseAnimating_vtable, 0x0);
		AFXADDR_SET(csgo_DT_Animationlayer_m_flCycle_fn, 0x0);
		//AFXADDR_SET(csgo_DT_Animationlayer_m_flPrevCycle_fn, 0x0);
		AFXADDR_SET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel, -1);
		AFXADDR_SET(csgo_C_BaseCombatWeapon_m_iState, -1);
		AFXADDR_SET(csgo_C_BaseEntity_ToolRecordEnties, 0x0);
		//AFXADDR_SET(csgo_C_BasePlayer_OFS_m_bDucked, (AfxAddr)-1);
		//AFXADDR_SET(csgo_C_BasePlayer_OFS_m_bDucking, (AfxAddr)-1);
		AFXADDR_SET(csgo_C_BasePlayer_OFS_m_skybox3d_scale, (AfxAddr)-1);
		//AFXADDR_SET(csgo_C_BasePlayer_OFS_m_flDuckAmount, (AfxAddr)-1);
		AFXADDR_SET(csgo_C_BasePlayer_RecvProxy_ObserverTarget, 0x0);
		AFXADDR_SET(csgo_CGlowOverlay_Draw, 0x0);
		AFXADDR_SET(csgo_CCSGO_HudDeathNotice_FireGameEvent, 0x0);
		AFXADDR_SET(csgo_CUnknown_GetPlayerName, 0x0);
		AFXADDR_SET(csgo_CCSGameMovement_vtable, 0x0);
		AFXADDR_SET(csgo_CSkyboxView_Draw, 0x0);
		//AFXADDR_SET(csgo_CViewRender_Render, 0x0);
		AFXADDR_SET(csgo_CViewRender_RenderView_AfterVGui_DrawHud, 0x0);
		AFXADDR_SET(csgo_pLocalPlayer, 0x0);
		AFXADDR_SET(csgo_view, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_vtable, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw, 0x0);
		AFXADDR_SET(csgo_CCSViewRender_RenderSmokeOverlay_OnBeforeExitFunc, 0x0);
		//AFXADDR_SET(csgo_mystique_animation, 0x0);
		AFXADDR_SET(csgo_Unknown_GetTeamsSwappedOnScreen, 0x0);
		AFXADDR_SET(csgo_C_CSPlayer_IClientNetworkable_entindex, 0x0);
	}

	//AFXADDR_SET(csgo_CPredictionCopy_TransferData_DSZ, 0x0a);
	AFXADDR_SET(csgo_C_BaseEntity_ToolRecordEnties_DSZ, 0xd);
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
	AFXADDR_SET(csgo_CUnknown_GetPlayerName_DSZ, 0x08);
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
