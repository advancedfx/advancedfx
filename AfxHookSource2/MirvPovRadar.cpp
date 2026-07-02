#include "stdafx.h"

#include "MirvPovRadar.h"

#include "ClientEntitySystem.h"
#include "Globals.h"

#include "../shared/AfxConsole.h"
#include "../shared/binutils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdint.h>
#include <string.h>

static uint8_t * g_pRadarSpectatorTargetPatchAddr = nullptr;
static uint8_t g_RadarSpectatorTargetOrigByte = 0;
static bool g_bRadarSpectatorTargetPatched = false;

// Patch 2: NOP "or byte ptr [rbx+17760h], 1" (prevent show-all flag)
static uint8_t * g_pRadarShowAllPatchAddr = nullptr;
static uint8_t g_RadarShowAllOriginalBytes[7] = {0};
static bool g_bRadarShowAllPatched = false;

// Patch 8: Hook radar spot visibility check (inspired by MulNX_CS2)
// Pattern: "38 5C 24 ?? 0F 84 ?? ?? ?? ?? 48 8B 0D"
static uint8_t * g_pRadarSpotCheckPatchAddr = nullptr;
static uint8_t g_RadarSpotCheckOrigBytes[16] = {0};
static uint8_t * g_pRadarSpotCheckTrampoline = nullptr;
static size_t g_RadarSpotCheckPatchSize = 0;
static bool g_bRadarSpotCheckPatched = false;

// Patch 10 (enemy-red, ported from MulNX_CS2 Pos_WriteMaybeEnumToChangeRadarPlayerDraw):
// mid-function hook at the instruction that writes the radar blip color enum (in rbx)
// to radar-entry +0x16C. Enum values: 9=draw-as-CT, 13=draw-as-T, 17=draw-as-enemy,
// 21=local. In a demo the observer has no team so opponents render as their own team
// color instead of red. We rewrite the enum to 17 when the blip's team color differs
// from the observed player's team. Does NOT depend on the v7/competitive path.
// Pattern: "48 8B 6C 24 ?? 41 39 9E 6C 01 00 00" at 0x180e087d3.
static uint8_t * g_pRadarColorPatchAddr = nullptr;
static uint8_t g_RadarColorOrigBytes[16] = {0};
static uint8_t * g_pRadarColorTrampoline = nullptr;
static size_t g_RadarColorPatchSize = 0;
static bool g_bRadarColorPatched = false;

