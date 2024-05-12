#include "stdafx.h"

#include "CampathDrawer.h"
#include "ClientEntitySystem.h"
#include "GameEvents.h"
#include "hlaeFolder.h"
#include "RenderSystemDX11Hooks.h"
#include "WrpConsole.h"
#include "AfxHookSource2Rs.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/AfxHookSource/SourceInterfaces.h"
#include "../deps/release/prop/cs2/Source2Client.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/convar.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
#include "../deps/release/prop/cs2/sdk_src/public/icvar.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameuiservice.h"

#include "../shared/AfxCommandLine.h"
#include "../shared/AfxConsole.h"
#include "../shared/AfxDetours.h"
#include "../shared/ConsolePrinter.h"
#include "../shared/StringTools.h"
#include "../shared/binutils.h"
#include "../shared/CommandSystem.h"
#include "../shared/ImageBufferPoolThreadSafe.h"
#include "../shared/ThreadPool.h"
#include "../shared/MirvCamIO.h"
#include "../shared/MirvCampath.h"
#include "../shared/MirvInput.h"
#include "../shared/MirvSkip.h"

#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <sstream>
#include <mutex>

HMODULE g_h_engine2Dll = 0;
HMODULE g_H_ClientDll = 0;

advancedfx::CCommandLine  * g_CommandLine = nullptr;

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookSource2", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}

FovScaling GetDefaultFovScaling() {
	return FovScaling_AlienSwarm;
}

void PrintInfo() {
	advancedfx::Message(
		"|" "\n"
		"| AfxHookSource2 (" __DATE__ " " __TIME__ ")" "\n"
		"| https://advancedfx.org/" "\n"
		"|" "\n"
	);
}

void * g_pGameResourceService = nullptr;

int g_nIgnoreNextDisconnects = 0;

typedef void (*Unknown_ExecuteClientCommandFromNetChan_t)(void * Ecx, void * Edx, void *R8);
Unknown_ExecuteClientCommandFromNetChan_t g_Old_Unknown_ExecuteClientCommandFromNetChan = nullptr;
void New_Unknown_ExecuteClientCommandFromNetChan(void * Ecx, void * Edx, SOURCESDK::CS2::CCommand *r8Command) {
	for(int i = 0; i < r8Command->ArgC(); i++) {
		advancedfx::Message("Command %i: %s\n",i,r8Command->Arg(i));
	}
	if(0 == stricmp("connect",r8Command->Arg(0))) {
		if(IDYES != MessageBoxA(0,"YOU ARE TRYING TO CONNECT TO A SERVER - THIS WILL GET YOU VAC BANNED.\nARE YOU SURE?", "HLAE WARNING", MB_YESNOCANCEL|MB_ICONHAND|MB_DEFBUTTON2))
			return;
	}
	if(0 < g_nIgnoreNextDisconnects && 0 < r8Command->ArgC()) {
		if(0 == stricmp("disconnect",r8Command->Arg(0))) {
			if(0 < g_nIgnoreNextDisconnects) g_nIgnoreNextDisconnects--;
			return;
		}
	}
	g_Old_Unknown_ExecuteClientCommandFromNetChan(Ecx, Edx, r8Command);
}


void HookEngineDll(HMODULE engineDll) {

	static bool bFirstCall = true;
	if(!bFirstCall) return;
	bFirstCall = false;
	
	// Unknown_ExecuteClientCommandFromNetChan: // Last checked 2023-07-19
	/*
		The function we hook is called in the function referencing the string
		"Client %s(%d) tried to execute command \"%s\" before being fully connected.\n"
		or the other function referencing "SV: Cheat command '%s' ignored.\n"
		as follows:

		loc_1801842F0:
		mov     r8, rdi
		lea     rdx, [rsp+0D68h+var_D38]
		lea     rcx, [rsp+0D68h+arg_18]
		call    sub_180329DD0 <---
		lea     rcx, [rsp+0D68h+var_D30]
		call    sub_180183A60	
	*/
	{
		Afx::BinUtils::ImageSectionsReader sections((HMODULE)engineDll);
		Afx::BinUtils::MemRange textRange = sections.GetMemRange();
		Afx::BinUtils::MemRange result = FindPatternString(textRange, "4C 8B D1 48 8B 0D ?? ?? ?? ?? 48 85 C9 74 13 48 8B 01 4D 8B C8 4C 8B C2 49 8B 12 48 FF A0 90 00 00 00 C3");
		if (!result.IsEmpty()) {
			g_Old_Unknown_ExecuteClientCommandFromNetChan = (Unknown_ExecuteClientCommandFromNetChan_t)result.Start;	
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)g_Old_Unknown_ExecuteClientCommandFromNetChan, New_Unknown_ExecuteClientCommandFromNetChan);
			if(NO_ERROR != DetourTransactionCommit())
				ErrorBox("Failed to detour Unknown_ExecuteClientCommandFromNetChan.");
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

}

SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient = nullptr;

////////////////////////////////////////////////////////////////////////////////

//TODO: Some bellow here might be not accurate yet.

typedef void * Cs2Gloabls_t;
Cs2Gloabls_t g_pGlobals = nullptr;

struct {
	bool IsPaused = false;
	float FirstPausedCurtime = 0.0f;
	float FirstPausedInterpolationAmount = 0.0f;
} g_DemoPausedData;

float curtime_get(void)
{
	if(g_DemoPausedData.IsPaused) {
		return g_DemoPausedData.FirstPausedCurtime;
	}

	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals + 11*4) : 0;
}

int framecount_get(void)
{
	return g_pGlobals ? *(int *)((unsigned char *)g_pGlobals + 1*4) : 0;
}

float frametime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +2*4) : 0;
}

float absoluteframetime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +3*4) : 0;
}

float interval_per_tick_get(void)
{
	return g_pGlobals ? 1.0f / *(int *)((unsigned char *)g_pGlobals +4*4) : 1.0f/64;
}

float interpolation_amount_get(void)
{
	if(g_DemoPausedData.IsPaused) {
		return g_DemoPausedData.FirstPausedInterpolationAmount;
	}

	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +13*4) : 0;
}

CON_COMMAND(__mirv_info,"") {
	PrintInfo();
}

CON_COMMAND(__mirv_test,"") {
	static int offset = 13;

	if(2 <= args->ArgC()) offset = atoi(args->ArgV(1));

	advancedfx::Message("g_pGlobals[%i]: int: %i , float: %f\n",offset,(g_pGlobals ? *(int *)((unsigned char *)g_pGlobals +offset*4) : 0),(g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +offset*4) : 0));
}

extern const char * GetStringForSymbol(int value);

CON_COMMAND(__mirv_get_string_for_symbol,"") {
	if (2<= args->ArgC()) {
		advancedfx::Message("%i: %s\n",atoi(args->ArgV(1)),GetStringForSymbol(atoi(args->ArgV(1))));
	}
}

CON_COMMAND(__mirv_find_vtable,"") {
	if(args->ArgC()<5) return;

	HMODULE hModule = GetModuleHandleA(args->ArgV(1));
	size_t addr = hModule != 0 ? Afx::BinUtils::FindClassVtable(hModule,args->ArgV(2),atoi(args->ArgV(3)),atoi(args->ArgV(4))) : 0;
	DWORD offset = (DWORD)(addr-(size_t)hModule);
	advancedfx::Message("Result: 0x%016llx (Offset: 0x%08x)\n",addr,offset);
}

/*CON_COMMAND(mirv_exec,"") {
    std::ostringstream oss;

	for(int i=1; i < args->ArgC(); i++) {
		if(1 < i ) oss << " ";
		std::string strArg(args->ArgV(i));

		// Escape quotes:
		for (size_t pos = strArg.find('\"', 0); std::string::npos != pos; pos = strArg.find('\"', pos + 2 ) ) strArg.replace(pos, 1, "\\\"");

		oss << "\"" << strArg << "\"";
	}

    if(g_pEngineToClient) g_pEngineToClient->ExecuteClientCmd(0, oss.str().c_str(), false);	
}*/

////////////////////////////////////////////////////////////////////////////////

SOURCESDK::CS2::IGameUIService * g_pGameUIService = nullptr;

class MirvInputEx : private IMirvInputDependencies
{
public:
	MirvInputEx() {
		LastWidth = 1920;
		LastHeight = 1080;

		LastCameraOrigin[0] = 0.0;
		LastCameraOrigin[1] = 0.0;
		LastCameraOrigin[2] = 0.0;
		LastCameraAngles[0] = 0.0;
		LastCameraAngles[1] = 0.0;
		LastCameraAngles[2] = 0.0;
		LastCameraFov = 90.0;

		GameCameraOrigin[0] = 0.0;
		GameCameraOrigin[1] = 0.0;
		GameCameraOrigin[2] = 0.0;
		GameCameraAngles[0] = 0.0;
		GameCameraAngles[1] = 0.0;
		GameCameraAngles[2] = 0.0;
		GameCameraFov = 90.0;

		LastFrameTime = 0;

		m_MirvInput = new MirvInput(this);
	}

	~MirvInputEx() {
		delete m_MirvInput;
	}

	MirvInput * m_MirvInput;

	double LastCameraOrigin[3];
	double LastCameraAngles[3];
	double LastCameraFov;

	double GameCameraOrigin[3];
	double GameCameraAngles[3];
	double GameCameraFov;

	double LastFrameTime;

	int LastWidth;
	int LastHeight;

private:
	virtual bool GetSuspendMirvInput() override {
		return g_pGameUIService && g_pGameUIService->Con_IsVisible();
	}

	virtual void GetLastCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) override {
		x = LastCameraOrigin[0];
		y = LastCameraOrigin[1];
		z = LastCameraOrigin[2];
		rX = LastCameraAngles[0];
		rY = LastCameraAngles[1];
		rZ = LastCameraAngles[2];
		fov = LastCameraFov;
	}

	virtual void GetGameCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) override {
		x = GameCameraOrigin[0];
		y = GameCameraOrigin[1];
		z = GameCameraOrigin[2];
		rX = GameCameraAngles[0];
		rY = GameCameraAngles[1];
		rZ = GameCameraAngles[2];
		fov = GameCameraFov;
	}

	virtual double GetInverseScaledFov(double fov) override {
		return ScaleFovInverse(LastWidth, LastHeight, fov);
	}

