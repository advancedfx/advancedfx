#include "stdafx.h"

enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;

typedef void(__stdcall *GiveFnptrsToDll_t)( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals );

GiveFnptrsToDll_t Old_GiveFnptrsToDll;

void __stdcall New_GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;

	Old_GiveFnptrsToDll(pengfuncsFromEngine, pGlobals);
}

FARPROC Hook_GiveFnptrsToDll(FARPROC oldProc)
{
	Old_GiveFnptrsToDll = (GiveFnptrsToDll_t)oldProc;

	return (FARPROC)&New_GiveFnptrsToDll;
}
