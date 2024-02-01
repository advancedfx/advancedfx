#include "stdafx.h"

#include <shared/StringTools.h>

#include <shared/AfxDetours.h>

#include <deps/release/halflife/external/SDL2/SDL.h>

#include "HookHw.h"
#include "OpenGl32Hooks.h"
#include "gdi32Hooks.h"
#include "user32Hooks.h"

#include "DemoPlayer/DemoPlayer.h"
#include "hw/ClientFunctions.h"
#include "hw/CL_Disconnect.h"
#include "hw/Host_Init.h"
#include "hw/Host_Frame.h"
#include "hw/Mod_LeafPvs.h"
#include "hw/R_DrawEntitiesOnList.h"
#include "hw/R_DrawParticles.h"
#include "hw/R_DrawSkyBoxEx.h"
#include "hw/R_DrawViewModel.h"
#include "hw/R_PolyBlend.h"
#include "hw/R_RenderView.h"
#include "hw/UnkDrawHud.h"
#include "client/HookClient.h"
#include "mirv_input.h"

#include "svdll/Server_GetBlendingInterface.h"
#include "svdll/GiveFnptrsToDll.h"

#include "../hl_addresses.h"
#ifdef AFX_GUI
#include "../gui/Gui.h"
#endif // AFX_GUI

#include <shared/AfxConsole.h>
#include <shared/MirvInput.h>
#include <shared/ConsolePrinter.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

struct cl_enginefuncs_s * pEngfuncs		= (struct cl_enginefuncs_s *)0;
struct engine_studio_api_s * pEngStudio	= (struct engine_studio_api_s *)0;
struct playermove_s * ppmove			= (struct playermove_s *)0;


class CConsolePrint_Message : public IConsolePrint {
public:
	virtual void Print(const char * text) {
		pEngfuncs->Con_Printf(const_cast<char*>(text));
	}
};

CConsolePrinter * g_ConsolePrinter = nullptr;

void Mirv_Msg(const char* fmt, ...) {
	CConsolePrint_Message consolePrint;
	va_list args;
	va_start(args, fmt);
	g_ConsolePrinter->Print(&consolePrint, fmt, args);
	va_end(args);
}

void Mirv_DevMsg(int level, const char* fmt, ...) {
	CConsolePrint_Message consolePrint;
	va_list args;
	va_start(args, fmt);
	if(level <= pEngfuncs->pfnGetCvarFloat("developer")) {
		g_ConsolePrinter->Print(&consolePrint, fmt, args);
	}
	va_end(args);
}

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
PFNWGLCREATECONTEXTATTRIBSARBPROC g_Old_wglCreateContextAttribsARB = nullptr;
HGLRC WINAPI New_wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList) {

	class CCreateContext : public ICreateContext {
	public:
		CCreateContext(HGLRC hShareContext, const int *attribList) : hShareContext(hShareContext), attribList(attribList) {}

		virtual HGLRC CreateContext(HDC hDc) {
			return g_Old_wglCreateContextAttribsARB(hDc, hShareContext, attribList);
		}
	private:
		HGLRC hShareContext;
		const int *attribList;
	} createContext(hShareContext,attribList);

	return OnWglCreateContextEx(&createContext, hDC);
}

PROC WINAPI New_wglGetProcAddress(LPCSTR unnamedParam1) {
	PROC result = wglGetProcAddress(unnamedParam1);
	if(result && 0 == strcmp(unnamedParam1, "wglCreateContextAttribsARB") && nullptr == g_Old_wglCreateContextAttribsARB) {
		g_Old_wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)result;
		result = (PROC)New_wglCreateContextAttribsARB;
	}
	return result;
}

#define GL_MAJOR_VERSION 0x821B

HGLRC WINAPI NewWglCreateContext(HDC hDC) {
	GLint majVers = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majVers);
	if(majVers >= 3) {
		class CCreateContext : public ICreateContext {
		public:
			CCreateContext() {}

			virtual HGLRC CreateContext(HDC hDc) {
				return wglCreateContext(hDc);
			}
		private:
			HGLRC hShareContext;
			const int *attribList;
		} createContext;

		return OnWglCreateContextEx(&createContext, hDC);
	}

	return wglCreateContext(hDC);
}