private:

	double ScaleFovInverse(double width, double height, double fov) {
		if (!height) return fov;

		double engineAspectRatio = width / height;
		double defaultAscpectRatio = 4.0 / 3.0;
		double ratio = engineAspectRatio / defaultAscpectRatio;
		double t = tan(0.5 * fov * (2.0 * M_PI / 360.0));
		double halfAngle = atan(t / ratio);
		return 2.0 * halfAngle / (2.0 * M_PI / 360.0);
	}

} g_MirvInputEx;

CON_COMMAND(mirv_input, "Input mode configuration.")
{
	g_MirvInputEx.m_MirvInput->ConCommand(args);
}

WNDPROC g_NextWindProc;
static bool g_afxWindowProcSet = false;

LRESULT CALLBACK new_Afx_WindowProc(
	__in HWND hwnd,
	__in UINT uMsg,
	__in WPARAM wParam,
	__in LPARAM lParam
)
{
//	if (AfxHookSource::Gui::WndProcHandler(hwnd, uMsg, wParam, lParam))
//		return 0;

	switch(uMsg)
	{
	case WM_ACTIVATE:
		g_MirvInputEx.m_MirvInput->Supply_Focus(LOWORD(wParam) != 0);
		break;
	case WM_CHAR:
		if(g_MirvInputEx.m_MirvInput->Supply_CharEvent(wParam, lParam))
			return 0;
		break;
	case WM_KEYDOWN:
		if(g_MirvInputEx.m_MirvInput->Supply_KeyEvent(MirvInput::KS_DOWN, wParam, lParam))
			return 0;
		break;
	case WM_KEYUP:
		if(g_MirvInputEx.m_MirvInput->Supply_KeyEvent(MirvInput::KS_UP,wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
		if (g_MirvInputEx.m_MirvInput->Supply_MouseEvent(uMsg, wParam, lParam))
			return 0;
		break;
	}
	return CallWindowProcW(g_NextWindProc, hwnd, uMsg, wParam, lParam);
}

// TODO: this is risky, actually we should track the hWnd maybe.
LONG_PTR WINAPI new_GetWindowLongPtrW(
  __in HWND hWnd,
  __in int  nIndex
)
{
	if(nIndex == GWLP_WNDPROC)
	{
		if(g_afxWindowProcSet)
		{
			return (LONG_PTR)g_NextWindProc;
		}
	}

	return GetWindowLongPtrW(hWnd, nIndex);
}

// TODO: this is risky, actually we should track the hWnd maybe.
LONG_PTR WINAPI new_SetWindowLongPtrW(
  __in HWND     hWnd,
  __in int      nIndex,
  __in LONG_PTR dwNewLong
)
{
	if(nIndex == GWLP_WNDPROC)
	{
		LONG lResult = SetWindowLongPtrW(hWnd, nIndex, (LONG_PTR)new_Afx_WindowProc);

		if(!g_afxWindowProcSet)
		{
			g_afxWindowProcSet = true;
		}
		else
		{
			lResult = (LONG_PTR)g_NextWindProc;
		}

		g_NextWindProc = (WNDPROC)dwNewLong;

		return lResult;
	}

	return SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
}

BOOL WINAPI new_GetCursorPos(
	__out LPPOINT lpPoint
)
{
	BOOL result = GetCursorPos(lpPoint);

//	if (AfxHookSource::Gui::OnGetCursorPos(lpPoint))
//		return TRUE;

	g_MirvInputEx.m_MirvInput->Supply_GetCursorPos(lpPoint);

	return result;
}

BOOL WINAPI new_SetCursorPos(
	__in int X,
	__in int Y
)
{
//	if (AfxHookSource::Gui::OnSetCursorPos(X, Y))
//		return TRUE;

	g_MirvInputEx.m_MirvInput->Supply_SetCursorPos(X,Y);

	return SetCursorPos(X,Y);
}

HCURSOR WINAPI new_SetCursor(__in_opt HCURSOR hCursor)
{
//	HCURSOR result;

//	if (AfxHookSource::Gui::OnSetCursor(hCursor, result))
//		return result;

	return SetCursor(hCursor);
}

HWND WINAPI new_SetCapture(__in HWND hWnd)
{
//	HWND result;

//	if (AfxHookSource::Gui::OnSetCapture(hWnd, result))
//		return result;

	return SetCapture(hWnd);
}

BOOL WINAPI new_ReleaseCapture()
{
//	if (AfxHookSource::Gui::OnReleaseCapture())
//		return TRUE;

	return ReleaseCapture();
}


////////////////////////////////////////////////////////////////////////////////

CamPath g_CamPath;

class CMirvCampath_Time : public IMirvCampath_Time
{
public:
	virtual double GetTime() {
		// Can be paused time, we don't support that currently.

		return curtime_get();
	}
	virtual double GetCurTime() {
		return curtime_get();
	}
	virtual bool GetCurrentDemoTick(int& outTick) {
		if(g_pEngineToClient) {
			if(SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile()) {
				outTick = pDemoFile->GetDemoTick();
				return true;
			}
		}
		return false;
	}
	virtual bool GetCurrentDemoTime(double& outDemoTime) {
		int tick;
		if(GetCurrentDemoTick(tick)) {
			outDemoTime = (tick + interpolation_amount_get())* (double)interval_per_tick_get();
			return true;
		}


		return false;
	}
	virtual bool GetDemoTickFromDemoTime(double curTime, double time, int& outTick) {
		outTick = (int)round(time / interval_per_tick_get());
		return true;
	}
	virtual bool GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime) {
		double current_demo_time;
		if(GetCurrentDemoTime(current_demo_time)) {
			outDemoTime = time - (curTime - current_demo_time);
			return true;
		}
		return false;
	}
    virtual bool GetDemoTickFromClientTime(double curTime, double targetTime, int& outTick)
    {
        double demoTime;
        return GetDemoTimeFromClientTime(curTime, targetTime, demoTime) && GetDemoTickFromDemoTime(curTime, demoTime, outTick);
    }
} g_MirvCampath_Time;

class CMirvCampath_Camera : public IMirvCampath_Camera
{
public:
	virtual SMirvCameraValue GetCamera() {
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
} g_MirvCampath_Camera;

class CMirvCampath_Drawer : public IMirvCampath_Drawer
{
public:
	virtual bool GetEnabled() {
		return g_CampathDrawer.Draw_get();
	}
	virtual void SetEnabled(bool value) {
		g_CampathDrawer.Draw_set(value);
	}
	virtual bool GetDrawKeyframeAxis() {
		return g_CampathDrawer.GetDrawKeyframeAxis();
	}
	virtual void SetDrawKeyframeAxis(bool value) {
		g_CampathDrawer.SetDrawKeyframeAxis(value);
	}
	virtual bool GetDrawKeyframeCam() {
		return g_CampathDrawer.GetDrawKeyframeCam();
	}
	virtual void SetDrawKeyframeCam(bool value) {
		g_CampathDrawer.SetDrawKeyframeCam(value);
	}

	virtual float GetDrawKeyframeIndex() { return g_CampathDrawer.GetDrawKeyframeIndex(); }
	virtual void SetDrawKeyframeIndex(float value) { g_CampathDrawer.SetDrawKeyframeIndex(value); }

} g_MirvCampath_Drawer;

CON_COMMAND(mirv_campath, "camera paths")
{
	if (nullptr == g_pGlobals)
	{
		advancedfx::Warning("Error: Hooks not installed.\n");
		return;
	}

	MirvCampath_ConCommand(args, advancedfx::Message, advancedfx::Warning, &g_CamPath, &g_MirvCampath_Time, &g_MirvCampath_Camera, &g_MirvCampath_Drawer);
}

double MirvCamIO_GetTimeFn(void) {
	return curtime_get();
}

CamImport * g_CamImport = nullptr;
CamExport * g_CamExport = nullptr;

CON_COMMAND(mirv_camio, "New camera motion data import / export.") {
	MirvCamIO_ConsoleCommand(args, g_CamImport, g_CamExport, MirvCamIO_GetTimeFn);
}


static bool g_bViewOverriden = false;
static float g_fFovOverride = 90.0f;
static float * g_pFov = nullptr;
int g_iWidth = 1920;
int g_iHeight = 1080;
SOURCESDK::VMatrix g_WorldToScreenMatrix;

extern bool g_b_on_c_view_render_setup_view;

bool CS2_Client_CSetupView_Trampoline_IsPlayingDemo(void *ThisCViewSetup) {
	if(!g_pEngineToClient) return false;

	bool originOrAnglesOverriden = false;

	float curTime = curtime_get(); //TODO: + m_PausedTime
	float absTime = absoluteframetime_get();

	int *pWidth = (int*)((unsigned char *)ThisCViewSetup + 0x474);
	int *pHeight = (int*)((unsigned char *)ThisCViewSetup + 0x47c);

	float *pFov = (float*)((unsigned char *)ThisCViewSetup + 0x4d8);
	float *pViewOrigin = (float*)((unsigned char *)ThisCViewSetup + 0x4e0);
	float *pViewAngles = (float*)((unsigned char *)ThisCViewSetup + 0x4f8);

	int width = *pWidth;
	int height = *pHeight;
	float Tx = pViewOrigin[0];
	float Ty = pViewOrigin[1];
	float Tz = pViewOrigin[2];
	float Rx = pViewAngles[0];
	float Ry = pViewAngles[1];
	float Rz = pViewAngles[2];
	float Fov = *pFov;

	//advancedfx::Message("Console: %i [%ix%i]\n", (g_pGameUIService->Con_IsVisible()?1:0),width,height);

	//advancedfx::Message("%f: (%f,%f,%f) (%f,%f,%f) [%f]\n",curTime,pViewOrigin[0],pViewOrigin[1],pViewOrigin[2],pViewAngles[0],pViewAngles[1],pViewAngles[2],*pFov);

	g_MirvInputEx.GameCameraOrigin[0] = Tx;
	g_MirvInputEx.GameCameraOrigin[1] = Ty;
	g_MirvInputEx.GameCameraOrigin[2] = Tz;
	g_MirvInputEx.GameCameraAngles[0] = Rx;
	g_MirvInputEx.GameCameraAngles[1] = Ry;
	g_MirvInputEx.GameCameraAngles[2] = Rz;
	g_MirvInputEx.GameCameraFov = Fov;

	if (g_CamPath.Enabled_get() && g_CamPath.CanEval())
	{
		double campathCurTime = curTime - g_CamPath.GetOffset();
		if(g_CamPath.GetHold()) {
			if(campathCurTime > g_CamPath.GetUpperBound()) campathCurTime = g_CamPath.GetUpperBound();
			else if(campathCurTime < g_CamPath.GetLowerBound()) campathCurTime = g_CamPath.GetLowerBound();
		}

		// no extrapolation:
		if (g_CamPath.GetLowerBound() <= campathCurTime && campathCurTime <= g_CamPath.GetUpperBound())
		{
			CamPathValue val = g_CamPath.Eval(campathCurTime);
			QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

			//Tier0_Msg("================",curTime);
			//Tier0_Msg("currenTime = %f",curTime);
			//Tier0_Msg("vCp = %f %f %f\n", val.X, val.Y, val.Z);

			originOrAnglesOverriden = true;

			Tx = (float)val.X;
			Ty = (float)val.Y;
			Tz = (float)val.Z;

			Rx = (float)ang.Pitch;
			Ry = (float)ang.Yaw;
			Rz = (float)ang.Roll;

			Fov = (float)val.Fov;
		}
	}

	if (g_CamImport)
	{
		CamIO::CamData camData;

		if (g_CamImport->GetCamData(curTime, width, height, camData))
		{
			originOrAnglesOverriden = true;

			Tx = (float)camData.XPosition;
			Ty = (float)camData.YPosition;
			Tz = (float)camData.ZPosition;
			Rx = (float)camData.YRotation;
			Ry = (float)camData.ZRotation;
			Rz = (float)camData.XRotation;
			Fov = (float)camData.Fov;
		}
	}	

	if(g_MirvInputEx.m_MirvInput->Override(g_MirvInputEx.LastFrameTime, Tx,Ty,Tz,Rx,Ry,Rz,Fov)) originOrAnglesOverriden = true;

	if(g_b_on_c_view_render_setup_view) {
		AfxHookSourceRsView currentView = {Tx,Ty,Tz,Rx,Ry,Rz,Fov};
		AfxHookSourceRsView gameView = {(float)g_MirvInputEx.GameCameraOrigin[0],(float)g_MirvInputEx.GameCameraOrigin[1],(float)g_MirvInputEx.GameCameraOrigin[2],(float)g_MirvInputEx.GameCameraAngles[0],(float)g_MirvInputEx.GameCameraAngles[1],(float)g_MirvInputEx.GameCameraAngles[2],(float)g_MirvInputEx.GameCameraFov};
		AfxHookSourceRsView lastView = {(float)g_MirvInputEx.LastCameraOrigin[0],(float)g_MirvInputEx.LastCameraOrigin[1],(float)g_MirvInputEx.LastCameraOrigin[2],(float)g_MirvInputEx.LastCameraAngles[0],(float)g_MirvInputEx.LastCameraAngles[1],(float)g_MirvInputEx.LastCameraAngles[2],(float)g_MirvInputEx.LastCameraFov};
		if(AfxHookSource2Rs_OnCViewRenderSetupView(
			curTime, absTime, (float)g_MirvInputEx.LastFrameTime,
			currentView, gameView, lastView,
			width,height
		)) {
			Tx = currentView.x;
			Ty = currentView.y;
			Tz = currentView.z;
			Rx = currentView.rx;
			Ry = currentView.ry;
			Rz = currentView.rz;
			Fov = currentView.fov;
			originOrAnglesOverriden = true;
		}		
	}

	if (g_CamExport)
	{
		CamIO::CamData camData;

		camData.Time = curTime;
		camData.XPosition = Tx;
		camData.YPosition = Ty;
		camData.ZPosition = Tz;
		camData.YRotation = Rx;
		camData.ZRotation = Ry;
		camData.XRotation = Rz;
		camData.Fov = Fov;

		g_CamExport->WriteFrame(width, height, camData);
	}	

	if(originOrAnglesOverriden) {
		pViewOrigin[0] = Tx;
		pViewOrigin[1] = Ty;
		pViewOrigin[2] = Tz;

		pViewAngles[0] = Rx;
		pViewAngles[1] = Ry;
		pViewAngles[2] = Rz;

		*pFov = Fov;

		g_bViewOverriden = true;
		g_fFovOverride = Fov;
		g_pFov = pFov;
	} else {
		g_bViewOverriden = false;
	}

	g_iWidth = width;
	g_iHeight = height;

	g_MirvInputEx.LastCameraOrigin[0] = Tx;
	g_MirvInputEx.LastCameraOrigin[1] = Ty;
	g_MirvInputEx.LastCameraOrigin[2] = Tz;
	g_MirvInputEx.LastCameraAngles[0] = Rx;
	g_MirvInputEx.LastCameraAngles[1] = Ry;
	g_MirvInputEx.LastCameraAngles[2] = Rz;
	g_MirvInputEx.LastCameraFov = Fov;

	g_MirvInputEx.LastFrameTime = absTime;

	g_MirvInputEx.LastWidth = width;
	g_MirvInputEx.LastHeight = height;

	g_MirvInputEx.m_MirvInput->Supply_MouseFrameEnd();

	return g_pEngineToClient->IsPlayingDemo();
}

float CS2_Client_CSetupView_InsideComputeViewMatrix(void) {

	if(g_bViewOverriden) {
		float * pWeaponFov = g_pFov + 1;
		float oldFov = *g_pFov;
		*g_pFov = g_fFovOverride;
		return 0 != *pWeaponFov ? g_fFovOverride - oldFov : 0; // update fov difference if necessary.
	}
	return 0;
}

/*size_t ofsProj = 0;

DirectX::XMMATRIX g_Mul = {
	{1,0,0,0},
	{0,1,0,0},
	{0,0,1,0},
	{0,0,0,1}
};

bool transpose = false;

CON_COMMAND(__mirv_t,"") {
	if(args->ArgC()>=2) transpose = 0 != atoi(args->ArgV(1));
}

CON_COMMAND(__mirv_o,"") {
	advancedfx::Message("=> %i\n",ofsProj);

	float b0 = ((ofsProj>>0) & 1) ? -1 : 1;
	float b1 = ((ofsProj>>1) & 1) ? -1 : 1;
	float b2 = ((ofsProj>>2) & 1) ? -1 : 1;

	switch((ofsProj>>3)%6) {
	default:
	case 0:
		// 0 1 2
		g_Mul = DirectX::XMMATRIX(
			b0, 0, 0, 0,
			0, b1, 0, 0,
			0, 0, b2, 0,
			0, 0, 0, 1
		);
		break;
	case 1:
		// 0 2 1
		g_Mul = DirectX::XMMATRIX(
			b0, 0, 0, 0,
			0, 0, b2, 0,
			0, b1, 0, 0,
			0, 0, 0, 1
		);
		break;
	case 2:
		// 1 0 2
		g_Mul = DirectX::XMMATRIX(
			0, b1, 0, 0,
			b0, 0, 0, 0,
			0, 0, b2, 0,
			0, 0, 0, 1
		);
		break;
	case 3:
		// 1 2 0
		// 0 1 2
		g_Mul = DirectX::XMMATRIX(
			0, b1, 0, 0,
			0, 0, b2, 0,
			b0, 0, 0, 0,
			0, 0, 0, 1
		);
		break;
	case 4:
		// 2 0 1
		g_Mul = DirectX::XMMATRIX(
			0, 0, b2, 0,
			b0, 0, 0, 0,
			0, b1, 0, 0,
			0, 0, 0, 1
		);		
		break;
	case 5:
		// 2 1 0
		g_Mul = DirectX::XMMATRIX(
			0, 0, b2, 0,
			0, b1, 0, 0,
			b0, 0, 0, 0,
			0, 0, 0, 1
		);		
		break;
	}


	ofsProj = (ofsProj + 1)%48;
}*/

typedef void (__fastcall * CViewRender_UnkMakeMatrix_t)(void* This);
CViewRender_UnkMakeMatrix_t g_Old_CViewRender_UnkMakeMatrix = nullptr;
void __fastcall New_CViewRender_UnkMakeMatrix(void* This) {
	g_Old_CViewRender_UnkMakeMatrix(This);
	//memcpy(g_WorldToScreenMatrix.m,(unsigned char*)This + 0x1b8,sizeof(g_WorldToScreenMatrix.m));


	/*DirectX::XMMATRIX * proj = (DirectX::XMMATRIX *)((unsigned char*)This + 0x298);
	DirectX::XMMATRIX result = g_Mul * *proj;
	if(transpose) result = DirectX::XMMatrixTranspose(result);

	g_WorldToScreenMatrix.m[0][0] = result(0,0);
	g_WorldToScreenMatrix.m[0][1] = result(0,1);
	g_WorldToScreenMatrix.m[0][2] = result(0,2);
	g_WorldToScreenMatrix.m[0][3] = result(0,3);
	g_WorldToScreenMatrix.m[1][0] = result(1,0);
	g_WorldToScreenMatrix.m[1][1] = result(1,1);
	g_WorldToScreenMatrix.m[1][2] = result(1,2);
	g_WorldToScreenMatrix.m[1][3] = result(1,3);
	g_WorldToScreenMatrix.m[2][0] = result(2,0);
	g_WorldToScreenMatrix.m[2][1] = result(2,1);
	g_WorldToScreenMatrix.m[2][2] = result(2,2);
	g_WorldToScreenMatrix.m[2][3] = result(2,3);
	g_WorldToScreenMatrix.m[3][0] = result(3,0);
	g_WorldToScreenMatrix.m[3][1] = result(3,1);
	g_WorldToScreenMatrix.m[3][2] = result(3,2);
	g_WorldToScreenMatrix.m[3][3] = result(3,3);*/

	float * proj = (float *)((unsigned char*)This + 0x298);

	g_WorldToScreenMatrix.m[0][0] = proj[4*0+0];
	g_WorldToScreenMatrix.m[0][1] = proj[4*0+1];
	g_WorldToScreenMatrix.m[0][2] = proj[4*0+2];
	g_WorldToScreenMatrix.m[0][3] = proj[4*0+3];
	g_WorldToScreenMatrix.m[1][0] = proj[4*1+0];
	g_WorldToScreenMatrix.m[1][1] = proj[4*1+1];
	g_WorldToScreenMatrix.m[1][2] = proj[4*1+2];
	g_WorldToScreenMatrix.m[1][3] = proj[4*1+3];
	g_WorldToScreenMatrix.m[2][0] = proj[4*2+0];
	g_WorldToScreenMatrix.m[2][1] = proj[4*2+1];
	g_WorldToScreenMatrix.m[2][2] = proj[4*2+2];
	g_WorldToScreenMatrix.m[2][3] = proj[4*2+3];
	g_WorldToScreenMatrix.m[3][0] = proj[4*3+0];
	g_WorldToScreenMatrix.m[3][1] = proj[4*3+1];
	g_WorldToScreenMatrix.m[3][2] = proj[4*3+2];
	g_WorldToScreenMatrix.m[3][3] = proj[4*3+3];

	g_CampathDrawer.OnEngineThread_SetupViewDone();
}

/*
class CCSGOVScriptGameSystem;
CCSGOVScriptGameSystem * g_pCCSGOVScriptGameSystem = nullptr;
typedef void (__fastcall * CCSGOVScriptGameSystem_UnkAddon_t)(CCSGOVScriptGameSystem *This); //:000
typedef unsigned long long int (__fastcall * CCSGOVScriptGameSystem_UnkLoadScriptFile_t)(CCSGOVScriptGameSystem *This, const char * pszFileName, bool bDebugPrint); //:008
CCSGOVScriptGameSystem_UnkAddon_t g_Old_CCSGOVScriptGameSystem_UnkAddon = nullptr;
CCSGOVScriptGameSystem_UnkLoadScriptFile_t g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile = nullptr;

void __fastcall New_CSGOVScriptGameSystem_UnkAddon(CCSGOVScriptGameSystem *This) {
	g_pCCSGOVScriptGameSystem = This;
	//advancedfx::Message("GOT IT\n");
	g_Old_CCSGOVScriptGameSystem_UnkAddon(This);
}

unsigned long long int __fastcall New_CCSGOVScriptGameSystem_UnkLoadScriptFile(CCSGOVScriptGameSystem *This, const char * pszFileName, bool bDebugPrint) {
	g_pCCSGOVScriptGameSystem = This;
	advancedfx::Message("LoadScriptFile: %s\n",pszFileName);
	return g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile(This, pszFileName, bDebugPrint);
}

CON_COMMAND(mirv_vscript_exec,"") {
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if(2 <= argC) {
		if(g_pCCSGOVScriptGameSystem) {
			g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile(g_pCCSGOVScriptGameSystem,args->ArgV(1),3 <= argC ? (0 != atoi(args->ArgV(2))) : true);
		} else advancedfx::Warning("Missing hooks.\n");
		return;
	}
	advancedfx::Message("%s <script_file_name> [<debug_print=0|1>]\n",arg0);
}*/

/*
typedef void (__fastcall * CViewRender_RenderView_t)(void* This, void * pViewSetup, void * pHudViewSetup, void * nClearFlags, void * whatToDraw);
CViewRender_RenderView_t g_Old_CViewRender_RenderView = nullptr;

enum ClearFlags_t
{
	VIEW_CLEAR_COLOR = 0x1,
	VIEW_CLEAR_DEPTH = 0x2,
	VIEW_CLEAR_FULL_TARGET = 0x4,
	VIEW_NO_DRAW = 0x8,
	VIEW_CLEAR_OBEY_STENCIL = 0x10, // Draws a quad allowing stencil test to clear through portals
	VIEW_CLEAR_STENCIL = 0x20,
};
void __fastcall New_CViewRender_RenderView(void* This, void * pViewSetup, void * pHudViewSetup, void * nClearFlags, void * whatToDraw) {
	return;
	g_Old_CViewRender_RenderView(This, pViewSetup, pHudViewSetup, nClearFlags, whatToDraw);
	//if((nClearFlags & VIEW_CLEAR_COLOR)&&(nClearFlags && VIEW_CLEAR_DEPTH)) {
		DrawCampath();
	//}
}*/

void HookClientDll(HMODULE clientDll) {
	static bool bFirstCall = true;
	if(!bFirstCall) return;
	bFirstCall = false;

	Afx::BinUtils::MemRange textRange = Afx::BinUtils::MemRange::FromEmpty();
	Afx::BinUtils::MemRange dataRange = Afx::BinUtils::MemRange::FromEmpty();
	{
		Afx::BinUtils::ImageSectionsReader sections((HMODULE)clientDll);
		if(!sections.Eof()) {
			textRange = sections.GetMemRange();
			sections.Next();
			if(!sections.Eof()){
				dataRange = sections.GetMemRange();
			}
		}
	}

	/*
		This is where it checks for engine->IsPlayingDemo() (and afterwards for cl_demoviewoverride (float))
		before under these conditions it is calling CalcDemoViewOverride, so this is in CViewRender::SetUpView:

.text:000000018076F3C1                 mov     rcx, cs:qword_18167FE18
.text:000000018076F3C8                 mov     rax, [rcx]
.text:000000018076F3CB                 call    qword ptr [rax+118h]
.text:000000018076F3D1                 mov     rbp, [rsp+948h]
.text:000000018076F3D9                 xorps   xmm6, xmm6
.text:000000018076F3DC                 test    al, al
.text:000000018076F3DE                 jz      short loc_18076F457
.text:000000018076F3E0                 mov     edx, 0FFFFFFFFh
	*/
	{
		Afx::BinUtils::MemRange result = FindPatternString(textRange, "48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 90 30 01 00 00 48 8B AC 24 48 09 00 00 0F 57 F6 84 C0 74 77 BA FF FF FF FF");
		if (!result.IsEmpty()) {
			/*
				These are the top 16 bytes we change to:

00007fff`95518b22 4889f1               mov     rcx, rsi
00007fff`95518b25 48b8???????????????? mov     rax, ???????????????? <-- here we load our hook's address
00007fff`95518b2f ff10                 call    qword ptr [rax]
00007fff`95518b31 90                   nop
			*/
			unsigned char asmCode[16]={
				0x48, 0x89, 0xf1,
				0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
				0xff, 0x10,
				0x90
			};
			static LPVOID ptr = CS2_Client_CSetupView_Trampoline_IsPlayingDemo;
			LPVOID ptrPtr = &ptr;
			memcpy(&asmCode[5], &ptrPtr, sizeof(LPVOID));

			MdtMemBlockInfos mbis;
			MdtMemAccessBegin((LPVOID)result.Start, 16, &mbis);
			memcpy((LPVOID)result.Start, asmCode, 16);
			MdtMemAccessEnd(&mbis);
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}	

	/*
		The FOV is overridden / computed a second time in the function called at the very end of
		CViewRender::SetUpView (see hook above on how to find it):

.text:000000018076F387                 movss   xmm0, dword ptr [rax]
.text:000000018076F38B                 movss   dword ptr [rbp+4E8h], xmm0
.text:000000018076F393
.text:000000018076F393 loc_18076F393:                          ; CODE XREF: sub_18076F120+460↓j
.text:000000018076F393                 xorps   xmm6, xmm6
.text:000000018076F396
.text:000000018076F396 loc_18076F396:                          ; CODE XREF: sub_18076F120+47D↓j
                                       <-- snip -->
.text:000000018076F396                 mov     ecx, ebx
.text:000000018076F398                 call    sub_1807F27D0
.text:000000018076F39D                 mov     rcx, rax
.text:000000018076F3A0                 mov     rdx, [rax]
                                       <-- snap -->
.text:000000018076F3A3                 call    qword ptr [rdx+0D8h]
	*/

	{
		Afx::BinUtils::MemRange result = FindPatternString(textRange, "F3 0F 10 00 F3 0F 11 85 E8 04 00 00 0F 57 F6 8B CB E8 ?? ?? ?? ?? 48 8B C8 48 8B 10 FF 92 D8 00 00 00");
		if (!result.IsEmpty()) {
			MdtMemBlockInfos mbis;
			MdtMemAccessBegin((LPVOID)(result.Start+15), 13, &mbis);

			static LPVOID ptr2 = CS2_Client_CSetupView_InsideComputeViewMatrix;
			LPVOID ptrPtr2 = &ptr2;
			size_t pCallAddress3 = result.Start+15+2+5+(*(unsigned int*)(result.Start+15+3));
			static LPVOID ptr3 = (LPVOID)pCallAddress3;
			LPVOID ptrPtr3 = &ptr3;
			size_t pCallAddress4 = result.Start+15+13;
			static LPVOID ptr4 = (LPVOID)pCallAddress4;
			LPVOID ptrPtr4 = &ptr4;
			unsigned char asmCode2[48]={
				0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // mov rax, qword addr
				0xff, 0x10, // call    qword ptr [rax] // our function to reapply FOV
				0xf3, 0x0f, 0x5c, 0xf0, // subss   xmm6, xmm0 // update intermediate FOV value with the difference
				0x8B, 0xCB, // mov     ecx, ebx
				0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // mov rax, qword addr
				0xff, 0x10, // call    qword ptr [rax] // the original function in the detour area
				0x48, 0x8B, 0xC8, // mov     rcx, rax
				0x48, 0x8B, 0x10, // rdx, [rax]
				0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // mov rax, qword addr
				0xff, 0x20 // jmp [rax] // back to where to continue
			};
			memcpy(&asmCode2[2], &ptrPtr2, sizeof(LPVOID));
			memcpy(&asmCode2[20], &ptrPtr3, sizeof(LPVOID));
			memcpy(&asmCode2[38], &ptrPtr4, sizeof(LPVOID));

			LPVOID pTrampoline = MdtAllocExecuteableMemory(48);
			memcpy(pTrampoline, asmCode2, 48);

/*
	00007ff9`3170f396 48b80000000000000000 mov     rax, 0
	00007ff9`3170f3a0 ff20                 jmp     qword ptr [rax]
	00007ff9`3170f3a2 90                   nop 
*/
			unsigned char asmCode[13]={
				0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // mov rax, qword addr
				0xff, 0x20, // jmp [rax]
				0x90 // nop
			};
			static LPVOID ptr = pTrampoline;
			LPVOID ptrPtr = &ptr;
			memcpy(&asmCode[2], &ptrPtr, sizeof(LPVOID));

			memcpy((LPVOID)(result.Start+15), asmCode, 13);
			MdtMemAccessEnd(&mbis);
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
/*
	if(void ** vtable = (void**)Afx::BinUtils::FindClassVtable(clientDll,".?AVCRenderingPipelineCsgo@@", 0, 0x0)) {
		g_Old_CViewRender_RenderView = (CViewRender_RenderView_t)vtable[0] ;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Old_CViewRender_RenderView,New_CViewRender_RenderView);
        // doesn't work without error // DetourAttach(&(PVOID&)g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile, New_CCSGOVScriptGameSystem_UnkLoadScriptFile);
        if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));*/

	if(!Hook_CGameEventManager((void*)clientDll)) ErrorBox(MkErrStr(__FILE__, __LINE__));
/*
	if(void ** vtable = (void**)Afx::BinUtils::FindClassVtable(clientDll,".?AVCCSGOVScriptGameSystem@@", 0, 0x10)) {
		g_Old_CCSGOVScriptGameSystem_UnkAddon = (CCSGOVScriptGameSystem_UnkAddon_t)vtable[0] ;
		g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile = (CCSGOVScriptGameSystem_UnkLoadScriptFile_t)vtable[5] ;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Old_CCSGOVScriptGameSystem_UnkAddon,New_CSGOVScriptGameSystem_UnkAddon);
        // doesn't work without error // DetourAttach(&(PVOID&)g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile, New_CCSGOVScriptGameSystem_UnkLoadScriptFile);
        if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
		else AfxDetourPtr((PVOID *)&(vtable[7]), New_CCSGOVScriptGameSystem_UnkLoadScriptFile, (PVOID*)&g_Old_CCSGOVScriptGameSystem_UnkLoadScriptFile);
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));*/

	if(void ** vtable = (void**)Afx::BinUtils::FindClassVtable(clientDll,".?AVCViewRender@@", 0, 0x0)) {
		g_Old_CViewRender_UnkMakeMatrix = (CViewRender_UnkMakeMatrix_t)vtable[4] ;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Old_CViewRender_UnkMakeMatrix,New_CViewRender_UnkMakeMatrix);
        if(NO_ERROR != DetourTransactionCommit()) ErrorBox(MkErrStr(__FILE__, __LINE__));
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));

	// client entity system related
	//
	// This is inside the callback function for cl_showents and this function references the string "Ent %3d: %s class %s name %s\n".
	/*
       180733490 40 53           PUSH       RBX
       180733492 48 81 ec        SUB        RSP,0x230
                 30 02 00 00
       180733499 48 8b 0d        MOV        RCX,qword ptr [DAT_181916778]                    = ??
                 d8 32 1e 01
       1807334a0 48 8d 94        LEA        RDX,[RSP + 0x250]
                 24 50 02 
                 00 00
       1807334a8 33 db           XOR        EBX,EBX	
	*/
	{
		Afx::BinUtils::MemRange range_cl_show_ents_callback = Afx::BinUtils::FindPatternString(textRange, "40 53 48 81 ec 30 02 00 00 48 8b 0d ?? ?? ?? ?? 48 8d 94 24 50 02 00 00 33 db e8 ?? ?? ?? ??");	
		if(!range_cl_show_ents_callback.IsEmpty()) {
			void * pEntityList = (void *)(range_cl_show_ents_callback.Start+0x9+7+*(int*)(range_cl_show_ents_callback.Start+0x9+3));
			void * pFnGetHighestEntityHandle = (void *)(range_cl_show_ents_callback.Start+0x1a+5+*(int*)(range_cl_show_ents_callback.Start+0x1a+1));
			Afx::BinUtils::MemRange range_call_get_entity_from_index = Afx::BinUtils::FindPatternString(Afx::BinUtils::MemRange::FromSize(range_cl_show_ents_callback.Start+0x49,5).And(textRange), "E8 ?? ?? ?? ??");
			if(!range_call_get_entity_from_index.IsEmpty()) {
				void * pFnGetEntityFromIndex = (void *)(range_call_get_entity_from_index.Start+5+*(int*)(range_call_get_entity_from_index.Start+1));
				if(! Hook_ClientEntitySystem( pEntityList, pFnGetHighestEntityHandle, pFnGetEntityFromIndex )) ErrorBox(MkErrStr(__FILE__, __LINE__));
			} else ErrorBox(MkErrStr(__FILE__, __LINE__));
		} else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
}

SOURCESDK::CreateInterfaceFn g_AppSystemFactory = nullptr;
SOURCESDK::CS2::IMemAlloc *SOURCESDK::CS2::g_pMemAlloc = nullptr;
SOURCESDK::CS2::ICvar * SOURCESDK::CS2::cvar = nullptr;
SOURCESDK::CS2::ICvar * SOURCESDK::CS2::g_pCVar = nullptr;
void * g_pSceneSystem = nullptr;

typedef bool (__fastcall * CSceneSystem_WaitForRenderingToComplete_t)(void * This);
CSceneSystem_WaitForRenderingToComplete_t g_Old_CSceneSystem_WaitForRenderingToComplete = nullptr;

bool __fastcall New_CSceneSystem_WaitForRenderingToComplete(void * This) {
	bool result = g_Old_CSceneSystem_WaitForRenderingToComplete(This);
	//DrawCampath();
	return result;
}

typedef int(* CCS2_Client_Connect_t)(void* This, SOURCESDK::CreateInterfaceFn appSystemFactory);
CCS2_Client_Connect_t old_CCS2_Client_Connect;
int new_CCS2_Client_Connect(void* This, SOURCESDK::CreateInterfaceFn appSystemFactory) {
	static bool bFirstCall = true;

	if (bFirstCall) {
		bFirstCall = false;

		void * iface = NULL;

		if (SOURCESDK::CS2::g_pCVar = SOURCESDK::CS2::cvar = (SOURCESDK::CS2::ICvar*)appSystemFactory(SOURCESDK_CS2_CVAR_INTERFACE_VERSION, NULL)) {
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));

		if (g_pEngineToClient = (SOURCESDK::CS2::ISource2EngineToClient*)appSystemFactory(SOURCESDK_CS2_ENGINE_TO_CLIENT_INTERFACE_VERSION, NULL)) {
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));

		if (g_pGameUIService = (SOURCESDK::CS2::IGameUIService*)appSystemFactory(SOURCESDK_CS2_GAMEUISERVICE_INTERFACE_VERSION, NULL)) {
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));

	}

	return old_CCS2_Client_Connect(This, appSystemFactory);
}

CON_COMMAND(mirv_suppress_disconnects, "Suppresses given number disconnect commands. Can help to test demo system in the CS2 Limited Test.") {
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);
	if(2 <= argC) {
			g_nIgnoreNextDisconnects = atoi(args->ArgV(1));
			return;
	}
	advancedfx::Message(
		"mirv_suppress_disconnects <iSuppressTimes> - Use -1 to always suppress, or a positive number to suppress a certain count.\n"
		"Eample: \"mirv_suppress_disconnects 1; playdemo test.dem\" - Deprecated command. May get removed in the future.\n"
		"Current value: %i\n",
		g_nIgnoreNextDisconnects
	);
}