// Patch 11 (teammate competitive colors): in sub_180E15350 (the match radar color
// applier) force the spectator/team-counter branch off so the match color path runs.
// The demo competitive-mode predicates in sub_180826B30 and sub_18080E180 are scoped
// to the radar tick so valid m_iCompTeammateColor data can drive teammate colors.
static bool MirvPov_ShouldForceRadarSpot(CEntityInstance * targetPawn) {
    if(!MirvPov_IsEnabled()) return false;
    if(nullptr == targetPawn) return false;

    __try {
        if(!targetPawn->IsPlayerPawn()) return false;
        CEntityInstance * fakeController = GetFakePovRadarController();
        if(nullptr == fakeController) return false;
        int fakeTeam = fakeController->GetTeam();
        if(fakeTeam != 2 && fakeTeam != 3) return false;
        return targetPawn->GetTeam() == fakeTeam;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void EmitU8(uint8_t * code, size_t & pos, uint8_t value) {
    code[pos++] = value;
}

static void EmitU32(uint8_t * code, size_t & pos, uint32_t value) {
    memcpy(code + pos, &value, sizeof(value));
    pos += sizeof(value);
}

static void EmitU64(uint8_t * code, size_t & pos, uint64_t value) {
    memcpy(code + pos, &value, sizeof(value));
    pos += sizeof(value);
}

static bool EmitRel32Jump(uint8_t * code, size_t & pos, uint8_t * target) {
    uint8_t * next = code + pos + 5;
    intptr_t rel = target - next;
    if(rel < INT32_MIN || rel > INT32_MAX) return false;
    EmitU8(code, pos, 0xE9);
    EmitU32(code, pos, (uint32_t)(int32_t)rel);
    return true;
}

static bool EmitRel32Jcc(uint8_t * code, size_t & pos, uint8_t conditionOpcode, uint8_t * target) {
    uint8_t * next = code + pos + 6;
    intptr_t rel = target - next;
    if(rel < INT32_MIN || rel > INT32_MAX) return false;
    EmitU8(code, pos, 0x0F);
    EmitU8(code, pos, conditionOpcode);
    EmitU32(code, pos, (uint32_t)(int32_t)rel);
    return true;
}

static uint8_t * MirvPov_AllocNear(uint8_t * target, size_t size) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    size_t granularity = systemInfo.dwAllocationGranularity;
    uintptr_t targetAddr = (uintptr_t)target;
    uintptr_t minAddr = targetAddr > 0x7fff0000 ? targetAddr - 0x7fff0000 : 0;
    uintptr_t maxAddr = targetAddr + 0x7fff0000;

    for(uintptr_t offset = 0; offset < 0x7fff0000; offset += granularity) {
        if(targetAddr >= offset + granularity) {
            uintptr_t addr = (targetAddr - offset) & ~(granularity - 1);
            if(addr >= minAddr) {
                if(void * result = VirtualAlloc((void*)addr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) {
                    return (uint8_t*)result;
                }
            }
        }

        uintptr_t addr = (targetAddr + offset + granularity - 1) & ~(granularity - 1);
        if(addr <= maxAddr) {
            if(void * result = VirtualAlloc((void*)addr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) {
                return (uint8_t*)result;
            }
        }
    }

    return nullptr;
}


// MulNX latest color-path patches inside sub_180E15350:
// 1) Pos_CmpToSetColor  @ 0x180e15412: force bl=0 so the match-color branch is taken.
// 2) Pos_CmpToSetCTColor @ 0x180e15540: force eax=3 (CT) in the CT compare branch.
// 3) Pos_CmpToSetTColor  @ 0x180e155d4: force eax=2 (T)  in the T compare branch.
// Implemented as local code patches / trampolines (not direct detours) so we can
// keep control over the exact register state at each mid-function site.
static uint8_t * g_pRadarMulColorPatchAddr = nullptr;   // site 1
static uint8_t g_RadarMulColorOrigBytes[16] = {0};
static uint8_t * g_pRadarMulColorTrampoline = nullptr;
static size_t g_RadarMulColorPatchSize = 0;
static bool g_bRadarMulColorPatched = false;

static uint8_t * g_pRadarMulCTPatchAddr = nullptr;      // site 2
static uint8_t g_RadarMulCTOrigBytes[16] = {0};
static uint8_t * g_pRadarMulCTTrampoline = nullptr;
static size_t g_RadarMulCTPatchSize = 0;
static bool g_bRadarMulCTPatched = false;

static uint8_t * g_pRadarMulTPatchAddr = nullptr;       // site 3
static uint8_t g_RadarMulTOrigBytes[16] = {0};
static uint8_t * g_pRadarMulTTrampoline = nullptr;
static size_t g_RadarMulTPatchSize = 0;
static bool g_bRadarMulTPatched = false;

static bool MirvPov_PatchRadarMulColor(HMODULE clientDll) {
    if(g_bRadarMulColorPatched) return true;

    size_t matchAddr = getAddress(clientDll, "4C 89 6C 24 ?? 84 DB 0F 84");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] MulColor path pattern not found\n");
        return false;
    }

    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 13;
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarMulColorOrigBytes, patchAddr, patchSize);

    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 96);
    if(nullptr == trampoline) {
        advancedfx::Message("[mirv_pov_radar_patch] Failed to allocate MulColor trampoline\n");
        return false;
    }

    int32_t jccRel = *(int32_t *)(patchAddr + 9);
    uint8_t * matchColorAddr = patchAddr + patchSize + jccRel;

    size_t pos = 0;
    EmitU8(trampoline, pos, 0x4C); EmitU8(trampoline, pos, 0x89); EmitU8(trampoline, pos, 0x6C); EmitU8(trampoline, pos, 0x24); EmitU8(trampoline, pos, patchAddr[4]);
    EmitU8(trampoline, pos, 0x30); EmitU8(trampoline, pos, 0xDB);
    if(!EmitRel32Jump(trampoline, pos, matchColorAddr)) { VirtualFree(trampoline, 0, MEM_RELEASE); return false; }

    DWORD oldProtect;
    if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5);
        patchAddr[0] = 0xE9; *(int32_t *)(patchAddr + 1) = (int32_t)rel; memset(patchAddr + 5, 0x90, patchSize - 5);
        FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize);
        DWORD dummy; VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarMulColorPatchAddr = patchAddr; g_pRadarMulColorTrampoline = trampoline; g_RadarMulColorPatchSize = patchSize; g_bRadarMulColorPatched = true;
        return true;
    }

    VirtualFree(trampoline, 0, MEM_RELEASE);
    return false;
}

