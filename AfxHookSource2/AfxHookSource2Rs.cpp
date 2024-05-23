#include "stdafx.h"
#include "AfxHookSource2Rs.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/AfxHookSource/SourceInterfaces.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

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

bool g_b_on_game_event = false;

void afx_hook_source2_enable_on_game_event(bool value) {
    g_b_on_game_event = value;
}

bool g_b_on_c_view_render_setup_view = false;

void afx_hook_source2_enable_on_c_view_render_setup_view(bool value) {
    g_b_on_c_view_render_setup_view = value;
}

bool g_b_on_client_frame_stage_notify = false;

void afx_hook_source2_enable_on_client_frame_stage_notify(bool value) {
    g_b_on_client_frame_stage_notify = value;
}

int afx_hook_source2_make_handle(int entryIndex, int serialNumber) {
    return SOURCESDK::CS2::CEntityHandle(entryIndex, serialNumber).ToInt();
}

bool afx_hook_source2_is_handle_valid(int handle) {
    return SOURCESDK::CS2::CEntityHandle(handle).IsValid();
}

int afx_hook_source2_get_handle_entry_index(int handle) {
    return SOURCESDK::CS2::CEntityHandle(handle).GetEntryIndex();
}

int afx_hook_source2_get_handle_serial_number(int handle) {
    return SOURCESDK::CS2::CEntityHandle(handle).GetSerialNumber();
}

int afx_hook_source2_get_highest_entity_index();

void * afx_hook_source2_get_entity_ref_from_index(int index);

void afx_hook_source2_add_ref_entity_ref(void * pRef);

void afx_hook_source2_release_entity_ref(void * pRef);

bool g_b_on_add_entity = false;

void afx_hook_source2_enable_on_add_entity(bool value) {
    g_b_on_add_entity = value;
}

bool g_b_on_remove_entity = false;

void afx_hook_source2_enable_on_remove_entity(bool value) {
    g_b_on_remove_entity = value;
}

bool afx_hook_source2_getEntityRefIsValid(void * pRef);

const char * afx_hook_source2_getEntityRefName(void * pRef);

const char * afx_hook_source2_getEntityRefDebugName(void * pRef);

const char * afx_hook_source2_getEntityRefClassName(void * pRef);

bool afx_hook_source2_getEntityRefIsPlayerPawn(void * pRef);
int afx_hook_source2_getEntityRefPlayerPawnHandle(void * pRef);

bool afx_hook_source2_getEntityRefIsPlayerController(void * pRef);
int afx_hook_source2_getEntityRefPlayerControllerHandle(void * pRef);

unsigned int afx_hook_source2_getEntityRefHealth(void * pRef);

void afx_hook_source2_getEntityRefOrigin(void * pRef, float & x, float & y, float & z);

void afx_hook_source2_getEntityRefRenderEyeOrigin(void * pRef, float & x, float & y, float & z);

void afx_hook_source2_getEntityRefRenderEyeAngles(void * pRef, float & x, float & y, float & z);

void* afx_hook_source2_getEntityRefViewEntityRef(void * pRef);

struct AfxHookSource2 {
    void (*message)(const char *);
    void (*warning)(const char *);
    void (*exec)(const char *);
    void (*enableOnGameEvent)(bool bValue);
    void (*enableOnCViewRenderSetupView)(bool bValue);
    void (*enableOnClientFrameStageNotify)(bool bValue);

    int (*makeHandle)(int entryIndex, int serialNumber);
    bool (*isHandleValid)(int handle);
    int (*getHandleEntryIndex)(int handle);
    int (*getHandleSerialNumber)(int handle);
    int (*getHighestEntityIndex)();
    void * (*getEntityRefFromIndex)(int index);

    void (*addRefEntityRef)(void *);
    void (*releaseEntityRef)(void *);

    void (*enableOnAddEntity)(bool bValue);
    void (*enableOnRemoveEntity)(bool bValue);

    bool (*getEntityRefIsValid)(void * pRef);
    
    const char * (*getEntityRefName)(void * pRef);

    // can return nullptr to indicate no debug name.
    const char * (*getEntityRefDebugName)(void * pRef);

    const char * (*getEntityRefClassName)(void * pRef);

    bool (*getEntityRefIsPlayerPawn)(void * pRef);
    int (*getEntityRefPlayerPawnHandle)(void * pRef);

    bool (*getEntityRefIsPlayerController)(void * pRef);
    int (*getEntityRefPlayerControllerHandle)(void * pRef);

    unsigned int (*getEntityRefHealth)(void * pRef);

    void (*getEntityRefOrigin)(void * pRef, float & x, float & y, float & z);

    void (*getEntityRefRenderEyeOrigin)(void * pRef, float & x, float & y, float & z);

    void (*getEntityRefRenderEyeAngles)(void * pRef, float & x, float & y, float & z);