CON_COMMAND(mirv_cvar_unhide_all, "Unlocks cmds and cvars.") {
	int total = 0;
	int nUnhidden = 0;
	for(size_t i = 0; i < 65536; i++ )
	{
		SOURCESDK::CS2::CCmd * cmd = SOURCESDK::CS2::g_pCVar->GetCmd(i);
		if(nullptr == cmd) break;
		int nFlags = cmd->GetFlags();
		if(nFlags == 0x400) break;
		total++;
		if(nFlags & (FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN)) {
//			fprintf(f1,"[+] %lli: 0x%08x: %s : %s\n", i, cmd->m_nFlags, cmd->m_pszName, cmd->m_pszHelpString);
			cmd->SetFlags(nFlags &= ~(SOURCESDK::int64)(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN));
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cmd->m_nFlags, cmd->m_pszName, cmd->m_pszHelpString);
		}
	}
	advancedfx::Message("==== Cmds total: %i (Cmds unhidden: %i) ====\n",total,nUnhidden);

	total = 0;
	nUnhidden = 0;
	for(size_t i = 0; i < 65536; i++ )
	{
		SOURCESDK::CS2::Cvar_s * cvar = SOURCESDK::CS2::g_pCVar->GetCvar(i);
		if(nullptr == cvar) break;
		total++;
		if(cvar->m_nFlags & (FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN)) {
//			fprintf(f1,"[+] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
			cvar->m_nFlags &= ~(SOURCESDK::int64)(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
		}
	}
	
	advancedfx::Message("==== Cvars total: %i (Cvars unhidden: %i) ====\n",total,nUnhidden);
}

