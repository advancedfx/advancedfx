#include "stdafx.h"

#include "MirvPovHud.h"

#include "Globals.h"

#include "../shared/AfxConsole.h"
#include "../shared/binutils.h"

#define WIN32_LEAN_AND_MEAN
#include "../deps/release/Detours/src/detours.h"

#include <stdint.h>
#include <string.h>

static int g_IsLocalPlayerHLTV_SuppressFrames = 0;
static int g_IsLocalPlayerHLTV_LastDemoTick = -1;

void MirvPovHud_UpdateSeekDetection(int curTick) {
    if(g_IsLocalPlayerHLTV_LastDemoTick >= 0) {
        int delta = curTick - g_IsLocalPlayerHLTV_LastDemoTick;
        if(delta < 0) delta = -delta;
        if(delta > 2) {
            g_IsLocalPlayerHLTV_SuppressFrames = 16;
        }
    }
    g_IsLocalPlayerHLTV_LastDemoTick = curTick;
    if(g_IsLocalPlayerHLTV_SuppressFrames > 0) {
        g_IsLocalPlayerHLTV_SuppressFrames--;
    }
}

bool MirvPovHud_ShouldSuppressFrame() {
    return g_IsLocalPlayerHLTV_SuppressFrames > 0;
}

// ============================================================================
// Approach C: Hook GetLocalPlayerController + byte-patch spectator mode
// ============================================================================

// Hook sub_180AD5580 (GetObserverMode) — returns observer mode for slot 0.
// This function calls sub_1808E0E70(0) directly, bypassing our
// GetLocalPlayerController hook. The radar uses it to decide whether to
// show spectator-mode behavior. By returning 0 (OBS_MODE_NONE) when our
// POV radar is active, the radar treats us as the observed player directly.
typedef int (__fastcall * GetObserverMode_t)();
static GetObserverMode_t g_Org_GetObserverMode = nullptr;
static bool g_bGetObserverModeHooked = false;

static int __fastcall New_GetObserverMode() {
    return g_Org_GetObserverMode();
}

// Hook sub_180AD55C0 (GetObserverTarget) — returns observer target handle for slot 0.
// Like GetObserverMode, this calls sub_1808E0E70(0) directly, bypassing our
// GetLocalPlayerController hook. By returning INVALID_EHANDLE during frame context,
// the radar won't try to use a spectator target position.
typedef unsigned int (__fastcall * GetObserverTarget_fn_t)(void* thisPtr);
static GetObserverTarget_fn_t g_Org_GetObserverTarget_fn = nullptr;
static bool g_bGetObserverTargetHooked = false;

static unsigned int __fastcall New_GetObserverTarget_fn(void* thisPtr) {
    return g_Org_GetObserverTarget_fn(thisPtr);
}

// Hook GameStateAPI::IsLocalPlayerHLTV (sub_180EFF830) — Panorama bridge callback.
// The radar JS calls this to decide spectator vs player color mode.
// Return original behavior on the stable baseline.
typedef bool (__fastcall * IsLocalPlayerHLTV_t)();
static IsLocalPlayerHLTV_t g_Org_IsLocalPlayerHLTV = nullptr;
static bool g_bIsLocalPlayerHLTVHooked = false;

static bool __fastcall New_IsLocalPlayerHLTV() {
    return g_Org_IsLocalPlayerHLTV();
}

// Hook GameStateAPI::IsDemoOrHltv (sub_180EFEEE0) — Panorama bridge callback.
// Stable baseline keeps original demo/HLTV behavior.
typedef bool (__fastcall * IsDemoOrHltv_t)();
static IsDemoOrHltv_t g_Org_IsDemoOrHltv = nullptr;
static bool g_bIsDemoOrHltvHooked = false;

static bool __fastcall New_IsDemoOrHltv() {
    return g_Org_IsDemoOrHltv();
}

// Hook sub_180BD7830 (GetEffectiveLocalPlayer for HUD) — this function is
// used by the HUD to determine spectator state. It calls sub_1808E0E70(0)
// directly, bypassing our GetLocalPlayerController hook.
// Instead of hooking the function (which crashes during demo transitions),
// we patch the HUD's spectator check: cmp byte ptr [rax+3EBh], 1 → 0xFF
static uint8_t * g_pHudSpectatorCheckPatchAddr = nullptr;
static uint8_t g_HudSpectatorCheckOrigByte = 0;
static bool g_bHudSpectatorCheckPatched = false;

