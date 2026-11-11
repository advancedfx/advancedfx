#include "stdafx.h"

#include "addresses.h"
#include "Globals.h"

#include "../shared/binutils.h"

#include <string>
#include <vector>

using namespace Afx::BinUtils;

AFXADDR_DEF(cs2_engine_HostStateRequest_Start)
AFXADDR_DEF(cs2_engine_CRenderService_OnClientOutput)

AFXADDR_DEF_INVALID(cs2_scenesystem_CSceneSystem_WaitForRenderingToComplete_vtable_idx)
AFXADDR_DEF_INVALID(cs2_scenesystem_CSceneSystem_FrameUpdate_vtable_idx)

AFXADDR_DEF_INVALID(cs2_client_CCSGO_HudDeathNotice_lifetime_offset)
AFXADDR_DEF_INVALID(cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset)

AFXADDR_DEF(cs2_client_RenderInitScopeOverlay)

AFXADDR_DEF(cs2_client_CCSGO_HudDeathNotice_HandlePlayerDeath)
AFXADDR_DEF(cs2_client_HashString)
AFXADDR_DEF(cs2_client_GetLocalSteamId)
AFXADDR_DEF_INVALID(cs2_client_CPanel2D_GetClientClass_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CUIPanel_GetAttributeString_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CUIPanel_SetAttributeString_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CUIEngine_MakeSymbol_vtable_idx)
AFXADDR_DEF(cs2_client_g_pHudPanel)
AFXADDR_DEF(cs2_client_g_pUIEngine)

AFXADDR_DEF(cs2_client_CGlowProperty_SetGlowColor)
AFXADDR_DEF(cs2_client_ApplySmokeProps)
AFXADDR_DEF(cs2_client_RenderSmokes)
AFXADDR_DEF(cs2_client_GetParticleManager)
AFXADDR_DEF(cs2_client_DrawStuff)
AFXADDR_DEF(cs2_client_CreateParticle)
AFXADDR_DEF(cs2_client_UpdateParticle)

AFXADDR_DEF(cs2_client_OnFlashMaxAlphaChanged)
AFXADDR_DEF(cs2_client_EOM)
AFXADDR_DEF(cs2_client_SetGlowProps)
AFXADDR_DEF(cs2_client_CGlowProperty_ShouldGlow)
AFXADDR_DEF(cs2_client_C_EnvSky_ForceUpdateSkybox)
AFXADDR_DEF_INVALID(cs2_client_C_EnvSky_UnkPtr_offset)

AFXADDR_DEF(cs2_client_ViewmodelFunc)
AFXADDR_DEF(cs2_client_ViewmodelHandFunc)

AFXADDR_DEF(cs2_client_GetDecoratedPlayerName)
AFXADDR_DEF_INVALID(cs2_client_GetDecoratedPlayerName_GetUserId_vtable_idx)
AFXADDR_DEF(cs2_client_CCSPlayerController_GetPlayerName)

AFXADDR_DEF_INVALID(cs2_client_CEntityInstance_GetClientClass_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CEntityInstance_IsPlayerPawn_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CEntityInstance_IsPlayerController_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CEntityInstance_GetRenderEyeOrigin_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CEntityInstance_GetRenderEyeAngles_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CGameEntitySystem_OnAddEntity_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CGameEntitySystem_OnRemoveEntity_vtable_idx)
AFXADDR_DEF(cs2_client_g_pEntityList)
AFXADDR_DEF(cs2_client_CGameEntitySystem_GetHighestEntityIterator)
AFXADDR_DEF(cs2_client_CGameEntitySystem_GetEntityFromIndex)
AFXADDR_DEF(cs2_client_LookupAttachment)
AFXADDR_DEF(cs2_client_GetAttachment)

AFXADDR_DEF(cs2_client_CGameEventManager_CreateEvent)
AFXADDR_DEF(cs2_client_CGameEventManager_FireEvent)
AFXADDR_DEF(cs2_client_CGameEventManager_FireEventClientSide)
AFXADDR_DEF(cs2_client_CGameEventManager_FreeEvent)

AFXADDR_DEF(cs2_client_CViewRender_SetUpView_IsPlayingDemo)
AFXADDR_DEF(cs2_client_CViewRender_UnkMakeMatrix)
AFXADDR_DEF(cs2_client_GetSplitScreenPlayer)
AFXADDR_DEF_INVALID(cs2_client_CSource2Client_Connect_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CSource2Client_Init_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CSource2Client_Shutdown_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CSource2Client_SetGlobals_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CSource2Client_LevelInitPreEntity_vtable_idx)
AFXADDR_DEF_INVALID(cs2_client_CSource2Client_FrameStageNotify_vtable_idx)

AFXADDR_DEF(cs2_panorama_CLayoutFile_LoadFromFile)
AFXADDR_DEF(cs2_panorama_CStylePropertyForegroundColor_Parse)
AFXADDR_DEF(cs2_panorama_CStylePropertyBackgroundColor_Parse)
AFXADDR_DEF(cs2_panorama_CStylePropertyBorder_Parse)
AFXADDR_DEF(cs2_panorama_CStylePropertyWashColor_Clone)
AFXADDR_DEF(cs2_panorama_CStylePropertyWashColor_Parse)
AFXADDR_DEF(cs2_panorama_CStylePropertyOpacity_vtable)
AFXADDR_DEF(cs2_panorama_CStyleSymbol_Resolve)
AFXADDR_DEF(cs2_panorama_g_StylePropertySymbols)
AFXADDR_DEF(cs2_panorama_CPanelStyle_SetStyleProperty)

namespace {

MemRange GetTextRange(AfxAddr dll) {
	ImageSectionsReader imageSectionsReader((HMODULE)dll);
	if (!imageSectionsReader.Eof())
		return imageSectionsReader.GetMemRange();

	ErrorBox(MkErrStr(__FILE__, __LINE__));
	return MemRange(0, 0);
}

/// <returns>Start of the first match or 0 if not found.</returns>
AfxAddr FindPattern(MemRange textRange, char const * pattern) {
	MemRange result = FindPatternString(textRange, pattern);
	return result.IsEmpty() ? 0 : result.Start;
}

/// <summary>Resolves a RIP relative target (e.g. of a CALL, JMP, LEA or MOV).</summary>
/// <param name="dispOfs">Offset of the int32 displacement from addr.</param>
/// <param name="instrEndOfs">Offset of the end of the instruction from addr.</param>
AfxAddr ResolveRel32(AfxAddr addr, size_t dispOfs, size_t instrEndOfs) {
	return addr + instrEndOfs + *(int32_t*)(addr + dispOfs);
}

/// <returns>Function at index of the vtable of mangledClass or 0 if the vtable is not found.</returns>
AfxAddr FindVtableFn(AfxAddr dll, char const * mangledClass, size_t index) {
	if (void ** vtable = (void **)FindClassVtable((HMODULE)dll, mangledClass, 0, 0))
		return (AfxAddr)vtable[index];
	return 0;
}

/// <summary>Collects addresses that could not be found in a DLL and reports them.</summary>
/// <remarks>Each missing address is printed as a warning to the console right away.
///   Required ones are also collected and shown in a single error box when Report() is called.</remarks>
class CMissingAddrs {
public:
	CMissingAddrs(char const * dllName) : m_DllName(dllName) {
	}

