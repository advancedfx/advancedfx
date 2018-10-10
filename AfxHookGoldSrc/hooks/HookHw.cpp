#include "stdafx.h"

#include <shared/StringTools.h>

#include <shared/detours.h>

#include <shared/halflife/external/SDL2/SDL.h>

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

#include "svdll/Server_GetBlendingInterface.h"
#include "svdll/GiveFnptrsToDll.h"

#include "../hl_addresses.h"
#ifdef AFX_GUI
#include "../gui/Gui.h"
#endif // AFX_GUI

#include <Windows.h>
#include <shared/Detours/src/detours.h>

struct cl_enginefuncs_s * pEngfuncs		= (struct cl_enginefuncs_s *)0;
struct engine_studio_api_s * pEngStudio	= (struct engine_studio_api_s *)0;
struct playermove_s * ppmove			= (struct playermove_s *)0;

FARPROC WINAPI NewSdlGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = GetProcAddress(hModule, lpProcName);

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

		if (!lstrcmp(lpProcName, "wglCreateContext"))
			return (FARPROC) &NewWglCreateContext;
		if (!lstrcmp(lpProcName, "wglDeleteContext"))
			return (FARPROC) &NewWglDeleteContext;
		if (!lstrcmp(lpProcName, "wglMakeCurrent"))
			return (FARPROC) &NewWglMakeCurrent;
	}

	return nResult;
}

typedef void * (*SDL_GL_GetProcAddress_t) (const char* proc);
SDL_GL_GetProcAddress_t g_Old_SDL_GL_GetProcAddress;

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

	return g_Old_SDL_GL_GetProcAddress(proc);
}

#ifdef AFX_GUI

typedef int (*SDL_SetRelativeMouseMode_t) (SDL_bool enabled);
SDL_SetRelativeMouseMode_t g_Old_SDL_SetRelativeMouseMode;

int New_SDL_SetRelativeMouseMode(SDL_bool enabled)
{
	return g_Old_SDL_SetRelativeMouseMode(enabled);
}

#endif // AFX_GUI

#ifdef AFX_GUI

typedef int (*SDL_WaitEventTimeout_t)(SDL_Event* event,int timeout);
SDL_WaitEventTimeout_t g_Old_SDL_WaitEventTimeout;

int New_SDL_WaitEventTimeout(SDL_Event* event,int timeout)
{
	int result;
	bool handled = false;

	while(0 != (result = g_Old_SDL_WaitEventTimeout(event, handled ? 0 : timeout))
		&& (handled = AfxGui_HandleSdlEvent(event)))
	;

	return result;
}

#endif // AFX_GUI

#ifdef AFX_GUI

typedef int (*SDL_PollEvent_t)(SDL_Event* event);
SDL_PollEvent_t g_Old_SDL_PollEvent;

int New_SDL_PollEvent(SDL_Event* event)
{
	int result;
	bool handled = false;

	while(0 != (result = g_Old_SDL_PollEvent(event))
		&& (handled = AfxGui_HandleSdlEvent(event)))
	;

	return result;
}

#endif // AFX_GUI

FARPROC WINAPI NewHwGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = GetProcAddress(hModule, lpProcName);

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

HMODULE WINAPI NewHwLoadLibraryA( LPCSTR lpLibFileName )
{
	static bool bClientLoaded = false;
	static bool bDemoPlayerLoaded = false;

	HMODULE hRet = LoadLibraryA( lpLibFileName );


	if( !bClientLoaded && StringEndsWith( lpLibFileName, "client.dll") && hRet )
	{
		bClientLoaded = true;

		Addresses_InitClientDll((AfxAddr)hRet, pEngfuncs->pfnGetGameDirectory());

		HookClient();
	}
	else if( !bDemoPlayerLoaded && StringEndsWith( lpLibFileName, "demoplayer.dll") && hRet )
	{
		bDemoPlayerLoaded = true;

		Hook_DemoPlayer((void *)hRet);
	}

	return hRet;
}

pfnEngSrc_pfnHookEvent_t True_pfnHookEvent;

