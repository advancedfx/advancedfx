#include "stdafx.h"

#include "ReplaceName.h"

#include "WrpConsole.h"
#include "Globals.h"
#include "SchemaSystem.h"

#include "../shared/binutils.h"
#include "../shared/AfxDetours.h"
#include "../shared/StringTools.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameevents.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/utlstring.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

#include "../deps/release/Detours/src/detours.h"

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <map>

std::map<int,std::string> g_Index_To_ReplaceName;
std::map<uint64_t,std::string> g_SteamId_To_ReplaceName;

typedef SOURCESDK::CS2::CEntityHandle * (__fastcall * CEntitityIntance_GetRefEHandle_t)(void * This, SOURCESDK::CS2::CEntityHandle & handle);

typedef void (__fastcall * CEntitityIntance_PostDataUpdate_t)(void * This);
CEntitityIntance_PostDataUpdate_t g_pOrg_CCSPlayerController_PostDataUpdate = nullptr;

void __fastcall New_CCSPlayerController_PostDataUpdate(void * This) {
    g_pOrg_CCSPlayerController_PostDataUpdate(This);

    if(!g_Index_To_ReplaceName.empty()){
        if(void * pEntityIndentity = *(void**)((unsigned char*)This + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
            SOURCESDK::CS2::CEntityHandle * pHandle = (SOURCESDK::CS2::CEntityHandle *)((unsigned char *)pEntityIndentity + 0x10);
            auto it = g_Index_To_ReplaceName.find(pHandle->GetEntryIndex());
            if(it != g_Index_To_ReplaceName.end()) {
                char * pszTarget = (char * )((unsigned char*)This + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName);
                strncpy(pszTarget,it->second.c_str(),127);
                pszTarget[128] = '\0';

                ((SOURCESDK::CS2::CUtlString *)((unsigned char*)This + g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName))->Set(it->second.c_str());
            }
        }
    }

    if(!g_SteamId_To_ReplaceName.empty()) {
        uint64_t steamid = *(uint64_t*)((unsigned char*)This + g_clientDllOffsets.CBasePlayerController.m_steamID);
        auto it = g_SteamId_To_ReplaceName.find(steamid);
        if(it!=g_SteamId_To_ReplaceName.end()) {
            char * pszTarget = (char * )((unsigned char*)This + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName);
            strncpy(pszTarget,it->second.c_str(),127);
            pszTarget[128] = '\0';

            ((SOURCESDK::CS2::CUtlString *)((unsigned char*)This + g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName))->Set(it->second.c_str());
        }
    }
}

bool HookReplaceName(HMODULE clientDll)
{
    static bool firstRun = true;
    static bool firstResult = false;
    if(firstRun) {
        firstRun = false;
        if(void ** vtable = (void **)Afx::BinUtils::FindClassVtable(clientDll, ".?AVCCSPlayerController@@", 0, 0)) {
            const size_t vtable_ofs = 8;
            MdtMemBlockInfos mbis;
            MdtMemAccessBegin((LPVOID)&(vtable[vtable_ofs]), sizeof(void*), &mbis);
            g_pOrg_CCSPlayerController_PostDataUpdate = (CEntitityIntance_PostDataUpdate_t)vtable[8];
            vtable[8] = &New_CCSPlayerController_PostDataUpdate;
			MdtMemAccessEnd(&mbis);
            firstResult = true;
        }
        else ErrorBox("HookReplaceName");
    }
    return firstResult;
}

extern void deathMsgPlayers_PrintHelp_Console();

CON_COMMAND(mirv_replace_name, "Replace player names")
{
    int argC = args->ArgC();
    const char * arg0 = args->ArgV(0);

    if(2 <= argC) {
        const char * arg1 = args->ArgV(1);
        if(0 == stricmp("byUserId", arg1)) {
            if(3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if(0 == stricmp("add", arg2) && 5 <= argC) {
                    g_Index_To_ReplaceName[atoi(args->ArgV(3))+1] = args->ArgV(4);
                    return;
                }
                else if(0 == stricmp("remove", arg2) && 4 <= argC) {
                    g_Index_To_ReplaceName.erase(atoi(args->ArgV(3))+1);
                    return;
                }
                else if(0 == stricmp("print", arg2) && 3 <= argC) {
                    for(auto it = g_Index_To_ReplaceName.begin(); it != g_Index_To_ReplaceName.end(); it++) {
                        advancedfx::Message("%i: %s\n",it->first-1,it->second.c_str());
                    }
                    return;
                }                
            }
            advancedfx::Message(
                "%s byUserId add <iUserId> <sValue>\n"
                "%s byUserId remove <iUserId>\n"
                "%s byUserId print\n",
                arg0,
                arg0,
                arg0
            );
            return;
        } else if(0 == stricmp("byXuid", arg1)) {
            if(3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if(0 == stricmp("add", arg2) && 5 <= argC) {
                    const char * arg3 = args->ArgV(3);
                    if(StringIBeginsWith(arg3,"x")) arg3++;
                    g_SteamId_To_ReplaceName[strtoull(arg3,nullptr,10)] = args->ArgV(4);
                    return;
                }
                else if(0 == stricmp("remove", arg2) && 4 <= argC) {
                    const char * arg3 = args->ArgV(3);
                    if(StringIBeginsWith(arg3,"x")) arg3++;                    
                    g_SteamId_To_ReplaceName.erase(strtoull(arg3,nullptr,10));
                    return;
                }
                else if(0 == stricmp("print", arg2) && 3 <= argC) {
                    for(auto it = g_SteamId_To_ReplaceName.begin(); it != g_SteamId_To_ReplaceName.end(); it++) {
                        advancedfx::Message("x%llu: %s\n",it->first,it->second.c_str());
                    }
                    return;
                }                
            }
            advancedfx::Message(
                "%s byXuid add x<ullXuid> <sValue>\n"
                "%s byXuid remove x<ullXuid>\n"
                "%s byXuid print\n",
                arg0,
                arg0,
                arg0
            );
            return;
        } else if(0 == stricmp("help", arg1)) {
            if(3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if(0 == stricmp("players", arg2)) {
                    deathMsgPlayers_PrintHelp_Console();
                    return;
                }
            }
        }
    }

    advancedfx::Message(
        "%s byUserId [...] - Replace player name by UserID\n"
        "%s byXuid [...] - Replace player name by SteamID\n"
        "%s help players - Print player info in console\n",
        arg0,
        arg0,
        arg0
    );
}