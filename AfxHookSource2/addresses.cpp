#include "stdafx.h"

#include "addresses.h"
#include "Globals.h"

#include "../shared/binutils.h"

using namespace Afx::BinUtils;

AFXADDR_DEF(cs2_engine_HostStateRequest_Start)
AFXADDR_DEF(cs2_engine_CRenderService_OnClientOutput);

AFXADDR_DEF(cs2_SceneSystem_WaitForRenderingToComplete_vtable_idx);
AFXADDR_DEF(cs2_SceneSystem_FrameUpdate_vtable_idx);

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
		MemRange result = FindPatternString(textRange, "48 89 5C 24 18 55 56 57 41 54 41 56 48 83 EC 70 48 8D 05 19");
																	  
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
    AFXADDR_SET(cs2_SceneSystem_FrameUpdate_vtable_idx, 73);
}

void Addresses_InitClientDll(AfxAddr clientDll) {

}