	void Add(char const * name, bool required = true) {
		advancedfx::Warning("AFXERROR: Could not find %s in %s.\n", name, m_DllName);
		if (required) m_Required.emplace_back(name);
	}

	void Report() {
		if (m_Required.empty()) return;

		std::string text = "Could not find the following in ";
		text += m_DllName;
		text += " (probably due to a game update), related features will not work:\n";
		for (char const * name : m_Required) {
			text += "\n";
			text += name;
		}
		ErrorBox(text.c_str());
	}

private:
	char const * m_DllName;
	std::vector<char const *> m_Required;
};

} // namespace {

#define ADDR_MISSING(name) missing.Add(#name, true)
#define ADDR_MISSING_OPTIONAL(name) missing.Add(#name, false)

void Addresses_InitEngine2Dll(AfxAddr engine2Dll)
{
	MemRange textRange = GetTextRange(engine2Dll);
	CMissingAddrs missing("engine2.dll");

    /*  cs2_engine_HostStateRequest_Start
        The function in question references this string: "HostStateRequest::Start(HSR_QUIT)\n"
                                FUN_180217fc0                                   XREF[4]:     FUN_18021a010:18021a18e(c),
                                                                                            1805ba12c(*), 1805ba13c(*),
                                                                                            18091557c(*)
        180217fc0 40 53           PUSH       RBX
        180217fc2 48 83 ec 40     SUB        RSP,0x40
        180217fc6 8b 01           MOV        EAX,dword ptr [RCX]
        180217fc8 48 8b d9        MOV        RBX,RCX
        180217fcb c6 41 18 01     MOV        byte ptr [RCX + 0x18],0x1
        180217fcf 83 f8 02        CMP        EAX,0x2
        180217fd2 74 07           JZ         LAB_180217fdb
        180217fd4 83 f8 04        CMP        EAX,0x4
        180217fd7 75 21           JNZ        LAB_180217ffa
        180217fd9 eb 0d           JMP        LAB_180217fe8
        [....]
        180218037 84 c0           TEST       AL,AL
        180218039 74 18           JZ         LAB_180218053
        18021803b 8b 0d f7        MOV        ECX,dword ptr [DAT_1808ec238]
                    41 6d 00
        180218041 4c 8d 05        LEA        R8,[s_HostStateRequest::Start(HSR_QUIT_1805648   = "HostStateRequest::Start(HSR_Q
                    d8 c7 34 00
        180218048 ba 02 00        MOV        EDX,0x2
                    00 00
        18021804d ff 15 fd        CALL       qword ptr [->TIER0.DLL::LoggingSystem_Log]       = 005e034a
                    17 25 00
        [....]
    */
    {
		MemRange result = FindPatternString(textRange, "40 53 48 83 ec 40 8b 01 48 8b d9 c6 41 18 01 83 f8 02 74 07 83 f8 04 75 21 eb 0d");
		if (!result.IsEmpty()) AFXADDR_SET(cs2_engine_HostStateRequest_Start, result.Start);
		else ADDR_MISSING(cs2_engine_HostStateRequest_Start);
    }

    /*  cs2_engine_CRenderService_OnClientOutput

        To find search for references to the strings given bellow.

                 24 18
       1801e5715 55              PUSH       RBP
       1801e5716 56              PUSH       RSI
       1801e5717 57              PUSH       RDI
       1801e5718 41 54           PUSH       R12
       1801e571a 41 56           PUSH       R14
       1801e571c 48 83 ec 70     SUB        RSP,0x70
       1801e5720 48 8d 05        LEA        RAX,[s_C:\buildworker\csgo_rel_win64\bu_18055a   = "C:\\buildworker\\csgo_rel_win
                 49 53 37 00
       1801e5727 48 c7 44        MOV        qword ptr [RSP + local_68[8]],0x19d
                 24 38 9d
                 01 00 00
       1801e5730 48 89 44        MOV        qword ptr [RSP + local_68[0]],RAX=>s_C:\buildw   = "C:\\buildworker\\csgo_rel_win
                 24 30
       1801e5735 4c 8d 44        LEA        R8=>local_48,[RSP + 0x50]
                 24 50
       1801e573a 0f 10 44        MOVUPS     XMM0,xmmword ptr [RSP + local_68[0]]
                 24 30
       1801e573f 48 8d 05        LEA        RAX,[s_OnClientOutput_18055a7c8]                 = "OnClientOutput"
                 82 50 37 00
       1801e5746 4c 8b f2        MOV        R14,RDX
       1801e5749 48 89 44        MOV        qword ptr [RSP + local_58],RAX=>s_OnClientOutp   = "OnClientOutput"
                 24 40
       1801e574e 48 8d 15        LEA        RDX,[PTR_s_Client_Rendering_1805f1050]           = 18055aab8
                 fb b8 40 00
       1801e5755 f2 0f 10        MOVSD      XMM1,qword ptr [RSP + local_58]
                 4c 24 40
       1801e575b 48 8b f1        MOV        RSI,RCX
       1801e575e 48 8d 0d        LEA        RCX,[s_RenderService::OnClientOutput_18055aa48]  = "RenderService::OnClientOutput"
                 e3 52 37 00
       1801e5765 f2 0f 11        MOVSD      qword ptr [RSP + local_38],XMM1=>s_OnClientOut   = "OnClientOutput"
                 4c 24 60
       1801e576b 33 ff           XOR        EDI,EDI
       1801e576d 0f 29 44        MOVAPS     xmmword ptr [RSP + local_48[0]],XMM0=>DAT_1805
                 24 50
       1801e5772 ff 15 f8        CALL       qword ptr [->TIER0.DLL::VProfScopeHelper<0,0>:   = 005e0b32
                 3e 28 00
       1801e5778 48 8b 96        MOV        RDX,qword ptr [RSI + 0x1c0]
                 c0 01 00 00
    */
	{
		// 49 c7 43 b0 xx xx 00 00 is a source line number (VProf scope), it changes with engine updates.
		MemRange result = FindPatternString(textRange, "4c 8b dc 49 89 5b 10 49 89 6b 18 49 89 73 20 57 41 56 41 57 48 83 ec 70 49 c7 43 b0 ?? ?? 00 00");
		if (!result.IsEmpty()) AFXADDR_SET(cs2_engine_CRenderService_OnClientOutput, result.Start);
		else ADDR_MISSING(cs2_engine_CRenderService_OnClientOutput);
	}

	missing.Report();
}