    void * (*getEntityRefViewEntityRef)(void * pRef);

} g_AfxHookSource2 = {
    &afx_hook_source2_message,
    &afx_hook_source2_warning,
    &afx_hook_source2_exec,
    &afx_hook_source2_enable_on_game_event,
    &afx_hook_source2_enable_on_c_view_render_setup_view,
    &afx_hook_source2_enable_on_client_frame_stage_notify,
    &afx_hook_source2_make_handle,
    &afx_hook_source2_is_handle_valid,
    &afx_hook_source2_get_handle_entry_index,
    &afx_hook_source2_get_handle_serial_number,
    &afx_hook_source2_get_highest_entity_index,
    &afx_hook_source2_get_entity_ref_from_index,
    &afx_hook_source2_add_ref_entity_ref,
    &afx_hook_source2_release_entity_ref,
    &afx_hook_source2_enable_on_add_entity,
    &afx_hook_source2_enable_on_remove_entity,
    &afx_hook_source2_getEntityRefIsValid,
    &afx_hook_source2_getEntityRefName,
    &afx_hook_source2_getEntityRefDebugName,
    &afx_hook_source2_getEntityRefClassName,
    &afx_hook_source2_getEntityRefIsPlayerPawn,
    &afx_hook_source2_getEntityRefPlayerPawnHandle,
    &afx_hook_source2_getEntityRefIsPlayerController,
    &afx_hook_source2_getEntityRefPlayerControllerHandle,
    &afx_hook_source2_getEntityRefHealth,
    &afx_hook_source2_getEntityRefOrigin,
    &afx_hook_source2_getEntityRefRenderEyeOrigin,
    &afx_hook_source2_getEntityRefRenderEyeAngles,
    &afx_hook_source2_getEntityRefViewEntityRef
};

typedef void AfxHookSource2Rs;

AfxHookSource2Rs * g_AfxHookSource2Rs_Engine = nullptr;

extern "C" AfxHookSource2Rs * afx_hook_source2_rs_new( struct AfxHookSource2 * iface );

extern "C" void afx_hook_source2_rs_run_jobs(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_destroy(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_execute(AfxHookSource2Rs * this_ptr, unsigned char * p_data, size_t len_data);

extern "C" void afx_hook_source2_rs_load(AfxHookSource2Rs * this_ptr, const char * file_path);

extern "C" void afx_hook_source2_rs_on_game_event(AfxHookSource2Rs * this_ptr, const char * event_name, int event_id, const char * json);

extern "C" bool afx_hook_source2_rs_on_c_view_render_setup_view(AfxHookSource2Rs * this_ptr, float cur_time, float abs_time, float last_abs_time, struct AfxHookSourceRsView & current_view, const struct AfxHookSourceRsView & game_view, const struct AfxHookSourceRsView & last_view, int width, int height);

extern "C" void afx_hook_source2_rs_on_client_frame_stage_notify(AfxHookSource2Rs * this_ptr, int event_id, bool is_before);

extern "C" void afx_hook_source2_rs_on_add_entity(AfxHookSource2Rs * this_ptr, void * p_ref, int handle);

extern "C" void afx_hook_source2_rs_on_remove_entity(AfxHookSource2Rs * this_ptr, void * p_ref, int handle);

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

void AfxHookSourceRs_Engine_Load(const char * szFilePath) {
    if(nullptr != g_AfxHookSource2Rs_Engine) {
        afx_hook_source2_rs_load(g_AfxHookSource2Rs_Engine, szFilePath);
    }
}

void AfxHookSourceRs_Engine_OnGameEvent(const char * event_name, int event_id, const char * json) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_game_event) {
        afx_hook_source2_rs_on_game_event(g_AfxHookSource2Rs_Engine, event_name, event_id, json);
    }
}

bool AfxHookSource2Rs_OnCViewRenderSetupView(float curTime, float absTime, float lastAbsTime, struct AfxHookSourceRsView & currentView, const struct AfxHookSourceRsView & gameView, const struct AfxHookSourceRsView & lastView, int width, int height) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_c_view_render_setup_view) {
        return afx_hook_source2_rs_on_c_view_render_setup_view(g_AfxHookSource2Rs_Engine, curTime, absTime, lastAbsTime, currentView, gameView, lastView, width, height);
    }
    return false;
}

void AfxHookSource2Rs_Engine_OnClientFrameStageNotify(int event_id, bool is_before) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_client_frame_stage_notify) {
        afx_hook_source2_rs_on_client_frame_stage_notify(g_AfxHookSource2Rs_Engine, event_id, is_before);
    }
}

void AfxHookSource2Rs_Engine_OnAddEntity(void * pEntityRef, int handle) {
    if(nullptr != g_AfxHookSource2Rs_Engine) {
        afx_hook_source2_rs_on_add_entity(g_AfxHookSource2Rs_Engine, pEntityRef, handle);
    }
}

void AfxHookSource2Rs_Engine_OnRemoveEntity(void * pEntityRef, int handle) {
    if(nullptr != g_AfxHookSource2Rs_Engine) {
        afx_hook_source2_rs_on_remove_entity(g_AfxHookSource2Rs_Engine, pEntityRef, handle);
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
        AfxHookSourceRs_Engine_Load(args->ArgV(1));
    }
}