CON_COMMAND(mirv_cvar_unlock_sv_cheats, "Unlocks sv_cheats on client (as much as possible).") {
	int total = 0;
	int nUnhidden = 0;
	for(size_t i = 0; i < 65536; i++ )
	{
		SOURCESDK::CS2::CCmd * cmd = SOURCESDK::CS2::g_pCVar->GetCmd(i);
		if(nullptr == cmd) break;
		int nFlags = cmd->GetFlags();
		if(nFlags == 0x400) break;
		total++;
		if(nFlags & (FCVAR_CHEAT)) {
//			fprintf(f1,"[+] %lli: 0x%08x: %s : %s\n", i, cmd->m_nFlags, cmd->m_pszName, cmd->m_pszHelpString);
			cmd->SetFlags(nFlags &= ~(SOURCESDK::int64)(FCVAR_CHEAT));
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cmd->m_nFlags, cmd->m_pszName, cmd->m_pszHelpString);
		}
	}
	advancedfx::Message("==== Cmds total: %i (Cmds unlocked: %i) ====\n",total,nUnhidden);

	total = 0;
	nUnhidden = 0;
	for(size_t i = 0; i < 65536; i++ )
	{
		SOURCESDK::CS2::Cvar_s * cvar = SOURCESDK::CS2::g_pCVar->GetCvar(i);
		if(nullptr == cvar) break;
		total++;
		if(cvar->m_nFlags & (FCVAR_CHEAT)) {
//			fprintf(f1,"[+] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
			cvar->m_nFlags &= ~(SOURCESDK::int64)(FCVAR_CHEAT);
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
		}
		if(0 == strcmp("sv_cheats",cvar->m_pszName)) {
			cvar->m_nFlags &= ~(SOURCESDK::int64)(FCVAR_REPLICATED|FCVAR_NOTIFY);
			cvar->m_nFlags |= FCVAR_CLIENTDLL;
			cvar->m_Value.m_bValue = true;			
		}
	}
	
	advancedfx::Message("==== Cvars total: %i (Cvars unlocked: %i) ====\n",total,nUnhidden);
}