void Addresses_InitSceneSystemDll(AfxAddr sceneSystemDll) {

    /*cs2_scenesystem_CSceneSystem_WaitForRenderingToComplete_vtable_idx

       To find the right function search for references to strings:
       - "WaitForRenderingToComplete".
    */
    AFXADDR_SET(cs2_scenesystem_CSceneSystem_WaitForRenderingToComplete_vtable_idx, 26);

    /*cs2_scenesystem_CSceneSystem_FrameUpdate_vtable_idx

       To find the right function search for references to strings:
       - "FrameUpdate"
       - "CSceneSystem::FrameUpdate"
       - "Invalid width/height for ScratchTarget, Size=%i, Width=%i/Height=%i"
    */
    AFXADDR_SET(cs2_scenesystem_CSceneSystem_FrameUpdate_vtable_idx, 74);
}

void Addresses_InitClientDll(AfxAddr clientDll) {
	MemRange textRange = GetTextRange(clientDll);
	CMissingAddrs missing("client.dll");

	/*  cs2_client_CCSGO_HudDeathNotice_lifetime_offset
	    cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset

	    In the end of g_Original_handlePlayerDeath function (see DeatMsg.cpp):

            FUN_180e05690(lVar11,DAT_182413994,*(undefined4 *)(PTR_DAT_18208ed60 + 0x30));
            FUN_180e05690(lVar11,DAT_18241399c,*(undefined4 *)(param_1 + 0x78));
            if ((cVar4 == '\0') && ((char)uVar19 == '\0')) {
            uVar5 = 0x3f800000;
            }
            else {
            uVar5 = *(undefined4 *)(param_1 + 0x7c);
            }
            FUN_180e05690(lVar11,DAT_1824139a4,uVar5);
            return;
	*/
	{
		MemRange result = FindPatternString(textRange, "f3 0f 10 53 ?? e8 ?? ?? ?? ?? 45 84 e4 75 0f 45 84 ff 75 ?? f3 0f 10 15 ?? ?? ?? ?? eb 05 f3 0f 10 53 ??");
		if (!result.IsEmpty()) {
			AFXADDR_SET(cs2_client_CCSGO_HudDeathNotice_lifetime_offset, *(uint8_t*)(result.Start + 4));
			AFXADDR_SET(cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset, *(uint8_t*)(result.Start + 34));
		}
		else {
			ADDR_MISSING(cs2_client_CCSGO_HudDeathNotice_lifetime_offset);
			ADDR_MISSING(cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset);
		}
	}

      /*    cs2_client_RenderInitScopeOverlay

            The function is called in the render pipleline function that refrences strings such as "Create PerViewConstantBufferCsgo_t",
            "intermediate_depth_rt", "intermediate_color_rt", somewhat after the string ref "Engine2/RenderingPipeline/AmbientOcclusionProxies"
            in a pattern that looks like this:

            _local_1ed8 = ZEXT816(0);
            pfVar23 = (float *)FUN_18036dc30(local_1e60,local_e98,0x978903ad,local_1ed8);
            fVar2 = *pfVar23;
            FUN_1811d0100(local_14d8);
            if ((local_1ce7 == '\0') && (lVar24 = FUN_180c7d0d0(), lVar24 != 0)) {
                  FUN_1808e2410(lVar24,local_14d8); // This is the RenderScopeOverlay function we want to hook.
            }
            ppcVar22 = (code **)0x0


            void FUN_1808e2410(longlong *param_1,undefined4 *param_2)
            {
                  // ...
                  local_res10[0] = 0x3f800000;
                  local_res18[0] = 0;
                  FUN_18075e050(local_res8,*(undefined4 *)(param_1[2] + 0x38));
                  // ...
            }
      */
	{
		MemRange result = FindPatternString(textRange, "48 8b c4 53 57 48 83 ec 68 48 8b fa 44 0f 29 40 b8");
		if (!result.IsEmpty()) AFXADDR_SET(cs2_client_RenderInitScopeOverlay, result.Start);
		else ADDR_MISSING(cs2_client_RenderInitScopeOverlay);
	}

	//
	// mirv_deathmsg (DeathMsg.cpp)

	// can be found with strings like "attacker" and "userid", etc. it basically takes all info from player_death event, called by rbanch in a function that references "player_death", "realtime_passthrough"
	if (AfxAddr addr = FindPattern(textRange, "48 89 4C 24 ?? 55 53 41 55 41 56 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 02"))
		AFXADDR_SET(cs2_client_CCSGO_HudDeathNotice_HandlePlayerDeath, addr);
	else ADDR_MISSING(cs2_client_CCSGO_HudDeathNotice_HandlePlayerDeath);

	// called in multiple places with strings like "userid", "attacker", etc. as first argument, length as second argument and length XOR 0x31415926
	// e.g. in function above too
	if (AfxAddr addr = FindPattern(textRange, "48 83 EC 28 45 8B D0 4C 8B C9 48 83 FA 04 0F 82 ?? ?? ?? ?? 0F B6 09 48 89 5C 24 20 8D 41 BF 3C 19 77 03 80 C1 20"))
		AFXADDR_SET(cs2_client_HashString, addr);
	else ADDR_MISSING(cs2_client_HashString);

	// snippet from function handlePlayerDeath above
	//   if (*(char *)(lVar17 + 0xb8) == '\0') {
	//     uVar18 = FUN_1808a1a00();
	//
	//     iVar10 = FUN_1808af610(uVar18); // the one we need called here, it returns local steamid, function has 2 xrefs
	//									   // later there is check if attackersteamid is equal to local one
	//
	//     if (((iVar10 != 0) && (plVar14 != (longlong *)0x0)) &&
	//        (piVar13 = (int *)FUN_18056a170(plVar14,&uStackX_20), *piVar13 == iVar10)) {
	//       bVar4 = true;
	//     }
	//   }
	if (AfxAddr addr = FindPattern(textRange, "40 53 48 83 EC ?? 8B 51 ?? 48 8B D9 83 FA FF 0F 84 ?? ?? ?? ?? 4C 8B 0D ?? ?? ?? ??"))
		AFXADDR_SET(cs2_client_GetLocalSteamId, addr);
	else ADDR_MISSING(cs2_client_GetLocalSteamId);

	// In case it breaks see nearby functions in vtable, it just returns DAT.
	AFXADDR_SET(cs2_client_CPanel2D_GetClientClass_vtable_idx, 61);

	// Panorama (credit https://github.com/danielkrupinski/Osiris)

	/* In the middle of big function with MULTIPLE (3+) references to "Attempted to cast panel '%s' to type '%s'" and multiple to "file://{images}/%s.png":
        }
LAB_1809a7daf:
        if (DAT_181fc46d8 != (longlong *)0x0) {
          local_230 = FUN_1809bf960; <-- next 2 sigs in this one!!!
          local_238 = lVar12;
          (**(code **)(*DAT_181fc46d8 + 0x120))(DAT_181fc46d8,DAT_181c51f4c,plVar11,&local_238);
        }
      }
LAB_1809a7de1
	*/
	/*
                             LAB_1809bf9e6                                   XREF[1]:     1809bf9d9(j)
       1809bf9e6 48 8b 4f 08     MOV        RCX,qword ptr [RDI + 0x8]
       1809bf9ea 4c 8d 05        LEA        R8,[DAT_1814ed000]
                 0f d6 b2 00
       1809bf9f1 0f b7 12        MOVZX      EDX=>DAT_181e808b8,word ptr [RDX]
       1809bf9f4 48 8b 01        MOV        RAX,qword ptr [RCX]
       1809bf9f7 ff 90 d0        CALL       qword ptr [RAX + 0x8d0]
                 08 00 00
       1809bf9fd 48 8b f0        MOV        RSI,RAX
       1809bfa00 48 85 c0        TEST       RAX,RAX
	*/
	if (AfxAddr addr = FindPattern(textRange, "48 8b 4f 08 4c 8d 05 ?? ?? ?? ?? 0f b7 12 48 8b 01 ff 90 ?? ?? ?? ?? 48 8b f0 48 85 c0"))
		AFXADDR_SET(cs2_client_CUIPanel_GetAttributeString_vtable_idx, *(int32_t*)(addr + 19) / sizeof(void*));
	else ADDR_MISSING(cs2_client_CUIPanel_GetAttributeString_vtable_idx);

	/*
                             LAB_1809bfa6a                                   XREF[1]:     1809bfa5d(j)
       1809bfa6a 48 8b 4f 08     MOV        RCX,qword ptr [RDI + 0x8]
       1809bfa6e 4c 8d 05        LEA        R8,[DAT_1814ed000]
                 8b d5 b2 00
       1809bfa75 0f b7 13        MOVZX      EDX,word ptr [RBX]=>DAT_181e808b8
       1809bfa78 48 8b 01        MOV        RAX,qword ptr [RCX]
       1809bfa7b ff 90 00        CALL       qword ptr [RAX + 0x900]
                 09 00 00
       1809bfa81 b0 01           MOV        AL,0x1
       1809bfa83 e9 1a ff        JMP        LAB_1809bf9a2
                 ff ff
	*/
	if (AfxAddr addr = FindPattern(textRange, "48 8b 4f 08 4c 8d 05 ?? ?? ?? ?? 0f b7 13 48 8b 01 ff 90 ?? ?? ?? ?? b0 01 e9 ?? ?? ?? ??"))
		AFXADDR_SET(cs2_client_CUIPanel_SetAttributeString_vtable_idx, *(int32_t*)(addr + 19) / sizeof(void*));
	else ADDR_MISSING(cs2_client_CUIPanel_SetAttributeString_vtable_idx);

	// "Can\'t call Panorama Symbol constructor outside panorama.dll until UIEngine is i nitialized! Symbol: %s"
	if (AfxAddr addr = FindPattern(textRange, "48 8B 01 4C 8B C3 BA ?? ?? ?? ?? FF 90 ?? ?? ?? ?? 48 8B 5C 24 ?? 66 89 07"))
		AFXADDR_SET(cs2_client_CUIEngine_MakeSymbol_vtable_idx, *(int32_t*)(addr + 13) / sizeof(void*));
	else ADDR_MISSING(cs2_client_CUIEngine_MakeSymbol_vtable_idx);

	// function has "file://{resources}/layout/hud/hud.xml" string and also references CCSGO_Hud vftable
	// hudpanel is DAT that param_1 assigned to
	if (AfxAddr addr = FindPattern(textRange, "48 89 86 ?? ?? ?? ?? 48 89 35 ?? ?? ?? ??"))
		AFXADDR_SET(cs2_client_g_pHudPanel, ResolveRel32(addr, 10, 14));
	else ADDR_MISSING(cs2_client_g_pHudPanel);

	// function has CreatePanelWithCurrentContext string
	// engine is DAT that param_1 assigned to
	if (AfxAddr addr = FindPattern(textRange, "48 89 78 ?? 48 89 0D ?? ?? ?? ??"))
		AFXADDR_SET(cs2_client_g_pUIEngine, ResolveRel32(addr, 7, 11));
	else ADDR_MISSING(cs2_client_g_pUIEngine);

	//
	// mirv_colors (MirvColors.cpp)

	// called with offset to m_Glow of C_BaseModelEntity as first argument
	// Since 2026-09-23 the color is compared as one dword (cmp [rcx], edx) instead of 4 byte compares.
	if (AfxAddr addr = FindPattern(textRange, "40 53 48 83 EC 20 48 8B D9 48 83 C1 40 39 11 74 02 89 11 E8 ?? ?? ?? ?? 48 8B 4B 18 48 85 C9"))
		AFXADDR_SET(cs2_client_CGlowProperty_SetGlowColor, addr);
	else ADDR_MISSING(cs2_client_CGlowProperty_SetGlowColor);

	// can be found with offsets to m_vSmokeColor and m_vSmokeDetonationPos
	if (AfxAddr addr = FindPattern(textRange, "40 53 48 83 EC ?? 8B 91 ?? ?? ?? ?? 48 8B D9 85 D2 75"))
		AFXADDR_SET(cs2_client_ApplySmokeProps, addr);
	else ADDR_MISSING(cs2_client_ApplySmokeProps);

	//   has "SmokeRenderer", "LockDynamicConstantBuffer failed in %s", "Unable to render more than %d smokes.\n"
	//   see also 1 fn in ".?AVCSmokeVolumeSceneObjectDesc@@" vtable, it's in the last else block
	if (AfxAddr addr = FindPattern(textRange, "48 89 54 24 ?? 55 57 41 54 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ??"))
		AFXADDR_SET(cs2_client_RenderSmokes, addr);
	else ADDR_MISSING(cs2_client_RenderSmokes);

	// can be found close to functions below before calling them
	if (AfxAddr addr = FindPattern(textRange, "48 8B 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 10 57"))
		AFXADDR_SET(cs2_client_GetParticleManager, addr);
	else ADDR_MISSING(cs2_client_GetParticleManager);

	// next three functions can be found with "particles/entity/spectator_utility_trail.vpcf" or with offsets to m_nSnapshotTrajectoryEffectIndex, etc.
	if (AfxAddr addr = FindPattern(textRange, "40 55 53 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 80 B9"))
		AFXADDR_SET(cs2_client_DrawStuff, addr);
	else ADDR_MISSING(cs2_client_DrawStuff);

	if (AfxAddr addr = FindPattern(textRange, "4C 8B DC 53 48 81 EC ?? ?? ?? ?? F2 0F 10 05"))
		AFXADDR_SET(cs2_client_CreateParticle, addr);
	else ADDR_MISSING(cs2_client_CreateParticle);

	if (AfxAddr addr = FindPattern(textRange, "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? F3 0F 10 1D ?? ?? ?? ?? 41 8B F8 8B DA 4C 8D 05"))
		AFXADDR_SET(cs2_client_UpdateParticle, addr);
	else ADDR_MISSING(cs2_client_UpdateParticle);

	//
	// MirvCommands.cpp

	// near "OnFlashMaxAlphaChanged"
	// checks for m_flFlashMaxAlpha in param_2 and sets it to param_3 if it's not the same
	if (AfxAddr addr = FindPattern(textRange, "F3 41 0F 10 08 F3 0F 10 82 ?? ?? ?? ?? 0F 2E C1 7A ?? 74 ??"))
		AFXADDR_SET(cs2_client_OnFlashMaxAlphaChanged, addr);
	else ADDR_MISSING(cs2_client_OnFlashMaxAlphaChanged);

	// called in func with "cs_win_panel_match", "cs_game_disconnected", "cs_match_end_restart","nextlevel_changed","hltv_replay" in the end inside if statement
	// has strings "team_intro_end", "hide_deathpanel"
	if (AfxAddr addr = FindPattern(textRange, "48 8B C4 88 50 ?? 55 41 56 48 8B EC 48 83 EC ?? 80 79 ?? 00 4C 8B F1"))
		AFXADDR_SET(cs2_client_EOM, addr);
	else ADDR_MISSING(cs2_client_EOM);

	// See where spec_show_xray is checked, has offsets to glowProperty
	// Also called first in 242 vtable function for C_BaseModelEntity and other 100+ entities with m_Glow (CGlowProperty)
	if (AfxAddr addr = FindPattern(textRange, "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B 05 ?? ?? ?? ?? 48 8B D9 F3 0F 10 41"))
		AFXADDR_SET(cs2_client_SetGlowProps, addr);
	else ADDR_MISSING(cs2_client_SetGlowProps);

	// C_BaseModelEntity vtable 242 it should have 2 function calls with m_Glow (CGlowProperty)
	// go to second function call, there go to first function call
	// that function should return m_bGlowing of CGlowProperty
	if (AfxAddr addr = FindPattern(textRange, "E8 ?? ?? ?? ?? 45 33 F6 84 C0 0F 84"))
		AFXADDR_SET(cs2_client_CGlowProperty_ShouldGlow, ResolveRel32(addr, 1, 5));
	else ADDR_MISSING(cs2_client_CGlowProperty_ShouldGlow);

	// Has offset to material of skybox (other members too), pCSceneSystem and it's function to update skybox.
	//
	// Could be found if you see callstack for function of vtable for CSceneSystem (currently 39th)
	// To trigger update just skip demo backwards or load level.
	//
	// Also it's referenced in 10th vtable function of C_EnvSky (we are getting it from there)
	//
	// 1801c02cb 48  8b  0d       MOV        RCX, qword ptr [DAT_182025518] // CSceneSystem
	//           46  52  e6  01
	// 1801c02d2 48  8d  55  30   LEA        RDX => local_res8, [RBP + 0x30]
	// 1801c02d6 48  8b  01       MOV        RAX, qword ptr [RCX]
	// 1801c02d9 4c  8b  88       MOV        R9, qword ptr [RAX + 0x138] // 39th function of CSceneSystem
	//           38  01  00  00
	// 1801c02e0 48  8b  87       MOV        RAX, qword ptr [RDI + 0xec0] // offset to m_hSkyMaterial of C_EnvSky
	//           c0  0e  00  00
	// 1801c02e7 48  89  45  30   MOV        qword ptr [RBP + local_res8], RAX
	// 1801c02eb 41  ff  d1       CALL       R9
	if (AfxAddr addr = FindPattern(textRange, "33 DB 48 8D 05 ?? ?? ?? ?? 48 8B CF 48 89 44 24 ??"))
		AFXADDR_SET(cs2_client_C_EnvSky_ForceUpdateSkybox, ResolveRel32(addr, 5, 9));
	else ADDR_MISSING(cs2_client_C_EnvSky_ForceUpdateSkybox);

	// Must hit the lea inside ForceUpdateSkybox (C_EnvSky pointer field, 0x10E8 in 2026-09-23 build).
	// The shorter "48 8D B3 ?? ?? ?? ?? 48 8B 0E" first matches an unrelated function there (offset 0x118).
	if (AfxAddr addr = FindPattern(textRange, "48 8D B3 ?? ?? ?? ?? 48 8B 0E 48 85 C9"))
		AFXADDR_SET(cs2_client_C_EnvSky_UnkPtr_offset, *(uint32_t*)(addr + 3));
	else ADDR_MISSING(cs2_client_C_EnvSky_UnkPtr_offset);

	//
	// mirv_viewmodel (ViewModel.cpp)

	// This function references the cvars viewmodel_offset_x, viewmodel_offset_y, viewmodel_offset_z, viewmodel_offset_fov (usually 3rd reference).
	// can be also found with 00 00 88 42 (68.0 in float), which is max limit for fov
	// there are 2 similiar functions, make sure to test if it breakes again to not confuse them
	if (AfxAddr addr = FindPattern(textRange, "40 55 53 56 41 56 41 57 48 8B EC 48 83 EC 20 4D 8B F8 4C 8B F2 48 8B F1"))
		AFXADDR_SET(cs2_client_ViewmodelFunc, addr);
	else ADDR_MISSING(cs2_client_ViewmodelFunc);

	// vtable byte offset 0xac8 for "C_CSGO_TeamPreviewModel" class (4th from end.).
	// This function is called right after the first call to cs2_client_ViewmodelFunc function.
	if (AfxAddr addr = FindPattern(textRange, "40 53 48 83 EC ?? 80 B9 ?? ?? ?? ?? ?? 48 8B D9 0F 84 ?? ?? ?? ?? 48 8B 89 ?? ?? ?? ?? 48 85 C9 75"))
		AFXADDR_SET(cs2_client_ViewmodelHandFunc, addr);
	else ADDR_MISSING(cs2_client_ViewmodelHandFunc);

	//
	// mirv_replace_name (ReplaceName.cpp)

	// references "<failure>"
	if (AfxAddr addr = FindPattern(textRange, "40 55 53 41 54 41 55 41 56 41 57 48 8d ac 24 ?? ?? ?? ?? 48 81 ec ?? ?? ?? ?? 4c 8b ea 4c 8b e1 45 84 c9 75 24 44 8b 4a 04 41 f7 c1 ff ff ff 3f"))
		AFXADDR_SET(cs2_client_GetDecoratedPlayerName, addr);
	else ADDR_MISSING(cs2_client_GetDecoratedPlayerName);

	// Called on the This of cs2_client_GetDecoratedPlayerName.
	AFXADDR_SET(cs2_client_GetDecoratedPlayerName_GetUserId_vtable_idx, 7);

	// fn has 3rd reference to string "WWWWWWWWWWWWWWWW"
	// vtable index was 226 before 2026-09-23 build
	if (AfxAddr addr = FindVtableFn(clientDll, ".?AVCCSPlayerController@@", 230))
		AFXADDR_SET(cs2_client_CCSPlayerController_GetPlayerName, addr);
	else ADDR_MISSING(cs2_client_CCSPlayerController_GetPlayerName);

	//
	// Entities (ClientEntitySystem.cpp)

	// GetClientClass function.
	// find it by searching for 4th full-ptr ref to "C_PlantedC4" subtract sizeof(void*) (0x8) and search function that references this struct.
	// you need to search for raw bytes, GiHidra doesn't seem to find the reference.
	// was 48 before 2026-09-23 build
	AFXADDR_SET(cs2_client_CEntityInstance_GetClientClass_vtable_idx, 49);

	// See cl_ent_text drawing function.
	// was 155 before 2026-09-23 build
	AFXADDR_SET(cs2_client_CEntityInstance_IsPlayerPawn_vtable_idx, 158);

	// See cl_ent_text drawing function. Near "Pawn: (%d) Name: %s".
	// was 156 before 2026-09-23 build
	AFXADDR_SET(cs2_client_CEntityInstance_IsPlayerController_vtable_idx, 159);

	// GetRenderEyeAngles vtable offset minus 1
	// was 170 before 2026-09-23 build
	AFXADDR_SET(cs2_client_CEntityInstance_GetRenderEyeOrigin_vtable_idx, 173);

	// See cl_track_render_eye_angles. Near "Render eye angles: %.7f, %.7f, %.7f\n".
	// was 171 before 2026-09-23 build
	AFXADDR_SET(cs2_client_CEntityInstance_GetRenderEyeAngles_vtable_idx, 174);

	// https://github.com/bruhmoment21/cs2-sdk
	AFXADDR_SET(cs2_client_CGameEntitySystem_OnAddEntity_vtable_idx, 15);
	AFXADDR_SET(cs2_client_CGameEntitySystem_OnRemoveEntity_vtable_idx, 16);

	// "Entities/Client Entity Count"
	if (AfxAddr addr = FindPattern(textRange, "40 55 53 48 8d ac 24 ?? ?? ?? ?? 48 81 ec ?? ?? ?? ?? 48 8b 0d ?? ?? ?? ?? 33 d2 e8 ?? ?? ?? ??")) {
		AFXADDR_SET(cs2_client_g_pEntityList, ResolveRel32(addr, 18 + 3, 18 + 7));
		AFXADDR_SET(cs2_client_CGameEntitySystem_GetHighestEntityIterator, ResolveRel32(addr, 27 + 1, 27 + 5));
	}
	else {
		ADDR_MISSING(cs2_client_g_pEntityList);
		ADDR_MISSING(cs2_client_CGameEntitySystem_GetHighestEntityIterator);
	}

	// see near "no such entity %d\n" called with pEntityList and uint
	// or near "Format: ent_find_index <index>\n" called only with uint and there's pEntityList inside with uint
	if (AfxAddr addr = FindPattern(textRange, "4c 8d 49 10 81 fa fe 7f 00 00"))
		AFXADDR_SET(cs2_client_CGameEntitySystem_GetEntityFromIndex, addr);
	else ADDR_MISSING(cs2_client_CGameEntitySystem_GetEntityFromIndex);

	// these two called one after each other
	// there is only one placed where they are called together
	//
	//                             LAB_1802066cd                                   XREF[1]:     18020660b (j)
	// 1802066cd 8b  4f  6c       MOV        ECX ,dword ptr [RDI  + 0x6c ]
	// 1802066d0 83  e9  01       SUB        ECX ,0x1
	// 1802066d3 0f  84  3d       JZ         LAB_180206816
	//           01  00  00
	// 1802066d9 83  f9  01       CMP        ECX ,0x1
	// 1802066dc 75  4c           JNZ        LAB_18020672a
	// 1802066de 4c  8b  47  30    MOV        R8,qword ptr [RDI  + 0x30 ]
	// 1802066e2 48  8d  95       LEA        RDX =>Stack [0x28 ],[RBP  + 0xc0 ]
	//           c0  00  00  00
	// 1802066e9 48  8b  ce       MOV        RCX ,RSI
	// 1802066ec e8  9f  bf       CALL       FUN_1808c2690                                    undefined FUN_1808c2690()
	//           6b  00
	// 1802066f1 0f  b6  95       MOVZX      EDX ,byte ptr [RBP  + Stack [0x28 ]]
	//           c0  00  00  00
	// 1802066f8 84  d2           TEST       DL,DL
	// 1802066fa 74  29           JZ         LAB_180206725
	// 1802066fc 4c  8d  45  b0    LEA        R8=>local_e8 ,[RBP  + -0x50 ]
	// 180206700 48  8b  ce       MOV        RCX ,RSI
	// 180206703 e8  78  ca       CALL       FUN_1808b3180                                    undefined FUN_1808b3180()
	//
	// to find this place find function with 5 arguments near "Unable to create non-precached breakable%s\n"
	// then in that function find place like this
	//
	//   else if (((*(int *)((longlong)param_4 + 0x6c) == 2) &&
	//          (FUN_1808c2690(param_3,&param_5,param_4[6]), (char)param_5 != '\0')) &&
	//         (FUN_1808b3180(param_3,(char)param_5,&local_e8), cVar2 != '\0')) {
	//   FUN_1815f22b0(param_2,(char)param_5,local_158,&local_108);
	// }
	//
	// first function can be found near "attachment_point" or called with "muzzle_flash" as last arg
	// second function in some places can be found with offset to m_nAttachmentIndex of CEffectData as 2nd arg
	//
	// Since 2026-09-23 the je between the calls is near (0F 84 rel32) instead of short, so the second call is at +27.
	if (AfxAddr addr = FindPattern(textRange, "E8 ?? ?? ?? ?? 0F B6 95 ?? ?? ?? ?? 84 D2 0F 84 ?? ?? ?? ?? 4C 8D 45 B0 48 8B CE E8 ?? ?? ?? ??")) {
		AFXADDR_SET(cs2_client_LookupAttachment, ResolveRel32(addr, 1, 5));
		AFXADDR_SET(cs2_client_GetAttachment, ResolveRel32(addr, 27 + 1, 27 + 5));
	}
	else {
		ADDR_MISSING(cs2_client_LookupAttachment);
		ADDR_MISSING(cs2_client_GetAttachment);
	}

	//
	// Game events (GameEvents.cpp)
	{
		void ** vtable = (void **)FindClassVtable((HMODULE)clientDll, ".?AVCGameEventManager@@", 0, 0);
		if (vtable) {
			AFXADDR_SET(cs2_client_CGameEventManager_CreateEvent, (AfxAddr)vtable[6]);
			AFXADDR_SET(cs2_client_CGameEventManager_FireEvent, (AfxAddr)vtable[7]);
			AFXADDR_SET(cs2_client_CGameEventManager_FireEventClientSide, (AfxAddr)vtable[8]);
			AFXADDR_SET(cs2_client_CGameEventManager_FreeEvent, (AfxAddr)vtable[10]);
		}
		else {
			ADDR_MISSING(cs2_client_CGameEventManager_CreateEvent);
			ADDR_MISSING(cs2_client_CGameEventManager_FireEvent);
			ADDR_MISSING(cs2_client_CGameEventManager_FireEventClientSide);
			ADDR_MISSING(cs2_client_CGameEventManager_FreeEvent);
		}
	}

	//
	// main.cpp

	/*
		This is where it checks for engine->IsPlayingDemo() (and afterwards for cl_demoviewoverride (float))
		before under these conditions it is calling CalcDemoViewOverride, so this is in CViewRender::SetUpView:

00007ffa`178e34ec 488b0d05b37e01   mov     rcx, qword ptr [7FFA190CE7F8h]
00007ffa`178e34f3 488b01           mov     rax, qword ptr [rcx]
00007ffa`178e34f6 ff9050010000     call    qword ptr [rax+150h]
00007ffa`178e34fc 0f57ff           xorps   xmm7, xmm7
00007ffa`178e34ff 84c0             test    al, al
00007ffa`178e3501 7457             je      00007FFA178E355A
00007ffa`178e3503 baffffffff       mov     edx, 0FFFFFFFFh
                 ff ff

		Since the 2026-09-23 build the cl_demoviewoverride read after the je is "lea rcx, cvar" (was "mov edx, -1")
		and CViewSetup is held in rbx (mov rbx, rdx at function start), not in r14 anymore.
	*/
	if (AfxAddr addr = FindPattern(textRange, "48 8b 0d ?? ?? ?? ?? 48 8b 01 ff 90 50 01 00 00 0f 57 ff 84 c0 74 ?? 48 8d 0d"))
		AFXADDR_SET(cs2_client_CViewRender_SetUpView_IsPlayingDemo, addr);
	else ADDR_MISSING(cs2_client_CViewRender_SetUpView_IsPlayingDemo);

	if (AfxAddr addr = FindVtableFn(clientDll, ".?AVCViewRender@@", 4))
		AFXADDR_SET(cs2_client_CViewRender_UnkMakeMatrix, addr);
	else ADDR_MISSING(cs2_client_CViewRender_UnkMakeMatrix);

	/*
	   GetSplitScreenPlayer(int):
	   This function is called in GetLocalPlayerController script function in clientDll,
	   go inside of function there and it's called with 0.

       1808541f0 40 53           PUSH       RBX
       1808541f2 48 83 ec 20     SUB        RSP,0x20
       1808541f6 8b 91 9c        MOV        EDX,dword ptr [RCX + 0x9c]
                 00 00 00
       1808541fc 48 8b d9        MOV        RBX,RCX
       1808541ff 83 fa ff        CMP        EDX,-0x1
       180854202 0f 84 27        JZ         LAB_18085432f
                 01 00 00
       180854208 4c 8b 0d        MOV        R9,qword ptr [DAT_1818b7d68]
                 59 3b 06 01
       18085420f 4d 85 c9        TEST       R9,R9
	*/
	if (AfxAddr addr = FindPattern(textRange, "48 83 EC ?? 83 F9 ?? 75 ?? 48 8B 0D ?? ?? ?? ?? 48 8D 54 24 ?? 48 8B 01 FF 90 ?? ?? ?? ?? 8B 08 48 63 C1 48 8D 0D ?? ?? ?? ?? 48 8B 04 C1 48 83 C4 ?? C3"))
		AFXADDR_SET(cs2_client_GetSplitScreenPlayer, addr);
	else ADDR_MISSING(cs2_client_GetSplitScreenPlayer);

	// Source2Client interface (vtable of the interface returned by client.dll's CreateInterface):
	AFXADDR_SET(cs2_client_CSource2Client_Connect_vtable_idx, 0);
	AFXADDR_SET(cs2_client_CSource2Client_Init_vtable_idx, 3);
	AFXADDR_SET(cs2_client_CSource2Client_Shutdown_vtable_idx, 4);
	AFXADDR_SET(cs2_client_CSource2Client_SetGlobals_vtable_idx, 11);
	AFXADDR_SET(cs2_client_CSource2Client_LevelInitPreEntity_vtable_idx, 35);
	AFXADDR_SET(cs2_client_CSource2Client_FrameStageNotify_vtable_idx, 36);

	missing.Report();
}

