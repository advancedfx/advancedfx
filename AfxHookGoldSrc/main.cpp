#include "stdafx.h"

//
// Additional libraries:

#pragma comment(lib,"OpenGL32.lib")

#ifdef AFX_SCRIPT
#pragma comment(lib,"mozjs-24.lib")
#endif // AFX_SCRIPT

#ifdef AFX_GUI
#pragma comment(lib,"RocketCore.lib")
#endif // AFX_GUI

/*
// Direct show:
#ifdef _DEBUG
#pragma comment(lib,"strmbasd.lib") // BaseClasses release will not work in debug.
#else
#pragma comment(lib,"strmbase.lib")
#endif
#pragma comment(lib,"winmm.lib")
*/

// Additonal dependencies:
//#pragma comment(linker, "\"/manifestdependency:type='win32' name='Mozilla.SpiderMonkey.JS' version='1.7.0.0' processorArchitecture='x86' publicKeyToken='0000000000000000'\"")

//

#include "hooks/HookHl.h"

#ifdef AFX_SCRIPT
#include "scripting.h"
#endif // AFX_SCRIPT

#ifdef AFX_GUI
#include "gui/Gui.h"
#include "Rocket/Core/ReferenceCountable.h"
#endif

#include "CampathDrawer.h"

#include <shared/ConsolePrinter.h>

extern CConsolePrinter * g_ConsolePrinter;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason) 
	{ 
		case DLL_PROCESS_ATTACH:
		{
#ifdef _DEBUG
			MessageBox(0,"DLL_PROCESS_ATTACH","MDT_DEBUG",MB_OK);
#endif
			g_ConsolePrinter = new CConsolePrinter();

			HookHl();

			g_CampathDrawer.Begin();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
#if 0
			MessageBox(0,"DLL_PROCESS_DETACH","MDT_DEBUG",MB_OK);
#endif

			g_CampathDrawer.End();

#ifdef AFX_SCRIPT
			g_Script_CanConsolePrint = false;
#endif // AFX_SCRIPT

			delete g_ConsolePrinter;

#ifdef AFX_SCRIPT
			ScriptEngine_ShutDown();
#endif // AFX_SCRIPT

#ifdef AFX_GUI
			AfxGui_ShutDown();
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