typedef int(* CCS2_Client_Init_t)(void* This);
CCS2_Client_Init_t old_CCS2_Client_Init;
int new_CCS2_Client_Init(void* This) {
	int result = old_CCS2_Client_Init(This);

	if(!Hook_ClientEntitySystem2()) ErrorBox(MkErrStr(__FILE__, __LINE__));

	WrpRegisterCommands();

	AfxHookSource2Rs_Engine_Init();

	PrintInfo();

	return result;
}

typedef void(* CCS2_Client_Shutdown_t)(void* This);
CCS2_Client_Shutdown_t old_CCS2_Client_Shutdown;
void new_CCS2_Client_Shutdown(void* This) {
	AfxHookSource2Rs_Engine_Shutdown();

	old_CCS2_Client_Shutdown(This);
}


typedef void * (* CS2_Client_SetGlobals_t)(void* This, void * pGlobals);
CS2_Client_SetGlobals_t old_CS2_Client_SetGlobals;
void *  new_CS2_Client_SetGlobals(void* This, void * pGlobals) {

	g_pGlobals = (Cs2Gloabls_t)pGlobals;

	return old_CS2_Client_SetGlobals(This, pGlobals);
}

class CExecuteClientCmdForCommandSystem : public IExecuteClientCmdForCommandSystem {
public:
	virtual void ExecuteClientCmd(const char * value) {
		if(g_pEngineToClient) g_pEngineToClient->ExecuteClientCmd(0,value,true);
	}
} g_ExecuteClientCmdForCommandSystem;

