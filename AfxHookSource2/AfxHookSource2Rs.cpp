#include "stdafx.h"
#include "AfxHookSource2Rs.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/AfxHookSource/SourceInterfaces.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"

#include "../shared/AfxConsole.h"
#include "../shared/StringTools.h"

#include "WrpConsole.h"

#include <string>

extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;

void afx_hook_source2_message(const char * pszValue) {
    advancedfx::Message(pszValue);
}

void afx_hook_source2_warning(const char * pszValue) {
    advancedfx::Warning(pszValue);
}

void afx_hook_source2_exec(const char * pszValue) {
    if(g_pEngineToClient) g_pEngineToClient->ExecuteClientCmd(0,pszValue,true);
}

struct AfxHookSource2 {
    void (*message)(const char *);
    void (*warning)(const char *);
    void (*exec)(const char *);
} g_AfxHookSource2 = {
    &afx_hook_source2_message,
    &afx_hook_source2_warning,
    &afx_hook_source2_exec
};

typedef void AfxHookSource2Rs;

AfxHookSource2Rs * g_AfxHookSource2Rs_Engine = nullptr;

extern "C" AfxHookSource2Rs * afx_hook_source2_rs_new( struct AfxHookSource2 * iface );

extern "C" void afx_hook_source2_rs_run_jobs(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_destroy(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_execute(AfxHookSource2Rs * this_ptr, unsigned char * p_data, size_t len_data);

void AfxHookSource2Rs_Engine_Init() {
    if(nullptr == g_AfxHookSource2Rs_Engine) {
        g_AfxHookSource2Rs_Engine = afx_hook_source2_rs_new(&g_AfxHookSource2);
    }
}

void AfxHookSource2Rs_Engine_RunJobQueue() {
    if(nullptr != g_AfxHookSource2Rs_Engine) {
        afx_hook_source2_rs_run_jobs(g_AfxHookSource2Rs_Engine);
    }
}

void AfxHookSource2Rs_Engine_Shutdown() {
    if(nullptr != g_AfxHookSource2Rs_Engine) {
        afx_hook_source2_rs_destroy(g_AfxHookSource2Rs_Engine);
        g_AfxHookSource2Rs_Engine = nullptr;
    }
}

void AfxHookSourceRs_Engine_Execute(unsigned char * pData, size_t lenData) {
    if(nullptr != g_AfxHookSource2Rs_Engine) {
        afx_hook_source2_rs_execute(g_AfxHookSource2Rs_Engine, pData, lenData);
    }
}

CON_COMMAND(mirv_script_exec, "Execute script.")
{
    std::string str;

	for(int i=1; i < args->ArgC(); i++) {
		if(1 < i ) str.append(" ");
        str.append(args->ArgV(i));
	}

    AfxHookSourceRs_Engine_Execute((unsigned char *)str.c_str(), str.length());
}

CON_COMMAND(mirv_script_load, "Load script from file and execute")
{
    int argC = args->ArgC();

    if(2 <= argC) {
        const char * arg1 = args->ArgV(1);
        std::wstring wsFileName;
        if(UTF8StringToWideString(arg1, wsFileName)) {

            FILE * file = 0;
            bool bOk = 0 == _wfopen_s(&file, wsFileName.c_str(), L"rb");
            unsigned char * so = 0;
            size_t size = 0;

            bOk = bOk 
                && 0 != file
                && 0 == fseek(file, 0, SEEK_END)
            ;

            if(bOk)
            {
                size = ftell(file);

                so = (unsigned char *)malloc(size);
                bOk = 0 != so
                    && 0 == fseek(file, 0, SEEK_SET);
            }

            if(bOk)
            {
                bOk = size == fread(so, 1, size, file);
            }

            if(file) fclose(file);

            if(bOk) {
                AfxHookSourceRs_Engine_Execute(so, size);
            } else {
                advancedfx::Warning("Failed to load and execute \"%s\".\n",arg1);
            }

            free(so);
        } else advancedfx::Warning("Can not convert \"%s\" from UTF-8 to wide string\.n",arg1);
    }
}