FARPROC WINAPI NewSdlGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_sdl2_KERNEL32_GetProcAddress("GetProcAddress", NewSdlGetProcAddress);
FARPROC WINAPI NewSdlGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = g_Import_sdl2_KERNEL32_GetProcAddress.TrueFunc(hModule, lpProcName);

	if (HIWORD(lpProcName))
	{
#if 0
		static bool bFirst = true;
		static FILE *f1=NULL;
		static char ttt[100];

		if( !f1 ) f1=fopen("mdt_log_NewSdlGetProcAddress.txt","wb");
		GetModuleFileName(hModule,ttt,99);
		fprintf(f1,"%s %s\n",ttt, lpProcName);
		fflush(f1);
#endif

		if (!lstrcmp(lpProcName, "GetProcAddress"))
			return (FARPROC) &NewSdlGetProcAddress;

		if(!lstrcmp(lpProcName,"wglGetProcAddress"))
			return (FARPROC) &New_wglGetProcAddress;
		if (!lstrcmp(lpProcName, "wglCreateContext"))
			return (FARPROC) &NewWglCreateContext;
		if (!lstrcmp(lpProcName, "wglDeleteContext"))
			return (FARPROC) &NewWglDeleteContext;
		if (!lstrcmp(lpProcName, "wglMakeCurrent"))
			return (FARPROC) &NewWglMakeCurrent;
	}

	return nResult;
}

bool g_b_SDL2_ShowCursor = true;
int New_SDL_ShowCursor(int toggle);
CAfxImportFuncHook<int (*)(int toggle)> g_Import_hw_sdl2_SDL_ShowCursor("SDL_ShowCursor", New_SDL_ShowCursor);
int New_SDL_ShowCursor(int toggle) {
	int result = g_Import_hw_sdl2_SDL_ShowCursor.TrueFunc(toggle);

	if(0 <= result && (0 == toggle || 1 == toggle)) g_b_SDL2_ShowCursor = toggle == 1;

	return result;
}

bool g_b_SDL2_RelativeMouseMode = false;
int New_SDL_SetRelativeMouseMode(int enabled);
CAfxImportFuncHook<int (*)(int enabled)> g_Import_hw_sdl2_SDL_SetRelativeMouseMode("SDL_SetRelativeMouseMode", New_SDL_SetRelativeMouseMode);
int New_SDL_SetRelativeMouseMode(int enabled) {
	int result = g_Import_hw_sdl2_SDL_SetRelativeMouseMode.TrueFunc(enabled);

	if(0 == result) g_b_SDL2_RelativeMouseMode = 0 != enabled;

	return result;
}

void* New_SDL_GL_GetProcAddress(const char* proc);
CAfxImportFuncHook<void* (*)(const char*)> g_Import_hw_sdl2_SDL_GL_GetProcAddress("SDL_GL_GetProcAddress", New_SDL_GL_GetProcAddress);
void *New_SDL_GL_GetProcAddress(const char* proc)
{
	if (!lstrcmp(proc, "glBegin"))
		return (void *) &NewGlBegin;
	if (!lstrcmp(proc, "glEnd"))
		return (void *) &NewGlEnd;
	if (!lstrcmp(proc, "glViewport"))
		return (void *) &NewGlViewport;
	if (!lstrcmp(proc, "glClear"))
		return (void *) &NewGlClear;
	if (!lstrcmp(proc, "glFrustum"))
		return (void *) &NewGlFrustum;
	if (!lstrcmp(proc, "glBlendFunc"))
		return (void *) &NewGlBlendFunc;

	return g_Import_hw_sdl2_SDL_GL_GetProcAddress.TrueFunc(proc);
}

#ifdef AFX_GUI

int New_SDL_WaitEventTimeout(SDL_Event* event, int timeout);
CAfxImportFuncHook<int (*)(SDL_event*, int)> g_hw_sdl2_SDL_WaitEventTimeout("SDL_WaitEventTimeout", New_SDL_WaitEventTimeout);
int New_SDL_WaitEventTimeout(SDL_Event* event,int timeout)
{
	int result;
	bool handled = false;

	while(0 != (result = g_hw_sdl2_SDL_WaitEventTimeout.TrueFunc(event, handled ? 0 : timeout))
		&& (handled = AfxGui_HandleSdlEvent(event)))
	;

	return result;
}

