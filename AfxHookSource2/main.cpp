#include "stdafx.h"

#include "WrpConsole.h"

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
#include "../shared/StringTools.h"
#include "../shared/binutils.h"
#include "../shared/MirvCampath.h"
#include "../shared/MirvInput.h"

#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <mutex>

advancedfx::CCommandLine  * g_CommandLine = nullptr;

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookCS2", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}


int g_nIgnoreNextDisconnects = 0;

typedef void (*Unknown_ExecuteClientCommandFromNetChan_t)(void * Ecx, void * Edx, void *R8);
Unknown_ExecuteClientCommandFromNetChan_t g_Old_Unknown_ExecuteClientCommandFromNetChan = nullptr;
void New_Unknown_ExecuteClientCommandFromNetChan(void * Ecx, void * Edx, SOURCESDK::CS2::CCommand *r8Command) {
	if(0 < g_nIgnoreNextDisconnects && 0 < r8Command->ArgC()) {
		if(0 == stricmp("disconnect",r8Command->ArgV(0))) {
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

float curtime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals + 0x2c) : 0;
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
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +0x44) : 1;
}

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
		delete this;
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
	case WM_INPUT:
		{
			HRAWINPUT hRawInput = (HRAWINPUT)lParam;
			RAWINPUT inp;
			UINT size = sizeof(inp);

			UINT getRawInputResult = GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

			if(-1 != getRawInputResult && inp.header.dwType == RIM_TYPEMOUSE)
			{
				RAWMOUSE * rawmouse = &inp.data.mouse;
				LONG dX, dY;

				if((rawmouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
				{
					dX = rawmouse->lLastX;
					dY = rawmouse->lLastY;
				}
				else
				{
					static bool initial = true;
					static LONG lastX = 0;
					static LONG lastY = 0;

					if(initial)
					{
						initial = false;
						lastX = rawmouse->lLastX;
						lastY = rawmouse->lLastY;
					}

					dX = rawmouse->lLastX -lastX;
					dY = rawmouse->lLastY -lastY;

					lastX = rawmouse->lLastX;
					lastY = rawmouse->lLastY;
				}

				if (g_MirvInputEx.m_MirvInput->Supply_RawMouseMotion(dX, dY))
					return DefWindowProcW(hwnd, uMsg, wParam, lParam);
			}
		}
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
			outDemoTime = tick * (double)interval_per_tick_get();
			return true;
		}


		return false;
	}
	virtual bool GetDemoTickFromDemoTime(double curTime, double time, int& outTick) {
		outTick = (int)round(time / interval_per_tick_get());
		return true;
	}
	virtual bool GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime) {
		int current_tick;
		if(GetCurrentDemoTick(current_tick)) {
			outDemoTime = time - (curTime - current_tick * (double)interval_per_tick_get());
			return true;
		}
		return false;
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


CON_COMMAND(mirv_campath, "camera paths")
{
	if (nullptr == g_pGlobals)
	{
		advancedfx::Warning("Error: Hooks not installed.\n");
		return;
	}

	MirvCampath_ConCommand(args, advancedfx::Message, advancedfx::Warning, &g_CamPath, &g_MirvCampath_Time, &g_MirvCampath_Camera, nullptr);
}

static bool g_bViewOverriden = false;
static float g_fFovOverride = 90.0f;
static float * g_pFov = nullptr;

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

	if(g_MirvInputEx.m_MirvInput->Override(g_MirvInputEx.LastFrameTime, Tx,Ty,Tz,Rx,Ry,Rz,Fov)) originOrAnglesOverriden = true;

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

void HookClientDll(HMODULE clientDll) {
	static bool bFirstCall = true;
	if(!bFirstCall) return;
	bFirstCall = false;

	Afx::BinUtils::ImageSectionsReader sections((HMODULE)clientDll);
	Afx::BinUtils::MemRange textRange = sections.GetMemRange();

	/*
		This is where it checks for engine->IsPlayingDemo() (and afterwards for cl_demoviewoverride (float))
		before under these conditions it is calling CalcDemoViewOverride, so this is in CViewRender::SetUpView:

.text:000000018076F3C1                 mov     rcx, cs:qword_18167FE18
.text:000000018076F3C8                 mov     rax, [rcx]
.text:000000018076F3CB                 call    qword ptr [rax+110h]
.text:000000018076F3D1                 mov     rbp, [rsp+948h]
.text:000000018076F3D9                 xorps   xmm6, xmm6
.text:000000018076F3DC                 test    al, al
.text:000000018076F3DE                 jz      short loc_18076F457
.text:000000018076F3E0                 mov     edx, 0FFFFFFFFh
	*/
	{
		Afx::BinUtils::MemRange result = FindPatternString(textRange, "48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 90 10 01 00 00 48 8B AC 24 48 09 00 00 0F 57 F6 84 C0 74 77 BA FF FF FF FF");
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
}

SOURCESDK::CreateInterfaceFn g_AppSystemFactory = 0;
SOURCESDK::CS2::IMemAlloc *SOURCESDK::CS2::g_pMemAlloc = 0;
SOURCESDK::CS2::ICvar * SOURCESDK::CS2::cvar = 0;
SOURCESDK::CS2::ICvar * SOURCESDK::CS2::g_pCVar = 0;

typedef int(* CCS2_Client_Connect_t)(void* This, SOURCESDK::CreateInterfaceFn appSystemFactor);
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

CON_COMMAND(mirv_suppress_disconnects, "Suppresses given number disconnect commands. Can help to test demo system.") {
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);
	if(2 <= argC) {
			g_nIgnoreNextDisconnects = atoi(args->ArgV(1));
			return;
	}
	advancedfx::Message(
		"mirv_suppress_disconnects <iSuppressTimes> - Use -1 to always suppress, or a positive number to suppress a certain count.\n"
		"Eample: \"mirv_suppress_disconnects 1; playdemo test.dem\" - Please don't report bugs for this to Valve, the system is not meant to be used yet!\n"
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
			cmd->SetFlags(nFlags &= ~(int)(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN));
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
			cvar->m_nFlags &= ~(int)(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
		}
	}
	
	advancedfx::Message("==== Cvars total: %i (Cvars unhidden: %i) ====\n",total,nUnhidden);
}

typedef int(* CCS2_Client_Init_t)(void* This);
CCS2_Client_Init_t old_CCS2_Client_Init;
int new_CCS2_Client_Init(void* This) {
	int result = old_CCS2_Client_Init(This);

	WrpRegisterCommands();

	return result;
}

typedef void * (* CS2_Client_SetGlobals_t)(void* This, void * pGlobals);
CS2_Client_SetGlobals_t old_CS2_Client_SetGlobals;
void *  new_CS2_Client_SetGlobals(void* This, void * pGlobals) {

	g_pGlobals = (Cs2Gloabls_t)pGlobals;

	return old_CS2_Client_SetGlobals(This, pGlobals);
}


void CS2_HookClientDllInterface(void * iface)
{
	void ** vtable = *(void***)iface;

	AfxDetourPtr((PVOID *)&(vtable[0]), new_CCS2_Client_Connect, (PVOID*)&old_CCS2_Client_Connect);
	AfxDetourPtr((PVOID *)&(vtable[3]), new_CCS2_Client_Init, (PVOID*)&old_CCS2_Client_Init);
	AfxDetourPtr((PVOID *)&(vtable[11]), new_CS2_Client_SetGlobals, (PVOID*)&old_CS2_Client_SetGlobals);
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


HMODULE g_H_ClientDll = 0;

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

HMODULE g_h_engine2Dll;


CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_tier0_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD)> g_Import_tier0_KERNEL32_LoadLibraryExW("LoadLibraryExW", &new_LoadLibraryExW);
CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_tier0_KERNEL32_GetProcAddress("GetProcAddress", &new_tier0_GetProcAddress);

CAfxImportDllHook g_Import_tier0_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_tier0_KERNEL32_LoadLibraryExA
	, &g_Import_tier0_KERNEL32_LoadLibraryExW
	, &g_Import_tier0_KERNEL32_GetProcAddress }));

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


CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_filesystem_stdio_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_filesystem_stdio_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);


CAfxImportDllHook g_Import_filesystem_stdio_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_filesystem_stdio_KERNEL32_LoadLibraryA
	, &g_Import_filesystem_stdio_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_filesystem_stdio(CAfxImportsHooks({
	&g_Import_filesystem_stdio_KERNEL32 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_engine2_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_engine2_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_engine2_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_engine2_KERNEL32_LoadLibraryA
	, &g_Import_engine2_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_engine2(CAfxImportsHooks({
	&g_Import_engine2_KERNEL32 }));

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

CAfxImportDllHook g_Import_SDL3_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_SDL3_USER32_GetWindowLongW,
	&g_Import_SDL3_USER32_SetWindowLongW,
	&g_Import_SDL3_USER32_SetCursor,
	&g_Import_SDL3_USER32_SetCapture,
	&g_Import_SDL3_USER32_ReleaseCapture,
	&g_Import_SDL3_USER32_GetCursorPos,
	&g_Import_SDL3_USER32_SetCursorPos }));

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

class IConsolePrint {
public:
	virtual void Print(const char * text) = 0;
};

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

class CConsolePrinter {
public:
	void Print(IConsolePrint * pConsolePrint, const char* fmt, va_list args) {
		std::unique_lock<std::mutex> lock(m_Print_Mutex);
		if(nullptr == m_Print_Memory) {
			m_Print_Memory = (char *)malloc(sizeof(char)*512);
			if(nullptr != m_Print_Memory) m_Print_MemorySize = 512;
			else return;
		}
		int chars = vsnprintf(m_Print_Memory,m_Print_MemorySize,fmt,args);
		if(chars < 0) return;
		if(chars >= m_Print_MemorySize) {
			m_Print_Memory = (char *)realloc(m_Print_Memory,sizeof(char)*(chars+1));
			if(nullptr != m_Print_Memory) m_Print_MemorySize = chars+1;
			else return;		
			vsnprintf(m_Print_Memory,m_Print_MemorySize,fmt,args);
		}
		char * ptr = m_Print_Memory;
		for(int i = 0; true; ) {
			bool bEndReached = false;
			bool bNewLine = false;
			size_t length = 1;
			size_t size = 1;
			unsigned char lb = ptr[i];
			if (( lb & 0x80 ) == 0 ) { // lead bit is zero, must be a single ascii
				switch(lb) {
				case '\0':
					length = 0;
					bEndReached = true;
					break;
				case '\n':
					length = 0;
					bNewLine = true;
					break;
				default:
					length = 1;
				}
			}
			else if (( lb & 0xE0 ) == 0xC0 ) { // 110x xxxx
				size = 2;
			} else if (( lb & 0xF0 ) == 0xE0 ) { // 1110 xxxx
				size = 3;
			} else if (( lb & 0xF8 ) == 0xF0 ) { // 1111 0xxx
				size = 4;
			} else {
				// invalid UTF-8 length.
				ptr[i] = '\0';
				bEndReached = true;
			}
			if(i+size > chars) {
				// UTF-8 too long / invalid
				ptr[i] = '\0';
				bEndReached = true;
				length = 0;
				size = 1;
			} else {
				for(size_t j=1; j < size; j++) {		
					if((ptr[i+j] & 0xC0) != 0x8) {
						// following octets must be 0x10xxxxxx, so  this is invalid.
						ptr[i] = '\0';
						bEndReached = true;
						length = 0;
						size = 1;
						break;
					}
				}
			}
			if(bEndReached) {
				pConsolePrint->Print(ptr);
				m_Print_Length += length;
				break;
			}
			bool bSizeLimitReached = i + size >= m_Print_SizeLimit;
			if(bSizeLimitReached) {
				char tmp = ptr[i];
				ptr[i] = '\0';
				pConsolePrint->Print(ptr);
				ptr[i] = tmp;
				ptr = &(ptr[i]);
				m_Print_Length += length;
				chars -= i;
				i = 0;
				continue;
			}
			if(bNewLine) {
				i += size;
				m_Print_Length = 0;
			} else {
				bool bLineLimitReached = 0 < m_Print_LineLimit && m_Print_Length + length >= m_Print_LineLimit;
				if(bLineLimitReached) {
					char tmp = ptr[i];
					char tmp2 = ptr[i+1];
					ptr[i] = '\n';
					ptr[i+1] = '\0';
					pConsolePrint->Print(ptr);
					ptr[i] = tmp;
					ptr[i+1] = tmp2;
					ptr = &(ptr[i]);
					m_Print_Length = 0;
					chars -= i;
					i = 0;
					continue;
				} else {
					i += size;
					m_Print_Length += length;
				}
			}
		}
	}

private:
	std::mutex m_Print_Mutex;
	char * m_Print_Memory = nullptr;
	size_t m_Print_MemorySize = 0;
	size_t m_Print_Length = 0;
	size_t m_Print_SizeLimit = 200;
	size_t m_Print_LineLimit = 240; // There's currently a bug with the CS2 console where it won't show text if there's no line-break after 300 characters.
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
	/*else if(bFirstfilesystem_stdio && StringEndsWithW( lpLibFileName, L"filesystem_stdio.dll"))
	{
		bFirstfilesystem_stdio = false;
		
		g_Import_filesystem_stdio.Apply(hModule);
	}*/
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

		//g_Import_engine2.Apply(hModule);
	}
	/*else if(bFirstMaterialsystem2 && StringEndsWithW( lpLibFileName, L"materialsystem2.dll"))
	{
		bFirstMaterialsystem2 = false;

		g_Import_materialsystem2.Apply(hModule);
	}*/
	else if(bFirstClient && StringEndsWithW(lpLibFileName, L"csgo\\bin\\win64\\client.dll"))
	{
		bFirstClient = false;

		g_H_ClientDll = hModule;

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


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason) 
	{ 
		case DLL_PROCESS_ATTACH:
		{
			g_CommandLine = new advancedfx::CCommandLine();

			if(!g_CommandLine->FindParam(L"-insecure"))
			{
				ErrorBox("Please add -insecure to launch options, AfxHookCS2 will refuse to work without it!");

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

			g_ConsolePrinter = new CConsolePrinter();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			// actually this gets called now.

			delete g_ConsolePrinter;

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
