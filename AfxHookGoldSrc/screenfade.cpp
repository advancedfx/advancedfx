// Project :  Half-Life Advanced Effects
// File    :  dllg/screenfade.cpp

// Comment :  Allows to alter or surpress pfnGetScreenFade requests

#include <wrect.h>
#include <cl_dll.h>
#include <cdll_int.h>
#include <cvardef.h>

#include "hl_addresses.h"
#include "detours.h"
#include <screenfade.h>
#include <shake.h>

#include <string.h>


#include "cmdregister.h"


extern cl_enginefuncs_s *pEngfuncs;




typedef void ( *pfnSetScreenFade_t ) ( struct screenfade_s *fade );
pfnSetScreenFade_t g_hooked_pfnSetScreenFade = NULL;

struct New_Fade_Options_s
{
	bool bDisable;
	bool bR, bG, bB, bA;
	unsigned char ucR, ucG, ucB, ucA;
} g_New_Fade_Options;

void Hookig_pfnSetScreenFade ( struct screenfade_s *fade )
{
	if(g_New_Fade_Options.bDisable)
		return;

	screenfade_s myfade = *fade;

	if( fade )
	{

		if( g_New_Fade_Options.bR ) myfade.fader = g_New_Fade_Options.ucR; 
		if( g_New_Fade_Options.bG ) myfade.fadeg = g_New_Fade_Options.ucG; 
		if( g_New_Fade_Options.bB ) myfade.fadeb = g_New_Fade_Options.ucB; 
		if( g_New_Fade_Options.bA ) myfade.fadealpha = g_New_Fade_Options.ucA; 
	}
	g_hooked_pfnSetScreenFade( &myfade );
}

void Hook_pfnSetScreenFade(void)
{
	pEngfuncs->Con_DPrintf("0x%08x\n",(unsigned int)(pEngfuncs->pfnSetScreenFade));
	if( ! g_hooked_pfnSetScreenFade )
	{
		memset(&g_New_Fade_Options,0,sizeof(g_New_Fade_Options));
		g_hooked_pfnSetScreenFade = (pfnSetScreenFade_t) DetourApply((BYTE *)(pEngfuncs->pfnSetScreenFade), (BYTE *)Hookig_pfnSetScreenFade, (int)HL_ADDR_DTOURSZ_cl_enginefuncs_pfnScreenFade);
	}
}

REGISTER_DEBUGCMD_FUNC(fx_screenfade)
{
	bool bShowHelp = true;
	
	Hook_pfnSetScreenFade();

	int iargc = pEngfuncs->Cmd_Argc();

	if ( iargc >= 3)
	{
		char *pcmd = pEngfuncs->Cmd_Argv(1);
		char *pchan = pEngfuncs->Cmd_Argv(2);

		bool bR = 0!=strchr(pchan,'r');
		bool bG = 0!=strchr(pchan,'g');
		bool bB = 0!=strchr(pchan,'b');
		bool bA = 0!=strchr(pchan,'a');
		
		bool bBlock = 0!=strchr(pchan,'1');

		if( 0 == strcmp( pcmd, "block" ))
		{
			g_New_Fade_Options.bDisable = bBlock;
		}
		else if( 0 == strcmp( pcmd, "hook" ) && 4 == iargc)
		{
			unsigned char ucVal = (unsigned char)(atoi(pEngfuncs->Cmd_Argv(3)));

			if(bR) { g_New_Fade_Options.bR = true; g_New_Fade_Options.ucR = ucVal; } 
			if(bG) { g_New_Fade_Options.bG = true; g_New_Fade_Options.ucG = ucVal; } 
			if(bB) { g_New_Fade_Options.bB = true; g_New_Fade_Options.ucB = ucVal; } 
			if(bA) { g_New_Fade_Options.bA = true; g_New_Fade_Options.ucA = ucVal; } 

			bShowHelp = false;
		}
		else if( 0 == strcmp( pcmd, "release" ) )
		{
			if(bR) { g_New_Fade_Options.bR = false; } 
			if(bG) { g_New_Fade_Options.bG = false; } 
			if(bB) { g_New_Fade_Options.bB = false; } 
			if(bA) { g_New_Fade_Options.bA = false; } 

			bShowHelp = false;
		}
	}
	else if ( iargc >= 2)
	{
		char *pcmd = pEngfuncs->Cmd_Argv(1);	
		if( 0 == strcmp( pcmd, "test" ) )
		{
				screenfade_t sf;
				pEngfuncs->pfnGetScreenFade( &sf );

				float ftime = pEngfuncs->GetClientTime();

				sf.fadeSpeed = 40;
				sf.fadeEnd = ftime+3;
				sf.fadeTotalEnd = ftime+6;
				sf.fadeReset = ftime+8;
				sf.fader = 255;
				sf.fadeg = 255;
				sf.fadeb = 255;
				sf.fadealpha = 255;
				sf.fadeFlags = FFADE_IN;

				pEngfuncs->pfnSetScreenFade( &sf );

				bShowHelp = false;
		}
	}

	if( bShowHelp )
	{
		pEngfuncs->Con_Printf(
			"Usage:\n"
			"\t" PREFIX "fx_screenfade block <0|1> <0-255>\n"
			"\t" PREFIX "fx_screenfade hook <r|g|b|a> <0-255>\n"
			"\t" PREFIX "fx_screenfade release <r|g|b|a>\n"
			"\t" PREFIX "fx_screenfade test\n"
		);
	}
}