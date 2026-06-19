#include "stdafx.h"

#include "MirvPovVoice.h"

#include "ClientEntitySystem.h"
#include "Globals.h"
#include "MirvTime.h"

#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;

#include "../shared/AfxConsole.h"
#include "../shared/AfxDetours.h"
#include "../shared/binutils.h"

#define WIN32_LEAN_AND_MEAN
#include "../deps/release/Detours/src/detours.h"

#include <intrin.h>
#include <limits.h>

#pragma intrinsic(_ReturnAddress)

static constexpr int kMirvPovIsPlayingDemoVtableIndex = 42;
static constexpr int kMirvPovVoiceSeekThresholdTicks = 16;
static constexpr float kMirvPovSyntheticSpeakingSeconds = 0.65f;

static int g_MirvPovVoiceLastDemoTick = INT_MIN;
static int g_MirvPovVoiceLastControllerIndex = -1;
static int g_MirvPovVoiceLastTeam = -1;
static float g_MirvPovSyntheticSpeakingUntil[64] = {};
static size_t g_MirvPovShowSpeakerRetAddr = 0;
static size_t g_MirvPovServerVoiceDataAddr = 0;
static size_t g_MirvPovVoiceStatusGetAddr = 0;
static size_t g_MirvPovVoiceStatusUpdateSpeakerStatusAddr = 0;

typedef bool (*MirvPov_IsPlayingDemo_t)(void * This);
static MirvPov_IsPlayingDemo_t g_Org_MirvPov_IsPlayingDemo = nullptr;
static bool g_bMirvPovIsPlayingDemoHooked = false;

typedef __int64 (__fastcall * MirvPov_ServerVoiceData_t)(__int64 This, __int64 msg);
static MirvPov_ServerVoiceData_t g_Org_MirvPov_ServerVoiceData = nullptr;
static bool g_bMirvPovServerVoiceDataHooked = false;

typedef __int64 (__fastcall * MirvPov_VoiceStatus_Get_t)();
typedef __int64 (__fastcall * MirvPov_VoiceStatus_UpdateSpeakerStatus_t)(__int64 voiceStatus, unsigned int playerSlot, int localSlot, unsigned __int8 talking);

class CEntityInstance * GetFakePovRadarController();
int GetFakePovRadarControllerIndex();

void MirvPov_UpdateVoiceTeam() {
    if(!MirvPov_IsEnabled() || !g_pEngineToClient) return;

    CEntityInstance * controller = GetFakePovRadarController();
    int controllerIndex = GetFakePovRadarControllerIndex();
    if(nullptr == controller || !controller->IsPlayerController()) return;
    if(controllerIndex <= 0) controllerIndex = controller->GetHandle().GetEntryIndex();

    if(g_MirvPovVoiceLastControllerIndex == controllerIndex) return;
    g_MirvPovVoiceLastControllerIndex = controllerIndex;

    int team = controller->GetTeam();
    if(team != 2 && team != 3) return;
    if(g_MirvPovVoiceLastTeam == team) return;

    g_pEngineToClient->ExecuteClientCmd(0, 2 == team ? "mirv_script_voice t" : "mirv_script_voice ct", true);
    g_MirvPovVoiceLastTeam = team;
}

static bool MirvPov_IsVoiceHudReady() {
    return g_MirvPovVoiceStatusGetAddr && g_MirvPovVoiceStatusUpdateSpeakerStatusAddr;
}

void MirvPov_ClearSyntheticSpeaking() {
    for(int i = 0; i < 64; ++i) {
        if(0.0f < g_MirvPovSyntheticSpeakingUntil[i]) {
            __int64 voiceStatus = MirvPov_IsVoiceHudReady() ? ((MirvPov_VoiceStatus_Get_t)g_MirvPovVoiceStatusGetAddr)() : 0;
            if(voiceStatus) ((MirvPov_VoiceStatus_UpdateSpeakerStatus_t)g_MirvPovVoiceStatusUpdateSpeakerStatusAddr)(voiceStatus, i, -1, 0);
        }
        g_MirvPovSyntheticSpeakingUntil[i] = 0.0f;
    }
}

static void MirvPov_SetSyntheticSpeaking(unsigned int playerSlot, bool speaking) {
    if(64 <= playerSlot || !MirvPov_IsVoiceHudReady()) return;

    __int64 voiceStatus = ((MirvPov_VoiceStatus_Get_t)g_MirvPovVoiceStatusGetAddr)();
    if(!voiceStatus) return;

    ((MirvPov_VoiceStatus_UpdateSpeakerStatus_t)g_MirvPovVoiceStatusUpdateSpeakerStatusAddr)(voiceStatus, playerSlot, -1, speaking ? 1 : 0);
    g_MirvPovSyntheticSpeakingUntil[playerSlot] = speaking ? g_MirvTime.curtime_get() + kMirvPovSyntheticSpeakingSeconds : 0.0f;
}

static bool MirvPov_IsVoicePlayerSlotOnWatchedTeam(unsigned int playerSlot) {
    if(64 <= playerSlot) return false;

    int watchedTeam = g_MirvPovVoiceLastTeam;
    if(watchedTeam != 2 && watchedTeam != 3) {
        CEntityInstance * controller = GetFakePovRadarController();
        if(nullptr == controller || !controller->IsPlayerController()) return false;
        watchedTeam = controller->GetTeam();
    }
    if(watchedTeam != 2 && watchedTeam != 3) return false;

    CEntityInstance * voiceController = GetEntityFromIndex((int)playerSlot + 1);
    if(nullptr == voiceController || !voiceController->IsPlayerController()) return false;

    return voiceController->GetTeam() == watchedTeam;
}