static bool MirvPov_PatchRadarMulCT(HMODULE clientDll) {
    if(g_bRadarMulCTPatched) return true;
    size_t matchAddr = getAddress(clientDll, "E8 ?? ?? ?? ?? 83 F8 03 75 ?? 8B D3");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] MulCT path pattern not found\n");
        return false;
    }
    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 5;
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarMulCTOrigBytes, patchAddr, patchSize);
    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 64);
    if(nullptr == trampoline) return false;

    size_t pos = 0;
    EmitU8(trampoline, pos, 0xB8); EmitU32(trampoline, pos, 3);
    if(!EmitRel32Jump(trampoline, pos, returnAddr)) { VirtualFree(trampoline,0,MEM_RELEASE); return false; }

    DWORD oldProtect; if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5); patchAddr[0]=0xE9; *(int32_t*)(patchAddr+1)=(int32_t)rel; FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize); DWORD dummy; VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarMulCTPatchAddr = patchAddr; g_pRadarMulCTTrampoline = trampoline; g_RadarMulCTPatchSize = patchSize; g_bRadarMulCTPatched = true; return true;
    }
    VirtualFree(trampoline,0,MEM_RELEASE); return false;
}

static bool MirvPov_PatchRadarMulT(HMODULE clientDll) {
    if(g_bRadarMulTPatched) return true;
    size_t matchAddr = getAddress(clientDll, "E8 ?? ?? ?? ?? 41 3B C5 0F 85 ?? ?? ?? ?? F6 86");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] MulT path pattern not found\n");
        return false;
    }
    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 5;
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarMulTOrigBytes, patchAddr, patchSize);
    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 64);
    if(nullptr == trampoline) return false;

    size_t pos = 0;
    EmitU8(trampoline, pos, 0xB8); EmitU32(trampoline, pos, 2);
    if(!EmitRel32Jump(trampoline, pos, returnAddr)) { VirtualFree(trampoline,0,MEM_RELEASE); return false; }

    DWORD oldProtect; if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5); patchAddr[0]=0xE9; *(int32_t*)(patchAddr+1)=(int32_t)rel; FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize); DWORD dummy; VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarMulTPatchAddr = patchAddr; g_pRadarMulTTrampoline = trampoline; g_RadarMulTPatchSize = patchSize; g_bRadarMulTPatched = true; return true;
    }
    VirtualFree(trampoline,0,MEM_RELEASE); return false;
}

static void MirvPov_RestoreRadarMulColor() {
    if(g_bRadarMulColorPatched && g_pRadarMulColorPatchAddr) {
        DWORD oldProtect; if(VirtualProtect(g_pRadarMulColorPatchAddr, g_RadarMulColorPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pRadarMulColorPatchAddr, g_RadarMulColorOrigBytes, g_RadarMulColorPatchSize); FlushInstructionCache(GetCurrentProcess(), g_pRadarMulColorPatchAddr, g_RadarMulColorPatchSize); DWORD dummy; VirtualProtect(g_pRadarMulColorPatchAddr, g_RadarMulColorPatchSize, oldProtect, &dummy);
        }
        if(g_pRadarMulColorTrampoline) VirtualFree(g_pRadarMulColorTrampoline,0,MEM_RELEASE);
        g_bRadarMulColorPatched=false; g_pRadarMulColorPatchAddr=nullptr; g_pRadarMulColorTrampoline=nullptr; g_RadarMulColorPatchSize=0;
    }
}