class CGetTickForCommandSystem : public IGetTickForCommandSystem {
public:
	virtual float GetTick() {
		float tick = 0;
		if(g_pEngineToClient) {
			if(SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile()) {
				tick = (float)pDemoFile->GetDemoTick() + interpolation_amount_get();
			}
		}
		return tick;
	}
} g_GetTickForCommandSystem;

class CGetTimeForCommandSystem : public IGetTimeForCommandSystem {
public:
	virtual float GetTime() {
		return curtime_get();
	}
} g_GetTimeForCommandSystem;

class CommandSystem g_CommandSystem(&g_ExecuteClientCmdForCommandSystem, &g_GetTickForCommandSystem, &g_GetTimeForCommandSystem);

CON_COMMAND(mirv_cmd, "Command system (for scheduling commands).")
{
	g_CommandSystem.Console_Command(args);
}

class CMirvSkip_GotoDemoTick : public IMirvSkip_GotoDemoTick {
	virtual void GotoDemoTick(int tick) {
        std::ostringstream oss;
        oss << "demo_gototick " << tick;
        if(g_pEngineToClient) g_pEngineToClient->ExecuteClientCmd(0,oss.str().c_str(),true);		
	}
} g_MirvSkip_GotoDemoTick;

CON_COMMAND(mirv_skip, "for skipping through demos (uses demo_gototick)")
{
    MirvSkip_ConsoleCommand(args, &g_MirvCampath_Time, &g_MirvSkip_GotoDemoTick);
}

typedef void * (* CS2_Client_LevelInitPreEntity_t)(void* This, void * pUnk1, void * pUnk2);
CS2_Client_LevelInitPreEntity_t old_CS2_Client_LevelInitPreEntity;
void * new_CS2_Client_LevelInitPreEntity(void* This, void * pUnk1, void * pUnk2) {
	void * result = old_CS2_Client_LevelInitPreEntity(This, pUnk1, pUnk2);
	g_CommandSystem.OnLevelInitPreEntity();
	return result;
}

typedef void (* CS2_Client_FrameStageNotify_t)(void* This, SOURCESDK::CS2::ClientFrameStage_t curStage);
CS2_Client_FrameStageNotify_t old_CS2_Client_FrameStageNotify;
void  new_CS2_Client_FrameStageNotify(void* This, SOURCESDK::CS2::ClientFrameStage_t curStage) {

	// React to demo being paused / unpaused to work around Valve's new bandaid client time "fix":
	bool bIsDemoPaused = false;
	if(g_pEngineToClient) {
		if(SOURCESDK::CS2::IDemoFile * pDemoFile = g_pEngineToClient->GetDemoFile()) {
			if(pDemoFile->IsPlayingDemo())
				bIsDemoPaused = pDemoFile->IsDemoPaused();
		}
	}
	if(bIsDemoPaused != g_DemoPausedData.IsPaused) {
		if(bIsDemoPaused) {
			g_DemoPausedData.FirstPausedCurtime = curtime_get();
			g_DemoPausedData.FirstPausedInterpolationAmount = interpolation_amount_get();
			g_DemoPausedData.IsPaused = true;
		} else {
			g_DemoPausedData.IsPaused = false;
		}
	}	

	switch(curStage) {
	case SOURCESDK::CS2::FRAME_RENDER_START:
		// This apparently doesn't get called when demo is paused.
		g_CommandSystem.OnExecuteCommands();
		break;
	}

	AfxHookSource2Rs_Engine_OnClientFrameStageNotify(curStage, true);

	old_CS2_Client_FrameStageNotify(This, curStage);

	AfxHookSource2Rs_Engine_OnClientFrameStageNotify(curStage, false);

	switch(curStage) {
	case SOURCESDK::CS2::FRAME_RENDER_END:
		AfxHookSource2Rs_Engine_RunJobQueue();
		break;
	}
}


void CS2_HookClientDllInterface(void * iface)
{
	void ** vtable = *(void***)iface;

	AfxDetourPtr((PVOID *)&(vtable[0]), new_CCS2_Client_Connect, (PVOID*)&old_CCS2_Client_Connect);
	AfxDetourPtr((PVOID *)&(vtable[3]), new_CCS2_Client_Init, (PVOID*)&old_CCS2_Client_Init);
	AfxDetourPtr((PVOID *)&(vtable[4]), new_CCS2_Client_Shutdown, (PVOID*)&old_CCS2_Client_Shutdown);
	AfxDetourPtr((PVOID *)&(vtable[11]), new_CS2_Client_SetGlobals, (PVOID*)&old_CS2_Client_SetGlobals);
	AfxDetourPtr((PVOID *)&(vtable[34]), new_CS2_Client_LevelInitPreEntity, (PVOID*)&old_CS2_Client_LevelInitPreEntity);
	AfxDetourPtr((PVOID *)&(vtable[35]), new_CS2_Client_FrameStageNotify, (PVOID*)&old_CS2_Client_FrameStageNotify);
}

SOURCESDK::CreateInterfaceFn old_Client_CreateInterface = 0;

void* new_Client_CreateInterface(const char *pName, int *pReturnCode)
{
	static bool bFirstCall = true;

	void * pRet = old_Client_CreateInterface(pName, pReturnCode);

	if(bFirstCall)
	{
		bFirstCall = false;

		void * iface = NULL;
		
		if (iface = old_Client_CreateInterface(SOURCESDK_CS2_Source2Client_VERSION, NULL)) {
			CS2_HookClientDllInterface(iface);
		}
		else
		{
			ErrorBox("Could not get a supported VClient interface.");
		}
	}

	return pRet;
}


FARPROC WINAPI new_tier0_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = GetProcAddress(hModule, lpProcName);

	if (!nResult)
		return nResult;

	if (HIWORD(lpProcName))
	{
		if (!lstrcmp(lpProcName, "GetProcAddress"))
			return (FARPROC) &new_tier0_GetProcAddress;

		if (
			hModule == g_H_ClientDll
			&& !lstrcmp(lpProcName, "CreateInterface")
		) {
			old_Client_CreateInterface = (SOURCESDK::CreateInterfaceFn)nResult;
			return (FARPROC) &new_Client_CreateInterface;
		}
	}

	return nResult;
}

HMODULE WINAPI new_LoadLibraryA(LPCSTR lpLibFileName);
HMODULE WINAPI new_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI new_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

HANDLE
WINAPI
new_CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
);

BOOL
WINAPI
new_CreateDirectoryW(
    _In_ LPCWSTR lpPathName,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

BOOL
WINAPI
new_GetFileAttributesExW(
    _In_ LPCWSTR lpFileName,
    _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
    _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation
    );

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_tier0_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD)> g_Import_tier0_KERNEL32_LoadLibraryExW("LoadLibraryExW", &new_LoadLibraryExW);
CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_tier0_KERNEL32_GetProcAddress("GetProcAddress", &new_tier0_GetProcAddress);
CAfxImportFuncHook<HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE)> g_Import_tier0_KERNEL32_CreateFileW("CreateFileW", &new_CreateFileW);
CAfxImportFuncHook<BOOL(WINAPI*)(LPCWSTR, LPSECURITY_ATTRIBUTES)> g_Import_tier0_KERNEL32_CreateDirectoryW("CreateDirectoryW", &new_CreateDirectoryW);
CAfxImportFuncHook<BOOL(WINAPI*)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID)> g_Import_tier0_KERNEL32_GetFileAttributesExW("GetFileAttributesExW", &new_GetFileAttributesExW);

