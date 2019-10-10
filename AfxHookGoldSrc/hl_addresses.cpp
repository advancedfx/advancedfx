#include "stdafx.h"

#include "hl_addresses.h"

#include <shared/binutils.h>

using namespace Afx::BinUtils;

AFXADDR_DEF(pEngfuncs)
AFXADDR_DEF(ppmove)
AFXADDR_DEF(pstudio)
AFXADDR_DEF(engine_ClientFunctionTable)
AFXADDR_DEF(CL_Disconnect)
AFXADDR_DEF(CL_EmitEntities)
AFXADDR_DEF(CL_ParseServerMessage_CmdRead)
AFXADDR_DEF(CL_ParseServerMessage_CmdRead_DSZ)
AFXADDR_DEF(p_cmd_functions)
AFXADDR_DEF(GetSoundtime)
AFXADDR_DEF(_Host_Frame)
AFXADDR_DEF(Host_Init)
AFXADDR_DEF(Mod_LeafPVS)
AFXADDR_DEF(R_DrawEntitiesOnList)
AFXADDR_DEF(R_DrawParticles)
AFXADDR_DEF(R_DrawSkyBoxEx)
AFXADDR_DEF(R_DrawViewModel)
AFXADDR_DEF(R_PolyBlend)
AFXADDR_DEF(R_PushDlights)
AFXADDR_DEF(R_RenderView)
AFXADDR_DEF(SND_PickChannel)
AFXADDR_DEF(S_PaintChannels)
AFXADDR_DEF(S_TransferPaintBuffer)
AFXADDR_DEF(UnkDrawHudIn)
AFXADDR_DEF(UnkDrawHudInCall)
AFXADDR_DEF(UnkDrawHudInContinue)
AFXADDR_DEF(UnkDrawHudOut)
AFXADDR_DEF(UnkDrawHudOutCall)
AFXADDR_DEF(UnkDrawHudOutContinue)
AFXADDR_DEF(Draw_DecalMaterial)
AFXADDR_DEF(clientDll)
AFXADDR_DEF(cstrike_CHudDeathNotice_Draw)
AFXADDR_DEF(cstrike_CHudDeathNotice_Draw_YRes)
AFXADDR_DEF(cstrike_CHudDeathNotice_Draw_YRes_DSZ)
AFXADDR_DEF(cstrike_CHudDeathNotice_MsgFunc_DeathMsg)
AFXADDR_DEF(cstrike_EV_CreateSmoke)
AFXADDR_DEF(cstrike_MsgFunc_DeathMsg)
AFXADDR_DEF(cstrike_UnkCrosshairFn)
AFXADDR_DEF(cstrike_UnkCrosshairFn_add_fac)
AFXADDR_DEF(cstrike_UnkCrosshairFn_mul_fac)
AFXADDR_DEF(cstrike_rgDeathNoticeList)
AFXADDR_DEF(g_fov)
AFXADDR_DEF(hlExe)
AFXADDR_DEF(hwDll)
AFXADDR_DEF(host_frametime)
AFXADDR_DEF(msg_readcount)
AFXADDR_DEF(net_message)
AFXADDR_DEF(paintbuffer)
AFXADDR_DEF(paintedtime)
AFXADDR_DEF(r_refdef)
AFXADDR_DEF(shm)
AFXADDR_DEF(skytextures)
AFXADDR_DEF(soundtime)
AFXADDR_DEF(tfc_CHudDeathNotice_Draw)
AFXADDR_DEF(tfc_CHudDeathNotice_Draw_YRes)
AFXADDR_DEF(tfc_CHudDeathNotice_Draw_YRes_DSZ)
AFXADDR_DEF(tfc_CHudDeathNotice_MsgFunc_DeathMsg)
AFXADDR_DEF(tfc_MsgFunc_DeathMsg)
AFXADDR_DEF(tfc_TeamFortressViewport_UpdateSpecatorPanel)
AFXADDR_DEF(tfc_rgDeathNoticeList)
AFXADDR_DEF(valve_TeamFortressViewport_UpdateSpecatorPanel)

