#include "stdafx.h"

#include "VScript.h"

#include "../shared/AfxCommandLine.h"

#include "addresses.h"
#include "Globals.h"

#include <wchar.h>

#include "../deps/release/Detours/src/detours.h"


extern advancedfx::CCommandLine  * g_CommandLine;

typedef int (__fastcall * cs2_client_CCSGOVScriptGameSystem_GetMode_t)(void * pThis);
cs2_client_CCSGOVScriptGameSystem_GetMode_t g_Old_cs2_client_CCSGOVScriptGameSystem_GetMode = nullptr;
int __fastcall New_cs2_client_CCSGOVScriptGameSystem_GetMode(void * pThis) {
    static bool bFirsRun = true;
    static int result = 2;
    if(bFirsRun) {
        bFirsRun = false;
        result = g_Old_cs2_client_CCSGOVScriptGameSystem_GetMode(pThis);
		if (int idx = g_CommandLine->FindParam(L"-afxVScriptModeClient")) {
			if (idx + 1 < g_CommandLine->GetArgC()) {
				result = (int)wcstol( g_CommandLine->GetArgV(idx + 1), nullptr, 10);
			}
		}        
    }
    return result;
}

void Hook_cs2_client_CCSGOVScriptGameSystem_GetMode() {
    if (int idx = g_CommandLine->FindParam(L"-afxVScriptModeClient")) {
        if(AFXADDR_GET(cs2_client_CCSGOVScriptGameSystem_GetMode)) {
            g_Old_cs2_client_CCSGOVScriptGameSystem_GetMode = (cs2_client_CCSGOVScriptGameSystem_GetMode_t)AFXADDR_GET(cs2_client_CCSGOVScriptGameSystem_GetMode);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_Old_cs2_client_CCSGOVScriptGameSystem_GetMode,New_cs2_client_CCSGOVScriptGameSystem_GetMode);
            if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));        
        }
    }
}

bool ExecuteClientVScript(const char * pszScript) {
    if(AFXADDR_GET(cs2_client_ClientScriptVM)) {
        void *This = *(void**)AFXADDR_GET(cs2_client_ClientScriptVM);
        void ** vtable = *(void***)This;        
        void * pScript = ((void* (__fastcall *)(void *, const char *, void *)) (vtable[AFXADDR_GET(cs2_ScriptVM_CompileString_vtable_offset)])) (This,pszScript,nullptr);
        if(nullptr == pScript) return false;
        int result = ((int (__fastcall *)(void *, void *, unsigned char, unsigned char)) (vtable[AFXADDR_GET(cs2_ScriptVM_RunScript_vtable_offset)])) (This,pScript,0,1);
        ((void (__fastcall *)(void *, void *)) (vtable[AFXADDR_GET(cs2_ScriptVM_FreeScript_vtable_offset)])) (This,pScript);
        return -1 != result;
    }
    return false;
}