static void MirvPov_RestoreRadarMulCT() {
    if(g_bRadarMulCTPatched && g_pRadarMulCTPatchAddr) {
        DWORD oldProtect; if(VirtualProtect(g_pRadarMulCTPatchAddr, g_RadarMulCTPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pRadarMulCTPatchAddr, g_RadarMulCTOrigBytes, g_RadarMulCTPatchSize); FlushInstructionCache(GetCurrentProcess(), g_pRadarMulCTPatchAddr, g_RadarMulCTPatchSize); DWORD dummy; VirtualProtect(g_pRadarMulCTPatchAddr, g_RadarMulCTPatchSize, oldProtect, &dummy);
        }
        if(g_pRadarMulCTTrampoline) VirtualFree(g_pRadarMulCTTrampoline,0,MEM_RELEASE);
        g_bRadarMulCTPatched=false; g_pRadarMulCTPatchAddr=nullptr; g_pRadarMulCTTrampoline=nullptr; g_RadarMulCTPatchSize=0;
    }
}

static void MirvPov_RestoreRadarMulT() {
    if(g_bRadarMulTPatched && g_pRadarMulTPatchAddr) {
        DWORD oldProtect; if(VirtualProtect(g_pRadarMulTPatchAddr, g_RadarMulTPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_pRadarMulTPatchAddr, g_RadarMulTOrigBytes, g_RadarMulTPatchSize); FlushInstructionCache(GetCurrentProcess(), g_pRadarMulTPatchAddr, g_RadarMulTPatchSize); DWORD dummy; VirtualProtect(g_pRadarMulTPatchAddr, g_RadarMulTPatchSize, oldProtect, &dummy);
        }
        if(g_pRadarMulTTrampoline) VirtualFree(g_pRadarMulTTrampoline,0,MEM_RELEASE);
        g_bRadarMulTPatched=false; g_pRadarMulTPatchAddr=nullptr; g_pRadarMulTTrampoline=nullptr; g_RadarMulTPatchSize=0;
    }
}