//
// Documentation (in HLAE source code)
//
// [1] doc/notes_goldsrc/debug_cstrike_crosshair.txt
// [2] doc/notes_goldsrc/debug_cstrike_deathmessage.txt
// [3] doc/notes_goldsrc/debug_cstrike_smoke.txt
// [4] doc/notes_goldsrc/debug_tfc_UpdateSpectatorPanel.txt
// [5] doc/notes_goldsrc/debug_engine_ifaces.txt
// [6] doc/notes_goldsrc/debug_sound.txt
// [7] doc/notes_goldsrc/debug_SCR_UpdateScreen.txt
// [8] doc/notes_goldsrc/debug_Host_Frame.txt
// [9] doc/notes_goldsrc/debug_ClientFunctionTable
// [10] doc/notes_goldsrc/debug_CL_ParseServerMessage.txt
// [11] doc/notes_goldsrc/debug_R_DrawWorld_and_sky.txt
// [12] doc/notes_goldsrc/debug_R_DecalShoot.txt
// *[14] doc/notes_goldsrc/debug_tfc_deathmessage.txt
// *[15] doc/notes_goldsrc/debug_sv_variables.txt
// [16] doc/notes_goldsrc/debug_CL_Disconnect.txt
// [17] doc/notes_goldsrc/debug_fov.txt
// [18] doc/notes_goldsrc/debug_Host_Init.txt

void Addresses_InitHlExe(AfxAddr hlExe)
{
	AFXADDR_SET(hlExe, hlExe);
}

void ErrorBox(char const * messageText)
{
	MessageBox(0, messageText, "AfxHookGoldSrc Error", MB_OK | MB_ICONERROR);
}

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

