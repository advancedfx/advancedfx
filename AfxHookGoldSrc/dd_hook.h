/*
File        : dd_hook.h
Started     : 2007-09-09 11:53:06
Project     : Mirv Demo Tool
Authors     : Dominik Tugend // add yourself here if u change s.th.
Description : This header contains the functions for hooking DirectDraw in order to force a specific resolution
*/

#ifndef DD_HOOK_H
#define DD_HOOK_H

#include <windows.h>

#ifndef MAXLEN_HLRES
	#define MAXLEN_HLRES 5
#endif
#ifndef MAXLEN_HLCOLOR
	#define MAXLEN_HLCOLOR 2
#endif

// Known Functions the IDirectDraw Interface has:
#define IDD_IFACE_FUNCS_CNT 23

FARPROC WINAPI Hook_DirectDrawCreate(FARPROC fpOldAddress);
// Remarks:	This can be used to hook in a GetProcAddress Situation.
//			In the first call fpOldAddress is used to fill the global in order to remember the address (other calls are not examined).
//			This hook autmoatically checks the launchoptions when required in order to decide if there is s.th. to check or not, it checks for -mdtres WIDTHxHEIGHTxBPP
// Returns:	Address of our own function that should be called instead


#endif // #ifndef DD_HOOK_H