HANDLE WINAPI new_CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
)
{
	static bool bWasRecording = false; // allow startmovie wav-fixup by engine to get through one more time.
	if (AfxStreams_IsRcording() || bWasRecording) {
		std::wstring strFileName(lpFileName);
		for (auto& c : strFileName) c = std::tolower(c);
		if (StringEndsWithW(strFileName.c_str(), L"" ADVNACEDFX_STARTMOIVE_WAV_KEY ".wav")) {
			// Detours our wav to our folder.			
			bWasRecording = AfxStreams_IsRcording();
			std::wstring newPath(AfxStreams_GetTakeDir());
			newPath.append(L"\\audio.wav");
			return g_Import_tier0_KERNEL32_CreateFileW.TrueFunc(newPath.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		}
	}
	return g_Import_tier0_KERNEL32_CreateFileW.TrueFunc(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL
WINAPI
new_CreateDirectoryW(
    _In_ LPCWSTR lpPathName,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
    ) {

	if (AfxStreams_IsRcording()) {
		// Do not create dummy movie folders while recording startmovie wav.
		std::wstring strMovieFolder(L"\\\\?\\");
		strMovieFolder.append(GetProcessFolderW());
		strMovieFolder.append(L"csgo\\movie\\");
		if(StringBeginsWithW(lpPathName,strMovieFolder.c_str())) return TRUE;
	}

	return g_Import_tier0_KERNEL32_CreateDirectoryW.TrueFunc(lpPathName,lpSecurityAttributes);
}

BOOL
WINAPI
new_GetFileAttributesExW(
    _In_ LPCWSTR lpFileName,
    _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
    _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation
    ) {

	if (AfxStreams_IsRcording()) {
		std::wstring strFileName(lpFileName);
		for (auto& c : strFileName) c = std::tolower(c);
		if (StringEndsWithW(strFileName.c_str(), L"" ADVNACEDFX_STARTMOIVE_WAV_KEY ".wav")) {
			// Detours our wav to our folder.			
			std::wstring newPath(AfxStreams_GetTakeDir());
			newPath.append(L"\\audio.wav");
			return g_Import_tier0_KERNEL32_GetFileAttributesExW.TrueFunc(newPath.c_str(),fInfoLevelId,lpFileInformation);
		}
	}

	return g_Import_tier0_KERNEL32_GetFileAttributesExW.TrueFunc(lpFileName,fInfoLevelId,lpFileInformation);
}


CAfxImportDllHook g_Import_tier0_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_tier0_KERNEL32_LoadLibraryExA
	, &g_Import_tier0_KERNEL32_LoadLibraryExW
	, &g_Import_tier0_KERNEL32_GetProcAddress
	, &g_Import_tier0_KERNEL32_CreateFileW
	, &g_Import_tier0_KERNEL32_CreateDirectoryW
	, &g_Import_tier0_KERNEL32_GetFileAttributesExW}));

CAfxImportsHook g_Import_tier0(CAfxImportsHooks({
	&g_Import_tier0_KERNEL32 }));

void CommonHooks()
{
	static bool bFirstRun = true;
	static bool bFirstTier0 = true;

	// do not use messageboxes here, there is some friggin hooking going on in between by the
	// Source engine.

	if (bFirstRun)
	{
		bFirstRun = false;
	}
}

CAfxImportFuncHook<HMODULE (WINAPI *)(LPCSTR)> g_Import_launcher_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE (WINAPI *)(LPCSTR, HANDLE, DWORD)> g_Import_launcher_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_launcher_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_launcher_KERNEL32_LoadLibraryA
	, &g_Import_launcher_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_launcher(CAfxImportsHooks({
	&g_Import_launcher_KERNEL32 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_filesystem_steam_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_filesystem_steam_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_filesystem_steam_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_filesystem_steam_KERNEL32_LoadLibraryA
	, &g_Import_filesystem_steam_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_filesystem_steam(CAfxImportsHooks({
	&g_Import_filesystem_steam_KERNEL32 }));


//CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_engine2_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
//CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_engine2_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

//CAfxImportDllHook g_Import_engine2_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
//	&g_Import_engine2_KERNEL32_LoadLibraryA
//	, &g_Import_engine2_KERNEL32_LoadLibraryExA }));

void * New_SteamInternal_FindOrCreateUserInterface(void*pUser, const char * pIntervaceName);

CAfxImportFuncHook<void*(*)(void *, const char *)> g_Import_engine2_steam_api64_SteamInternal_FindOrCreateUserInterface("SteamInternal_FindOrCreateUserInterface", &New_SteamInternal_FindOrCreateUserInterface);

void * New_SteamInternal_FindOrCreateUserInterface(void*pUser, const char * pInterfaceName) {
	if(0 == strcmp(pInterfaceName,"STEAMREMOTESTORAGE_INTERFACE_VERSION016")) {
		if (int idx = g_CommandLine->FindParam(L"-afxDisableSteamStorage")) {
			return nullptr;
		}
	}
	
	return g_Import_engine2_steam_api64_SteamInternal_FindOrCreateUserInterface.GetTrueFuncValue()(pUser,pInterfaceName);
}

CAfxImportDllHook g_Import_engine2_steam_api64("steam_api64.dll", CAfxImportDllHooks({
	&g_Import_engine2_steam_api64_SteamInternal_FindOrCreateUserInterface }));

CAfxImportsHook g_Import_engine2(CAfxImportsHooks({
	//&g_Import_engine2_KERNEL32,
	&g_Import_engine2_steam_api64 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_materialsystem2_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_materialsystem2_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_materialsystem2_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_materialsystem2_KERNEL32_LoadLibraryA
	, &g_Import_materialsystem2_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_materialsystem2(CAfxImportsHooks({
	&g_Import_materialsystem2_KERNEL32 }));


CAfxImportFuncHook<LONG_PTR(WINAPI*)(HWND, int)> g_Import_SDL3_USER32_GetWindowLongW("GetWindowLongPtrW", &new_GetWindowLongPtrW);
CAfxImportFuncHook<LONG_PTR(WINAPI*)(HWND, int, LONG_PTR)> g_Import_SDL3_USER32_SetWindowLongW("SetWindowLongPtrW", &new_SetWindowLongPtrW);
CAfxImportFuncHook<HCURSOR(WINAPI*)(HCURSOR)> g_Import_SDL3_USER32_SetCursor("SetCursor", &new_SetCursor);
CAfxImportFuncHook<HWND(WINAPI*)(HWND)> g_Import_SDL3_USER32_SetCapture("SetCapture", &new_SetCapture);
CAfxImportFuncHook<BOOL(WINAPI*)()> g_Import_SDL3_USER32_ReleaseCapture("ReleaseCapture", &new_ReleaseCapture);
CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_SDL3_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos);
CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_SDL3_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);


UINT
WINAPI
New_GetRawInputBuffer(
    _Out_writes_bytes_opt_(*pcbSize) PRAWINPUT pData,
    _Inout_ PUINT pcbSize,
    _In_ UINT cbSizeHeader);

CAfxImportFuncHook<UINT(WINAPI*)(_Out_writes_bytes_opt_(*pcbSize) PRAWINPUT, _Inout_ PUINT, _In_ UINT cbSizeHeader)> g_Import_SDL3_USER32_GetRawInputBuffer("GetRawInputBuffer", &New_GetRawInputBuffer);

UINT
WINAPI
New_GetRawInputBuffer(
    _Out_writes_bytes_opt_(*pcbSize) PRAWINPUT pData,
    _Inout_ PUINT pcbSize,
    _In_ UINT cbSizeHeader) {
	UINT result = g_Import_SDL3_USER32_GetRawInputBuffer.GetTrueFuncValue()(pData,pcbSize,cbSizeHeader);

	result = g_MirvInputEx.m_MirvInput->Supply_RawInputBuffer(result, pData,pcbSize,cbSizeHeader);

	return result;

}


UINT WINAPI New_GetRawInputData(
    _In_ HRAWINPUT hRawInput,
    _In_ UINT uiCommand,
    _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,
    _Inout_ PUINT pcbSize,
    _In_ UINT cbSizeHeader);

CAfxImportFuncHook<UINT(WINAPI*)(_In_ HRAWINPUT, _In_ UINT, _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,_Inout_ PUINT,_In_ UINT)> g_Import_SDL3_USER32_GetRawInputData("GetRawInputData", &New_GetRawInputData);

UINT WINAPI New_GetRawInputData(
    _In_ HRAWINPUT hRawInput,
    _In_ UINT uiCommand,
    _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,
    _Inout_ PUINT pcbSize,
    _In_ UINT cbSizeHeader) {

	UINT result = g_Import_SDL3_USER32_GetRawInputData.GetTrueFuncValue()(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);

	result = g_MirvInputEx.m_MirvInput->Supply_RawInputData(result, hRawInput, uiCommand,pData,pcbSize,cbSizeHeader);

	return result;
}


CAfxImportDllHook g_Import_SDL3_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_SDL3_USER32_GetWindowLongW,
	&g_Import_SDL3_USER32_SetWindowLongW,
	&g_Import_SDL3_USER32_SetCursor,
	&g_Import_SDL3_USER32_SetCapture,
	&g_Import_SDL3_USER32_ReleaseCapture,
	&g_Import_SDL3_USER32_GetCursorPos,
	&g_Import_SDL3_USER32_SetCursorPos,
	&g_Import_SDL3_USER32_GetRawInputData,
	&g_Import_SDL3_USER32_GetRawInputBuffer }));

CAfxImportsHook g_Import_SDL3(CAfxImportsHooks({
	&g_Import_SDL3_USER32 }));

CAfxImportFuncHook<LONG_PTR(WINAPI*)(HWND, int)> g_Import_inputsystem_USER32_GetWindowLongW("GetWindowLongPtrW", &new_GetWindowLongPtrW);
CAfxImportFuncHook<LONG_PTR(WINAPI*)(HWND, int, LONG_PTR)> g_Import_inputsystem_USER32_SetWindowLongW("SetWindowLongPtrW", &new_SetWindowLongPtrW);
CAfxImportFuncHook<HCURSOR(WINAPI*)(HCURSOR)> g_Import_inputsystem_USER32_SetCursor("SetCursor", &new_SetCursor);
CAfxImportFuncHook<HWND(WINAPI*)(HWND)> g_Import_inputsystem_USER32_SetCapture("SetCapture", &new_SetCapture);
CAfxImportFuncHook<BOOL(WINAPI*)()> g_Import_inputsystem_USER32_ReleaseCapture("ReleaseCapture", &new_ReleaseCapture);
CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_inputsystem_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos);
CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_inputsystem_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);

CAfxImportDllHook g_Import_inputsystem_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_inputsystem_USER32_GetWindowLongW,
	&g_Import_inputsystem_USER32_SetWindowLongW,
	&g_Import_inputsystem_USER32_SetCursor,
	&g_Import_inputsystem_USER32_SetCapture,
	&g_Import_inputsystem_USER32_ReleaseCapture,
	&g_Import_inputsystem_USER32_GetCursorPos,
	&g_Import_inputsystem_USER32_SetCursorPos }));

CAfxImportsHook g_Import_inputsystem(CAfxImportsHooks({
	&g_Import_inputsystem_USER32 }));