#endif // AFX_GUI

#ifdef AFX_GUI

int New_SDL_PollEvent(SDL_Event* event);
CAfxImportDllHook<int (*)(SDL_Event*)> g_hw_sdl2_SDL_PollEvent("SDL_PollEvent", New_SDL_PollEvent);
int New_SDL_PollEvent(SDL_Event* event)
{
	int result;
	bool handled = false;

	while(0 != (result = g_hw_sdl2_SDL_PollEvent.TrueFunc(event))
		&& (handled = AfxGui_HandleSdlEvent(event)))
	;

	return result;
}

#endif // AFX_GUI


FARPROC WINAPI NewHwGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_hw_KERNEL32_GetProcAddress("GetProcAddress", NewHwGetProcAddress);
FARPROC WINAPI NewHwGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult = g_Import_hw_KERNEL32_GetProcAddress.TrueFunc(hModule, lpProcName);

	if (HIWORD(lpProcName))
	{
#if 0
		static bool bFirst = true;
		static FILE *f1=NULL;
		static char ttt[100];

		if( !f1 ) f1=fopen("mdt_log_NewHwGetProcAddress.txt","wb");
		GetModuleFileName(hModule,ttt,99);
		fprintf(f1,"%s %s\n",ttt, lpProcName);
		fflush(f1);
#endif

		if (!lstrcmp(lpProcName, "GetProcAddress"))
			return (FARPROC) &NewHwGetProcAddress;

		if(!lstrcmp(lpProcName, "Server_GetBlendingInterface"))
			return Hook_ServerGetBlendingInterface(nResult);

		if(!lstrcmp(lpProcName, "GiveFnptrsToDll"))
			return Hook_GiveFnptrsToDll(nResult);
	}

	return nResult;
}

HMODULE WINAPI NewHwLoadLibraryA(LPCSTR lpLibFileName);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_hw_KERNEL32_LoadLibraryA("LoadLibraryA", NewHwLoadLibraryA);
HMODULE WINAPI NewHwLoadLibraryA( LPCSTR lpLibFileName )
{
	static bool bClientLoaded = false;
	static bool bDemoPlayerLoaded = false;

	HMODULE hRet = g_Import_hw_KERNEL32_LoadLibraryA.TrueFunc(lpLibFileName);

	if( !bClientLoaded && StringEndsWith( lpLibFileName, "client.dll") && hRet )
	{
		bClientLoaded = true;

		Addresses_InitClientDll((AfxAddr)hRet, pEngfuncs->pfnGetGameDirectory());

		HookClient((void*)hRet);
	}
	else if( !bDemoPlayerLoaded && StringEndsWith( lpLibFileName, "demoplayer.dll") && hRet )
	{
		bDemoPlayerLoaded = true;

		Hook_DemoPlayer((void *)hRet);
	}

	return hRet;
}

pfnEngSrc_pfnHookEvent_t True_pfnHookEvent;

bool Hook_cstrike_events_usp(void * pAddress);
bool Hook_cstrike_events_m4a1(void * pAddress);

void MypfnHookEvent(char *name, void(*pfnEvent)(struct event_args_s *args))
{
	static const char *gamedir = pEngfuncs->pfnGetGameDirectory();

	if (gamedir && 0 == _stricmp("cstrike", gamedir) && name) {
		if(0 == strcmp("events/usp.sc", name)) Hook_cstrike_events_usp(pfnEvent);
		else if(0 == strcmp("events/m4a1.sc", name)) Hook_cstrike_events_m4a1(pfnEvent);
		else if(0 == strcmp("events/createsmoke.sc", name)) AFXADDR_SET(cstrike_EV_CreateSmoke, (DWORD)pfnEvent);
	}

	True_pfnHookEvent(name, pfnEvent);
}

