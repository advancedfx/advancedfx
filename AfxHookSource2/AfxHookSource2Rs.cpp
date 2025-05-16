#include "stdafx.h"
#include "AfxHookSource2Rs.h"
#include "MirvTime.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/AfxHookSource/SourceInterfaces.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
#include "../deps/release/prop/cs2/sdk_src/public/entityhandle.h"

#include "../shared/AfxConsole.h"
#include "../shared/CamPath.h"
#include "../shared/FFITools.h"
#include "../shared/StringTools.h"

#include "WrpConsole.h"

#include <string>

extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;

extern "C" void afx_hook_source2_message(const char * pszValue) {
    advancedfx::Message("%s",pszValue);
}

extern "C" void afx_hook_source2_warning(const char * pszValue) {
    advancedfx::Warning("%s",pszValue);
}

extern "C" void afx_hook_source2_exec(const char * pszValue) {
    if(g_pEngineToClient) g_pEngineToClient->ExecuteClientCmd(0,pszValue,true);
}

bool g_b_on_record_start = false;

extern "C" void afx_hook_source2_enable_on_record_start(FFIBool value) {
    g_b_on_record_start = FFIBOOL_TO_BOOL(value);
}

bool g_b_on_record_end = false;

extern "C" void afx_hook_source2_enable_on_record_end(FFIBool value) {
    g_b_on_record_end = FFIBOOL_TO_BOOL(value);
}


bool g_b_on_game_event = false;

extern "C" void afx_hook_source2_enable_on_game_event(FFIBool value) {
    g_b_on_game_event = FFIBOOL_TO_BOOL(value);
}

bool g_b_on_c_view_render_setup_view = false;

extern "C" void afx_hook_source2_enable_on_c_view_render_setup_view(FFIBool value) {
    g_b_on_c_view_render_setup_view = FFIBOOL_TO_BOOL(value);
}

bool g_b_on_client_frame_stage_notify = false;

extern "C" void afx_hook_source2_enable_on_client_frame_stage_notify(FFIBool value) {
    g_b_on_client_frame_stage_notify = FFIBOOL_TO_BOOL(value);
}

extern "C" int afx_hook_source2_make_handle(int entryIndex, int serialNumber) {
    return SOURCESDK::CS2::CEntityHandle(entryIndex, serialNumber).ToInt();
}

extern "C" FFIBool afx_hook_source2_is_handle_valid(int handle) {
    return BOOL_TO_FFIBOOL(SOURCESDK::CS2::CEntityHandle(handle).IsValid());
}

extern "C" int afx_hook_source2_get_handle_entry_index(int handle) {
    return SOURCESDK::CS2::CEntityHandle(handle).GetEntryIndex();
}

extern "C" int afx_hook_source2_get_handle_serial_number(int handle) {
    return SOURCESDK::CS2::CEntityHandle(handle).GetSerialNumber();
}

extern "C" int afx_hook_source2_get_highest_entity_index();

extern "C" void * afx_hook_source2_get_entity_ref_from_index(int index);

extern "C" void afx_hook_source2_add_ref_entity_ref(void * pRef);

extern "C" void afx_hook_source2_release_entity_ref(void * pRef);

bool g_b_on_add_entity = false;

extern "C" void afx_hook_source2_enable_on_add_entity(FFIBool value) {
    g_b_on_add_entity = FFIBOOL_TO_BOOL(value);
}

bool g_b_on_remove_entity = false;

extern "C" void afx_hook_source2_enable_on_remove_entity(FFIBool value) {
    g_b_on_remove_entity = FFIBOOL_TO_BOOL(value);
}

extern "C" FFIBool afx_hook_source2_get_entity_ref_is_valid(void * pRef);

extern "C" const char * afx_hook_source2_get_entity_ref_name(void * pRef);

extern "C" const char * afx_hook_source2_get_entity_ref_debug_name(void * pRef);

extern "C" const char * afx_hook_source2_get_entity_ref_class_name(void * pRef);

extern "C" FFIBool afx_hook_source2_get_entity_ref_is_player_pawn(void * pRef);
extern "C" int afx_hook_source2_get_entity_ref_player_pawn_handle(void * pRef);

extern "C" FFIBool afx_hook_source2_get_entity_ref_is_player_controller(void * pRef);
extern "C" int afx_hook_source2_get_entity_ref_player_controller_handle(void * pRef);

extern "C" int afx_hook_source2_get_entity_ref_health(void * pRef);

extern "C" int afx_hook_source2_get_entity_ref_team(void * pRef);

extern "C" void afx_hook_source2_get_entity_ref_origin(void * pRef, float & x, float & y, float & z);

extern "C" void afx_hook_source2_get_entity_ref_render_eye_origin(void * pRef, float & x, float & y, float & z);

extern "C" void afx_hook_source2_get_entity_ref_render_eye_angles(void * pRef, float & x, float & y, float & z);

