#include "stdafx.h"

#include "ReplaceName.h"

#include "WrpConsole.h"
#include "Globals.h"
#include "ClientEntitySystem.h"

#include "../shared/binutils.h"
#include "../shared/StringTools.h"

#include "../deps/release/prop/cs2/sdk_src/public/tier1/bufferstring.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameevents.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

#include "../deps/release/Detours/src/detours.h"

bool g_bDebug_GetPlayerName = false;
bool g_bDebug_GetDecoratedPlayerName = false;

enum class ReplaceNameFlags : int {
    None = 0,
    ClanTag = 1<<0,
    OriginalController = 1<<1,
    Puppeteer = 1 << 2,
    DeathNotification = 4 << 2
};

struct ReplaceName_s {
    ReplaceName_s(const char* name, const char * tag)
    : Name(name)
    , Tag(tag)
    {

    }
    std::string Name;
    std::string Tag;
};

std::map<int,ReplaceName_s> g_UserId_To_ReplaceName;
std::map<uint64_t,ReplaceName_s> g_SteamId_To_ReplaceName;;

ReplaceName_s * ResolveReplace(int userId) {
    if(-1 == userId) return nullptr;

    if(!g_UserId_To_ReplaceName.empty()) {
        auto it = g_UserId_To_ReplaceName.find(userId);
        if(it != g_UserId_To_ReplaceName.end()) {
            return &(it->second);
        }

    }

    if(!g_SteamId_To_ReplaceName.empty()) {
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,userId+1)) {
            uint64_t steamid = ent->GetSteamId();
            auto it = g_SteamId_To_ReplaceName.find(steamid);
            if(it!=g_SteamId_To_ReplaceName.end()) {
                return &(it->second);
            }
        }
    }

    return nullptr;
}

typedef const char * (__fastcall * CCSPlayerController_GetPlayerName_t)(void * This);
CCSPlayerController_GetPlayerName_t g_Org_CCSPlayerController_GetPlayerName = nullptr;

const char * __fastcall New_CCSPlayerController_GetPlayerName(CEntityInstance * This) {
    const char * result = g_Org_CCSPlayerController_GetPlayerName(This);

    if(g_bDebug_GetPlayerName) {
		auto handle = This->GetHandle();
		advancedfx::Message("GetPlayerName: %i -> %s\n", handle.GetEntryIndex(),result);
    }

    if(!g_UserId_To_ReplaceName.empty()){
		auto handle = This->GetHandle();
		if (handle.IsValid()) {
            if(auto result2 = ResolveReplace(handle.GetEntryIndex()-1)) {
                return result2->Name.c_str();
            }
		}
    }

    return result;
}

typedef void (__fastcall * GetDecoratedPlayerName_t)(void* This, SOURCESDK::CS2::CBufferString * pBufferString, unsigned int flags, bool bUnk3);
GetDecoratedPlayerName_t g_Org_GetDecoratedPlayerName = nullptr;

void __fastcall New_GetDecoratedPlayerName(void* This, SOURCESDK::CS2::CBufferString * pBufferString, unsigned int flags, bool bUnk3) {

    g_Org_GetDecoratedPlayerName(This, pBufferString, flags, bUnk3);

    int userId = -1;
    void ** vtable = *(void ***)This;
    int * (__fastcall * pfnGetUserId)(void * This, int * pHandle) = (int * (__fastcall *)(void *, int *))(vtable[7]);
    pfnGetUserId(This, &userId);

    if(g_bDebug_GetDecoratedPlayerName) {
        advancedfx::Message("GetDecoratedPlayerName: userId=%i, flags=%u, bUnk3=%i -> name=%s\n", userId, flags, bUnk3?1:0, pBufferString->Get());
    }

    int myFlags = flags;

    // follow the original logic in the function a bit:
    if(myFlags & (int)ReplaceNameFlags::DeathNotification) myFlags |= (int)ReplaceNameFlags::ClanTag;

    if(myFlags & ((int)ReplaceNameFlags::ClanTag | (int)ReplaceNameFlags::OriginalController)) {
        if(auto replace = ResolveReplace(userId)) {
            std::string result;
            if(myFlags & (int)ReplaceNameFlags::ClanTag) {
                result += replace->Tag.c_str();
            }
            if(myFlags & (int)ReplaceNameFlags::OriginalController) {
                if(0<result.size()) result += " ";
                result += replace->Name.c_str();
            }
            *pBufferString = result.c_str();
        }        
    }
}