void MypfnHookEvent(char *name, void(*pfnEvent)(struct event_args_s *args))
{
	static const char *gamedir = pEngfuncs->pfnGetGameDirectory();


	if (gamedir && 0 == _stricmp("cstrike", gamedir) && name && 0 == strcmp("events/createsmoke.sc", name))
	{
		AFXADDR_SET(cstrike_EV_CreateSmoke, (DWORD)pfnEvent);
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

	// Kernel32.dll:
	if(!InterceptDllCall(hHw, "Kernel32.dll", "LoadLibraryA", (DWORD) &NewHwLoadLibraryA)) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:Kernel32.dll!LoadLibraryA","MDT_ERROR",MB_OK|MB_ICONHAND); }
	if(!InterceptDllCall(hHw, "Kernel32.dll", "GetProcAddress", (DWORD) &NewHwGetProcAddress) ) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:Kernel32.dll!GetProcAddress","MDT_ERROR",MB_OK|MB_ICONHAND); }

	// sdl2.dll:
	if(!(g_Old_SDL_GL_GetProcAddress=(SDL_GL_GetProcAddress_t)InterceptDllCall(hHw, "sdl2.dll", "SDL_GL_GetProcAddress", (DWORD) &New_SDL_GL_GetProcAddress) )) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_GL_GetProcAddress","MDT_ERROR",MB_OK|MB_ICONHAND); }
#ifdef AFX_GUI
	if(!(g_Old_SDL_SetRelativeMouseMode=(SDL_SetRelativeMouseMode_t)InterceptDllCall(hHw, "sdl2.dll", "SDL_SetRelativeMouseMode", (DWORD) &New_SDL_SetRelativeMouseMode) )) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_SetRelativeMouseMode","MDT_ERROR",MB_OK|MB_ICONHAND); }
	if(!(g_Old_SDL_WaitEventTimeout=(SDL_WaitEventTimeout_t)InterceptDllCall(hHw, "sdl2.dll", "SDL_WaitEventTimeout", (DWORD) &New_SDL_WaitEventTimeout) )) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_WaitEventTimeout","MDT_ERROR",MB_OK|MB_ICONHAND); }
	if(!(g_Old_SDL_PollEvent=(SDL_PollEvent_t)InterceptDllCall(hHw, "sdl2.dll", "SDL_PollEvent", (DWORD) &New_SDL_PollEvent) )) { bIcepOk = false; MessageBox(0,"Interception failed:\nhw.dll:sdl2.dll!SDL_PollEvent","MDT_ERROR",MB_OK|MB_ICONHAND); }
#endif // AFX_GUI
	
	HMODULE hSdl = GetModuleHandle("sdl2.dll");
	if(hSdl)
	{
		if(!InterceptDllCall(hSdl, "Kernel32.dll", "GetProcAddress", (DWORD) &NewSdlGetProcAddress) ) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:Kernel32.dll!GetProcAddress","MDT_ERROR",MB_OK|MB_ICONHAND); }

		// user32.dll:
		if(!InterceptDllCall(hSdl, "user32.dll", "CreateWindowExW", (DWORD) &NewCreateWindowExW) ) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:user32.dll!CreateWindowExW","MDT_ERROR",MB_OK|MB_ICONHAND); }
		if(!InterceptDllCall(hSdl, "user32.dll", "DestroyWindow", (DWORD) &NewDestroyWindow) ) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:user32.dll!DestroyWindow","MDT_ERROR",MB_OK|MB_ICONHAND); }

		// gdi32.dll:
		if(!(OldWglSwapBuffers=(wglSwapBuffers_t)InterceptDllCall(hSdl, "gdi32.dll", "SwapBuffers", (DWORD) &NewWglSwapBuffers) )) { bIcepOk = false; MessageBox(0,"Interception failed:\nsdl2.dll:gdi32.dll!SwapBuffers","MDT_ERROR",MB_OK|MB_ICONHAND); }
		if(!InterceptDllCall(hSdl, "gdi32.dll", "SetPixelFormat", (DWORD) &NewSetPixelFormat) ) { bIcepOk = false; MessageBox(0,"Interception failed: gdi32.dll!SetPixelFormat","MDT_ERROR",MB_OK|MB_ICONHAND); }
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