extern "C" FFIBool afx_hook_source2_is_playing_demo() {
    if(g_pEngineToClient) {
		if(SOURCESDK::CS2::IDemoFile * pDemoPlayer = g_pEngineToClient->GetDemoFile()) {
			return BOOL_TO_FFIBOOL(pDemoPlayer->IsPlayingDemo());
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_is_demo_paused() {
    if(g_pEngineToClient) {
		if(SOURCESDK::CS2::IDemoFile * pDemoPlayer = g_pEngineToClient->GetDemoFile()) {
			return BOOL_TO_FFIBOOL(pDemoPlayer->IsDemoPaused());
        }
    }
    return FFIBOOL_FALSE;
}

extern CamPath g_CamPath;

extern "C" CamPath * afx_hook_source2_get_main_campath(void) {
    return &g_CamPath;
}

typedef void AfxHookSource2Rs;

AfxHookSource2Rs * g_AfxHookSource2Rs_Engine = nullptr;

extern "C" AfxHookSource2Rs * afx_hook_source2_rs_new();

extern "C" void afx_hook_source2_rs_run_jobs(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_destroy(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_execute(AfxHookSource2Rs * this_ptr, unsigned char * p_data, size_t len_data);

extern "C" void afx_hook_source2_rs_load(AfxHookSource2Rs * this_ptr, const char * file_path);

extern "C" void afx_hook_source2_rs_on_record_start(AfxHookSource2Rs * this_ptr, const char * take_folder_path);

extern "C" void afx_hook_source2_rs_on_record_end(AfxHookSource2Rs * this_ptr);

extern "C" void afx_hook_source2_rs_on_game_event(AfxHookSource2Rs * this_ptr, const char * event_name, int event_id, const char * json);

extern "C" FFIBool afx_hook_source2_rs_on_c_view_render_setup_view(AfxHookSource2Rs * this_ptr, float cur_time, float abs_time, float last_abs_time, struct AfxHookSourceRsView & current_view, const struct AfxHookSourceRsView & game_view, const struct AfxHookSourceRsView & last_view, int width, int height);

extern "C" void afx_hook_source2_rs_on_client_frame_stage_notify(AfxHookSource2Rs * this_ptr, int event_id, FFIBool is_before);

extern "C" void afx_hook_source2_rs_on_add_entity(AfxHookSource2Rs * this_ptr, void * p_ref, int handle);

extern "C" void afx_hook_source2_rs_on_remove_entity(AfxHookSource2Rs * this_ptr, void * p_ref, int handle);

extern "C" FFIBool afx_hook_source2_get_demo_tick(int& outTick) {
	return BOOL_TO_FFIBOOL(g_MirvTime.GetCurrentDemoTick(outTick));
};

extern "C" FFIBool afx_hook_source2_get_demo_time(double& outTime) {
	return BOOL_TO_FFIBOOL(g_MirvTime.GetCurrentDemoTime(outTime));
};

extern "C" void afx_hook_source2_get_cur_time(double& outCurTime) {
	outCurTime = g_MirvTime.curtime_get();
};

void AfxHookSource2Rs_Engine_Init() {
    if(nullptr == g_AfxHookSource2Rs_Engine) {
        g_AfxHookSource2Rs_Engine = afx_hook_source2_rs_new();
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

void AfxHookSourceRs_Engine_OnRecordStart(const char * take_folder_path) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_record_start) {
        afx_hook_source2_rs_on_record_start(g_AfxHookSource2Rs_Engine,take_folder_path);
    }
}

void AfxHookSourceRs_Engine_OnRecordEnd() {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_record_end) {
        afx_hook_source2_rs_on_record_end(g_AfxHookSource2Rs_Engine);
    }
}

void AfxHookSourceRs_Engine_OnGameEvent(const char * event_name, int event_id, const char * json) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_game_event) {
        afx_hook_source2_rs_on_game_event(g_AfxHookSource2Rs_Engine, event_name, event_id, json);
    }
}

bool AfxHookSource2Rs_OnCViewRenderSetupView(float curTime, float absTime, float lastAbsTime, struct AfxHookSourceRsView & currentView, const struct AfxHookSourceRsView & gameView, const struct AfxHookSourceRsView & lastView, int width, int height) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_c_view_render_setup_view) {
        return FFIBOOL_TO_BOOL(afx_hook_source2_rs_on_c_view_render_setup_view(g_AfxHookSource2Rs_Engine, curTime, absTime, lastAbsTime, currentView, gameView, lastView, width, height));
    }
    return false;
}

void AfxHookSource2Rs_Engine_OnClientFrameStageNotify(int event_id, bool is_before) {
    if(nullptr != g_AfxHookSource2Rs_Engine && g_b_on_client_frame_stage_notify) {
        afx_hook_source2_rs_on_client_frame_stage_notify(g_AfxHookSource2Rs_Engine, event_id, BOOL_TO_FFIBOOL(is_before));
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