bool HookpEnginefuncs(cl_enginefunc_t *pEnginefuncs)
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	LONG error = NO_ERROR;

	True_pfnHookEvent = pEngfuncs->pfnHookEvent;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)True_pfnHookEvent, MypfnHookEvent);
	error = DetourTransactionCommit();

	if (NO_ERROR != error)
	{
		firstResult = false;
		ErrorBox("Interception failed:\nHookpEnginefuncs()");
	}

	return firstResult;
}


CAfxImportDllHook g_Import_hw_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
		&g_Import_hw_KERNEL32_LoadLibraryA,
		&g_Import_hw_KERNEL32_GetProcAddress }));

CAfxImportDllHook g_Import_hw_sdl2("SDL2.dll", CAfxImportDllHooks({
	&g_Import_hw_sdl2_SDL_GL_GetProcAddress,
	&g_Import_hw_sdl2_SDL_SetRelativeMouseMode,
	&g_Import_hw_sdl2_SDL_ShowCursor
	}));

CAfxImportsHook g_Import_hw(CAfxImportsHooks({
	&g_Import_hw_KERNEL32,
	&g_Import_hw_sdl2,
	}));

CAfxImportDllHook g_Import_sdl2_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_sdl2_KERNEL32_GetProcAddress }));


UINT WINAPI New_GetRawInputData(
    _In_ HRAWINPUT hRawInput,
    _In_ UINT uiCommand,
    _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,
    _Inout_ PUINT pcbSize,
    _In_ UINT cbSizeHeader);

CAfxImportFuncHook<UINT(WINAPI*)(_In_ HRAWINPUT, _In_ UINT, _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,_Inout_ PUINT,_In_ UINT)> g_Import_SDL2_USER32_GetRawInputData("GetRawInputData", &New_GetRawInputData);

UINT WINAPI New_GetRawInputData(
    _In_ HRAWINPUT hRawInput,
    _In_ UINT uiCommand,
    _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,
    _Inout_ PUINT pcbSize,
    _In_ UINT cbSizeHeader) {

	UINT result = g_Import_SDL2_USER32_GetRawInputData.GetTrueFuncValue()(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);

	result = MirvInput_Get()->Supply_RawInputData(result, hRawInput, uiCommand,pData,pcbSize,cbSizeHeader);

	return result;
}	

CAfxImportDllHook g_Import_sdl2_USER32("USER32.dll", CAfxImportDllHooks({
	Get_Import_USER32_CreateWindowExW(),
	Get_Import_USER32_DestroyWindow(),
	Get_Import_USER32_GetCursorPos(),
	Get_Import_USER32_SetCursorPos(),
	&g_Import_SDL2_USER32_GetRawInputData}));

CAfxImportDllHook g_Import_sdl2_GDI32("GDI32.dll", CAfxImportDllHooks({
	Get_Import_GDI32_SetPixelFormat(),
	Get_Import_GDI32_SwapBuffers() }));


CAfxImportsHook g_Import_sdl2(CAfxImportsHooks({
	&g_Import_sdl2_KERNEL32,
	&g_Import_sdl2_USER32,
	& g_Import_sdl2_GDI32 }));


void HookHw(HMODULE hHw)
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return;
	firstRun = false;

	bool bIcepOk = true;

	Addresses_InitHwDll((AfxAddr)hHw);

	::pEngfuncs = (cl_enginefuncs_s *)AFXADDR_GET(pEngfuncs);
	::ppmove = (struct playermove_s *)AFXADDR_GET(ppmove);
	::pEngStudio = (struct engine_studio_api_s *)AFXADDR_GET(pstudio);

	if (::pEngfuncs)
	{
		advancedfx::Message = Mirv_Msg;
		advancedfx::Warning = Mirv_Msg;
		advancedfx::DevMessage = Mirv_DevMsg;
		advancedfx::DevWarning = Mirv_DevMsg;
}

#ifdef AFX_GUI
	g_Import_hw_sdl2.Append(CAfxImportDllHooks{
		&g_hw_sdl2_SDL_SetRelativeMouseMode,
		&g_hw_sdl2_SDL_WaitEventTimeout
		&g_hw_sdl2_SDL_PollEvent });