// Patch 1: Force [rbx+174F0h] = 0 (not spectating any target)
//   Original: test al, al / jz short +0A  (84 C0 74 0A)
//   Patched:  test al, al / jmp short +0A (84 C0 EB 0A)
// Patch 4: Hide spectator player panel (HudSpecplayerRoot--visible always false)
//   Original: mov sil, 1  (40 B6 01)
//   Patched:  xor sil, sil (40 32 F6)
//   Pattern context: test r14,r14 / jz +9 / test bl,bl / jnz +5 / [PATCH HERE] / jmp +3 / xor sil,sil
static uint8_t * g_pHudSpecPanelPatchAddr = nullptr;
static uint8_t g_HudSpecPanelOrigBytes[3] = {0};
static bool g_bHudSpecPanelPatched = false;

void MirvPovHud_ApplyPatches(HMODULE clientDll) {
    if(g_bHudSpectatorCheckPatched && g_bHudSpecPanelPatched && g_bGetObserverModeHooked && g_bGetObserverTargetHooked && g_bIsLocalPlayerHLTVHooked && g_bIsDemoOrHltvHooked) return;
    if(nullptr == clientDll) {
        advancedfx::Message("[mirv_pov_radar_patch] No client.dll handle\n");
        return;
    }

    // --- Hook GetObserverMode (sub_180AD5580) — return OBS_MODE_NONE during frame context ---
    if(!g_bGetObserverModeHooked) {
        size_t funcAddr = getAddress(clientDll, "48 83 EC 28 33 C9 E8 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 88 F8 11 00 00");
        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] GetObserverMode pattern not found\n");
        } else {
            g_Org_GetObserverMode = (GetObserverMode_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_GetObserverMode, New_GetObserverMode);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bGetObserverModeHooked = true;
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] GetObserverMode detour failed\n");
                g_Org_GetObserverMode = nullptr;
            }
        }
    }

    // --- Hook GetObserverTarget (sub_180AD55C0) — return INVALID_EHANDLE during frame context ---
    if(!g_bGetObserverTargetHooked) {
        size_t funcAddr = getAddress(clientDll, "40 53 48 83 EC 20 48 8B D9 33 C9 E8 ?? ?? ?? ?? 48 85 C0 74");
        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] GetObserverTarget pattern not found\n");
        } else {
            g_Org_GetObserverTarget_fn = (GetObserverTarget_fn_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_GetObserverTarget_fn, New_GetObserverTarget_fn);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bGetObserverTargetHooked = true;
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] GetObserverTarget detour failed\n");
                g_Org_GetObserverTarget_fn = nullptr;
            }
        }
    }

    // --- Hook IsLocalPlayerHLTV (Panorama GameStateAPI callback) ---
    // DISABLED: interferes with xray / head markers in demo POV. Kept code for reference.
    if(false) {
        size_t funcAddr = getAddress(clientDll, "48 83 EC ?? 33 C9 E8 ?? ?? ?? ?? 48 85 C0 74 ?? 80 B8");
        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] IsLocalPlayerHLTV pattern not found\n");
        } else {
            g_Org_IsLocalPlayerHLTV = (IsLocalPlayerHLTV_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_IsLocalPlayerHLTV, New_IsLocalPlayerHLTV);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bIsLocalPlayerHLTVHooked = true;
                advancedfx::Message("[mirv_pov_radar_patch] Hooked IsLocalPlayerHLTV at %p\n", (void*)funcAddr);
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] IsLocalPlayerHLTV detour failed\n");
                g_Org_IsLocalPlayerHLTV = nullptr;
            }
        }
    }

    // --- IsDemoOrHltv hook: DISABLED (interferes with xray / head markers in demo POV) ---
    if(false) {
        size_t funcAddr = 0;
        unsigned char * base = (unsigned char *)clientDll;
        IMAGE_DOS_HEADER * dosHeader = (IMAGE_DOS_HEADER *)base;
        IMAGE_NT_HEADERS * ntHeaders = (IMAGE_NT_HEADERS *)(base + dosHeader->e_lfanew);
        size_t size = ntHeaders->OptionalHeader.SizeOfImage;

        const char * searchStr = "IsDemoOrHltv";
        size_t searchLen = strlen(searchStr);
        size_t strAddr = 0;

        for(size_t i = 0; i + searchLen < size; i++) {
            if(0 == memcmp(base + i, searchStr, searchLen + 1)) {
                strAddr = (size_t)(base + i);
                break;
            }
        }

        if(strAddr) {
            // Find LEA instruction referencing this string (RIP-relative: REX.W 8D ModRM[rm=5] disp32)
            for(size_t i = 0; i + 7 < size; i++) {
                unsigned char * p = base + i;
                if((p[0] == 0x48 || p[0] == 0x4C) && p[1] == 0x8D && (p[2] & 0x07) == 0x05) {
                    int32_t disp = *(int32_t *)(p + 3);
                    size_t target = (size_t)(p + 7) + disp;
                    if(target == strAddr) {
                        // Found LEA loading "IsDemoOrHltv". Scan nearby for another LEA (function ptr).
                        for(int delta = -64; delta <= 64; delta++) {
                            if(delta >= -3 && delta <= 6) continue;
                            unsigned char * q = p + delta;
                            if(q < base || q + 7 >= base + size) continue;
                            if((q[0] == 0x48 || q[0] == 0x4C) && q[1] == 0x8D && (q[2] & 0x07) == 0x05) {
                                int32_t disp2 = *(int32_t *)(q + 3);
                                size_t candidate = (size_t)(q + 7) + disp2;
                                if(candidate >= (size_t)base && candidate < (size_t)base + size) {
                                    unsigned char * cand = (unsigned char *)candidate;
                                    // Heuristic: looks like function prologue
                                    if(cand[0] == 0x48 || cand[0] == 0x40 || cand[0] == 0x55 ||
                                       cand[0] == 0x53 || cand[0] == 0x56 || cand[0] == 0x41 ||
                                       cand[0] == 0xB0 || (cand[0] == 0x33 && cand[1] == 0xC0) ||
                                       cand[0] == 0x8B) {
                                        funcAddr = candidate;
                                        advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv: string at %p, LEA at %p, func at %p (bytes: %02X %02X %02X %02X)\n",
                                            (void*)strAddr, (void*)(size_t)p, (void*)funcAddr,
                                            cand[0], cand[1], cand[2], cand[3]);
                                        break;
                                    }
                                }
                            }
                        }
                        if(funcAddr) break;
                    }
                }
            }
        } else {
            advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv: string not found in client.dll\n");
        }

        if(0 == funcAddr) {
            advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv: function not found\n");
            g_bIsDemoOrHltvHooked = true;
        } else {
            g_Org_IsDemoOrHltv = (IsDemoOrHltv_t)funcAddr;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Org_IsDemoOrHltv, New_IsDemoOrHltv);
            if(NO_ERROR == DetourTransactionCommit()) {
                g_bIsDemoOrHltvHooked = true;
                advancedfx::Message("[mirv_pov_radar_patch] Hooked IsDemoOrHltv at %p\n", (void*)funcAddr);
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] IsDemoOrHltv detour failed\n");
                g_Org_IsDemoOrHltv = nullptr;
                g_bIsDemoOrHltvHooked = true;
            }
        }
    }

    // --- Patch 3: HUD spectator check (cmp byte ptr [rax+3EBh], 1 → 0xFF) ---
    // DISABLED: this toggles the Panorama "HUD--localplayer--spectator" CSS class,
    // which also drives spectator head markers / xray overlay. Forcing it off removed
    // those. Bottom spectator bar is still hidden separately by Patch 4.
    if(false) {
        size_t match3 = getAddress(clientDll, "80 B8 EB 03 00 00 01 48 8B 11 41 0F 94 C0");
        if(0 == match3) {
            advancedfx::Message("[mirv_pov_radar_patch] HUD spectator check pattern not found\n");
        } else {
            uint8_t * patchAddr = (uint8_t *)(match3 + 6);
            g_HudSpectatorCheckOrigByte = *patchAddr;

            DWORD oldProtect;
            if(VirtualProtect(patchAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *patchAddr = 0xFF;
                DWORD dummy;
                VirtualProtect(patchAddr, 1, oldProtect, &dummy);
                g_pHudSpectatorCheckPatchAddr = patchAddr;
                g_bHudSpectatorCheckPatched = true;
                advancedfx::Message("[mirv_pov_radar_patch] Patched HUD spectator check at %p (0x01->0xFF)\n", (void*)patchAddr);
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for HUD spectator patch (error %lu)\n", GetLastError());
            }
        }
    }

    // --- Patch 4: Hide spectator player panel (mov sil,1 → xor sil,sil) ---
    if(!g_bHudSpecPanelPatched) {
        size_t match4 = getAddress(clientDll, "4D 85 F6 74 09 84 DB 75 05 40 B6 01 EB 03 40 32 F6");
        if(0 == match4) {
            advancedfx::Message("[mirv_pov_radar_patch] HUD spec panel pattern not found\n");
        } else {
            uint8_t * patchAddr = (uint8_t *)(match4 + 9);
            memcpy(g_HudSpecPanelOrigBytes, patchAddr, 3);

            DWORD oldProtect;
            if(VirtualProtect(patchAddr, 3, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                patchAddr[0] = 0x40;
                patchAddr[1] = 0x32;
                patchAddr[2] = 0xF6;
                DWORD dummy;
                VirtualProtect(patchAddr, 3, oldProtect, &dummy);
                g_pHudSpecPanelPatchAddr = patchAddr;
                g_bHudSpecPanelPatched = true;
            } else {
                advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for HUD spec panel patch (error %lu)\n", GetLastError());
            }
        }
    }

    return;
}

void MirvPovHud_RemovePatches() {
    if(g_bGetObserverModeHooked && g_Org_GetObserverMode) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_GetObserverMode, New_GetObserverMode);
        DetourTransactionCommit();
        g_bGetObserverModeHooked = false;
    }

    if(g_bGetObserverTargetHooked && g_Org_GetObserverTarget_fn) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_GetObserverTarget_fn, New_GetObserverTarget_fn);
        DetourTransactionCommit();
        g_bGetObserverTargetHooked = false;
    }

    if(g_bIsLocalPlayerHLTVHooked && g_Org_IsLocalPlayerHLTV) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_IsLocalPlayerHLTV, New_IsLocalPlayerHLTV);
        DetourTransactionCommit();
        g_bIsLocalPlayerHLTVHooked = false;
        g_IsLocalPlayerHLTV_SuppressFrames = 0;
        g_IsLocalPlayerHLTV_LastDemoTick = -1;
        advancedfx::Message("[mirv_pov_radar_patch] Unhooked IsLocalPlayerHLTV\n");
    }

    if(g_bIsDemoOrHltvHooked && g_Org_IsDemoOrHltv) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_Org_IsDemoOrHltv, New_IsDemoOrHltv);
        DetourTransactionCommit();
        g_bIsDemoOrHltvHooked = false;
        advancedfx::Message("[mirv_pov_radar_patch] Unhooked IsDemoOrHltv\n");
    }

    if(g_bHudSpectatorCheckPatched && g_pHudSpectatorCheckPatchAddr) {
        DWORD oldProtect;
        if(VirtualProtect(g_pHudSpectatorCheckPatchAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *g_pHudSpectatorCheckPatchAddr = g_HudSpectatorCheckOrigByte;
            DWORD dummy;
            VirtualProtect(g_pHudSpectatorCheckPatchAddr, 1, oldProtect, &dummy);
        }
        g_bHudSpectatorCheckPatched = false;
        g_pHudSpectatorCheckPatchAddr = nullptr;
        advancedfx::Message("[mirv_pov_radar_patch] Restored HUD spectator check\n");
    }

    if(g_bHudSpecPanelPatched && g_pHudSpecPanelPatchAddr) {
        DWORD oldProtect;
        if(VirtualProtect(g_pHudSpecPanelPatchAddr, 3, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pHudSpecPanelPatchAddr, g_HudSpecPanelOrigBytes, 3);
            DWORD dummy;
            VirtualProtect(g_pHudSpecPanelPatchAddr, 3, oldProtect, &dummy);
        }
        g_bHudSpecPanelPatched = false;
        g_pHudSpecPanelPatchAddr = nullptr;
    }

}