void Addresses_InitPanoramaDll(AfxAddr panoramaDll) {
	MemRange textRange = GetTextRange(panoramaDll);
	CMissingAddrs missing("panorama.dll");

	// Refernces "CLayoutFile::LoadFromFile" string.
	// 48 C7 45 D0 xx xx = source line number
	if (AfxAddr addr = FindPattern(textRange, "48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 48 8D 05 ?? ?? ?? ?? 48 C7 45 D0 ?? ?? 00 00 48"))
		AFXADDR_SET(cs2_panorama_CLayoutFile_LoadFromFile, addr);
	else ADDR_MISSING(cs2_panorama_CLayoutFile_LoadFromFile);

	const size_t CStyleProperty_Clone_vtable_idx = 1;
	const size_t CStyleProperty_Parse_vtable_idx = 6;

	if (AfxAddr addr = FindVtableFn(panoramaDll, ".?AVCStylePropertyForegroundColor@panorama@@", CStyleProperty_Parse_vtable_idx))
		AFXADDR_SET(cs2_panorama_CStylePropertyForegroundColor_Parse, addr);
	else ADDR_MISSING(cs2_panorama_CStylePropertyForegroundColor_Parse);

	if (AfxAddr addr = FindVtableFn(panoramaDll, ".?AVCStylePropertyBackgroundColor@panorama@@", CStyleProperty_Parse_vtable_idx))
		AFXADDR_SET(cs2_panorama_CStylePropertyBackgroundColor_Parse, addr);
	else ADDR_MISSING(cs2_panorama_CStylePropertyBackgroundColor_Parse);

	if (AfxAddr addr = FindVtableFn(panoramaDll, ".?AVCStylePropertyBorder@panorama@@", CStyleProperty_Parse_vtable_idx))
		AFXADDR_SET(cs2_panorama_CStylePropertyBorder_Parse, addr);
	else ADDR_MISSING(cs2_panorama_CStylePropertyBorder_Parse);

	if (AfxAddr addr = FindVtableFn(panoramaDll, ".?AVCStylePropertyWashColor@panorama@@", CStyleProperty_Clone_vtable_idx))
		AFXADDR_SET(cs2_panorama_CStylePropertyWashColor_Clone, addr);
	else ADDR_MISSING(cs2_panorama_CStylePropertyWashColor_Clone);

	if (AfxAddr addr = FindVtableFn(panoramaDll, ".?AVCStylePropertyWashColor@panorama@@", CStyleProperty_Parse_vtable_idx))
		AFXADDR_SET(cs2_panorama_CStylePropertyWashColor_Parse, addr);
	else ADDR_MISSING(cs2_panorama_CStylePropertyWashColor_Parse);

	if (AfxAddr addr = FindClassVtable((HMODULE)panoramaDll, ".?AVCStylePropertyOpacity@panorama@@", 0, 0))
		AFXADDR_SET(cs2_panorama_CStylePropertyOpacity_vtable, addr);
	else ADDR_MISSING(cs2_panorama_CStylePropertyOpacity_vtable);

	// Resolve style properties through Panorama's own lookup function. This
	// avoids depending on the private map layout for normal operation while
	// retaining the map below for diagnostics and as a fallback.
	//
	// fn refernces string "panorama::CStyleSymbol::CStyleSymbol" 4 times
	if (AfxAddr addr = FindPattern(textRange, "40 55 56 57 41 54 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 44 8B 05 ?? ?? ?? ?? 48 8B F9 65 48 8B 04 25 58 00 00 00 45 33 E4 C6 01 FF 48 8B F2"))
		AFXADDR_SET(cs2_panorama_CStyleSymbol_Resolve, addr);
	else ADDR_MISSING_OPTIONAL(cs2_panorama_CStyleSymbol_Resolve);

	// (in function of cs2_panorama_CStyleSymbol_Resolve)
	// before it jumps to error branch for "Need to increase size of static g_StylePropertyRegistrations (MAX_PANORAMA_STYLE_SYMBOLS) before registering more styles, failed on %s"
	/*
		lVar9 = DAT_180510350;
		if ((uVar12 & 0x7fffffff) == 0) {
			lVar9 = lVar13;
		}
		puVar1 = (undefined4 *)((longlong)iVar5 * 0x20 + 0x10 + lVar9);
		*puVar1 = (undefined4)local_68;
		puVar1[1] = local_68._4_4_;
		puVar1[2] = (undefined4)uStack_60;
		puVar1[3] = uStack_60._4_4_;
		local_70 = local_80;
		FUN_180099640(&DAT_180510348,iVar5,local_78); <-- DAT_180510348 is what we are after, since might sometimes be the result of a function called before this one, but right now it's inlined.
		_DAT_18051035c = _DAT_18051035c + 1;
	*/
	// Only required if cs2_panorama_CStyleSymbol_Resolve is not available.
	if (AfxAddr addr = FindPattern(textRange, "0f 10 45 f7 48 8d 0d ?? ?? ?? ?? 41 f7 c0 ff ff ff 7f"))
		AFXADDR_SET(cs2_panorama_g_StylePropertySymbols, ResolveRel32(addr, 7, 11));
	else missing.Add("cs2_panorama_g_StylePropertySymbols", 0 == AFXADDR_GET(cs2_panorama_CStyleSymbol_Resolve));

	// Can be found in constructor for any CStyleProperty
	// e.g. see 44th fn in vtable for CPanelStyle
	if (AfxAddr addr = FindPattern(textRange, "E8 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 45 ?? EB"))
		AFXADDR_SET(cs2_panorama_CPanelStyle_SetStyleProperty, ResolveRel32(addr, 1, 5));
	else ADDR_MISSING(cs2_panorama_CPanelStyle_SetStyleProperty);

	missing.Report();
}
