#include "stdafx.h"

#include "MirvVoiceHudFix.h"

#include "WrpConsole.h"
#include "Globals.h"
#include "addresses.h"
#include "MirvTime.h"

#include "../shared/AfxDetours.h"
#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"

#include <intrin.h>
#include <limits.h>

#pragma intrinsic(_ReturnAddress)

extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;

namespace {

bool g_Enabled = false;

// ISource2EngineToClient::IsPlayingDemo. Confirmed from the client.dll speaker-icon
// call site (call qword [rax+0x150] => 0x150/8) and matches MulNX_CS2.
const int kIsPlayingDemoVtableIndex = 42;

// A backward jump, or a forward jump larger than this between render frames, is a seek.
const int kSeekThresholdTicks = 16;
const float kSyntheticSpeakingSeconds = 0.65f;

int g_LastDemoTick = INT_MIN;
float g_SyntheticSpeakingUntil[64] = {};

typedef bool (*IsPlayingDemo_t)(void* This);
IsPlayingDemo_t g_OldIsPlayingDemo = nullptr;
bool g_IsPlayingDemoHookInstalled = false;

typedef __int64 (__fastcall *ServerVoiceData_t)(__int64 This, __int64 msg);
ServerVoiceData_t g_OldServerVoiceData = nullptr;
bool g_ServerVoiceDataHookInstalled = false;

typedef __int64 (__fastcall *VoiceStatus_Get_t)();
typedef __int64 (__fastcall *VoiceStatus_UpdateSpeakerStatus_t)(__int64 voiceStatus, unsigned int playerSlot, int localSlot, unsigned __int8 talking);

bool IsVoiceStatusReady() {
    return AFXADDR_GET(cs2_client_VoiceStatus_Get) && AFXADDR_GET(cs2_client_VoiceStatus_UpdateSpeakerStatus);
}

void SetSyntheticSpeaking(unsigned int playerSlot, bool speaking) {
    if (64 <= playerSlot || !IsVoiceStatusReady()) return;

    __int64 voiceStatus = ((VoiceStatus_Get_t)AFXADDR_GET(cs2_client_VoiceStatus_Get))();
    if (!voiceStatus) return;

    ((VoiceStatus_UpdateSpeakerStatus_t)AFXADDR_GET(cs2_client_VoiceStatus_UpdateSpeakerStatus))(voiceStatus, playerSlot, -1, speaking ? 1 : 0);
    g_SyntheticSpeakingUntil[playerSlot] = speaking ? g_MirvTime.curtime_get() + kSyntheticSpeakingSeconds : 0.0f;
}

void ClearSyntheticSpeaking() {
    for (int i = 0; i < 64; ++i) {
        if (0.0f < g_SyntheticSpeakingUntil[i])
            SetSyntheticSpeaking(i, false);
        g_SyntheticSpeakingUntil[i] = 0.0f;
    }
}

void UpdateSyntheticSpeakingExpiry() {
    float curTime = g_MirvTime.curtime_get();
    for (int i = 0; i < 64; ++i) {
        if (0.0f < g_SyntheticSpeakingUntil[i] && g_SyntheticSpeakingUntil[i] <= curTime)
            SetSyntheticSpeaking(i, false);
    }
}

// The game gates the speaker icon behind this IsPlayingDemo() call. Forcing it to false
// *only at that call site* makes the non-demo (icon-drawing) path run during demos, while
// every other IsPlayingDemo caller still sees the real value.
bool new_IsPlayingDemo(void* This) {
    void* ret = _ReturnAddress();
    bool result = g_OldIsPlayingDemo(This);
    if (g_Enabled) {
        AfxAddr retAddr = AFXADDR_GET(cs2_client_ShowSpeaker_IsPlayingDemo_RetAddr);
        if (retAddr && (AfxAddr)ret == retAddr)
            return false;
    }
    return result;
}

__int64 __fastcall new_ServerVoiceData(__int64 This, __int64 msg) {
    unsigned int playerSlot = msg ? *(unsigned int *)(msg + 104) : 0xFFFFFFFF;
    __int64 result = g_OldServerVoiceData(This, msg);
    if (g_Enabled && playerSlot < 64)
        SetSyntheticSpeaking(playerSlot, true);
    return result;
}

bool EnsureHookInstalled() {
    if (!g_IsPlayingDemoHookInstalled) {
        if (!g_pEngineToClient) return false;
        void** vtable = *(void***)g_pEngineToClient;
        if (!vtable) return false;
        if (!AfxDetourPtr((PVOID*)&(vtable[kIsPlayingDemoVtableIndex]), new_IsPlayingDemo, (PVOID*)&g_OldIsPlayingDemo))
            return false;
        g_IsPlayingDemoHookInstalled = true;
    }

    if (!g_ServerVoiceDataHookInstalled) {
        if (!AFXADDR_GET(cs2_client_ServerVoiceData)) return false;
        g_OldServerVoiceData = (ServerVoiceData_t)AFXADDR_GET(cs2_client_ServerVoiceData);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_OldServerVoiceData, new_ServerVoiceData);
        if (NO_ERROR != DetourTransactionCommit())
            return false;
        g_ServerVoiceDataHookInstalled = true;
    }

