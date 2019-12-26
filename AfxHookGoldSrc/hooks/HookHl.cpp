#include "stdafx.h"

#include "HookHw.h"
#include <windows.h>
#include <shared/AfxDetours.h>
#include "HookHw.h"
#include "../hl_addresses.h"

#ifdef AFX_SCRIPT
#include "../scripting.h"
#endif // AFX_SCRIPT

HMODULE WINAPI new_LoadLibraryA(LPCSTR lpLibFileName);

CAfxImportFuncHook<HMODULE (__stdcall*)(LPCSTR)> g_Import_hl_KERNEL32_LoadLibraryA("LoadLibraryA", new_LoadLibraryA);

HMODULE WINAPI new_LoadLibraryA( LPCSTR lpLibFileName )
{
	static bool bHwLoaded = false;
	static bool firstRun = true;

	if(firstRun)
	{
		firstRun = false;

#ifdef AFX_SCRIPT
		// Start Js engine as early as possible on the main thread
		// (we cannot do this in DllMain, because that
		// attachment happens on a different thread).
		// JS_GC() will crash when run from a different thread.
		ScriptEngine_StartUp();
#endif // AFX_SCRIPT
	}


	if( !bHwLoaded && !lstrcmp( lpLibFileName, "hw.dll") )
	{
		bHwLoaded = true;
		HMODULE hHw = g_Import_hl_KERNEL32_LoadLibraryA.TrueFunc( lpLibFileName );

		if( hHw )
			HookHw(hHw);

		return hHw;
	}

	return g_Import_hl_KERNEL32_LoadLibraryA.TrueFunc( lpLibFileName );
}

CAfxImportDllHook g_Import_hl_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_hl_KERNEL32_LoadLibraryA }));
CAfxImportsHook g_Import_hl(CAfxImportsHooks({
	&g_Import_hl_KERNEL32 }));

void HookHl()
{
	HMODULE hHl = GetModuleHandle(NULL);

	Addresses_InitHlExe((AfxAddr)hHl);

	g_Import_hl.Apply(hHl);
		
	if(!g_Import_hl_KERNEL32_LoadLibraryA.TrueFunc) MessageBox(0,"Base interception failed","MDT_ERROR",MB_OK|MB_ICONHAND);
}
