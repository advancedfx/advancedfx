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
AFXADDR_DEF(S_Update_)
AFXADDR_DEF(_Host_Frame)
AFXADDR_DEF(Host_Init)
AFXADDR_DEF(Mod_LeafPVS)
AFXADDR_DEF(R_DrawEntitiesOnList_In)
AFXADDR_DEF(R_DrawEntitiesOnList_Out)
AFXADDR_DEF(R_DrawParticles)
AFXADDR_DEF(R_DrawSkyBox_Begin)
AFXADDR_DEF(R_DrawSkyBox_End)
AFXADDR_DEF(R_DrawViewModel)
AFXADDR_DEF(R_PolyBlend)
AFXADDR_DEF(R_PushDlights)
AFXADDR_DEF(R_RenderView)
AFXADDR_DEF(S_StartDynamicSound)
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
AFXADDR_DEF(cstrike_PM_CatagorizePositionFn)
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
AFXADDR_DEF(tfc_CHudDeathNotice_Draw)
AFXADDR_DEF(tfc_CHudDeathNotice_Draw_YRes)
AFXADDR_DEF(tfc_CHudDeathNotice_Draw_YRes_DSZ)
AFXADDR_DEF(tfc_CHudDeathNotice_MsgFunc_DeathMsg)
AFXADDR_DEF(tfc_MsgFunc_DeathMsg)
AFXADDR_DEF(tfc_TeamFortressViewport_UpdateSpecatorPanel)
AFXADDR_DEF(tfc_rgDeathNoticeList)
AFXADDR_DEF(valve_TeamFortressViewport_UpdateSpecatorPanel)
AFXADDR_DEF(hw_HUD_GetStudioModelInterface_version)
AFXADDR_DEF(hw_HUD_GetStudioModelInterface_pInterface)
AFXADDR_DEF(hw_HUD_GetStudioModelInterface_pStudio)

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

	// pEngfuncs // [5] // Checked: 2023-12-16
	// ppmove // [5] // Checked: 2023-12-16
	// pstudio // [5] // Checked: 2023-12-16
	{
		MemRange s1 = FindCString(data1Range, "ScreenFade");

		if (!s1.IsEmpty()) {

			MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

			if (!r1.IsEmpty()) {

				MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start + 0x09, r1.Start + 0x09 + 30)), "6A 07 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ?? a1 ?? ?? ?? ?? 83 c4 18 85 c0 74 ?? 68 ?? ?? ?? ??");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(pEngfuncs, *(DWORD *)(r2.Start + 3));
					AFXADDR_SET(ppmove, *(DWORD *)(r2.Start + 26));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));

		s1 = MemRange(data1Range.Start, data1Range.Start);

		for (int i = 0; i < 1; ++i)
		{
			s1 = FindCString(MemRange(s1.End, data1Range.End), "HUD_GetStudioModelInterface");
		}

		if (!s1.IsEmpty()) {

			MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

			if (!r1.IsEmpty()) {

				MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start + 0x4, r1.Start + 0x4 + 26)), "56 ff 15 ?? ?? ?? ?? a1 ?? ?? ?? ?? 85 c0 74 ?? 68 ?? ?? ?? ?? 68 ?? ?? ?? ??");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(pstudio, *(DWORD *)(r2.Start + 17));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// engine_ClientFunctionTable // [9] // Checked: 2023-12-20
	{
		MemRange tmp1 = FindCString(data1Range, "Initialize");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = textRange.And(MemRange(refTmp1.Start + 0x0f, refTmp1.Start + 0x0f + 0x7));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "ff d6 A3 ?? ?? ?? ??").IsEmpty())
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

	// Host_Init [18] // Checked: 2023-12-20
	{
		MemRange tmp1 = FindCString(data1Range, "-toconsole");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x84, refTmp1.Start - 0x84 + 0x3));
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

	// _Host_Frame [8] // Checked: 2023-12-20
	// host_frametime [8] // Checked: 2023-12-20
	// CL_EmitEntities // [8] // Checked: 2023-12-20
	{
		MemRange tmp1 = FindCString(data1Range, "host_killtime");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(data2Range, (const char *)&tmp1.Start, sizeof(void *));
			if (!refTmp1.IsEmpty())
			{
				DWORD tmpAddr = refTmp1.Start + 0x0C;

				MemRange refTmp2 = FindBytes(textRange, (const char *)&tmpAddr, sizeof(tmpAddr));
				if (!refTmp2.IsEmpty())
				{

					MemRange tmp2 = textRange.And(MemRange(refTmp2.Start - 0x635, refTmp2.Start - 0x635 + 0x3));
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
                             LAB_101d46b8                                    XREF[1]:     101d46b1(j)  
        101d46b8 f2 0f 10        MOVSD      XMM0,qword ptr [host_frametime]                  = ??
                 05 a8 9e 
                 24 11
        101d46c0 83 ec 08        SUB        ESP,0x8
        101d46c3 f2 0f 11        MOVSD      qword ptr [ESP]=>local_40,XMM0
                 04 24
        101d46c8 e8 13 27        CALL       ClientDLL_Frame                                  undefined ClientDLL_Frame(undefi
                 fc ff
        101d46cd e8 6e 87        CALL       FUN_101ace40                                     undefined FUN_101ace40()
                 fd ff
        101d46d2 e8 79 06        CALL       FUN_101a4d50                                     undefined FUN_101a4d50()
                 fd ff
        101d46d7 e8 94 7b        CALL       FUN_101ac270                                     undefined FUN_101ac270()
                 fd ff
        101d46dc f2 0f 10        MOVSD      XMM0,qword ptr [host_frametime]                  = ??
                 05 a8 9e 
                 24 11
        101d46e4 83 c4 04        ADD        ESP,0x4
        101d46e7 66 0f 5a c0     CVTPD2PS   XMM0,XMM0
        101d46eb f3 0f 11        MOVSS      dword ptr [ESP]=>local_40+0x4,XMM0
                 04 24
        101d46f0 e8 db 08        CALL       FUN_10234fd0                                     undefined FUN_10234fd0(undefined
                 06 00
        101d46f5 83 c4 04        ADD        ESP,0x4
        101d46f8 e8 63 8f        CALL       CL_EmitEntities                                  undefined CL_EmitEntities()
                 fc ff
        101d46fd e8 be cd        CALL       FUN_101a14c0                                     undefined FUN_101a14c0()
                 fc ff
                             LAB_101d4702                                    XREF[1]:     101d4709(j)  
        101d4702 e8 79 41        CALL       FUN_101a8880                                     undefined FUN_101a8880()
                 fd ff
        101d4707 85 c0           TEST       EAX,EAX
        101d4709 75 f7           JNZ        LAB_101d4702
        101d470b e8 b0 15        CALL       FUN_101a5cc0                                     undefined FUN_101a5cc0()
                 fd ff
        101d4710 83 3d e0        CMP        dword ptr [cls.state],0x1                        = ??
                 8d 40 11 01
        101d4717 75 42           JNZ        LAB_101d475b
        101d4719 a1 cc 2f        MOV        EAX,[PTR_DAT_10322fcc]                           = 102b429c
                 32 10
        101d471e 80 38 00        CMP        byte ptr [EAX]=>DAT_102b429c,0x0                 = 30h
        101d4721 75 38           JNZ        LAB_101d475b
        101d4723 f6 05 4c        TEST       byte ptr [DAT_10f7744c],0x4                      = ??
                 74 f7 10 04
        101d472a 75 14           JNZ        LAB_101d4740
        101d472c f3 0f 10        MOVSS      XMM0,dword ptr [DAT_1031a724]
                 05 24 a7 
                 31 10
        101d4734 0f 57 c9        XORPS      XMM1,XMM1
        101d4737 0f 2e c1        UCOMISS    XMM0,XMM1
        101d473a 9f              LAHF
        101d473b f6 c4 44        TEST       AH,0x44
        101d473e 7a 1b           JP         LAB_101d475b
                             LAB_101d4740                                    XREF[1]:     101d472a(j)  
        101d4740 a1 84 b6        MOV        EAX,[DAT_1031b684]                               = 00000005h
                 31 10
        101d4745 8b c8           MOV        ECX,EAX
        101d4747 48              DEC        EAX
        101d4748 a3 84 b6        MOV        [DAT_1031b684],EAX                               = 00000005h
                 31 10
        101d474d 85 c9           TEST       ECX,ECX
        101d474f 7f 14           JG         LAB_101d4765
        101d4751 c7 05 a4        MOV        dword ptr [DAT_10f776a4],0x2                     = ??
                 76 f7 10 
                 02 00 00 00
                             LAB_101d475b                                    XREF[3]:     101d4717(j), 101d4721(j), 
                                                                                          101d473e(j)  
        101d475b c7 05 84        MOV        dword ptr [DAT_1031b684],0x5                     = 00000005h
                 b6 31 10 
                 05 00 00 00
                             LAB_101d4765                                    XREF[1]:     101d474f(j)  
        101d4765 e8 c6 2d        CALL       FUN_101c7530                                     undefined FUN_101c7530()
                 ff ff
        101d476a e8 01 4e        CALL       STEAM_API.DLL::SteamAPI_RunCallbacks             undefined SteamAPI_RunCallbacks()
                 04 00
        101d476f e8 bc 23        CALL       FUN_10196b30                                     undefined FUN_10196b30()
                 fc ff
        101d4774 e8 07 89        CALL       FUN_101ad080                                     undefined FUN_101ad080()
                 fd ff
        101d4779 e8 c2 b6        CALL       FUN_1021fe40                                     undefined FUN_1021fe40()
                 04 00
        101d477e 83 3d 40        CMP        dword ptr [DAT_10538a40],0x0                     = ??
                 8a 53 10 00
        101d4785 dd 1d 00        FSTP       qword ptr [DAT_104b9500]                         = ??
                 95 4b 10
        101d478b 75 2c           JNZ        LAB_101d47b9
        101d478d e8 be 8d        CALL       SCR_UpdateScreen                                 undefined SCR_UpdateScreen()
                 07 00
        101d4792 83 3d a0        CMP        dword ptr [DAT_1124e9a0],0x0                     = ??
                 e9 24 11 00
        101d4799 74 1e           JZ         LAB_101d47b9
*/

					tmp2 = textRange.And(MemRange(refTmp2.Start - 0x34D, refTmp2.Start - 0x34D + 69));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "f2 0f 10 05 ?? ?? ?? ?? 83 ec 08 f2 0f 11 04 24 e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? f2 0f 10 05 ?? ?? ?? ?? 83 c4 04 66 0f 5a c0 f3 0f 11 04 24 e8 ?? ?? ?? ?? 83 c4 04 e8 ?? ?? ?? ??").IsEmpty())
						{
							AFXADDR_SET(host_frametime, *(DWORD *)(tmp2.Start + 4));
							AFXADDR_SET(CL_EmitEntities, *(DWORD *)(tmp2.Start + 69 -0x5 -0x4) + (DWORD)(tmp2.Start + 0x69 - 0x5)); // Decode Call
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

	// CL_Disconnect // [16] // Checked: 2023-11-22
	{
		MemRange s1 = FindCString(data1Range, "ExitGame");
		if (!s1.IsEmpty())
		{
			MemRange s1Ref = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));
			if(!s1Ref.IsEmpty()) {
				MemRange r1 = FindPatternString(MemRange(s1Ref.Start-0x89,s1Ref.Start).And(textRange), "55 8B EC");
				if (!r1.IsEmpty())
				{
					AFXADDR_SET(CL_Disconnect, r1.Start);
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	// p_cmd_functions // Checked: 2023-12-22
	// We are actually at the inlined Cmd_Exists in Cmd_AddCommand (shortly before the string we search) in order to get the cmd_functions root:
	{
		MemRange tmp1 = FindCString(data1Range, "Cmd_AddCommand: %s already defined\n");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x98, refTmp1.Start - 0x98 + 0x8));
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

	// UnkDrawHud* // [7] // Checked 2023-12-22
	{
		MemRange r1 = FindPatternString(textRange, "e8 ?? ?? ?? ?? 83 3d ?? ?? ?? ?? 05 75 33 83 3d ?? ?? ?? ?? 02 75 2a b8 01 00 00 00 33 f6 39 05 ?? ?? ?? ?? 6a 00 0f 44 f0 e8 ?? ?? ?? ?? 56 e8 ?? ?? ?? ?? 6a 01 e8 ?? ?? ?? ?? 8b 75 dc 83 c4 0c 83 3d ?? ?? ?? ?? 00 75 17 83 3d ?? ?? ?? ?? 01 75 09 83 3d ?? ?? ?? ?? 00 74 05 e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? 83 3d ?? ?? ?? ?? 05 75 0e 83 3d ?? ?? ?? ?? 02 75 05 e8 ?? ?? ?? ?? e8 ?? ?? ?? ??");

		if (!r1.IsEmpty())
		{
			AFXADDR_SET(UnkDrawHudInCall, *(DWORD *)(r1.Start + 1) + (DWORD)(r1.Start + 1 + 4)); // [7] // Decode Call
			AFXADDR_SET(UnkDrawHudOutCall, *(DWORD *)(r1.Start + 126) + (DWORD)(r1.Start + 126 + 4)); // [7] // Decode Call

			AFXADDR_SET(UnkDrawHudIn, r1.Start); // [7]
			AFXADDR_SET(UnkDrawHudInContinue, AFXADDR_GET(UnkDrawHudIn) + 0x5); // [7]
			AFXADDR_SET(UnkDrawHudOut, r1.Start + 125); // [7]
			AFXADDR_SET(UnkDrawHudOutContinue, AFXADDR_GET(UnkDrawHudOut) + 0x5); // [7]
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// R_PushDlights // [7] // Checked 2023-12-31
	{
		MemRange r1 = FindPatternString(textRange, "f3 0f 10 05 ?? ?? ?? ?? 0f 57 d2 0f 2e c2 9f f6 c4 44 7a ?? a1 ?? ?? ?? ?? 53 56 57 bf 01 00 00 00 40 a3 ?? ?? ?? ?? be ?? ?? ?? ?? 8d 5f 1f 90");

		if (!r1.IsEmpty())
		{
			AFXADDR_SET(R_PushDlights, r1.Start); // [7]
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// R_RenderView // [7] // Checked: 2023-12-31
	// R_DrawViewModel // [7] // Checked: 2023-12-31
	// R_PolyBlend // [7] // Checked: 2023-12-31
	// R_DrawEntitiesOnList_In, R_DrawEntitiesOnList_Out // [7] // Checked: 2024-01-04 // this one got inlined in 25th Aniversary, so we need to detour in/out
	// R_DrawParticles // [7] // Checked: 2024-01-04
	// r_refdef // [7] // Checked: 2024-01-04
	// Mod_LeafPVS // [7] // Checked: 2024-01-04
	{
		MemRange tmp1 = FindCString(data1Range, "R_RenderView: NULL worldmodel");
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				{
					MemRange tmp2 = textRange.And(MemRange(refTmp1.Start - 0x31, refTmp1.Start - 0x31 + 0x3));
					if (!tmp2.IsEmpty())
					{
						if (!FindPatternString(tmp2, "55 8B EC").IsEmpty())
						{
							AFXADDR_SET(R_RenderView, tmp2.Start);

							// Search "inside" R_RenderView function:																										 
							MemRange tmp3 = FindPatternString(MemRange(AFXADDR_GET(R_RenderView), textRange.End),  "e8 ?? ?? ?? ?? 83 3d ?? ?? ?? ?? ?? 75 ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ??");

							if (!tmp3.IsEmpty())
							{
								DWORD addr_R_RenderScene = *(DWORD *)(tmp3.Start + 1) + (DWORD)(tmp3.Start + 1 + 4); // Decode Call
								AFXADDR_SET(R_DrawViewModel, *(DWORD *)(tmp3.Start + 15) + (DWORD)(tmp3.Start + 15 + 4)); // Decode Call
								AFXADDR_SET(R_PolyBlend, *(DWORD *)(tmp3.Start + 20) + (DWORD)(tmp3.Start + 20 + 4)); // Decode Call

								// Look into R_RenderScene:
								MemRange range_R_RenderScene = textRange.And(MemRange(addr_R_RenderScene, addr_R_RenderScene +0x4D6));
								MemRange tmp4 = FindPatternString(range_R_RenderScene, "c7 05 ?? ?? ?? ?? 00 00 00 00 c7 05 ?? ?? ?? ?? 00 00 00 00 c7 05 ?? ?? ?? ?? 00 00 00 00 e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ??");
								if (!tmp4.IsEmpty())
								{
									DWORD addr_R_SetupGL = *(DWORD *)(tmp4.Start + 31) + (DWORD)(tmp4.Start + 31 +  4); // Decode Call
									DWORD addr_R_MarkLeaves = *(DWORD *)(tmp4.Start + 41) + (DWORD)(tmp4.Start + 41 +4); // Decode Call
									
									{
										/*
											102443b4 56              PUSH       ESI
											102443b5 33 f6           XOR        ESI,ESI
											102443b7 39 35 40        CMP        dword ptr [DAT_11253740],ESI                     = ??
													37 25 11
											102443bd 0f 8e 2f        JLE        LAB_102444f2
													01 00 00
										*/
										MemRange rangeIn = FindPatternString(MemRange(tmp4.End, range_R_RenderScene.End), "56 33 f6 39 35 ?? ?? ?? ?? 0f 8e 2f 01 00 00");

										/*
											We will detour on XOR ESI,ESI AND MOV (no need to adjust offsets), which is after the inlined R_DrawEntitiesOnList.

											102444ec 0f 8c d1        JL         LAB_102443c3
													fe ff ff
																LAB_102444f2                                    XREF[1]:     102443bd(j)  
											102444f2 33 f6           XOR        ESI,ESI
											102444f4 c7 05 b4        MOV        dword ptr [DAT_111c50b4],0x3f800000              = ??
													50 1c 11 
													00 00 80 3f																			
										
										*/
										MemRange rangeOut = FindPatternString(rangeIn.IsEmpty() ? MemRange::FromEmpty() : MemRange(rangeIn.End, range_R_RenderScene.End), "0f 8c ?? ?? ?? ?? 33 f6 c7 05 ?? ?? ?? ?? 00 00 80 3f");

										if(!rangeIn.IsEmpty() && !rangeOut.IsEmpty()) {
											AFXADDR_SET(R_DrawEntitiesOnList_In, rangeIn.Start);
											AFXADDR_SET(R_DrawEntitiesOnList_Out, rangeOut.Start + 6);

											MemRange range = FindPatternString(MemRange(rangeOut.End, range_R_RenderScene.End), "75 ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ??");
											if(!range.IsEmpty()) {
												AFXADDR_SET(R_DrawParticles, *(DWORD *)(tmp4.Start + 13) + (DWORD)(tmp4.Start + 13 +  4)); // Decode Call
											} else ErrorBox(MkErrStr(__FILE__, __LINE__));

										} else ErrorBox(MkErrStr(__FILE__, __LINE__));
									}

									/* Look into R_SetupGL:
										...
										102450df 8b 0d 20        MOV        ECX,dword ptr [DAT_10dc6320]                     = ?? <--
												63 dc 10
										102450e5 8b 3d 88        MOV        EDI,dword ptr [DAT_109b3d88]                     = ??
												3d 9b 10
										102450eb 8b d7           MOV        EDX,EDI
										102450ed 2b 3d 2c        SUB        EDI,dword ptr [DAT_10dc632c]                     = ??
												63 dc 10
									*/
									MemRange tmp7 = FindPatternString(textRange.And(MemRange(addr_R_SetupGL, addr_R_SetupGL + 0x665)), "8b 0d ?? ?? ?? ?? 8b 3d ?? ?? ?? ?? 8b d7 2b 3d ?? ?? ?? ??");
									if (!tmp7.IsEmpty())
									{
										AFXADDR_SET(r_refdef, *(DWORD *)(tmp7.Start + 2));
									}
									else ErrorBox(MkErrStr(__FILE__, __LINE__));

									/* Look into R_MarkLeaves:
										...
										1024b73d 50              PUSH       EAX
										1024b73e 8b c6           MOV        EAX,ESI
										1024b740 68 ff 00        PUSH       0xff
												00 00
										1024b745 50              PUSH       EAX
										1024b746 e8 d5 f8        CALL       FUN_101bb020                                     undefined FUN_101bb020(undefined
												f6 ff
										1024b74b 83 c4 0c        ADD        ESP,0xc
										1024b74e eb 15           JMP        LAB_1024b765
															LAB_1024b750                                    XREF[1]:     1024b71e(j)  
										1024b750 ff 35 50        PUSH       dword ptr [DAT_113fcc50]                         = ??
												cc 3f 11
										1024b756 51              PUSH       ECX
										1024b757 e8 04 b8        CALL       Mod_LeafPVS <--
												f6 ff
									*/
									MemRange tmp8 = FindPatternString(textRange.And(MemRange(addr_R_MarkLeaves, addr_R_MarkLeaves +0x12C)), "50 8b c6 68 ff 00 00 00 50 e8 ?? ?? ?? ?? 83 c4 0c eb ?? ff 35 ?? ?? ?? ?? 51 e8 ?? ?? ?? ??");
									if (!tmp8.IsEmpty())
									{
										AFXADDR_SET(Mod_LeafPVS, *(DWORD *)(tmp8.Start + 27) + (DWORD)(tmp8.Start + 27 + 4)); // Decode Call
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

	// R_DrawSkyBox_Begin, R_DrawSkyBox_End // [11] // Checked: 2024-01-16 // this one got inlined in 25th Aniversary, so we need to detour in/out and adjust jmp addresses for out hook
	// skytextures // [11] // Checked: 2024-01-16
	{
		/*
								LAB_10251521                                    XREF[2]:     10251501(j), 10251510(j)  
			10251521 f3 0f 10        MOVSS      XMM3,dword ptr [DAT_10306750]                    = BF800000h  <-- we can safely detour here, there's enough space
					1d 50 67 
					30 10
			10251529 33 f6           XOR        ESI,ESI
			1025152b 0f 1f 44        NOP        dword ptr [EAX + EAX*0x1]
					00 00
								LAB_10251530                                    XREF[1]:     102516e0(j)  
			10251530 f3 0f 10        MOVSS      XMM0,dword ptr [ESI*0x4 + DAT_109b3c60]          = ??
					04 b5 60 
					3c 9b 10
			10251539 0f 2f 04        COMISS     XMM0,dword ptr [ESI*0x4 + DAT_109b3c20]          = ??
					b5 20 3c 
					9b 10
			10251541 0f 83 95        JNC        LAB_102516dc
					01 00 00
			10251547 f3 0f 10        MOVSS      XMM0,dword ptr [ESI*0x4 + DAT_109b3c78]          = ??
					04 b5 78 
					3c 9b 10
			10251550 0f 2f 04        COMISS     XMM0,dword ptr [ESI*0x4 + DAT_109b3c38]          = ??
					b5 38 3c 
					9b 10
			10251558 0f 83 7e        JNC        LAB_102516dc
					01 00 00
			1025155e 8b 04 b5        MOV        EAX,dword ptr [ESI*0x4 + DAT_103235e8]           = 00000001h
					e8 35 32 10
			10251565 0f 57 c0        XORPS      XMM0,XMM0
		*/
		MemRange range_R_DrawSkyBox_Begin = FindPatternString(textRange, "f3 0f 10 1d ?? ?? ?? ?? 33 f6 0f 1f 44 00 00 f3 0f 10 04 b5 ?? ?? ?? ?? 0f 2f 04 b5 ?? ?? ?? ?? 0f 83 ?? ?? ?? ?? f3 0f 10 04 b5 ?? ?? ?? ?? 0f 2f 04 b5 ?? ?? ?? ?? 0f 83 ?? ?? ?? ?? 8b 04 b5 ?? ?? ?? ?? 0f 57 c0");
		if(!range_R_DrawSkyBox_Begin.IsEmpty()) {
			/*
				102515d8 0f 87 fe        JA         LAB_102516dc
						00 00 00
				102515de 8b 04 b5        MOV        EAX,dword ptr [ESI*0x4 + DAT_103231d0]
						d0 31 32 10
				102515e5 ff 34 85        PUSH       dword ptr [EAX*0x4 + skytextures]                = ??
						2c 6a 9a 10
				102515ec e8 cf a7        CALL       FUN_1023bdc0                                     undefined FUN_1023bdc0(undefined
						fe ff
			*/
			MemRange range_skytextures = FindPatternString(MemRange(range_R_DrawSkyBox_Begin.End, range_R_DrawSkyBox_Begin.End + 0x200).And(textRange), "0f 87 ?? ?? ?? ?? 8b 04 b5 ?? ?? ?? ?? ff 34 85 ?? ?? ?? ?? e8 ?? ?? ?? ??");
			if(!range_skytextures.IsEmpty()) {
				AFXADDR_SET(skytextures, *(DWORD *)(range_skytextures.Start + 16));

				/*
										LAB_102516dc                                    XREF[3]:     10251541(j), 10251558(j), 
																									102515d8(j)  
					102516dc 46              INC        ESI
					102516dd 83 fe 06        CMP        ESI,0x6
					102516e0 0f 8c 4a        JL         LAB_10251530
							fe ff ff
					102516e6 85 ff           TEST       EDI,EDI <-- We want to detour here, but that will require us to replace the jmp bellow
					102516e8 5f              POP        EDI
					102516e9 5e              POP        ESI
					102516ea 74 0a           JZ         LAB_102516f6

				*/
				MemRange range_R_DrawSkyBox_End = FindPatternString(MemRange(range_skytextures.End, range_skytextures.End+0x100).And(textRange), "46 83 fe 06 0f 8c ?? ?? ?? ?? 85 ff 5f 5e 74 0a");
				if(!range_R_DrawSkyBox_End.IsEmpty()) {
					AFXADDR_SET(R_DrawSkyBox_Begin, range_R_DrawSkyBox_Begin.Start);
					AFXADDR_SET(R_DrawSkyBox_End, range_R_DrawSkyBox_End.Start + 0xa);

				} else ErrorBox(MkErrStr(__FILE__, __LINE__));

			} else ErrorBox(MkErrStr(__FILE__, __LINE__));
		} else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// Draw_DecalMaterial // [12] // Checked 2024-01-16
	{
		MemRange tmp1 = FindCString(data1Range, "Failed to load custom decal for player #%i:%s using default decal 0.\n"); // Find String inside Draw_DecalMaterial
		if (!tmp1.IsEmpty())
		{
			MemRange refTmp1 = FindBytes(textRange, (const char *)&tmp1.Start, sizeof(tmp1.Start));
			if (!refTmp1.IsEmpty())
			{
				MemRange r2 = FindPatternString(textRange.And(MemRange(refTmp1.Start - 0x6e, refTmp1.Start - 0x6e + 3)), "55 8B EC");
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

	// g_fov // [17] // Checked 2024-01-16
	{
		MemRange r1 = FindPatternString(textRange, "56 ff d0 83 c4 08 85 c0 74 ?? 83 3d ?? ?? ?? ?? 00 75 ?? f3 0f 10 46 0c f3 0f 11 05 ?? ?? ?? ?? f3 0f 10 46 10 f3 0f 11 05 ?? ?? ?? ?? f3 0f 10 46 14 f3 0f 11 05 ?? ?? ?? ?? f3 0f 10 46 1c f3 0f 11 05 ?? ?? ?? ??");

		if (!r1.IsEmpty())
		{
			AFXADDR_SET(g_fov, *(DWORD *)(r1.Start + 67));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	//
	// Sound system related:
	//

	// S_PaintChannels // [6] // Checked 2024-01-19
	// paintedtime // [6] // Checked 2024-01-19
	// shm // [6] // Checked 2024-01-19
	// paintbuffer // [6] // Checked 2024-01-19
	// S_TransferPaintBuffer // [6] // Checked 2024-01-19
	{
		MemRange s1 = FindCString(data1Range, "Couldn't get sound buffer status\n");

		if (!s1.IsEmpty()) {

			MemRange refS1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

			if (!refS1.IsEmpty()) {

				/*
			        101feef0 55              PUSH       EBP
        			101feef1 8b ec           MOV        EBP,ESP
        			101feef3 51              PUSH       ECX				
				*/
				MemRange r2a = FindPatternString(textRange.And(MemRange(refS1.Start - 0x125, refS1.Start - 0x125 + 0x4)), "55 8b ec 51");

				if (!r2a.IsEmpty())
				{
					AFXADDR_SET(S_Update_, r2a.Start);

					/*
						101fef0e a1 a4 0c        MOV        EAX,[shm]                                        = ??
								53 10
						101fef13 56              PUSH       ESI
						101fef14 57              PUSH       EDI
						101fef15 85 c0           TEST       EAX,EAX
						101fef17 74 13           JZ         LAB_101fef2c					
					*/

					AFXADDR_SET(shm, *(size_t *)(r2a.Start + 0x1e + 1));

					/*
						101fef44 81 3d b0        CMP        dword ptr [paintedtime],0x40000000               = ??
								2e 1c 11 
								00 00 00 40					
					*/
					AFXADDR_SET(paintedtime, *(size_t *)(r2a.Start + 0x54 + 2));

					/*
											LAB_101ff04f                                    XREF[2]:     101ff002(j), 101ff03a(j)  
						101ff04f d1 ee           SHR        ESI,0x1
						101ff051 56              PUSH       ESI
						101ff052 e8 89 2b        CALL       S_PaintChannels                                  undefined S_PaintChannels(undefi
								00 00
						101ff057 83 c4 04        ADD        ESP,0x4
						101ff05a e8 f1 4a        CALL       SNDDMA_Submit                                    undefined SNDDMA_Submit()
								00 00
						101ff05f 5f              POP        EDI
						101ff060 5e              POP        ESI
											LAB_101ff061                                    XREF[2]:     101feefb(j), 101fef08(j)  
						101ff061 8b e5           MOV        ESP,EBP
						101ff063 5d              POP        EBP
						101ff064 c3              RET					
					*/		

					MemRange r2b = FindPatternString(textRange.And(MemRange(r2a.Start + 0x15f, r2a.Start + 0x15f  + 8)), "d1 ee 56 E8 ?? ?? ?? ??");

					if (!r2b.IsEmpty())
					{
						/*
							10201be0 55              PUSH       EBP
							10201be1 8b ec           MOV        EBP,ESP
							10201be3 51              PUSH       ECX
							10201be4 8b 0d b0        MOV        ECX,dword ptr [paintedtime]                      = ??
									2e 1c 11						
						*/
						AFXADDR_SET(S_PaintChannels, *(DWORD *)(r2b.Start + 4) + (DWORD)(r2b.Start + 4 + 4));

						/*
												LAB_10201c0f                                    XREF[1]:     10201c04(j)  
							10201c0f 8b f7           MOV        ESI,EDI
							10201c11 2b f1           SUB        ESI,ECX
							10201c13 8d 04 f5        LEA        EAX,[ESI*0x8 + 0x0]
									00 00 00 00
							10201c1a 50              PUSH       EAX
							10201c1b 6a 00           PUSH       0x0
							10201c1d 68 40 91        PUSH       paintbuffer                                      = ??
									1a 11
							10201c22 e8 f9 93        CALL       memset                                           undefined memset(undefined4 para
									fb ff
						*/
						MemRange r3 = textRange.And(MemRange(AFXADDR_GET(S_PaintChannels) + 0x3a, AFXADDR_GET(S_PaintChannels) + 0x3a + 13));
						MemRange r4 = FindPatternString(r3, "50 6a 00 68 ?? ?? ?? ?? e8 ?? ?? ?? ??");
						if (!r4.IsEmpty()) {
							AFXADDR_SET(paintbuffer, *(size_t *)(r4.Start + 4));

							/*
													LAB_10201de2                                    XREF[1]:     10201dc8(j)  
								10201de2 57              PUSH       EDI
								10201de3 e8 28 00        CALL       S_TransferPaintBuffer                            undefined S_TransferPaintBuffer(
										00 00						
							*/
							MemRange r5 = FindPatternString(MemRange(r4.End, AFXADDR_GET(S_PaintChannels) + 0x225).And(textRange), "83 c4 0c 57 E8 ?? ?? ?? ??");
							if (!r5.IsEmpty()) {
								AFXADDR_SET(S_TransferPaintBuffer, *(DWORD *)(r5.Start + 5) + (DWORD)(r5.Start + 5 + 4));
							}
							else ErrorBox(MkErrStr(__FILE__, __LINE__));

						} else ErrorBox(MkErrStr(__FILE__, __LINE__));
					}
					else ErrorBox(MkErrStr(__FILE__, __LINE__));					

				} else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	// S_StartDynamicSound // [6] // Checked 2024-01-19
	{
		MemRange s1 = FindCString(data1Range, "S_StartDynamicSound: %s volume > 255");
		if (!s1.IsEmpty()) {
			MemRange refS1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));
			if (!refS1.IsEmpty()) {

				/*
						101fe3e0 55              PUSH       EBP
						101fe3e1 8b ec           MOV        EBP,ESP
						101fe3e3 83 ec 5c        SUB        ESP,0x5c
				*/
				MemRange r1 = FindPatternString(MemRange(refS1.Start - 0x9B, refS1.Start - 0x9B + 6).And(textRange), "55 8b ec 83 ec 5c");
				if(!r1.IsEmpty()) {
					AFXADDR_SET(S_StartDynamicSound, r1.Start);
				} else ErrorBox(MkErrStr(__FILE__, __LINE__));
			} else ErrorBox(MkErrStr(__FILE__, __LINE__));
		} else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	
	//
	// Demo parsing related:
	//

	// Warning: When these are updated,the hook code might need to be updated as well!
	// CL_ParseServerMessage_CmdRead // [10] // Checked 2024-01-20
	// CL_ParseServerMessage_CmdRead_DSZ // [10] // Checked 2024-01-20
	// msg_readcount // [10] // Checked 2024-01-20
	// net_message // [10] // Checked 2024-01-20
	{
		MemRange s1 = FindCString(data1Range, "CL_ParseServerMessage: Bad server message");

		if (!s1.IsEmpty()) {

			MemRange s1Ref = FindBytes(textRange, (const char *)(&s1.Start), sizeof(s1.Start));

			if (!s1Ref.IsEmpty()) {

				/*
						101a7ddc 89 9d f4        MOV        dword ptr [EBP + local_110],EBX
								fe ff ff
						101a7de2 e8 99 1d        CALL       FUN_101b9b80                                     undefined FUN_101b9b80()
								01 00
				*/
				MemRange r2 = FindPatternString(textRange.And(MemRange(s1Ref.Start + 0x12, s1Ref.Start + 0x12 + 0x11)), "89 9d ?? ?? ?? ?? e8 ?? ?? ?? ??");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(CL_ParseServerMessage_CmdRead, r2.Start);
					AFXADDR_SET(CL_ParseServerMessage_CmdRead_DSZ, 0x12);

					DWORD addMessageReadByte = *(DWORD *)(r2.Start + 7) + (DWORD)(r2.Start + 7 + 4);

					MemRange rMessageReadByte = textRange.And(MemRange(addMessageReadByte, addMessageReadByte + 0x2F));

					/*
											undefined MSG_ReadByte()                                                                                          [more]
						101b9b80 8b 0d 84        MOV        ECX,dword ptr [msg_readcount]                    = ??
								c9 24 11
						101b9b86 8d 51 01        LEA        EDX,[ECX + 0x1]
						101b9b89 3b 15 f0        CMP        EDX,dword ptr [net_message.cursize]              = ??
								5f 24 11
					*/
					MemRange r3 = FindPatternString(rMessageReadByte, "8b 0d ?? ?? ?? ?? 8d 51 01 3b 15 ?? ?? ?? ??");

					if (!r3.IsEmpty() && r3.Start == rMessageReadByte.Start) {

						AFXADDR_SET(msg_readcount, *(DWORD *)(r3.Start + 2));
						AFXADDR_SET(net_message, *(DWORD *)(r3.Start + 11) - 0x10);
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
	// Studio interface related:
	//
	// 2024-01-22
	//
	{
		MemRange s1 = FindCString(data1Range, "HUD_GetStudioModelInterface");

		if (!s1.IsEmpty()) {
			MemRange s1Ref = FindBytes(textRange, (const char*)(&s1.Start), sizeof(s1.Start));

			if (!s1Ref.IsEmpty()) {

				/*
				push    offset off_1E53248
				push    offset off_1E5330C
				push    1
				call    eax
				*/
				MemRange r2 = FindPatternString(textRange.And(MemRange(s1Ref.Start + 0x14, s1Ref.Start + 0x14 + 0xE)), "68 ?? ?? ?? ?? 68 ?? ?? ?? ?? 6A ?? FF ??");

				if (!r2.IsEmpty()) {

					AFXADDR_SET(hw_HUD_GetStudioModelInterface_pStudio, *(DWORD*)(r2.Start + 1));
					AFXADDR_SET(hw_HUD_GetStudioModelInterface_pInterface, *(DWORD*)(r2.Start + 6));
					AFXADDR_SET(hw_HUD_GetStudioModelInterface_version, *(BYTE*)(r2.Start + 11));
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
		// cstrike_UnkCrosshairFn // [1] // Checked 2024-01-24
		// cstrike_UnkCrosshairFn_mul_fac // [1] // Checked 2024-01-24
		// cstrike_UnkCrosshairFn_add_fac // [1] // Checked 2024-01-24
		{
			/*
									cstrike_UnkCrosshairFn                          XREF[2]:     FUN_10043a60:10043afe(c), 
																								FUN_10043a60:10043c15(c)  
				10043ff0 55              PUSH       EBP
				10043ff1 8b ec           MOV        EBP,ESP
				10043ff3 83 ec 0c        SUB        ESP,0xc
				10043ff6 0f 57 c9        XORPS      XMM1,XMM1
				10043ff9 53              PUSH       EBX
				10043ffa 8b 5d 0c        MOV        EBX,dword ptr [EBP + param_2]
				10043ffd 56              PUSH       ESI
				10043ffe 4b              DEC        EBX
				10043fff 57              PUSH       EDI
				10044000 8b f9           MOV        EDI,this
				10044002 83 fb 1d        CMP        EBX,0x1d
				10044005 77 7d           JA         switchD_10044007::caseD_1			
			*/
			MemRange r1 = FindPatternString(textRange, "55 8b ec 83 ec 0c 0f 57 c9 53 8b 5d 0c 56 4b 57 8b f9 83 fb 1d");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(cstrike_UnkCrosshairFn, r1.Start);

				MemRange r2 = textRange.And(MemRange(r1.Start, r1.Start + 0x760));

				/*
											LAB_10044225                                    XREF[1]:     100441e3(j)  
						10044225 0f 5a c9        CVTPS2PD   XMM1,XMM1
						10044228 8d 41 02        LEA        EAX,[this + 0x2]
						1004422b 89 47 48        MOV        dword ptr [EDI + 0x48],EAX
						1004422e 0f 28 c1        MOVAPS     XMM0,XMM1
						10044231 f2 0f 59        MULSD      XMM0,qword ptr [cstrike_UnkCrosshairFn_mul_fac]  = 39h    9
								05 50 eb 
								0c 10
						10044239 f2 0f 58        ADDSD      XMM0,qword ptr [cstrike_UnkCrosshairFn_add_fac]  = 3FB999999999999Ah
								05 48 fc 
								0b 10
						10044241 f2 0f 5c c8     SUBSD      XMM1,XMM0
						10044245 66 0f 5a c1     CVTPD2PS   XMM0,XMM1
						10044249 f3 0f 11        MOVSS      dword ptr [EDI + 0x44],XMM0
								47 44
											LAB_1004424e                                    XREF[2]:     10044215(j), 10044223(j)  
						1004424e 8b 0d 24        MOV        this,dword ptr [DAT_10117724]                    = ??
								77 11 10
						10044254 81 f9 58        CMP        this,0x258
								02 00 00
						1004425a 7e 13           JLE        LAB_1004426f
				*/

				MemRange r3 = FindPatternString(r2, "0f 28 c1 f2 0f 59 05 ?? ?? ?? ?? f2 0f 58 05 ?? ?? ?? ??");

				if (!r3.IsEmpty()) {
					AFXADDR_SET(cstrike_UnkCrosshairFn_mul_fac, *(DWORD *)(r3.Start + 7));
					AFXADDR_SET(cstrike_UnkCrosshairFn_add_fac, *(DWORD *)(r3.Start + 15));
				}
				else ErrorBox(MkErrStr(__FILE__, __LINE__));
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// cstrike_PM_CatagorizePositionFn // Checked 2024-01-20
		//
		// cstrike spectator fix, gets PM_CatagorizePosition
		/*
			PM_CatagorizePosition gets called in void PM_PlayerMove ( qboolean server ) (see pm_shared.c in SDK),
			which references the string "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n":

								switchD_1006146d::caseD_2                       XREF[3]:     10061460(j), 1006146d(j), 
								switchD_1006146d::caseD_4                                    100622c0(*)  
								switchD_1006146d::caseD_7
								switchD_1006146d::caseD_9
								switchD_1006146d::caseD_1
			10062264 ff 76 04        PUSH       dword ptr [ESI + 0x4]
			10062267 8b 86 64        MOV        EAX,dword ptr [ESI + 0x4f564]
					f5 04 00
			1006226d 51              PUSH       ECX
			1006226e 68 fc 0f        PUSH       s_Bogus_pmove_player_movetype_%i_o_100d0ffc      = "Bogus pmove player movetype %
					0d 10
			10062273 ff d0           CALL       EAX
			10062275 83 c4 0c        ADD        ESP,0xc
			10062278 5f              POP        EDI
			10062279 5e              POP        ESI
			1006227a 8b 8c 24        MOV        ECX,dword ptr [ESP + 0xdc]
					dc 00 00 00
			10062281 33 cc           XOR        ECX,ESP
			10062283 e8 96 20        CALL       FUN_100b431e                                     undefined FUN_100b431e(int param
					05 00
			10062288 8b e5           MOV        ESP,EBP
			1006228a 5d              POP        EBP
			1006228b c3              RET
								LAB_1006228c                                    XREF[2]:     10061277(j), 10061283(j)  
			1006228c e8 5f 01        CALL       FUN_100623f0                                     undefined FUN_100623f0(void)
					00 00
			10062291 e8 4a b6        CALL       PM_CatagorizePosition                            undefined PM_CatagorizePosition(
					ff ff

		*/
		{
			MemRange s1 = FindCString(data1Range, "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n");
			if (!s1.IsEmpty()) {
				MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));
				if (!r1.IsEmpty()) {
					MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start + 0x1d, r1.Start +0x1d + 10)), "e8 ?? ?? ?? ?? e8 ?? ?? ?? ??");
					if(!r2.IsEmpty()) {
						AFXADDR_SET(cstrike_PM_CatagorizePositionFn, *(size_t *)(r2.Start + 6) + (DWORD)(r2.Start + 6 + 4)); // Decode call address

					} else ErrorBox(MkErrStr(__FILE__, __LINE__));
				} else ErrorBox(MkErrStr(__FILE__, __LINE__));
			} else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}

		// cstrike_EV_CreateSmoke: // [3]
		// This is set in MypfnHookEvent.

		// cstrike DeathMsg related (client.dll offsets):
		// [2] // Checked 2024-01-26
		{
			MemRange s1 = FindCString(data1Range, "DeathMsg");

			if (!s1.IsEmpty()) {

				MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

				if (!r1.IsEmpty()) {

					MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start - 6, r1.Start +4 + 6)), "68 ?? ?? ?? ?? 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ??");

					if (!r2.IsEmpty()) {

						AFXADDR_SET(cstrike_MsgFunc_DeathMsg, *(DWORD *)(r2.Start + 1));

						MemRange r3 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(cstrike_MsgFunc_DeathMsg), AFXADDR_GET(cstrike_MsgFunc_DeathMsg) + 0x1A)), "55 8b ec ff 75 10 b9 ?? ?? ?? ?? ff 75 0c ff 75 08 e8 ?? ?? ?? ?? 5d c3");

						if (!r3.IsEmpty())
						{
							AFXADDR_SET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg, *(DWORD *)(r3.Start + 17+1) + (DWORD)(r3.Start + 17+1+4)); // Decode call address.

							MemRange r4 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg), AFXADDR_GET(cstrike_CHudDeathNotice_MsgFunc_DeathMsg) +0x553)), "68 80 02 00 00 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? e8 ?? ?? ?? ??");

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

			MemRange r1 = FindPatternString(textRange, "55 8b ec 83 ec 64 83 3d ?? ?? ?? ?? 00 8b c1 53 89 45 fc 8b 58 1c 89 5d c0 75 0a ff 15 ?? ?? ?? ?? 85 c0 74 11");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(cstrike_CHudDeathNotice_Draw, r1.Start);

				// Check that our Y-Res adjusting code is still correct (otherwise it needs to be adjusted):
				/*
					10047eec 8b 75 c8        MOV        ESI,dword ptr [EBP + local_3c]
					10047eef 8b 78 18        MOV        EDI,dword ptr [EAX + 0x18]
					10047ef2 0f af 7d f4     IMUL       EDI,dword ptr [EBP + local_10]				
				*/
				MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start, r1.Start + 0x410)), "8b 75 c8 8b 78 18 0f af 7d f4");

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
		// [14] // Checked 2024-01-26 (no change)
		{
			MemRange s1 = FindCString(data2Range, "DeathMsg");

			if (!s1.IsEmpty()) {

				MemRange r1 = FindBytes(textRange, (const char *)&s1.Start, sizeof(s1.Start));

				if (!r1.IsEmpty()) {

					MemRange r2 = FindPatternString(textRange.And(MemRange(r1.Start - 6, r1.Start + 4 + 6)), "68 ?? ?? ?? ?? 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ??");

					if (!r2.IsEmpty()) {

						AFXADDR_SET(tfc_MsgFunc_DeathMsg, *(DWORD *)(r2.Start + 1));

						MemRange r3 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(tfc_MsgFunc_DeathMsg), AFXADDR_GET(tfc_MsgFunc_DeathMsg) + 0x1A)), "8b 44 24 0c 8b 4c 24 08 8b 54 24 04 50 51 52 b9 ?? ?? ?? ?? e8 ?? ?? ?? ?? c3");

						if (!r3.IsEmpty())
						{
							AFXADDR_SET(tfc_CHudDeathNotice_MsgFunc_DeathMsg, *(DWORD *)(r3.Start + 21) + (DWORD)(r3.Start + 25)); // Decode call address.

							MemRange r4 = FindPatternString(textRange.And(MemRange(AFXADDR_GET(tfc_CHudDeathNotice_MsgFunc_DeathMsg), AFXADDR_GET(tfc_CHudDeathNotice_MsgFunc_DeathMsg) + 0x363)), "68 70 02 00 00 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? e8 ?? ?? ?? ??");

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

		// tfc_TeamFortressViewport_UpdateSpecatorPanel // [4] // Checked 2024-01-26 (no change since 2018-10-06)
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
		// valve_TeamFortressViewport_UpdateSpecatorPanel // [4] // Checked 2024-01-26
		{
			MemRange r1 = FindPatternString(textRange, "55 8b ec 81 ec 4c 02 00 00 a1 ?? ?? ?? ?? 33 c5 89 45 fc a1 ?? ?? ?? ?? 53 8b d9 89 9d b8 fd ff ff 8b 8b 18 0a 00 00 89 83 24 02 00 00 a1 ?? ?? ?? ?? 89 83 28 02 00 00 a1 ?? ?? ?? ?? 89 83 2c 02 00 00 85 c9 0f 84 e3 03 00 00 83 3d ?? ?? ?? ?? 00");

			if (!r1.IsEmpty()) {

				AFXADDR_SET(valve_TeamFortressViewport_UpdateSpecatorPanel, r1.Start);
			}
			else ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
	}
}
