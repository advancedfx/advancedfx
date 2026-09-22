#include "stdafx.h"

#include "addresses.h"
#include "Globals.h"

#include "../shared/binutils.h"

using namespace Afx::BinUtils;

AFXADDR_DEF(cs2_engine_HostStateRequest_Start)
AFXADDR_DEF(cs2_engine_CRenderService_OnClientOutput);
AFXADDR_DEF(cs2_engine_AdvanceTime);

AFXADDR_DEF(cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx);
AFXADDR_DEF(cs2_SceneSystem_FrameUpdate_vtable_idx);

AFXADDR_DEF(cs2_deathmsg_lifetime_offset)
AFXADDR_DEF(cs2_deathmsg_lifetimemod_offset)

void Addresses_InitEngine2Dll(AfxAddr engine2Dll)
{
	MemRange textRange = MemRange(0, 0);
	{
		ImageSectionsReader imageSectionsReader((HMODULE)engine2Dll);
		if (!imageSectionsReader.Eof())
		{
			textRange = imageSectionsReader.GetMemRange();
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// Optional demo clock fix. Besides the entry point, verify the field layout
	// used by MirvFix.cpp. The function references "AdvanceTime ticks this frame".
	// Leave unavailable on a missing/ambiguous signature; do not interrupt startup.
	{
		const char * signature = "48 8b c4 f2 0f 11 50 18 f2 0f 11 48 10 55 57 41 56 41 57 48 8d a8 18 ff ff ff 48 81 ec c8 01 00 00";
		MemRange result = FindPatternString(textRange, signature);
		if (!result.IsEmpty() && result.Start + 0x250 <= textRange.End
			&& FindPatternString(MemRange(result.End, textRange.End), signature).IsEmpty()) {
			MemRange body(result.Start, result.Start + 0x250);
			// tick interval at 0x140; double simulation remainder at 0xf0.
			MemRange remainder = FindPatternString(body,
				"f3 0f 10 9f 40 01 00 00 48 8d 9f f0 00 00 00 f2 0f 10 13 f2 41 0f 58 16 0f 5a cb 89 73 08 f2 0f 11 13");
			// Clock modes 0 and 3 use the validated simulation branch.
			MemRange mode = FindPatternString(body,
				"8b 8f 60 01 00 00 85 c9 0f 84 ?? ?? ?? ?? 41 2b cf 74 09 41 3b cf 0f 85");
			// The frame-time helper reads cached host_framerate at 0x14c.
			MemRange frameTimeCall = FindPatternString(body,
				"0f 28 cb e8 ?? ?? ?? ?? f2 41 0f 10 06");
			if (!remainder.IsEmpty() && !mode.IsEmpty() && !frameTimeCall.IsEmpty()) {
				AfxAddr frameTime = frameTimeCall.Start + 8 + *reinterpret_cast<const int32_t *>(frameTimeCall.Start + 4);
				if (textRange.Start <= frameTime && frameTime <= textRange.End - 0x180
					&& !FindPatternString(MemRange(frameTime, frameTime + 0x180), "f3 0f 10 8f 4c 01 00 00").IsEmpty())
					AFXADDR_SET(cs2_engine_AdvanceTime, result.Start);
			}
		}
	}

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
																	  
		if (!result.IsEmpty()) {
            AFXADDR_SET(cs2_engine_HostStateRequest_Start, result.Start);
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
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
																	  
		if (!result.IsEmpty()) {
            AFXADDR_SET(cs2_engine_CRenderService_OnClientOutput, result.Start);
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
}

void Addresses_InitSceneSystemDll(AfxAddr sceneSystemDll) {

    /*cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx 
    
       To find the right function search for references to strings:
       - "WaitForRenderingToComplete".
    */
    AFXADDR_SET(cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx, 26);

    /*cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx 
    
       To find the right function search for references to strings:
       - "FrameUpdate"
       - "CSceneSystem::FrameUpdate"
       - "Invalid width/height for ScratchTarget, Size=%i, Width=%i/Height=%i"
    */
    AFXADDR_SET(cs2_SceneSystem_FrameUpdate_vtable_idx, 74);
}

void Addresses_InitClientDll(AfxAddr clientDll) {
	MemRange textRange = MemRange(0, 0);
	{
		ImageSectionsReader imageSectionsReader((HMODULE)clientDll);
		if (!imageSectionsReader.Eof())
		{
			textRange = imageSectionsReader.GetMemRange();
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	// in the end of g_Original_handlePlayerDeath function (see DeatMsg.cpp)
      /*
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
                  AFXADDR_SET(cs2_deathmsg_lifetime_offset, *(uint8_t*)(result.Start + 4));
                  AFXADDR_SET(cs2_deathmsg_lifetimemod_offset, *(uint8_t*)(result.Start + 34));
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
}
