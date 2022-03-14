#include "stdafx.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "addresses.h"
#include <shared/AfxDetours.h>
#include <Windows.h>
#include <deps/release/Detours/src/detours.h>
#include <vector>


bool g_csgo_speactortools_force_enabled = false;


typedef bool(__fastcall * csgo_client_CCSGO_MapOverview_CanShowOverview_t)(void * This, void * Edx);

csgo_client_CCSGO_MapOverview_CanShowOverview_t True_csgo_client_CCSGO_MapOverview_CanShowOverview;

bool __fastcall My_csgo_client_CCSGO_MapOverview_CanShowOverview(void * This, void * Edx)
{
    if(g_csgo_speactortools_force_enabled) return true;

	return True_csgo_client_CCSGO_MapOverview_CanShowOverview(This,Edx);
}


bool csgo_speactortools_force_enable(bool value) {

    unsigned char movEax1[5] = {0xB8, 0x01, 0x00, 0x00, 0x00};

    bool bOk = true;

    if(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS)) {
        static bool bInitialized = false;
        static std::vector<unsigned char> tmp(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_LEN));
        MdtMemBlockInfos mbis;
        MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS), tmp.size(), &mbis);
        if(!bInitialized) {
            bInitialized = true;
            memcpy(&tmp[0], (LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS), tmp.size());
        }
        if(value) {
            memset((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS), 0x90, tmp.size());
            memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS), movEax1, 5);
        }
        else memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_1_OFS), &tmp[0], tmp.size());
        MdtMemAccessEnd(&mbis);
    } else bOk = false;

    if(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS)) {
        static bool bInitialized = false;
        static std::vector<unsigned char> tmp(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_LEN));
        MdtMemBlockInfos mbis;
        MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS), tmp.size(), &mbis);
        if(!bInitialized) {
            bInitialized = true;
            memcpy(&tmp[0], (LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS), tmp.size());
        }
        if(value) {
            memset((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS), 0x90, tmp.size());
            memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS), movEax1, 5);
        }
        else memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_2_OFS), &tmp[0], tmp.size());
        MdtMemAccessEnd(&mbis);
    } else bOk = false;

    if(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS)) {
        static bool bInitialized = false;
        static std::vector<unsigned char> tmp(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_LEN));
        MdtMemBlockInfos mbis;
        MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS), tmp.size(), &mbis);
        if(!bInitialized) {
            bInitialized = true;
            memcpy(&tmp[0], (LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS), tmp.size());
        }
        if(value) {
            memset((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS), 0x90, tmp.size());
            memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS), movEax1, 5);
        }
        else memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_3_OFS), &tmp[0], tmp.size());
        MdtMemAccessEnd(&mbis);
    } else bOk = false;

    if(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS)) {
        static bool bInitialized = false;
        static std::vector<unsigned char> tmp(AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_LEN));
        MdtMemBlockInfos mbis;
        MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS), tmp.size(), &mbis);
        if(!bInitialized) {
            bInitialized = true;
            memcpy(&tmp[0], (LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS), tmp.size());
        }
        if(value) {
            memset((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS), 0x90, tmp.size());
            memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS), movEax1, 5);
        }
        else memcpy((LPVOID)AFXADDR_GET(csgo_client_CanSeeSpectatorOnlyTools_4_OFS), &tmp[0], tmp.size());
        MdtMemAccessEnd(&mbis);
    } else bOk = false;

    if(AFXADDR_GET(csgo_client_CCSGO_MapOverview_CanShowOverview)) {
        if(nullptr == True_csgo_client_CCSGO_MapOverview_CanShowOverview) {
            True_csgo_client_CCSGO_MapOverview_CanShowOverview = (csgo_client_CCSGO_MapOverview_CanShowOverview_t)AFXADDR_GET(csgo_client_CCSGO_MapOverview_CanShowOverview);
            
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)True_csgo_client_CCSGO_MapOverview_CanShowOverview, My_csgo_client_CCSGO_MapOverview_CanShowOverview);
            if(NO_ERROR != DetourTransactionCommit()) bOk = false;
        }
    } else bOk = false;

    if(AFXADDR_GET(csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr)) {
        if(g_csgo_speactortools_force_enabled != value) {
            MdtMemBlockInfos mbis;
            MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr),4, &mbis);
            DWORD * pAddr = (DWORD *)AFXADDR_GET(csgo_client_CCSGO_Scoreboard_OpenScoreboard_jz_addr);
            if(!g_csgo_speactortools_force_enabled && value) {
                *pAddr -= 0x15D;                
            } else if (g_csgo_speactortools_force_enabled && !value) {
                *pAddr += 0x15D;                
            }
            MdtMemAccessEnd(&mbis);                        
        }
    } else bOk = false;  

    g_csgo_speactortools_force_enabled = value;  

    return bOk;
}


CON_COMMAND(mirv_force_spectatortools, "Enable / Disable forcing allowing spectator tools") {
	int argC = args->ArgC();

	if (2 <= argC)
	{ 
        if(!csgo_speactortools_force_enable(0 != atoi(args->ArgV(1))))
            Tier0_Warning("AFX_ERROR: missing hooks.\n");
        return;
	}

    Tier0_Msg("mirv_force_spectatortools 0|1 - If to force spectator tools.\n");
}