static bool MirvPov_PatchRadarSpotCheck(HMODULE clientDll) {
    if(g_bRadarSpotCheckPatched) return true;

    size_t matchAddr = getAddress(clientDll, "38 5C 24 ?? 0F 84 ?? ?? ?? ?? 48 8B 0D");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] Radar spot check pattern not found\n");
        return false;
    }

    uint8_t * patchAddr = (uint8_t *)matchAddr;
    uint8_t * renderAddr = patchAddr + 10;
    int32_t originalSkipRel = *(int32_t *)(patchAddr + 6);
    uint8_t * skipAddr = patchAddr + 10 + originalSkipRel;
    size_t patchSize = 10;
    memcpy(g_RadarSpotCheckOrigBytes, patchAddr, patchSize);

    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 256);
    if(nullptr == trampoline) {
        advancedfx::Message("[mirv_pov_radar_patch] Failed to allocate nearby radar spot trampoline\n");
        return false;
    }

    size_t pos = 0;
    uint8_t stackOffset = patchAddr[3];

    uint8_t pushRegs[] = {
        0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57,
        0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,
        0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57
    };
    memcpy(trampoline + pos, pushRegs, sizeof(pushRegs));
    pos += sizeof(pushRegs);

    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xEC); EmitU8(trampoline, pos, 0x28);
    EmitU8(trampoline, pos, 0x4C); EmitU8(trampoline, pos, 0x89); EmitU8(trampoline, pos, 0xF9);
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0xB8); EmitU64(trampoline, pos, (uint64_t)&MirvPov_ShouldForceRadarSpot);
    EmitU8(trampoline, pos, 0xFF); EmitU8(trampoline, pos, 0xD0);
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xC4); EmitU8(trampoline, pos, 0x28);
    EmitU8(trampoline, pos, 0x84); EmitU8(trampoline, pos, 0xC0);

    uint8_t * jnzPos = trampoline + pos;
    pos += 6;

    uint8_t popRegs[] = {
        0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C,
        0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58,
        0x5F, 0x5E, 0x5D, 0x5B, 0x5A, 0x59, 0x58
    };
    memcpy(trampoline + pos, popRegs, sizeof(popRegs));
    pos += sizeof(popRegs);

    EmitU8(trampoline, pos, 0x38); EmitU8(trampoline, pos, 0x5C); EmitU8(trampoline, pos, 0x24); EmitU8(trampoline, pos, stackOffset);
    if(!EmitRel32Jcc(trampoline, pos, 0x84, skipAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }
    if(!EmitRel32Jump(trampoline, pos, renderAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    uint8_t * forceRenderAddr = trampoline + pos;
    memcpy(trampoline + pos, popRegs, sizeof(popRegs));
    pos += sizeof(popRegs);
    if(!EmitRel32Jump(trampoline, pos, renderAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    intptr_t forceRel = forceRenderAddr - (jnzPos + 6);
    if(forceRel < INT32_MIN || forceRel > INT32_MAX) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }
    jnzPos[0] = 0x0F;
    jnzPos[1] = 0x85;
    *(int32_t *)(jnzPos + 2) = (int32_t)forceRel;

    DWORD oldProtect;
    if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5);
        if(rel < INT32_MIN || rel > INT32_MAX) {
            DWORD dummy;
            VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            return false;
        }

        patchAddr[0] = 0xE9;
        *(int32_t *)(patchAddr + 1) = (int32_t)rel;
        memset(patchAddr + 5, 0x90, patchSize - 5);
        FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize);

        DWORD dummy;
        VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarSpotCheckPatchAddr = patchAddr;
        g_pRadarSpotCheckTrampoline = trampoline;
        g_RadarSpotCheckPatchSize = patchSize;
        g_bRadarSpotCheckPatched = true;
        return true;
    }

    VirtualFree(trampoline, 0, MEM_RELEASE);
    advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for radar spot check (error %lu)\n", GetLastError());
    return false;
}

static void MirvPov_RestoreRadarSpotCheck() {
    if(!g_bRadarSpotCheckPatched || !g_pRadarSpotCheckPatchAddr) return;

    DWORD oldProtect;
    if(VirtualProtect(g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckOrigBytes, g_RadarSpotCheckPatchSize);
        FlushInstructionCache(GetCurrentProcess(), g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckPatchSize);
        DWORD dummy;
        VirtualProtect(g_pRadarSpotCheckPatchAddr, g_RadarSpotCheckPatchSize, oldProtect, &dummy);
    }
    if(g_pRadarSpotCheckTrampoline) {
        VirtualFree(g_pRadarSpotCheckTrampoline, 0, MEM_RELEASE);
    }
    g_bRadarSpotCheckPatched = false;
    g_pRadarSpotCheckPatchAddr = nullptr;
    g_pRadarSpotCheckTrampoline = nullptr;
    g_RadarSpotCheckPatchSize = 0;
}

// Helper for Patch 10: given the radar blip color enum (9=CT,13=T,17=enemy,21=local),
// return 17 (enemy red) when the blip's team color is opposite to the observed
// player's team. Mirrors MulNX's enemy-red logic.
static int __fastcall MirvPov_AdjustRadarColor(int enumVal) {
    if(!MirvPov_IsEnabled()) return enumVal;
    __try {
        CEntityInstance * fakeController = GetFakePovRadarController();
        if(nullptr == fakeController) return enumVal;
        int obsTeam = fakeController->GetTeam(); // 2=T, 3=CT
        if(obsTeam == 3 && enumVal == 13) return 17; // observing CT: a T-colored blip is enemy
        if(obsTeam == 2 && enumVal == 9)  return 17; // observing T:  a CT-colored blip is enemy
        return enumVal;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return enumVal;
    }
}

