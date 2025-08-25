#pragma once

#include "Globals.h"
#include "../shared/AfxDetours.h"

extern CAfxImportsHook g_Import_SceneSystem;
extern CAfxImportsHook g_Import_panorama;

void HookFixClient(HMODULE clientDll);
void UpdateAnimGraph();

struct MirvFix {
    struct Time {
        bool enabled = true;
        float value = -1;
        float oldValue = -1;

        enum Mode {
            AUTO = 0,
            USER
        }; 

        Mode mode = AUTO;

    } time;

	bool fixAnimations = false;
};

extern MirvFix g_MirvFix;

