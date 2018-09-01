#include "stdafx.h"

#include "hl_addresses.h"

#include <shared/binutils.h>

using namespace Afx::BinUtils;

AFXADDR_DEF(engine_LoadClientLibrary)
AFXADDR_DEF(engine_ClientFunctionTable)
AFXADDR_DEF(CL_Disconnect)
AFXADDR_DEF(CL_EmitEntities)
AFXADDR_DEF(CL_ParseServerMessage_CmdRead)
AFXADDR_DEF(CL_ParseServerMessage_CmdRead_DSZ)
AFXADDR_DEF(p_cmd_functions)
AFXADDR_DEF(DTOURSZ_GetSoundtime)
AFXADDR_DEF(DTOURSZ_Mod_LeafPVS)
AFXADDR_DEF(DTOURSZ_R_DrawEntitiesOnList)
AFXADDR_DEF(DTOURSZ_R_DrawParticles)
AFXADDR_DEF(DTOURSZ_R_DrawViewModel)
AFXADDR_DEF(DTOURSZ_R_PolyBlend)
AFXADDR_DEF(DTOURSZ_R_RenderView)
AFXADDR_DEF(DTOURSZ_SND_PickChannel)
AFXADDR_DEF(DTOURSZ_S_PaintChannels)
AFXADDR_DEF(DTOURSZ_S_TransferPaintBuffer)
AFXADDR_DEF(GetSoundtime)
AFXADDR_DEF(_Host_Frame)
AFXADDR_DEF(Host_Init)
AFXADDR_DEF(Mod_LeafPVS)
AFXADDR_DEF(R_DrawEntitiesOnList)
AFXADDR_DEF(R_DrawParticles)
AFXADDR_DEF(R_DrawSkyBoxEx)
AFXADDR_DEF(R_DrawSkyBoxEx_DSZ)
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
AFXADDR_DEF(UnkGetDecalTexture)
AFXADDR_DEF(UnkGetDecalTexture_DSZ)
AFXADDR_DEF(clientDll)
AFXADDR_DEF(cstrike_CHudDeathNotice_Draw)
AFXADDR_DEF(cstrike_CHudDeathNotice_Draw_DSZ)
AFXADDR_DEF(cstrike_CHudDeathNotice_MsgFunc_DeathMsg)
AFXADDR_DEF(cstrike_CHudDeathNotice_MsgFunc_DeathMsg_DSZ)
AFXADDR_DEF(cstrike_EV_CreateSmoke)
AFXADDR_DEF(cstrike_EV_CreateSmoke_DSZ)
AFXADDR_DEF(cstrike_MsgFunc_DeathMsg)
AFXADDR_DEF(cstrike_MsgFunc_DeathMsg_DSZ)
AFXADDR_DEF(cstrike_UnkCrosshairFn)
AFXADDR_DEF(cstrike_UnkCrosshairFn_DSZ)
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
AFXADDR_DEF(tfc_CHudDeathNotice_Draw_DSZ)
AFXADDR_DEF(tfc_CHudDeathNotice_MsgFunc_DeathMsg)
AFXADDR_DEF(tfc_CHudDeathNotice_MsgFunc_DeathMsg_DSZ)
AFXADDR_DEF(tfc_MsgFunc_DeathMsg)
AFXADDR_DEF(tfc_MsgFunc_DeathMsg_DSZ)
AFXADDR_DEF(tfc_TeamFortressViewport_UpdateSpecatorPanel)
AFXADDR_DEF(tfc_TeamFortressViewport_UpdateSpecatorPanel_DSZ)
AFXADDR_DEF(tfc_rgDeathNoticeList)
AFXADDR_DEF(valve_TeamFortressViewport_UpdateSpecatorPanel)
AFXADDR_DEF(valve_TeamFortressViewport_UpdateSpecatorPanel_DSZ)