static void MirvPov_UpdateSyntheticSpeakingExpiry() {
    float curTime = g_MirvTime.curtime_get();
    for(int i = 0; i < 64; ++i) {
        if(0.0f < g_MirvPovSyntheticSpeakingUntil[i] && g_MirvPovSyntheticSpeakingUntil[i] <= curTime) {
            MirvPov_SetSyntheticSpeaking(i, false);
        }
    }
}

static bool New_MirvPov_IsPlayingDemo(void * This) {
    void * ret = _ReturnAddress();
    bool result = g_Org_MirvPov_IsPlayingDemo(This);
    if(MirvPov_IsEnabled() && g_MirvPovShowSpeakerRetAddr && (size_t)ret == g_MirvPovShowSpeakerRetAddr) {
        return false;
    }
    return result;
}

static __int64 __fastcall New_MirvPov_ServerVoiceData(__int64 This, __int64 msg) {
    unsigned int playerSlot = msg ? *(unsigned int *)(msg + 104) : 0xFFFFFFFF;
    __int64 result = g_Org_MirvPov_ServerVoiceData(This, msg);
    if(MirvPov_IsEnabled() && MirvPov_IsVoicePlayerSlotOnWatchedTeam(playerSlot)) {
        MirvPov_SetSyntheticSpeaking(playerSlot, true);
    }
    return result;
}

static bool MirvPov_ResolveVoiceHud(HMODULE clientDll) {
    if(!clientDll) return false;

    if(!g_MirvPovShowSpeakerRetAddr) {
        size_t matchAddr = getAddress(clientDll, "48 63 C3 48 8D 0D ?? ?? ?? ?? C6 84 08 ?? ?? ?? ?? 01 48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 90 ?? ?? ?? ?? 84 C0 0F 85");
        if(matchAddr) g_MirvPovShowSpeakerRetAddr = matchAddr + 0x22;
    }
    if(!g_MirvPovServerVoiceDataAddr) {
        g_MirvPovServerVoiceDataAddr = getAddress(clientDll, "48 89 4C 24 ?? 53 55 56 57 41 54 41 55 41 57 48 81 EC");
    }
    if(!g_MirvPovVoiceStatusGetAddr) {
        g_MirvPovVoiceStatusGetAddr = getAddress(clientDll, "48 8B 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 8D 05");
    }
    if(!g_MirvPovVoiceStatusUpdateSpeakerStatusAddr) {
        g_MirvPovVoiceStatusUpdateSpeakerStatusAddr = getAddress(clientDll, "44 88 4C 24 ?? 44 89 44 24 ?? 89 54 24");
    }

    return g_MirvPovShowSpeakerRetAddr && g_MirvPovServerVoiceDataAddr && MirvPov_IsVoiceHudReady();
}

void MirvPov_HookVoiceHud(HMODULE clientDll) {
    if(!MirvPov_ResolveVoiceHud(clientDll)) {
        advancedfx::Message("[mirv_pov_voice_hud] voice HUD patterns not found\n");
        return;
    }

    if(!g_bMirvPovIsPlayingDemoHooked) {
        if(!g_pEngineToClient) return;
        void ** vtable = *(void ***)g_pEngineToClient;
        if(!vtable) return;
        if(!AfxDetourPtr((PVOID*)&(vtable[kMirvPovIsPlayingDemoVtableIndex]), New_MirvPov_IsPlayingDemo, (PVOID*)&g_Org_MirvPov_IsPlayingDemo)) return;
        g_bMirvPovIsPlayingDemoHooked = true;
    }

    if(!g_bMirvPovServerVoiceDataHooked) {
        g_Org_MirvPov_ServerVoiceData = (MirvPov_ServerVoiceData_t)g_MirvPovServerVoiceDataAddr;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Org_MirvPov_ServerVoiceData, New_MirvPov_ServerVoiceData);
        if(NO_ERROR != DetourTransactionCommit()) return;
        g_bMirvPovServerVoiceDataHooked = true;
    }
}

void MirvPov_UpdateVoiceHud() {
    if(!MirvPov_IsEnabled() || !g_pEngineToClient) return;

    SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile();
    if(!pDemoFile) {
        g_MirvPovVoiceLastDemoTick = INT_MIN;
        MirvPov_ClearSyntheticSpeaking();
        return;
    }

    int curTick = pDemoFile->GetDemoTick();
    if(g_MirvPovVoiceLastDemoTick != INT_MIN) {
        int delta = curTick - g_MirvPovVoiceLastDemoTick;
        if(delta < 0 || delta > kMirvPovVoiceSeekThresholdTicks) {
            g_pEngineToClient->ExecuteClientCmd(0, "servervoice_clear", true);
            MirvPov_ClearSyntheticSpeaking();
        }
    }
    g_MirvPovVoiceLastDemoTick = curTick;
    MirvPov_UpdateVoiceTeam();
    MirvPov_UpdateSyntheticSpeakingExpiry();
}

void MirvPov_ResetVoiceHud() {
    g_MirvPovVoiceLastDemoTick = INT_MIN;
    g_MirvPovVoiceLastControllerIndex = -1;
    g_MirvPovVoiceLastTeam = -1;
}