    return true;
}

} // namespace

void MirvVoiceHudFix_OnRenderPass() {
    if (!g_Enabled || !g_pEngineToClient) return;

    auto pDemoFile = g_pEngineToClient->GetDemoFile();
    if (!pDemoFile) { g_LastDemoTick = INT_MIN; ClearSyntheticSpeaking(); return; }

    int curTick = pDemoFile->GetDemoTick();
    if (g_LastDemoTick != INT_MIN) {
        int delta = curTick - g_LastDemoTick;
        if (delta < 0 || delta > kSeekThresholdTicks) {
            g_pEngineToClient->ExecuteClientCmd(0, "servervoice_clear", true);
            ClearSyntheticSpeaking();
        }
    }
    g_LastDemoTick = curTick;
    UpdateSyntheticSpeakingExpiry();
}

CON_COMMAND(mirv_voiceHudFix, "Show speaker (voice) icons in demo HUD; resets them on demo seek.") {
    int argc = args->ArgC();
    auto arg0 = args->ArgV(0);

    if (2 <= argc) {
        bool enable = 0 != atoi(args->ArgV(1));
        if (enable) {
            if (0 == AFXADDR_GET(cs2_client_ShowSpeaker_IsPlayingDemo_RetAddr)) {
                advancedfx::Warning("%s: speaker code site not found in this client.dll build; cannot enable.\n", arg0);
                return;
            }
            if (0 == AFXADDR_GET(cs2_client_ServerVoiceData)
                || 0 == AFXADDR_GET(cs2_client_VoiceStatus_Get)
                || 0 == AFXADDR_GET(cs2_client_VoiceStatus_UpdateSpeakerStatus)) {
                advancedfx::Warning("%s: voice HUD helper code not found in this client.dll build; cannot enable.\n", arg0);
                return;
            }
            if (!EnsureHookInstalled()) {
                advancedfx::Warning("%s: failed to install hook (engine interface not ready yet?).\n", arg0);
                return;
            }
        }
        else {
            ClearSyntheticSpeaking();
        }
        g_Enabled = enable;
        g_LastDemoTick = INT_MIN;
        advancedfx::Message("%s: %s\n", arg0, g_Enabled ? "enabled" : "disabled");
        return;
    }

    advancedfx::Message(
        "%s <0|1> - Show speaker (voice) icons in demo playback (default: 0).\n"
        "Current value: %d\n"
        "Speaker code site %s.\n"
        "Server voice hook site %s.\n"
        "VoiceStatus helpers %s.\n"
        , arg0, g_Enabled ? 1 : 0
        , AFXADDR_GET(cs2_client_ShowSpeaker_IsPlayingDemo_RetAddr) ? "resolved" : "NOT FOUND"
        , AFXADDR_GET(cs2_client_ServerVoiceData) ? "resolved" : "NOT FOUND"
        , IsVoiceStatusReady() ? "resolved" : "NOT FOUND"
    );
}