//
// Documentation (in HLAE source code)
//
// o[1] doc/notes_goldsrc/debug_cstrike_crosshair.txt
// o[2] doc/notes_goldsrc/debug_cstrike_deathmessage.txt
// n[3] doc/notes_goldsrc/debug_cstrike_smoke.txt
// *[4] doc/notes_goldsrc/debug_tfc_UpdateSpectatorPanel.txt
// [5] doc/notes_goldsrc/debug_engine_ifaces.txt
// *[6] doc/notes_goldsrc/debug_sound.txt
// *[7] doc/notes_goldsrc/debug_SCR_UpdateScreen.txt
// [8] doc/notes_goldsrc/debug_Host_Frame.txt
// [9] doc/notes_goldsrc/debug_ClientFunctionTable
// *[10] doc/notes_goldsrc/debug_CL_ParseServerMessage.txt
// *[11] doc/notes_goldsrc/debug_R_DrawWorld_and_sky.txt
// n[12] doc/notes_goldsrc/debug_R_DecalShoot.txt
// *[14] doc/notes_goldsrc/debug_tfc_deathmessage.txt
// *[15] doc/notes_goldsrc/debug_sv_variables.txt
// [16] doc/notes_goldsrc/debug_CL_Disconnect.txt
// n[17] doc/notes_goldsrc/debug_fov.txt
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

	// engine_LoadClientLibrary // [9] // Checked: 2018-09-01
	// engine_ClientFunctionTable // [9] // Checked: 2018-09-01
	{
		MemRange tmp1 = FindCString(data2Range, "Initialize");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = tmp1.And(MemRange(refTmp1.Start - 0x33, refTmp1.Start - 0x33 + 0x3));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "55 8B EC").IsEmpty())
						{
							AFXADDR_SET(engine_LoadClientLibrary, tmp2.Start);
						}
						else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));
				}

				{
					MemRange tmp2 = tmp1.And(MemRange(refTmp1.Start + 0x11, refTmp1.Start + 0x11 + 0x7));
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
		MemRange tmp1 = FindCString(data2Range, "-console");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = tmp1.And(MemRange(refTmp1.Start - 0x66, refTmp1.Start - 0x66 + 0x3));
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
				DWORD tmpAddr = data2Range.Start + 0x0C;

				MemRange refTmp2 = FindBytes(textRange, (const char *)&tmpAddr, sizeof(tmpAddr));
				if (!refTmp2.IsEmpty())
				{

					MemRange tmp2 = tmp1.And(MemRange(tmp1.Start - 0x18D, tmp1.Start - 0x18D + 0x3));
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

					tmp2 = tmp1.And(MemRange(tmp1.Start - 0x0D4, tmp1.Start - 0x0D4 + 0x33));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "8B 15 ?? ?? ?? ?? A1 ?? ?? ?? ?? 52 50 E8 ?? ?? ?? ?? 83 C4 08 E8 ?? ?? ?? ?? E8 ?? ?? ?? ??  E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ??").IsEmpty())
						{
							AFXADDR_SET(host_frametime, *(DWORD *)(tmp2.Start +0x7));
							AFXADDR_SET(CL_EmitEntities, *(DWORD *)(tmp2.Start + 0x33 -0x5 -0x4));
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

	// CL_Disconnect // [16] // Checked: 2018-09-01
	{
		MemRange tmp1 = FindCString(data2Range, "cd stop\n");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				MemRange tmp2 = tmp1.And(MemRange(tmp1.Start - 0x6, tmp1.Start - 0x6 + 0x5));
				if (!tmp2.IsEmpty())
				{
					if (!FindPatternString(tmp2, "E8 ?? ?? ?? ??").IsEmpty())
					{
						AFXADDR_SET(CL_Disconnect, *(DWORD *)(tmp2.Start + 0x1));
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
				MemRange tmp2 = tmp1.And(MemRange(tmp1.Start - 0x42, tmp1.Start - 0x42 + 0x8));
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

	AFXADDR_SET(UnkDrawHudInCall, hwDll +0x4F780); // *[7]
	AFXADDR_SET(UnkDrawHudOutCall, hwDll +0x4F950); // *[7]
	AFXADDR_SET(UnkDrawHudIn, hwDll +0xB52B4); // *[7]
	AFXADDR_SET(UnkDrawHudInContinue, AFXADDR_GET(UnkDrawHudIn) + 0x5); // *[7]
	AFXADDR_SET(UnkDrawHudOut, hwDll +0xB5339); // *[7]
	AFXADDR_SET(UnkDrawHudOutContinue, AFXADDR_GET(UnkDrawHudOut) + 0x5); // *[7]
		
	AFXADDR_SET(R_PushDlights, hwDll + 0x42290); // *[7]

	AFXADDR_SET(R_RenderView, hwDll + 0x452E0); // *[7]
	AFXADDR_SET(DTOURSZ_R_RenderView, 0x6); // *[7]
	
	AFXADDR_SET(R_DrawViewModel, hwDll +0x43990); // *[7]
	AFXADDR_SET(DTOURSZ_R_DrawViewModel, 0x06); // *[7]
	
	AFXADDR_SET(R_PolyBlend, hwDll +0x43EF0); // *[7]
	AFXADDR_SET(DTOURSZ_R_PolyBlend, 0x06); // *[7]
	
	AFXADDR_SET(r_refdef, hwDll + 0xEC61E0); // *[7]
	
	AFXADDR_SET(Mod_LeafPVS, hwDll + 0x28270); // *[7]
	AFXADDR_SET(DTOURSZ_Mod_LeafPVS, 0x06); // *[7]
	
	AFXADDR_SET(R_DrawEntitiesOnList, hwDll + 0x43750); // *[7]
	AFXADDR_SET(DTOURSZ_R_DrawEntitiesOnList, 0x06); // *[7]
	
	AFXADDR_SET(R_DrawParticles, hwDll + 0x7B1B0); // *[7]
	AFXADDR_SET(DTOURSZ_R_DrawParticles, 0x06); // *[7]
	
	AFXADDR_SET(R_DrawSkyBoxEx, hwDll + 0x4F55E); // *[11]
	AFXADDR_SET(R_DrawSkyBoxEx_DSZ,  0x06); // *[11]
	
	AFXADDR_SET(skytextures, hwDll + 0x63FF48); // *[11]
	
	AFXADDR_SET(UnkGetDecalTexture, hwDll + 0x2EB30); // *[12]
	AFXADDR_SET(UnkGetDecalTexture_DSZ, 0x06); // *[12]

	AFXADDR_SET(g_fov , hwDll +0x144A8C); // *[17]
	
	//
	// Sound system related:
	//
	
	AFXADDR_SET(GetSoundtime, hwDll + 0x8B140); // *[6]
	AFXADDR_SET(DTOURSZ_GetSoundtime, 0x0a); // *[6]
	
	AFXADDR_SET(S_PaintChannels, hwDll + 0x8CD60); // *[6]
	AFXADDR_SET(DTOURSZ_S_PaintChannels, 0x08); // *[6]
	
	AFXADDR_SET(paintedtime, hwDll + 0xA2B860); // *[6]
	AFXADDR_SET(shm, hwDll + 0x6B7CD8); // *[6]
	AFXADDR_SET(soundtime, hwDll + 0xA2B85C); // *[6]
	
	AFXADDR_SET(paintbuffer, hwDll + 0xA21720); // *[6]
	
	AFXADDR_SET(S_TransferPaintBuffer, hwDll + 0x8C800); // *[6]
	AFXADDR_SET(DTOURSZ_S_TransferPaintBuffer, 0x06); // *[6]
	
	AFXADDR_SET(SND_PickChannel, hwDll + 0x8A3E0); // *[6]
	AFXADDR_SET(DTOURSZ_SND_PickChannel, 0x07); // *[6]
	
	//
	// Demo parsing related:
	//
	
	AFXADDR_SET(CL_ParseServerMessage_CmdRead, hwDll + 0x1CFC6); // *[10]
	AFXADDR_SET(CL_ParseServerMessage_CmdRead_DSZ, 0x07); // *[10]
	AFXADDR_SET(msg_readcount, hwDll + 0x1004D28); // *[10]
	AFXADDR_SET(net_message, hwDll +0xA9F230 - 0x10); // *[10]
}

/// <remarks>Not called when no client.dll is loaded.</remarks>
void Addresses_InitClientDll(AfxAddr clientDll)
{
	AFXADDR_SET(clientDll, clientDll);

	//
	// game: cstrike
	//
	
	// cstrike CrossHair fix related:
	AFXADDR_SET(cstrike_UnkCrosshairFn, clientDll + 0x41640); // *[1]
	AFXADDR_SET(cstrike_UnkCrosshairFn_DSZ, 0x0c); // at least 8 bytes req. // *[1]
	AFXADDR_SET(cstrike_UnkCrosshairFn_add_fac, clientDll + 0xC32C8); // *[1]
	AFXADDR_SET(cstrike_UnkCrosshairFn_mul_fac, clientDll + 0xCD4C8); // *[1]
	
	// cstrike EV_CreateSmoke:
	AFXADDR_SET(cstrike_EV_CreateSmoke, clientDll + 0xA080); // *[3]
	AFXADDR_SET(cstrike_EV_CreateSmoke_DSZ, 0x0a); // *[3]
	
	// cstrike DeathMsg related (client.dll offsets):
	AFXADDR_SET(cstrike_MsgFunc_DeathMsg, clientDll + 0x44490); // *[2]
	AFXADDR_SET(cstrike_MsgFunc_DeathMsg_DSZ, 0x08); // *[2]
	AFXADDR_SET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg, clientDll + 0x44970); // *[2]
	AFXADDR_SET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg_DSZ, 0x08); // at least 8 bytes req. // *[2]
	AFXADDR_SET(cstrike_rgDeathNoticeList, clientDll + 0x124EC0); // *[2]
	AFXADDR_SET(cstrike_CHudDeathNotice_Draw, clientDll + 0x445F0); // *[2]
	AFXADDR_SET(cstrike_CHudDeathNotice_Draw_DSZ, 0x0a); // at least 8 bytes req. // *[2]

	//
	// game: tfc
	//

	// tfc DeathMsg related:
	AFXADDR_SET(tfc_MsgFunc_DeathMsg, clientDll + 0x27F60); // *[14]
	AFXADDR_SET(tfc_MsgFunc_DeathMsg_DSZ, 0x08); // *[14]
	AFXADDR_SET(tfc_CHudDeathNotice_MsgFunc_DeathMsg, clientDll + 0x28300); // *[14]
	AFXADDR_SET(tfc_CHudDeathNotice_MsgFunc_DeathMsg_DSZ, 0x08); // at least 8 bytes req. // *[14]
	AFXADDR_SET(tfc_rgDeathNoticeList, clientDll + 0xA79B0); // *[14]
	AFXADDR_SET(tfc_CHudDeathNotice_Draw, clientDll + 0x28060); // *[14]
	AFXADDR_SET(tfc_CHudDeathNotice_Draw_DSZ, 0x09); // at least 8 bytes req. // *[14]

	//
	AFXADDR_SET(tfc_TeamFortressViewport_UpdateSpecatorPanel, clientDll + 0x4E830); // *[4]
	AFXADDR_SET(tfc_TeamFortressViewport_UpdateSpecatorPanel_DSZ, 0x0b); // at least 8 bytes req. // *[4]

	//
	// game: valve (Half-Life)
	//

	AFXADDR_SET(valve_TeamFortressViewport_UpdateSpecatorPanel, clientDll + 0x4ACB0); // *[4]
	AFXADDR_SET(valve_TeamFortressViewport_UpdateSpecatorPanel_DSZ, 0x0b); // at least 8 bytes req. // *[4]
}