//CAfxImportDllHook g_Import_client_steam_api64("steam_api64.dll", CAfxImportDllHooks({
//	&g_Import_client_steam_api64_SteamInternal_FindOrCreateUserInterface }));
//
//CAfxImportsHook g_Import_client(CAfxImportsHooks({
//&g_Import_client_steam_api64 }));

void LibraryHooksA(HMODULE hModule, LPCSTR lpLibFileName)
{
	CommonHooks();

	if(!hModule || !lpLibFileName)
		return;

#if 0
	static FILE *f1=NULL;

	if( !f1 ) f1=fopen("hlae_log_LibraryHooksA.txt","wb");
	fprintf(f1,"%s\n", lpLibFileName);
	fflush(f1);
#endif
}

advancedfx::Con_Printf_t Tier0_Message = nullptr;
advancedfx::Con_Printf_t Tier0_Warning = nullptr;
advancedfx::Con_DevPrintf_t Tier0_DevMessage = nullptr;
advancedfx::Con_DevPrintf_t Tier0_DevWarning = nullptr;

class CConsolePrint_Message : public IConsolePrint {
public:
	virtual void Print(const char * text) {
		Tier0_Message(text);
	}
};

class CConsolePrint_Warning : public IConsolePrint {
public:
	virtual void Print(const char * text) {
		Tier0_Warning(text);
	}
};

class CConsolePrint_DevMessage : public IConsolePrint {
public:
	CConsolePrint_DevMessage(int level)
	: m_Level(level) {

	}

	virtual void Print(const char * text) {
		Tier0_DevMessage(m_Level, text);
	}
private:
	int m_Level;
};

class CConsolePrint_DevWarning : public IConsolePrint {
public:
	CConsolePrint_DevWarning(int level)
	: m_Level(level) {

	}

	virtual void Print(const char * text) {
		Tier0_DevWarning(m_Level, text);
	}
private:
	int m_Level;
};

CConsolePrinter * g_ConsolePrinter = nullptr;

void My_Console_Message(const char* fmt, ...) {
	CConsolePrint_Message consolePrint;
	va_list args;
	va_start(args, fmt);
	g_ConsolePrinter->Print(&consolePrint, fmt, args);
	va_end(args);
}

void My_Console_Warning(const char* fmt, ...) {
	CConsolePrint_Warning consolePrint;
	va_list args;
	va_start(args, fmt);
	g_ConsolePrinter->Print(&consolePrint, fmt, args);
	va_end(args);
}

void My_Console_DevMessage(int level, const char* fmt, ...) {
	CConsolePrint_DevMessage consolePrint(level);
	va_list args;
	va_start(args, fmt);
	g_ConsolePrinter->Print(&consolePrint, fmt, args);
	va_end(args);
}

void My_Console_DevWarning(int level, const char* fmt, ...) {
	CConsolePrint_DevWarning consolePrint(level);
	va_list args;
	va_start(args, fmt);
	g_ConsolePrinter->Print(&consolePrint, fmt, args);
	va_end(args);
}

void LibraryHooksW(HMODULE hModule, LPCWSTR lpLibFileName)
{
	static bool bFirstTier0 = true;
	static bool bFirstClient = true;
	static bool bFirstEngine2 = true;
	static bool bFirstfilesystem_stdio = true;
	static bool bFirstMaterialsystem2 = true;
	static bool bFirstInputsystem = true;
	static bool bFirstSDL3 = true;
	static bool bFirstRenderSystemDX11 = true;
	
	CommonHooks();

	if (!hModule || !lpLibFileName)
		return;

#if 0
	static FILE *f1 = NULL;

	if (!f1) f1 = fopen("hlae_log_LibraryHooksW.txt", "wb");
	fwprintf(f1, L"%s\n", lpLibFileName);
	fflush(f1);
#endif


	if(bFirstTier0 && StringEndsWithW( lpLibFileName, L"tier0.dll"))
	{
		bFirstTier0 = false;
		
		g_Import_tier0.Apply(hModule);

		SOURCESDK::CS2::g_pMemAlloc = *(SOURCESDK::CS2::IMemAlloc **)GetProcAddress(hModule, "g_pMemAlloc");

		if(Tier0_Message = (Tier0MsgFn)GetProcAddress(hModule, "Msg"))
			advancedfx::Message = My_Console_Message;
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
		if(Tier0_Warning = (Tier0MsgFn)GetProcAddress(hModule, "Warning"))
			advancedfx::Warning = My_Console_Warning;
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
		if(Tier0_DevMessage = (Tier0DevMsgFn)GetProcAddress(hModule, "DevMsg"))
			advancedfx::DevMessage = My_Console_DevMessage;
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
		if(Tier0_DevWarning = (Tier0DevMsgFn)GetProcAddress(hModule, "DevWarning"))
			advancedfx::DevWarning = My_Console_DevWarning;
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	//else if(bFirstfilesystem_stdio && StringEndsWithW( lpLibFileName, L"filesystem_stdio.dll"))
	//{
	//	bFirstfilesystem_stdio = false;
	//	
	//	g_Import_filesystem_stdio.Apply(hModule);
	//}
	else if(bFirstInputsystem && StringEndsWithW(lpLibFileName, L"inputsystem.dll"))
	{
		bFirstInputsystem = false;

		g_Import_inputsystem.Apply(hModule);
	}	
	else if(bFirstSDL3 && StringEndsWithW(lpLibFileName, L"SDL3.dll"))
	{
		bFirstSDL3 = false;

		g_Import_SDL3.Apply(hModule);
	}
	else if(bFirstEngine2 && StringEndsWithW( lpLibFileName, L"engine2.dll"))
	{
		bFirstEngine2 = false;

		g_h_engine2Dll = hModule;

		HookEngineDll(hModule);

		g_Import_engine2.Apply(hModule);
	}
	/*else if(bFirstMaterialsystem2 && StringEndsWithW( lpLibFileName, L"materialsystem2.dll"))
	{
		bFirstMaterialsystem2 = false;

		g_Import_materialsystem2.Apply(hModule);
	}*/
	else if(bFirstRenderSystemDX11 && StringEndsWithW( lpLibFileName, L"rendersystemdx11.dll"))
	{
		bFirstRenderSystemDX11 = false;

		Hook_RenderSystemDX11((void*)hModule);
	}
	else if(bFirstClient && StringEndsWithW(lpLibFileName, L"csgo\\bin\\win64\\client.dll"))
	{
		bFirstClient = false;

		g_H_ClientDll = hModule;

		//if(!g_Import_client.Apply(hModule)) ErrorBox("client.dll steam_api64 hooks failed.");

		HookClientDll(hModule);
	}
	
}

HMODULE WINAPI new_LoadLibraryA( LPCSTR lpLibFileName ) {
	HMODULE hRet = LoadLibraryA(lpLibFileName);

	LibraryHooksA(hRet, lpLibFileName);

	return hRet;
}

HMODULE WINAPI new_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE hRet = LoadLibraryExA(lpLibFileName, hFile, dwFlags);

	LibraryHooksA(hRet, lpLibFileName);

	return hRet;
}

HMODULE WINAPI new_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE hRet = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	LibraryHooksW(hRet, lpLibFileName);

	return hRet;
}

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_PROCESS_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_PROCESS_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD)> g_Import_PROCESS_KERNEL32_LoadLibraryExW("LoadLibraryExW", &new_LoadLibraryExW);

CAfxImportDllHook g_Import_PROCESS_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_PROCESS_KERNEL32_LoadLibraryA
	, &g_Import_PROCESS_KERNEL32_LoadLibraryExA
	, &g_Import_PROCESS_KERNEL32_LoadLibraryExW }));

CAfxImportsHook g_Import_PROCESS(CAfxImportsHooks({
	&g_Import_PROCESS_KERNEL32 }));


advancedfx::CThreadPool * g_pThreadPool = nullptr;
advancedfx::CImageBufferPoolThreadSafe * g_pImageBufferPoolThreadSafe = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason) 
	{ 
		case DLL_PROCESS_ATTACH:
		{
			g_CommandLine = new advancedfx::CCommandLine();

			if(!g_CommandLine->FindParam(L"-insecure"))
			{
				ErrorBox("Please add -insecure to launch options, AfxHookSource2 will refuse to work without it!");

				HANDLE hproc = OpenProcess(PROCESS_TERMINATE, true, GetCurrentProcessId());
				TerminateProcess(hproc, 0);
				CloseHandle(hproc);
				
				do MessageBoxA(NULL, "Please terminate the game manually in the taskmanager!", "Cannot terminate, please help:", MB_OK | MB_ICONERROR);
				while (true);
			}

#if _DEBUG
			MessageBox(0,"DLL_PROCESS_ATTACH","MDT_DEBUG",MB_OK);
#endif
			g_Import_PROCESS.Apply(GetModuleHandle(NULL));

			if (!(g_Import_PROCESS_KERNEL32_LoadLibraryA.TrueFunc || g_Import_PROCESS_KERNEL32_LoadLibraryExA.TrueFunc || g_Import_PROCESS_KERNEL32_LoadLibraryExW.TrueFunc))
				ErrorBox();

			//
			// Remember we are not on the main program thread here,
			// instead we are on our own thread, so don't run
			// things here that would have problems with that.
			//

			size_t thread_pool_thread_count = advancedfx::CThreadPool::GetDefaultThreadCount();
			if (int idx = g_CommandLine->FindParam(L"-afxThreadPoolSize")) {
				if (idx + 1 < g_CommandLine->GetArgC()) {
					thread_pool_thread_count = (size_t)wcstoul( g_CommandLine->GetArgV(idx + 1), nullptr, 10);
				}
			}
			g_pThreadPool = new advancedfx::CThreadPool(thread_pool_thread_count);

			g_pImageBufferPoolThreadSafe = new advancedfx::CImageBufferPoolThreadSafe();

			g_ConsolePrinter = new CConsolePrinter();

			g_CampathDrawer.Begin();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			// actually this gets called now.

			g_CampathDrawer.End();

			delete g_CamExport;
			delete g_CamImport;

			delete g_ConsolePrinter;

			delete g_pImageBufferPoolThreadSafe;

			delete g_pThreadPool;

#ifdef _DEBUG
			_CrtDumpMemoryLeaks();
#endif

			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	return TRUE;
}
