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


bool g_bDebug_GetPlayerName = false;
bool g_bDebug_GetDecoratedPlayerName = false;

std::map<int,std::string> g_Index_To_ReplaceName;
std::map<uint64_t,std::string> g_SteamId_To_ReplaceName;

std::map<int,std::string> g_Index_To_DecoratedReplaceName;
std::map<uint64_t,std::string> g_SteamId_To_DecoratedReplaceName;


typedef SOURCESDK::CS2::CEntityHandle * (__fastcall * CEntitityIntance_GetRefEHandle_t)(void * This, SOURCESDK::CS2::CEntityHandle & handle);

typedef const char * (__fastcall * CCSPlayerController_GetPlayerName_t)(void * This);
CCSPlayerController_GetPlayerName_t g_Org_CCSPlayerController_GetPlayerName = nullptr;

const char * __fastcall New_CCSPlayerController_GetPlayerName(void * This) {
    const char * result = g_Org_CCSPlayerController_GetPlayerName(This);

    if(g_bDebug_GetPlayerName) {
        if(void * pEntityIndentity = *(void**)((unsigned char*)This + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
            SOURCESDK::CS2::CEntityHandle * pHandle = (SOURCESDK::CS2::CEntityHandle *)((unsigned char *)pEntityIndentity + 0x10);
            advancedfx::Message("GetPlayerName: %i -> %s\n",pHandle->GetEntryIndex(),result);
        }        
    }

    if(!g_Index_To_ReplaceName.empty()){
        if(void * pEntityIndentity = *(void**)((unsigned char*)This + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
            SOURCESDK::CS2::CEntityHandle * pHandle = (SOURCESDK::CS2::CEntityHandle *)((unsigned char *)pEntityIndentity + 0x10);
            auto it = g_Index_To_ReplaceName.find(pHandle->GetEntryIndex());
            if(it != g_Index_To_ReplaceName.end()) {
                result = it->second.c_str();
            }
        }
    }

    if(!g_SteamId_To_ReplaceName.empty()) {
        uint64_t steamid = *(uint64_t*)((unsigned char*)This + g_clientDllOffsets.CBasePlayerController.m_steamID);
        auto it = g_SteamId_To_ReplaceName.find(steamid);
        if(it!=g_SteamId_To_ReplaceName.end()) {
                result = it->second.c_str();
        }
    }

    return result;
}

typedef const char * (__fastcall * GetDecoratedPlayerName_t)(void *This_CCSPlayerController, char * pBuffer , unsigned int bufferSize, unsigned int maybeShortenLength);
GetDecoratedPlayerName_t g_Org_GetDecoratedPlayerName = nullptr;
const char * __fastcall New_GetDecoratedPlayerName(void *This_CCSPlayerController, char * pBuffer , unsigned int bufferSize, unsigned int maybeShortenLength) {
    const char * result = g_Org_GetDecoratedPlayerName(This_CCSPlayerController, pBuffer, bufferSize, maybeShortenLength);

    if(g_bDebug_GetDecoratedPlayerName) {
        if(void * pEntityIndentity = *(void**)((unsigned char*)This_CCSPlayerController + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
            SOURCESDK::CS2::CEntityHandle * pHandle = (SOURCESDK::CS2::CEntityHandle *)((unsigned char *)pEntityIndentity + 0x10);
            advancedfx::Message("GetDecoratedPlayerName: %i -> %s\n",pHandle->GetEntryIndex(),result);
        }        
    }    

    if(!g_Index_To_DecoratedReplaceName.empty()){
        if(void * pEntityIndentity = *(void**)((unsigned char*)This_CCSPlayerController + g_clientDllOffsets.CEntityInstance.m_pEntity)) {
            SOURCESDK::CS2::CEntityHandle * pHandle = (SOURCESDK::CS2::CEntityHandle *)((unsigned char *)pEntityIndentity + 0x10);
            auto it = g_Index_To_DecoratedReplaceName.find(pHandle->GetEntryIndex());
            if(it != g_Index_To_DecoratedReplaceName.end()) {
                strncpy(pBuffer,it->second.c_str(),bufferSize);
            }
        }
    }

    if(!g_SteamId_To_DecoratedReplaceName.empty()) {
        uint64_t steamid = *(uint64_t*)((unsigned char*)This_CCSPlayerController + g_clientDllOffsets.CBasePlayerController.m_steamID);
        auto it = g_SteamId_To_DecoratedReplaceName.find(steamid);
        if(it!=g_SteamId_To_DecoratedReplaceName.end()) {
           strncpy(pBuffer,it->second.c_str(),bufferSize);
        }
    }

    return result;
}

void HookReplaceName(HMODULE clientDll)
{
    static bool firstRun = true;
    if(firstRun) {
        firstRun = false;

        // GetDecoratedPlayerName
        // references "SFUI_bot_decorated_name"       
	    {
            Afx::BinUtils::ImageSectionsReader sections((HMODULE)clientDll);
            Afx::BinUtils::MemRange textRange = sections.GetMemRange();
            Afx::BinUtils::MemRange result = FindPatternString(textRange, "44 89 44 24 18 48 89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 8d ac 24 28 f5 ff ff");
            if (!result.IsEmpty()) {
                g_Org_GetDecoratedPlayerName = (GetDecoratedPlayerName_t)result.Start;	
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach(&(PVOID&)g_Org_GetDecoratedPlayerName, New_GetDecoratedPlayerName);
                if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
            }
            else ErrorBox(MkErrStr(__FILE__, __LINE__));
	    }

        /*
            GetDecoratedPlayerName references a function on "?AVCCSPlayerController@@" vtable as follows:
            ...
                             LAB_180599ba6                                   XREF[4]:     180599ac9(j), 180599ad5(j), 
                                                                                          180599b47(j), 180599ba1(j)  
       180599ba6 4c 8d 25        LEA        R12,[DAT_181418000]
                 53 e4 e7 00
       180599bad 48 85 c9        TEST       RCX,RCX
       180599bb0 74 10           JZ         LAB_180599bc2
       180599bb2 48 8b 01        MOV        RAX,qword ptr [RCX]
       180599bb5 ff 90 48        CALL       qword ptr [RAX + 0x748]
                 07 00 00
       180599bbb 48 89 44        MOV        qword ptr [RSP + local_bd8],RAX
                 24 40
       180599bc0 eb 05           JMP        LAB_180599bc7
            ...
            So now know the offset of the GetPlayerNameFunction
        */

        if(void ** vtable = (void **)Afx::BinUtils::FindClassVtable(clientDll, ".?AVCCSPlayerController@@", 0, 0)) {
            g_Org_CCSPlayerController_GetPlayerName = (CCSPlayerController_GetPlayerName_t)vtable[233];
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_CCSPlayerController_GetPlayerName, New_CCSPlayerController_GetPlayerName);
            if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
        }
        else ErrorBox(MkErrStr(__FILE__, __LINE__));
    }
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
        else if(0 == stricmp("decoByUserId", arg1)) {
            if(3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if(0 == stricmp("add", arg2) && 5 <= argC) {
                    g_Index_To_DecoratedReplaceName[atoi(args->ArgV(3))+1] = args->ArgV(4);
                    return;
                }
                else if(0 == stricmp("remove", arg2) && 4 <= argC) {
                    g_Index_To_DecoratedReplaceName.erase(atoi(args->ArgV(3))+1);
                    return;
                }
                else if(0 == stricmp("print", arg2) && 3 <= argC) {
                    for(auto it = g_Index_To_DecoratedReplaceName.begin(); it != g_Index_To_DecoratedReplaceName.end(); it++) {
                        advancedfx::Message("%i: %s\n",it->first-1,it->second.c_str());
                    }
                    return;
                }                
            }
            advancedfx::Message(
                "%s decoByUserId add <iUserId> <sValue>\n"
                "%s decoByUserId remove <iUserId>\n"
                "%s decoByUserId print\n",
                arg0,
                arg0,
                arg0
            );
            return;
        } else if(0 == stricmp("decoByXuid", arg1)) {
            if(3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if(0 == stricmp("add", arg2) && 5 <= argC) {
                    const char * arg3 = args->ArgV(3);
                    if(StringIBeginsWith(arg3,"x")) arg3++;
                    g_SteamId_To_DecoratedReplaceName[strtoull(arg3,nullptr,10)] = args->ArgV(4);
                    return;
                }
                else if(0 == stricmp("remove", arg2) && 4 <= argC) {
                    const char * arg3 = args->ArgV(3);
                    if(StringIBeginsWith(arg3,"x")) arg3++;                    
                    g_SteamId_To_DecoratedReplaceName.erase(strtoull(arg3,nullptr,10));
                    return;
                }
                else if(0 == stricmp("print", arg2) && 3 <= argC) {
                    for(auto it = g_SteamId_To_DecoratedReplaceName.begin(); it != g_SteamId_To_DecoratedReplaceName.end(); it++) {
                        advancedfx::Message("x%llu: %s\n",it->first,it->second.c_str());
                    }
                    return;
                }                
            }
            advancedfx::Message(
                "%s decoByXuid add x<ullXuid> <sValue>\n"
                "%s decoByXuid remove x<ullXuid>\n"
                "%s decoByXuid print\n",
                arg0,
                arg0,
                arg0
            );
            return;
        }
        else if(0==stricmp("debug",arg1)) {
            if(3 <= argC) {
                g_bDebug_GetPlayerName = 0 != atoi(args->ArgV(2));
                return;
            }

            advancedfx::Message(
                "%s debug 0|1\n"
                "Current Value: %i\n",
                arg0,
                g_bDebug_GetPlayerName?1:0
            );
            return;
        }
        else if(0==stricmp("decoDebug",arg1)) {
            if(3 <= argC) {
                g_bDebug_GetDecoratedPlayerName = 0 != atoi(args->ArgV(2));
                return;
            }

            advancedfx::Message(
                "%s decoDebug 0|1\n"
                "Current Value: %i\n",
                arg0,
                g_bDebug_GetDecoratedPlayerName?1:0
            );
            return;
        }
    }

    advancedfx::Message(
        "%s byUserId [...] - Replace player name by UserID\n"
        "%s byXuid [...] - Replace player name by SteamID\n"
        "%s help players - Print player info in console\n"
        "%s decoByUserId [...] - Replace decorated player name by UserID\n"
        "%s decoByXuid [...] - Replace decorated player name by SteamID\n"
        "%s debug 0|1\n"
        "%s decoDebug 0|1\n",
        arg0,
        arg0,
        arg0,
        arg0,
        arg0,
        arg0,
        arg0
    );
}