void Addresses_InitHwDll(AfxAddr hwDll)
{
	AFXADDR_SET(hwDll, hwDll);

	MemRange textRange = MemRange(0, 0);
	MemRange data1Range = MemRange(0, 0);
	MemRange data2Range = MemRange(0, 0);
	{
		ImageSectionsReader imageSectionsReader((HMODULE)hwDll);
		if (!imageSectionsReader.Eof())
		{
			textRange = imageSectionsReader.GetMemRange();
			imageSectionsReader.Next();

			if (!imageSectionsReader.Eof())
			{
				data1Range = imageSectionsReader.GetMemRange();
				imageSectionsReader.Next();

				if (!imageSectionsReader.Eof())
				{
					data2Range = imageSectionsReader.GetMemRange();
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// pEngfuncs // [5] // Checked: 2018-10-06
	// ppmove // [5] // Checked: 2018-10-06
	// pstudio // [5] // Checked: 2018-10-06
	{
		MemRange s1 = FindCString(data2Range, "ScreenFade");

		if (!s1.IsEmpty()) {

			MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

			if (!r1.IsEmpty()) {

				MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start + 0x09, r1.Start + 0x09 + 23)), "6A 07 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ?? 68 ?? ?? ?? ?? E8 ?? ?? ?? ??");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(pEngfuncs, *(DWORD *)(r2.Start + 3));
					AFXADDR_SET(ppmove, *(DWORD *)(r2.Start + 14));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));

		s1 = MemRange(data2Range.Start, data2Range.Start);

		for (int i = 0; i < 2; ++i)
		{
			s1 = FindCString(MemRange(s1.End, data2Range.End), "HUD_GetStudioModelInterface");
		}

		if (!s1.IsEmpty()) {

			MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

			if (!r1.IsEmpty()) {

				MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start + 0x14, r1.Start + 0x14 + 14)), "68 ?? ?? ?? ?? 68 ?? ?? ?? ?? 6A 01 FF D0");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(pstudio, *(DWORD *)(r2.Start + 1));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// engine_ClientFunctionTable // [9] // Checked: 2018-09-01
	{
		MemRange tmp1 = FindCString(data2Range, "Initialize");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = textRange.And(MemRange(refTmp1.Start + 0x11, refTmp1.Start + 0x11 + 0x7));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "85 C0 A3 ?? ?? ?? ??").IsEmpty())
						{
							AFXADDR_SET(engine_ClientFunctionTable, *(DWORD *)(tmp2.Start + 0x3));
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

	//
	// General engine hooks:
	//

	// Host_Init [18] // Checked: 2018-09-01
	{
		MemRange tmp1 = FindCString(data2Range, "-toconsole");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x77, refTmp1.Start - 0x77 + 0x3));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "55 8B EC").IsEmpty())
						{
							AFXADDR_SET(Host_Init, tmp2.Start);
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

	// _Host_Frame [8] // Checked: 2018-09-01
	// host_frametime [8] // Checked: 2018-09-01
	// CL_EmitEntities // [8] // Checked: 2018-09-01
	{
		MemRange tmp1 = FindCString(data2Range, "host_killtime");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(data2Range, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				DWORD tmpAddr = refTmp1.Start + 0x0C;

				MemRange refTmp2 = FindBytes(textRange, (const char *)&tmpAddr, sizeof(tmpAddr));
				if (!refTmp2.IsEmpty())
				{

					MemRange tmp2 = textRange.And(MemRange(refTmp2.Start - 0x18D, refTmp2.Start - 0x18D + 0x3));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "55 8B EC").IsEmpty())
						{
							AFXADDR_SET(_Host_Frame, tmp2.Start);
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));

/*
.text:01D562B9     loc_1D562B9:                            ; CODE XREF: sub_1D56200+...
.text:01D562B9 004                 mov     edx, dword ptr dbl_27B4068+4
.text:01D562BF 004                 mov     eax, dword ptr dbl_27B4068
.text:01D562C4 004                 push    edx
.text:01D562C5 008                 push    eax
.text:01D562C6 00C                 call    sub_1D0BE00 // ClientDLL_Frame(host_frametime);
.text:01D562CB 00C                 add     esp, 8
.text:01D562CE 004                 call    sub_1D20340
.text:01D562D3 004                 call    sub_1D16E50
.text:01D562D8 004                 call    sub_1D20350
.text:01D562DD 004                 call    sub_1D56100
.text:01D562E2 004                 call    sub_1D14A30 // CL_EmitEntities
.text:01D562E7 004                 call    sub_1D17BD0
.text:01D562EC
.text:01D562EC     loc_1D562EC:                            ; CODE XREF: sub_1D56200+...
*/

					tmp2 = textRange.And(MemRange(refTmp2.Start - 0x0D4, refTmp2.Start - 0x0D4 + 0x33));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "8B 15 ?? ?? ?? ?? A1 ?? ?? ?? ?? 52 50 E8 ?? ?? ?? ?? 83 C4 08 E8 ?? ?? ?? ?? E8 ?? ?? ?? ??  E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ??").IsEmpty())
						{
							AFXADDR_SET(host_frametime, *(DWORD *)(tmp2.Start +0x7));
							AFXADDR_SET(CL_EmitEntities, *(DWORD *)(tmp2.Start + 0x33 -0x5 -0x4) + (DWORD)(tmp2.Start + 0x33 - 0x5)); // Decode Call
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

	// CL_Disconnect // [16] // Checked: 2018-09-01
	{
		MemRange tmp1 = MemRange(data2Range.Start, data2Range.Start);
		
		for (int i = 0; i < 2; ++i)
		{
			tmp1 = FindCString(MemRange(tmp1.End, data2Range.End), "cd stop\n");
		}
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x6, refTmp1.Start - 0x6 + 0x5));
				if (!tmp2.IsEmpty())
				{
					if (!FindPatternString(tmp2, "E8 ?? ?? ?? ??").IsEmpty())
					{
						AFXADDR_SET(CL_Disconnect, *(DWORD *)(tmp2.Start + 0x1) + (DWORD)(tmp2.Start + 0x1 + 0x4)); // Decode Call
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	// p_cmd_functions // Checked: 2018-09-01
	// We are actually at the inlined Cmd_Exists in Cmd_AddCommand (shortly before the string we search) in order to get the cmd_functions root:
	{
		MemRange tmp1 = FindCString(data2Range, "Cmd_AddCommand: %s already defined\n");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x42, refTmp1.Start - 0x42 + 0x8));
				if (!tmp2.IsEmpty())
				{
/*
.text:01D279A6     loc_1D279A6:                            ; CODE XREF: sub_1D27960+...
.text:01D279A6 020                 mov     esi, dword_1FE08A0
.text:01D279AC 020                 test    esi, esi
.text:01D279AE 020                 jz      short loc_1D279C7
.text:01D279B0
.text:01D279B0     loc_1D279B0:                            ; CODE XREF: sub_1D27960+...
*/
					if (!FindPatternString(tmp2, "8B 35 ?? ?? ?? ?? 85 F6").IsEmpty())
					{
						AFXADDR_SET(p_cmd_functions, *(DWORD *)(tmp2.Start + 0x2));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	//
	// Rendering related:
	//

	// UnkDrawHud* // [7] // Checked 2018-09-08
	{
		MemRange r1 = FindPatternString(textRange, "E8 ?? ?? ?? ?? A1 ?? ?? ?? ?? BF 05 00 00 00 3B C7 75 39 83 3D ?? ?? ?? ?? 02 75 30 A1 ?? ?? ?? ?? 88 5D FC 83 F8 01 75 04 C6 45 FC 01 53 E8 ?? ?? ?? ?? 8B 4D FC 81 E1 FF 00 00 00 51 E8 ?? ?? ?? ?? 6A 01 E8 ?? ?? ?? ?? 83 C4 0C 39 1D ?? ?? ?? ?? 75 16 83 3D ?? ?? ?? ?? 01 75 08 39 1D ?? ?? ?? ?? 74 05 E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 39 3D ?? ?? ?? ?? 75 0E 83 3D ?? ?? ?? ?? 02 75 05 E8 ?? ?? ?? ?? E8 ?? ?? ?? ??");

		if (!r1.IsEmpty())
		{
			AFXADDR_SET(UnkDrawHudInCall, *(DWORD *)(r1.Start + 1) + (DWORD)(r1.Start + 1 + 4)); // [7] // Decode Call
			AFXADDR_SET(UnkDrawHudOutCall, *(DWORD *)(r1.Start + 134) + (DWORD)(r1.Start + 134 + 4)); // [7] // Decode Call

			AFXADDR_SET(UnkDrawHudIn, r1.Start); // [7]
			AFXADDR_SET(UnkDrawHudInContinue, AFXADDR_GET(UnkDrawHudIn) + 0x5); // [7]
			AFXADDR_SET(UnkDrawHudOut, r1.Start + 133); // [7]
			AFXADDR_SET(UnkDrawHudOutContinue, AFXADDR_GET(UnkDrawHudOut) + 0x5); // [7]
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// R_PushDlights // [7] // Checked 2018-09-08
	{
		MemRange r1 = FindPatternString(textRange, "D9 05 ?? ?? ?? ?? D8 1D ?? ?? ?? ?? DF E0 F6 C4 44 7A 61 A1 ?? ?? ?? ?? 56 40 57 A3 ?? ?? ?? ?? 33 F6 BF ?? ?? ?? ??");

		if (!r1.IsEmpty())
		{
			AFXADDR_SET(R_PushDlights, r1.Start); // [7]
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// R_RenderView // [7] // Checked: 2018-09-08
	// R_DrawViewModel // [7] // Checked: 2018-09-08
	// R_PolyBlend // [7] // Checked: 2018-09-08
	// R_DrawEntitiesOnList // [7] // Checked: 2018-09-08
	// R_DrawParticles // [7] // Checked: 2018-09-08
	// r_refdef // [7] // Checked: 2018-09-08
	// Mod_LeafPVS // [7] // Checked: 2018-09-08
	{
		MemRange tmp1 = FindCString(data2Range, "R_RenderView: NULL worldmodel");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x30, refTmp1.Start - 0x30 + 0x3));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "55 8B EC").IsEmpty())
						{
							AFXADDR_SET(R_RenderView, tmp2.Start);

							// Search "inside" R_RenderView function:
							MemRange tmp3 = FindPatternString(MemRange(AFXADDR_GET(R_RenderView), textRange.End), "E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? A1 ?? ?? ?? ?? 85 C0 75 0A E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ??");

							if (!tmp3.IsEmpty())
							{
								DWORD addr_R_RenderScene = *(DWORD *)(tmp3.Start + 6) + (DWORD)(tmp3.Start + 6 + 4); // Decode Call
								AFXADDR_SET(R_DrawViewModel, *(DWORD *)(tmp3.Start + 20) + (DWORD)(tmp3.Start + 20 + 4)); // Decode Call
								AFXADDR_SET(R_PolyBlend, *(DWORD *)(tmp3.Start + 25) + (DWORD)(tmp3.Start + 25 + 4)); // Decode CAll

								// Look into R_RenderScene:
								MemRange range_R_RenderScene = textRange.And(MemRange(addr_R_RenderScene, addr_R_RenderScene +0x0DD));
								MemRange tmp4 = FindPatternString(range_R_RenderScene, "83 C4 04 E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ??");
								if (!tmp4.IsEmpty())
								{
									DWORD addr_R_SetupGL = *(DWORD *)(tmp4.Start + 14) + (DWORD)(tmp4.Start + 14 +  4); // Decode Call
									DWORD addr_R_MarkLeaves = *(DWORD *)(tmp4.Start + 19) + (DWORD)(tmp4.Start + 19 +4); // Decode Call

									MemRange tmp5 = FindPatternString(MemRange(tmp4.End, range_R_RenderScene.End), "E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? A1 ?? ?? ?? ?? 85 C0 74 05");
									if (!tmp5.IsEmpty())
									{
										//DWORD addr_R_DrawWorld = *(DWORD *)(tmp5.Start + 1);

										AFXADDR_SET(R_DrawEntitiesOnList, *(DWORD *)(tmp5.Start + 11) +(DWORD)(tmp5.Start + 11 + 4)); // Decode call

										MemRange tmp6 = FindPatternString(MemRange(tmp5.End, range_R_RenderScene.End), "A1 ?? ?? ?? ?? 85 C0 75 0F E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E9 ?? ?? ?? ??");
										if (!tmp6.IsEmpty())
										{
											AFXADDR_SET(R_DrawParticles, *(DWORD *)(tmp6.Start + 10) + (DWORD)(tmp6.Start + 14)); // (Decod E9 JMP into R_DrawParticles sub).
										}
										else ErrorBox(MkErrStr(__FILE__, __LINE__));
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));

									// Look into R_SetupGL:
									MemRange tmp7 = FindPatternString(textRange.And(MemRange(addr_R_SetupGL, addr_R_SetupGL + 0x3FB)), "8B 1D ?? ?? ?? ?? 8B 3D ?? ?? ?? ?? A1 ?? ?? ?? ?? 8B 15 ?? ?? ?? ??");
									if (!tmp7.IsEmpty())
									{
										AFXADDR_SET(r_refdef, *(DWORD *)(tmp7.Start + 8));
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));

									// Look into R_MarkLeaves:
									MemRange tmp8 = FindPatternString(textRange.And(MemRange(addr_R_MarkLeaves, addr_R_MarkLeaves +0x106)), "51 68 FF 00 00 00 52 E8 ?? ?? ?? ?? 83 C4 0C EB 0C 50 51 E8 ?? ?? ?? ??");
									if (!tmp8.IsEmpty())
									{
										AFXADDR_SET(Mod_LeafPVS, *(DWORD *)(tmp8.Start + 20) + (DWORD)(tmp8.Start + 20 + 4)); // Decode Call
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
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// R_DrawSkyBoxEx // [11] // Checked: 2018-09-08
	// skytextures // [11] // Checked: 2018-09-08
	{
		MemRange r1 = FindPatternString(textRange, "55 8B EC 83 EC 1C A1 ?? ?? ?? ?? 53 56 BB 00 00 80 3F 57 C7 45 F8 00 00 00 00 85 C0");
		if (!r1.IsEmpty())
		{
			AFXADDR_SET(R_DrawSkyBoxEx, r1.Start);

			MemRange range_R_DrawSkyBoxEx = textRange.And(MemRange(AFXADDR_GET(R_DrawSkyBoxEx), AFXADDR_GET(R_DrawSkyBoxEx) + 0x1F0));
			MemRange r2 = FindPatternString(range_R_DrawSkyBoxEx, "8B 04 B5 ?? ?? ?? ?? 8B 0C 85 ?? ?? ?? ?? 51 E8 ?? ?? ?? ?? 8B 55 FC 83 C4 04 52 53 57 FF 15 ?? ?? ?? ?? 6A 07");
			if (!r2.IsEmpty())
			{
				AFXADDR_SET(skytextures, *(DWORD *)(r2.Start + 10));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// Draw_DecalMaterial // [12] // Checked 2018-09-19
	{
		MemRange tmp1 = FindCString(data2Range, "Failed to load custom decal for player #%i:%s using default decal 0.\n"); // Find String inside Draw_DecalMaterial
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				MemRange r2 = FindPatternString(textRange.And(MemRange(refTmp1.Start - 0x73, refTmp1.Start - 0x73 + 3)), "55 8B EC");
				if (!r2.IsEmpty())
				{
					AFXADDR_SET(Draw_DecalMaterial, r2.Start);
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// g_fov // [17] // Checked 2018-09-19
	{
		MemRange r1 = FindPatternString(textRange, "51 FF D0 83 C4 08 85 C0 74 23 8B 55 E8 8B 45 EC 8B 4D F0 89 15 ?? ?? ?? ?? 8B 55 F8 A3 ?? ?? ?? ?? 89 0D ?? ?? ?? ?? 89 15 ?? ?? ?? ??");

		if (!r1.IsEmpty())
		{
			AFXADDR_SET(g_fov, *(DWORD *)(r1.Start + 41));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	//
	// Sound system related:
	//

	// GetSoundtime // [6] // Checked 2018-09-22
	// S_PaintChannels // [6] // Checked 2018-09-22
	// paintedtime // [6] // Checked 2018-09-22
	// shm // [6] // Checked 2018-09-22
	// soundtime // [6] // Checked 2018-09-22
	// paintbuffer // [6] // Checked 2018-09-22
	// S_TransferPaintBuffer // [6] // Checked 2018-09-22
	{
		MemRange s1 = FindCString(data2Range, "Couldn't get sound buffer status\n");

		if (!s1.IsEmpty()) {

			MemRange refS1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

			if (!refS1.IsEmpty()) {

				MemRange r2a = FindPatternString(textRange.And(MemRange(refS1.Start - 0x73, refS1.Start - 0x73 + 0x7)), "56 57 E8 ?? ?? ?? ??");

				if (!r2a.IsEmpty())
				{
					AFXADDR_SET(GetSoundtime, *(DWORD *)(r2a.Start +3) + (DWORD)(r2a.Start + 7));

					MemRange r3 = textRange.And(MemRange(AFXADDR_GET(GetSoundtime), AFXADDR_GET(GetSoundtime) + 0x8F));

					MemRange r4 = FindPatternString(r3, "A1 ?? ?? ?? ?? 41 3D 00 00 00 40");

					if (!r4.IsEmpty()) {

						AFXADDR_SET(paintedtime, *(DWORD *)(r4.Start + 1));

						MemRange r5 = FindPatternString(MemRange(r4.End, r3.End), "E8 ?? ?? ?? ?? 83 C4 04 8B 0D ?? ?? ?? ??");

						if (!r5.IsEmpty()) {

							AFXADDR_SET(shm, *(DWORD *)(r5.Start + 10));

							MemRange r6 = FindPatternString(MemRange(r5.End, r3.End), "89 0D ?? ?? ?? ?? 5F 5E 8B E5 5D C3");

							if (!r6.IsEmpty()) {

								AFXADDR_SET(soundtime, *(DWORD *)(r6.Start + 2));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));




				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));

				MemRange r2b = FindPatternString(textRange.And(MemRange(refS1.Start + 0x34, refS1.Start + 0x34  + 8)), "D1 EF 57 E8 ?? ?? ?? ??");

				if (!r2b.IsEmpty())
				{
					AFXADDR_SET(S_PaintChannels, *(DWORD *)(r2b.Start + 4) + (DWORD)(r2b.Start + 4 + 4));

					MemRange r3 = textRange.And(MemRange(AFXADDR_GET(S_PaintChannels), AFXADDR_GET(S_PaintChannels) + 0x196));

					MemRange r4 = FindPatternString(r3, "8B F3 2B F0 8D 14 F5 00 00 00 00 52 6A 00 68 ?? ?? ?? ??");

					if (!r4.IsEmpty()) {

						AFXADDR_SET(paintbuffer, *(DWORD *)(r4.Start + 15));

						MemRange r5 = FindPatternString(MemRange(r4.End, r3.End), "53 E8 ?? ?? ?? ?? 8B 4D 08 83 C4 04");

						if (!r5.IsEmpty()) {

							AFXADDR_SET(S_TransferPaintBuffer, *(DWORD *)(r5.Start + 2) + (DWORD)(r5.Start + 6));
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
	
	// SND_PickChannel // [6] // Checked 2018-0922
	{
		MemRange r1 = FindPatternString(textRange, "55 8B EC 53 8B 5D 0C 56 83 FB 05 57 75 17 8B 45 10 50 E8 ?? ?? ?? ?? 83 C4 04 85 C0 74 07 5F 5E 33 C0 5B 5D C3 83 CF FF C7 45 10 FF FF FF 7F BE 04 00 00 00");

		if (!r1.IsEmpty()) {

			AFXADDR_SET(SND_PickChannel, r1.Start);
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	//
	// Demo parsing related:
	//

	// Warning: When these are updated,the hook code might need to be updated as well!
	// CL_ParseServerMessage_CmdRead // [10] // Checked 2018-09-22
	// CL_ParseServerMessage_CmdRead_DSZ // [10] // Checked 2018-09-22
	// msg_readcount // [10] // Checked 2018-09-22
	// net_message // [10] // Checked 2018-09-22
	{
		MemRange s1 = FindCString(data2Range, "CL_ParseServerMessage: Bad server message");

		if (!s1.IsEmpty()) {

			MemRange s1Ref = FindBytes(textRange, (const char *)(&s1.Start), sizeof(s1.Start));

			if (!s1Ref.IsEmpty()) {

				MemRange r2 = FindPatternString(textRange.And(MemRange(s1Ref.Start + 0x12, s1Ref.Start + 0x12 + 0x7)), "8B DF E8 ?? ?? ?? ??");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(CL_ParseServerMessage_CmdRead, r2.Start);
					AFXADDR_SET(CL_ParseServerMessage_CmdRead_DSZ, 0x07);

					DWORD addMessageReadByte = *(DWORD *)(r2.Start + 3) + (DWORD)(r2.Start + 7);

					MemRange rMessageReadByte = textRange.And(MemRange(addMessageReadByte, addMessageReadByte + 0x36));

					MemRange r3 = FindPatternString(rMessageReadByte, "A1 ?? ?? ?? ?? 8B 15 ?? ?? ?? ??");

					if (!r3.IsEmpty()) {

						AFXADDR_SET(msg_readcount, *(DWORD *)(r3.Start + 1));
						AFXADDR_SET(net_message, *(DWORD *)(r3.Start + 7));
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

/// <remarks>Not called when no client.dll is loaded.</remarks>
void Addresses_InitClientDll(AfxAddr clientDll, const char * gamedir)
{
	AFXADDR_SET(clientDll, clientDll);

	MemRange textRange = MemRange(0, 0);
	MemRange data1Range = MemRange(0, 0);
	MemRange data2Range = MemRange(0, 0);
	{
		ImageSectionsReader imageSectionsReader((HMODULE)clientDll);
		if (!imageSectionsReader.Eof())
		{
			textRange = imageSectionsReader.GetMemRange();
			imageSectionsReader.Next();

			if (!imageSectionsReader.Eof())
			{
				data1Range = imageSectionsReader.GetMemRange();
				imageSectionsReader.Next();

				if (!imageSectionsReader.Eof())
				{
					data2Range = imageSectionsReader.GetMemRange();
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}


	if (0 == _stricmp("cstrike", gamedir))
	{
		// cstrike CrossHair fix related:
		// cstrike_UnkCrosshairFn // [1] // Checked 2018-10-06
		// cstrike_UnkCrosshairFn_mul_fac // [1] // Checked 2018-10-06
		// cstrike_UnkCrosshairFn_add_fac // [1] // Checked 2018-10-06
		{
			MemRange r1 = FindPatternString(textRange, "83 EC 08 8B 44 24 10 53 55 56 57 8B F9 8D 48 FF 89 7C 24 14 83 F9 1D BA 04 00 00 00 BE 05 00 00 00");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(cstrike_UnkCrosshairFn, r1.Start);

				MemRange r2 = textRange.And(MemRange(r1.Start, r1.Start + 0x550));

				MemRange r3 = FindPatternString(r2, "D9 47 44 DD 05 ?? ?? ?? ?? D8 C9 83 C0 02 89 47 48 DC 05 ?? ?? ?? ??");

				if (!r3.IsEmpty()) {
					AFXADDR_SET(cstrike_UnkCrosshairFn_mul_fac, *(DWORD *)(r3.Start + 5));
					AFXADDR_SET(cstrike_UnkCrosshairFn_add_fac, *(DWORD *)(r3.Start + 19));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// cstrike_EV_CreateSmoke: // [3]
		// This is set in MypfnHookEvent.

		// cstrike DeathMsg related (client.dll offsets):
		// [2] // Checked 2018-10-06
		{
			MemRange s1 = FindCString(data2Range, "DeathMsg");

			if (!s1.IsEmpty()) {

				MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

				if (!r1.IsEmpty()) {

					MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start - 6, r1.Start +4 + 6)), "68 ?? ?? ?? ?? 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ??");

					if (!r2.IsEmpty()) {

						AFXADDR_SET(cstrike_MsgFunc_DeathMsg, *(DWORD *)(r2.Start + 1));

						MemRange r3 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(cstrike_MsgFunc_DeathMsg), AFXADDR_GET(cstrike_MsgFunc_DeathMsg) + 0x1A)), "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 52 B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? C3");

						if (!r3.IsEmpty())
						{
							AFXADDR_SET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg, *(DWORD *)(r3.Start + 21) + (DWORD)(r3.Start + 25)); // Decode call address.

							MemRange r4 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg), AFXADDR_GET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg) +0x43F)), "68 80 02 00 00 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? E8 ?? ?? ?? ??");

							if (!r4.IsEmpty()) {
								AFXADDR_SET(cstrike_rgDeathNoticeList, *(DWORD *)(r4.Start + 11));
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

			MemRange r1 = FindPatternString(textRange, "83 EC 3C 56 8B F1 89 74 24 0C 8B 46 1C 89 44 24 2C A1 ?? ?? ?? ?? 85 C0 75 0A FF 15 ?? ?? ?? ?? 85 C0 74 12");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(cstrike_CHudDeathNotice_Draw, r1.Start);

				// Check that our Y-Res adjusting code is still correct (otherwise it needs to be adjusted):
				//
				// .text:01944724 04C                 mov     ebp, [esi+18h]
				// .text:01944727 04C                 mov     edx, [esp+38h]
				// .text:0194472B 04C                 imul    ebp, ebx
				//
				MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start, r1.Start + 0x37D)), "8B 6E 18 8B 54 24 38 0F AF EB");

				if (!r2.IsEmpty())
				{
					AFXADDR_SET(cstrike_CHudDeathNotice_Draw_YRes, r2.Start);
					AFXADDR_SET(cstrike_CHudDeathNotice_Draw_YRes_DSZ, r2.End - r2.Start);
				}
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
	}
	else if (0 == _stricmp("tfc", gamedir))
	{
		// tfc DeathMsg related (client.dll offsets):
		// [14] // Checked 2018-10-06
		{
			MemRange s1 = FindCString(data2Range, "DeathMsg");

			if (!s1.IsEmpty()) {

				MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

				if (!r1.IsEmpty()) {

					MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start - 6, r1.Start + 4 + 6)), "68 ?? ?? ?? ?? 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ??");

					if (!r2.IsEmpty()) {

						AFXADDR_SET(tfc_MsgFunc_DeathMsg, *(DWORD *)(r2.Start + 1));

						MemRange r3 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(tfc_MsgFunc_DeathMsg), AFXADDR_GET(tfc_MsgFunc_DeathMsg) + 0x1A)), "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 52 B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? C3");

						if (!r3.IsEmpty())
						{
							AFXADDR_SET(tfc_CHudDeathNotice_MsgFunc_DeathMsg, *(DWORD *)(r3.Start + 21) + (DWORD)(r3.Start + 25)); // Decode call address.

							MemRange r4 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(tfc_CHudDeathNotice_MsgFunc_DeathMsg), AFXADDR_GET(tfc_CHudDeathNotice_MsgFunc_DeathMsg) + 0x363)), "68 70 02 00 00 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? E8 ?? ?? ?? ??");

							if (!r4.IsEmpty()) {
								AFXADDR_SET(tfc_rgDeathNoticeList, *(DWORD *)(r4.Start + 11));
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

			MemRange r1 = FindPatternString(textRange, "83 EC 30 53 55 56 57 33 FF BD 22 00 00 00 89 4C 24 2C 89 7C 24 18 C7 44 24 14 ?? ?? ?? ?? C7 44 24 10 ?? ?? ?? ?? 89 6C 24 28 C7 44 24 1C ?? ?? ?? ??");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(tfc_CHudDeathNotice_Draw, r1.Start);

				// Check that our Y-Res adjusting code is still correct (otherwise it needs to be adjusted):
				//
				// .text:01928069 040                 mov     ebp, 22h
				//

				AFXADDR_SET(cstrike_CHudDeathNotice_Draw_YRes, r1.Start + 9);
				AFXADDR_SET(cstrike_CHudDeathNotice_Draw_YRes_DSZ, r1.Start + 9 + 5);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// tfc_TeamFortressViewport_UpdateSpecatorPanel // 4 // Checked 2018-10-06
		{
			MemRange r1 = FindPatternString(textRange, "A1 ?? ?? ?? ?? 81 EC 44 02 00 00 56 8B F1 89 86 24 02 00 00 8B 0D ?? ?? ?? ?? 89 8E 28 02 00 00 8B 8E 18 0A 00 00 8B 15 ?? ?? ?? ?? 85 C9 89 96 2C 02 00 00 0F 84 ?? ?? ?? ??");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(tfc_TeamFortressViewport_UpdateSpecatorPanel, r1.Start);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
	}
	else if (0 == _stricmp("valve", gamedir)) // (Half-Life)
	{
		// valve_TeamFortressViewport_UpdateSpecatorPanel // 4 // Checked 2018-10-06
		{
			MemRange r1 = FindPatternString(textRange, "A1 ?? ?? ?? ?? 81 EC 44 02 00 00 56 8B F1 89 86 24 02 00 00 8B 0D ?? ?? ?? ?? 89 8E 28 02 00 00 8B 8E 18 0A 00 00 8B 15 ?? ?? ?? ?? 85 C9 89 96 2C 02 00 00 0F 84 ?? ?? ?? ??");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(valve_TeamFortressViewport_UpdateSpecatorPanel, r1.Start);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
	}
}
