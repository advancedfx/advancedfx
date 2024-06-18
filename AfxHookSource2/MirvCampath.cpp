#include "MirvCampath.h" 
#include "Globals.h"

CamPath g_CamPath;
CMirvCampath_Time g_MirvCampath_Time;
CMirvCampath_Camera g_MirvCampath_Camera;
CMirvCampath_Drawer g_MirvCampath_Drawer;

double CMirvCampath_Time::GetTime() {
	// Can be paused time, we don't support that currently.
	return curtime_get();
}

double CMirvCampath_Time::GetCurTime() {
	return curtime_get();
}
bool CMirvCampath_Time::GetCurrentDemoTick(int& outTick) {
	if(g_pEngineToClient) {
		if(SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile()) {
			outTick = pDemoFile->GetDemoTick();
			return true;
		}
	}
	return false;
}
bool CMirvCampath_Time::GetCurrentDemoTime(double& outDemoTime) {
	int tick;
	if(GetCurrentDemoTick(tick)) {
		outDemoTime = (tick + interpolation_amount_get())* (double)interval_per_tick_get();
		return true;
	}


	return false;
}
bool CMirvCampath_Time::GetDemoTickFromDemoTime(double curTime, double time, int& outTick) {
	outTick = (int)round(time / interval_per_tick_get());
	return true;
}
bool CMirvCampath_Time::GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime) {
	double current_demo_time;
	if(GetCurrentDemoTime(current_demo_time)) {
		outDemoTime = time - (curTime - current_demo_time);
		return true;
	}
	return false;
}
bool CMirvCampath_Time::GetDemoTickFromClientTime(double curTime, double targetTime, int& outTick)
{
	double demoTime;
	return GetDemoTimeFromClientTime(curTime, targetTime, demoTime) && GetDemoTickFromDemoTime(curTime, demoTime, outTick);
}

SMirvCameraValue CMirvCampath_Camera::GetCamera() { 
	return SMirvCameraValue(			
		g_MirvInputEx.LastCameraOrigin[0],
		g_MirvInputEx.LastCameraOrigin[1],
		g_MirvInputEx.LastCameraOrigin[2],
		g_MirvInputEx.LastCameraAngles[0],
		g_MirvInputEx.LastCameraAngles[1],
		g_MirvInputEx.LastCameraAngles[2],
		g_MirvInputEx.LastCameraFov
	);
}

bool CMirvCampath_Drawer::GetEnabled() {
	return g_CampathDrawer.Draw_get();
}
void CMirvCampath_Drawer::SetEnabled(bool value) {
	g_CampathDrawer.Draw_set(value);
}
bool CMirvCampath_Drawer::GetDrawKeyframeAxis() {
	return g_CampathDrawer.GetDrawKeyframeAxis();
}
void CMirvCampath_Drawer::SetDrawKeyframeAxis(bool value) {
	g_CampathDrawer.SetDrawKeyframeAxis(value);
}
bool CMirvCampath_Drawer::GetDrawKeyframeCam() {
	return g_CampathDrawer.GetDrawKeyframeCam();
}
void CMirvCampath_Drawer::SetDrawKeyframeCam(bool value) {
	g_CampathDrawer.SetDrawKeyframeCam(value);
}

float CMirvCampath_Drawer::GetDrawKeyframeIndex() { return g_CampathDrawer.GetDrawKeyframeIndex(); }
void CMirvCampath_Drawer::SetDrawKeyframeIndex(float value) { g_CampathDrawer.SetDrawKeyframeIndex(value); }
