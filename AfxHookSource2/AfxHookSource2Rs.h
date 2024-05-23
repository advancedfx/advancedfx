#pragma once

void AfxHookSource2Rs_Engine_Init();
void AfxHookSource2Rs_Engine_RunJobQueue();
void AfxHookSource2Rs_Engine_Shutdown();

void AfxHookSourceRs_Engine_OnGameEvent(const char * event_name, int event_id, const char * p_data);

struct AfxHookSourceRsView {
    float x;
    float y;
    float z;
    float rx;
    float ry;
    float rz;
    float fov;
};

bool AfxHookSource2Rs_OnCViewRenderSetupView(float curTime, float absTime, float lastAbsTime, struct AfxHookSourceRsView & currentView, const struct AfxHookSourceRsView & gameView, const struct AfxHookSourceRsView & lastView, int width, int height);

void AfxHookSource2Rs_Engine_OnClientFrameStageNotify(int event_id, bool is_before);


extern bool g_b_on_add_entity;
void AfxHookSource2Rs_Engine_OnAddEntity(void * pEntityRef, int handle);

extern bool g_b_on_remove_entity;
void AfxHookSource2Rs_Engine_OnRemoveEntity(void * pEntityRef, int handle);