static bool MirvPov_PatchRadarColor(HMODULE clientDll) {
    if(g_bRadarColorPatched) return true;

    size_t matchAddr = getAddress(clientDll, "48 8B 6C 24 ?? 41 39 9E 6C 01 00 00");
    if(0 == matchAddr) {
        advancedfx::Message("[mirv_pov_radar_patch] Radar color enum pattern not found\n");
        return false;
    }

    uint8_t * patchAddr = (uint8_t *)matchAddr;
    size_t patchSize = 12; // two instrs: mov rbp,[rsp+48] (5) + cmp [r14+16C],ebx (7)
    uint8_t * returnAddr = patchAddr + patchSize;
    memcpy(g_RadarColorOrigBytes, patchAddr, patchSize);

    uint8_t * trampoline = MirvPov_AllocNear(patchAddr, 256);
    if(nullptr == trampoline) {
        advancedfx::Message("[mirv_pov_radar_patch] Failed to allocate nearby radar color trampoline\n");
        return false;
    }

    size_t pos = 0;
    uint8_t pushRegs[] = {
        0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57,
        0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,
        0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57
    };
    memcpy(trampoline + pos, pushRegs, sizeof(pushRegs));
    pos += sizeof(pushRegs);

    EmitU8(trampoline, pos, 0x8B); EmitU8(trampoline, pos, 0xCB);                       // mov ecx, ebx
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xEC); EmitU8(trampoline, pos, 0x28); // sub rsp,0x28
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0xB8); EmitU64(trampoline, pos, (uint64_t)&MirvPov_AdjustRadarColor); // mov rax,&helper
    EmitU8(trampoline, pos, 0xFF); EmitU8(trampoline, pos, 0xD0);                       // call rax
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x83); EmitU8(trampoline, pos, 0xC4); EmitU8(trampoline, pos, 0x28); // add rsp,0x28
    EmitU8(trampoline, pos, 0x89); EmitU8(trampoline, pos, 0x84); EmitU8(trampoline, pos, 0x24); EmitU32(trampoline, pos, 0x58); // mov [rsp+0x58],eax (rbx slot)

    uint8_t popRegs[] = {
        0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C,
        0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58,
        0x5F, 0x5E, 0x5D, 0x5B, 0x5A, 0x59, 0x58
    };
    memcpy(trampoline + pos, popRegs, sizeof(popRegs));
    pos += sizeof(popRegs);

    // relocated original instructions
    EmitU8(trampoline, pos, 0x48); EmitU8(trampoline, pos, 0x8B); EmitU8(trampoline, pos, 0x6C); EmitU8(trampoline, pos, 0x24); EmitU8(trampoline, pos, 0x48); // mov rbp,[rsp+48]
    EmitU8(trampoline, pos, 0x41); EmitU8(trampoline, pos, 0x39); EmitU8(trampoline, pos, 0x9E); EmitU32(trampoline, pos, 0x16C); // cmp [r14+16C],ebx
    if(!EmitRel32Jump(trampoline, pos, returnAddr)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    DWORD oldProtect;
    if(VirtualProtect(patchAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        intptr_t rel = trampoline - (patchAddr + 5);
        if(rel < INT32_MIN || rel > INT32_MAX) {
            DWORD dummy;
            VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            return false;
        }
        patchAddr[0] = 0xE9;
        *(int32_t *)(patchAddr + 1) = (int32_t)rel;
        memset(patchAddr + 5, 0x90, patchSize - 5);
        FlushInstructionCache(GetCurrentProcess(), patchAddr, patchSize);
        DWORD dummy;
        VirtualProtect(patchAddr, patchSize, oldProtect, &dummy);
        g_pRadarColorPatchAddr = patchAddr;
        g_pRadarColorTrampoline = trampoline;
        g_RadarColorPatchSize = patchSize;
        g_bRadarColorPatched = true;
        return true;
    }

    VirtualFree(trampoline, 0, MEM_RELEASE);
    advancedfx::Message("[mirv_pov_radar_patch] VirtualProtect failed for radar color enum (error %lu)\n", GetLastError());
    return false;
}

static void MirvPov_RestoreRadarColor() {
    if(!g_bRadarColorPatched || !g_pRadarColorPatchAddr) return;
    DWORD oldProtect;
    if(VirtualProtect(g_pRadarColorPatchAddr, g_RadarColorPatchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_pRadarColorPatchAddr, g_RadarColorOrigBytes, g_RadarColorPatchSize);
        FlushInstructionCache(GetCurrentProcess(), g_pRadarColorPatchAddr, g_RadarColorPatchSize);
        DWORD dummy;
        VirtualProtect(g_pRadarColorPatchAddr, g_RadarColorPatchSize, oldProtect, &dummy);
    }
    if(g_pRadarColorTrampoline) {
        VirtualFree(g_pRadarColorTrampoline, 0, MEM_RELEASE);
    }
    g_bRadarColorPatched = false;
    g_pRadarColorPatchAddr = nullptr;
    g_pRadarColorTrampoline = nullptr;
    g_RadarColorPatchSize = 0;
}

void MirvPov_ApplyRadarPatches(HMODULE clientDll) {
    if(nullptr == clientDll) {
        advancedfx::Message("[mirv_pov_radar_patch] No client.dll handle\n");
        return;
    }

    if(!g_bRadarSpectatorTargetPatched) {
        g_bRadarSpectatorTargetPatched = true;
    }

    if(!g_bRadarMulColorPatched) MirvPov_PatchRadarMulColor(clientDll);
    if(!g_bRadarMulCTPatched)    MirvPov_PatchRadarMulCT(clientDll);
    if(!g_bRadarMulTPatched)     MirvPov_PatchRadarMulT(clientDll);

    if(!g_bRadarSpotCheckPatched) {
        MirvPov_PatchRadarSpotCheck(clientDll);
    }

    if(!g_bRadarColorPatched) {
        MirvPov_PatchRadarColor(clientDll);
    }
}

void MirvPov_RemoveShowAllNOP() {
    if(!g_bRadarShowAllPatched || !g_pRadarShowAllPatchAddr) return;

    DWORD oldProtect;
    if(VirtualProtect(g_pRadarShowAllPatchAddr, 7, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_pRadarShowAllPatchAddr, g_RadarShowAllOriginalBytes, 7);
        FlushInstructionCache(GetCurrentProcess(), g_pRadarShowAllPatchAddr, 7);
        DWORD dummy;
        VirtualProtect(g_pRadarShowAllPatchAddr, 7, oldProtect, &dummy);
    }
    g_bRadarShowAllPatched = false;
    g_pRadarShowAllPatchAddr = nullptr;
}

void MirvPov_RemoveRadarPatches() {
    if(g_bRadarSpectatorTargetPatched && g_pRadarSpectatorTargetPatchAddr) {
        DWORD oldProtect;
        if(VirtualProtect(g_pRadarSpectatorTargetPatchAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *g_pRadarSpectatorTargetPatchAddr = g_RadarSpectatorTargetOrigByte;
            DWORD dummy;
            VirtualProtect(g_pRadarSpectatorTargetPatchAddr, 1, oldProtect, &dummy);
        }
        g_bRadarSpectatorTargetPatched = false;
        g_pRadarSpectatorTargetPatchAddr = nullptr;
        advancedfx::Message("[mirv_pov_radar_patch] Restored spectator target jz\n");
    }

    if(g_bRadarShowAllPatched) {
        MirvPov_RemoveShowAllNOP();
    }

    if(g_bRadarSpotCheckPatched) {
        MirvPov_RestoreRadarSpotCheck();
    }

    if(g_bRadarColorPatched) {
        MirvPov_RestoreRadarColor();
    }

    if(g_bRadarMulColorPatched) MirvPov_RestoreRadarMulColor();
    if(g_bRadarMulCTPatched)    MirvPov_RestoreRadarMulCT();
    if(g_bRadarMulTPatched)     MirvPov_RestoreRadarMulT();
}