#endif // AFX_GUI
	g_Import_hw.Apply(hHw);

	if(!g_Import_hw_KERNEL32_LoadLibraryA.TrueFunc) { bIcepOk = false; MessageBox(0, "Interception failed:\nhw.dll:Kernel32.dll!LoadLibraryA", "MDT_ERROR", MB_OK | MB_ICONHAND); }
	if(!g_Import_hw_KERNEL32_GetProcAddress.TrueFunc) { bIcepOk = false; MessageBox(0, "Interception failed:\nhw.dll:Kernel32.dll!GetProcAddress", "MDT_ERROR", MB_OK | MB_ICONHAND); }
	if(!g_Import_hw_sdl2_SDL_GL_GetProcAddress.TrueFunc) { bIcepOk = false; MessageBox(0, "Interception failed:\nhw.dll:sdl2.dll!SDL_GL_GetProcAddress", "MDT_ERROR", MB_OK | MB_ICONHAND); }
#ifdef AFX_GUI
	if(!g_hw_sdl2_SDL_SetRelativeMouseMode.TrueFunc) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_SetRelativeMouseMode","MDT_ERROR",MB_OK|MB_ICONHAND); }
	if(!g_hw_sdl2_SDL_WaitEventTimeout.TrueFunc) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_WaitEventTimeout","MDT_ERROR",MB_OK|MB_ICONHAND); }
	if(!g_hw_sdl2_SDL_PollEvent.TrueFunc) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_PollEvent","MDT_ERROR",MB_OK|MB_ICONHAND); }
#endif // AFX_GUI
	
	HMODULE hSdl = GetModuleHandle("SDL2.dll");
	if(hSdl)
	{
		g_Import_sdl2.Apply(hSdl);

		if(!g_Import_sdl2_KERNEL32_GetProcAddress.TrueFunc) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:Kernel32.dll!GetProcAddress","MDT_ERROR",MB_OK|MB_ICONHAND); }
		if(!*Get_Import_USER32_CreateWindowExW()->GetTrueFunc()) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:user32.dll!CreateWindowExW","MDT_ERROR",MB_OK|MB_ICONHAND); }
		if(!*Get_Import_USER32_DestroyWindow()->GetTrueFunc()) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:user32.dll!DestroyWindow","MDT_ERROR",MB_OK|MB_ICONHAND); }
		if (!*Get_Import_USER32_GetCursorPos()->GetTrueFunc()) { /* That's ok. bIcepOk = false; MessageBox(0, "Interception failed:\nsdl2.dll:user32.dll!GetCursorPos", "MDT_ERROR", MB_OK | MB_ICONHAND); */ }
		if (!*Get_Import_USER32_SetCursorPos()->GetTrueFunc()) { bIcepOk = false; MessageBox(0, "Interception failed:\nsdl2.dll:user32.dll!Get_Import_USER32_SetCursorPos", "MDT_ERROR", MB_OK | MB_ICONHAND); }
		if(!*Get_Import_GDI32_SwapBuffers()->GetTrueFunc()) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:gdi32.dll!SwapBuffers","MDT_ERROR",MB_OK|MB_ICONHAND); }
		if(!*Get_Import_GDI32_SetPixelFormat()->GetTrueFunc()) { bIcepOk = false; MessageBox(0,"Interception failed: gdi32.dll!SetPixelFormat","MDT_ERROR",MB_OK|MB_ICONHAND); }
	}
	else
	{
		bIcepOk = false;
		MessageBox(0,"Could not get module handle for sdl2.dll.","MDT_ERROR",MB_OK|MB_ICONHAND);
	}

	if( !bIcepOk )
		MessageBox(0,"One or more interceptions failed","MDT_ERROR",MB_OK|MB_ICONHAND);

	Hook_CL_Disconnect();

	Hook_Host_Init();

	Hook_Host_Frame();

	Hook_Mod_LeafPvs();

	Hook_R_DrawEntitiesOnList();

	Hook_R_DrawParticles();

	Hook_R_DrawSkyBoxEx();

	Hook_R_DrawViewModel();

	Hook_R_PolyBlend();

	Hook_R_RenderView();

	Hook_UnkDrawHud();

	HookpEnginefuncs(pEngfuncs);
}