void HookReplaceName(HMODULE clientDll)
{
    static bool firstRun = true;
    if(firstRun) {
        firstRun = false;

        // GetDecoratedPlayerName
        // references "<failure>"        
		g_Org_GetDecoratedPlayerName = (GetDecoratedPlayerName_t)getAddress(clientDll, "40 55 53 56 41 54 41 55 41 56 48 8d ac 24 18 fe ff ff 48 81 ec e8 02 00 00 4c 8b ea 4c 8b e1 45 84 c9 75 21 8b 4a 04 f7 c1 ff ff ff 3f");	
		if (g_Org_GetDecoratedPlayerName != 0) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)g_Org_GetDecoratedPlayerName, New_GetDecoratedPlayerName);
			if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
		}
		else advancedfx::Warning("AFXWARNING: decorated player-name replacement is unavailable for this CS2 build.\n");

        // fn has 3rd reference to string "WWWWWWWWWWWWWWWW"
        if(void ** vtable = (void **)Afx::BinUtils::FindClassVtable(clientDll, ".?AVCCSPlayerController@@", 0, 0)) {
            g_Org_CCSPlayerController_GetPlayerName = (CCSPlayerController_GetPlayerName_t)vtable[226];
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_CCSPlayerController_GetPlayerName, New_CCSPlayerController_GetPlayerName);
            if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
        }
        else ErrorBox(MkErrStr(__FILE__, __LINE__));
    }
}

extern void deathMsgPlayers_PrintHelp_Console();

CON_COMMAND(mirv_replace_name, "Replace player names (and clan tags)")
{
    int argC = args->ArgC();
    const char * arg0 = args->ArgV(0);

    if(2 <= argC) {
        const char * arg1 = args->ArgV(1);
        if(0 == stricmp("byUserId", arg1)) {
            if(3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if(0 == stricmp("add", arg2) && 5 <= argC) {
                    int32_t userId = atoi(args->ArgV(3));
                    const char * name = args->ArgV(4);
                    const char * tag = 6 <= argC ? args->ArgV(5) : "";
                    g_UserId_To_ReplaceName.erase(userId);
                    g_UserId_To_ReplaceName.emplace(std::piecewise_construct
                        , std::forward_as_tuple(userId)
                        , std::forward_as_tuple(name, tag)
                    );
                    return;
                }
                else if(0 == stricmp("remove", arg2) && 4 <= argC) {
                    g_UserId_To_ReplaceName.erase(atoi(args->ArgV(3)));
                    return;
                }
                else if(0 == stricmp("print", arg2) && 3 <= argC) {
                    for(auto it = g_UserId_To_ReplaceName.begin(); it != g_UserId_To_ReplaceName.end(); it++) {
                        advancedfx::Message("%i: \"%s\" \"%s\"",it->first,it->second.Name.c_str(),it->second.Tag.c_str());
                    }
                    return;
                }                
            }
            advancedfx::Message(
                "%s byUserId add <iUserId> <sValue> [<sTagValue>]\n"
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
                    uint64_t steamId = strtoull(arg3,nullptr,10);
                    const char * name = args->ArgV(4);
                    const char * tag = 6 <= argC ? args->ArgV(5) : "";
                    g_SteamId_To_ReplaceName.erase(steamId);
                    g_SteamId_To_ReplaceName.emplace(std::piecewise_construct
                        , std::forward_as_tuple(steamId)
                        , std::forward_as_tuple(name, tag)
                    );
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
                        advancedfx::Message("%i: \"%s\" \"%s\"",it->first,it->second.Name.c_str(),it->second.Tag.c_str());
                    }
                    return;
                }                
            }
            advancedfx::Message(
                "%s byXuid add x<ullXuid> <sValue> [<sTagValue>]\n"
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
        else if(0==stricmp("oldDebug",arg1)) {
            if(3 <= argC) {
                g_bDebug_GetPlayerName = 0 != atoi(args->ArgV(2));
                return;
            }

            advancedfx::Message(
                "%s oldDebug 0|1\n"
                "Current Value: %i\n",
                arg0,
                g_bDebug_GetPlayerName?1:0
            );
            return;
        }
        else if(0==stricmp("debug",arg1)) {
            if(3 <= argC) {
                g_bDebug_GetDecoratedPlayerName = 0 != atoi(args->ArgV(2));
                return;
            }

            advancedfx::Message(
                "%s debug 0|1\n"
                "Current Value: %i\n",
                arg0,
                g_bDebug_GetDecoratedPlayerName?1:0
            );
            return;
        }
    }

    advancedfx::Message(
        "%s byUserId [...] - Replace player name and tag by UserID\n"
        "%s byXuid [...] - Replace player name and tag by SteamID\n"
        "%s help players - Print player info in console\n"
        "%s debug 0|1\n"
        "%s oldDebug 0|1\n",
        arg0,
        arg0,
        arg0,
        arg0,
        arg0,
        arg0,
        arg0
    );
}

#ifdef _DEBUG
CON_COMMAND(__mirv_get_decorated_player_name, "")
{
    int argC = args->ArgC();
    const char * arg0 = args->ArgV(0);

    if(5 <= argC) {
        const char * arg1 = args->ArgV(1);
        const char * arg2 = args->ArgV(2);
        const char * arg3 = args->ArgV(3);
        const char * arg4 = args->ArgV(4);

        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,atoi(arg1))) {
            SOURCESDK::CS2::CBufferString str;
            g_Org_GetDecoratedPlayerName((unsigned char *)ent + atoi(arg2), &str, atoi(arg3), atoi(arg4));
            advancedfx::Message("Result: %s\n",str.Get());
        }
    }
}
#endif
