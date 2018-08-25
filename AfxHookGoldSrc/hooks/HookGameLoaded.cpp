#include "stdafx.h"

#include "HookGameLoaded.h"

#ifdef AFX_GUI
#include "../gui/Gui.h"
#endif // AFX_GUI

#include "../mirv_commands.h"
#include "../cmdregister.h"
#include "../cmd_tools.h"
#include "../filming.h"
#include "../mirv_glext.h"
#include "../HltvFix.h"

#ifdef AFX_SCRIPT
#include "../scripting.h"
#endif // AFX_SCRIPT

#include "HookHw.h"
#include "hw/ClientFunctions.h"

#include <hlsdk.h>


//
// Commands
//

xcommand_t g_Old_connect = NULL;
void Hook_connect(void)
{
	// Don't print the warning when in TAS mode.
	if (pEngfuncs->pfnGetCvarFloat("mirv_tas_mode") != 0.0f)
		return g_Old_connect();

	int imbret = MessageBoxA(NULL,
		"WARNING: You are about to connect to a server.\n"
		"It is strongly recommended to NOT connect to any server while HLAE is running!\n"
		"\n"
		"You have the following options now:\n"
		"Press Yes\n"
		"\tto continue connecting.\n"
		"Press NO (recommended)\n"
		"\tto abort connecting (HLAE will terminate and lock-up the game).\n"
		"Cancel\n"
		"\tdoes the same as No\n"
		"\n"
		"Do you want to continue connecting?\n"
		,"Game tries to connect",
		MB_YESNOCANCEL|MB_ICONWARNING|MB_DEFBUTTON2
	);
	if(imbret != IDYES)
	{
		// do everything we can to abbort connection:
		do
		{
			try
			{
				HANDLE hproc = OpenProcess( PROCESS_TERMINATE, true, GetCurrentProcessId() );
				TerminateProcess(hproc, 1);
				CloseHandle(hproc);
			} catch (...)
			{
				do MessageBox(NULL,"Please terminate the game manually in the taskmanager!","Cannot terminate, please help:",MB_OK|MB_ICONERROR);
				while (true);
			}
		} while(true);
	}
	else if (imbret == IDYES) g_Old_connect();
}



void Hook_startmovie(void)
{
	if (g_Filming.isFilming())
	{
		pEngfuncs->Con_Printf("Already recording!\n");
		return;
	}

	if(pEngfuncs->Cmd_Argc()<3)
	{
		pEngfuncs->Con_Printf("startmovie <filename> <fps>\n");
		return;
	}

	static char psztemp [513];
	static char pszpath [256];

	const char *paddpath=NULL;
	char *pcmdfname=pEngfuncs->Cmd_Argv(1);
	
	char *pslashpos = strchr(pcmdfname,'/');
	char *pbackslashpos = strchr(pcmdfname,'\\');

	if (!(pslashpos || pbackslashpos))
		paddpath = pEngfuncs->pfnGetGameDirectory();
		//pEngfuncs->CheckParm( "-game", &paddpath );

	psztemp[sizeof(psztemp)-1]=0;
	pszpath[sizeof(pszpath)-1]=0;
	if (paddpath) _snprintf_s(pszpath,_TRUNCATE,"%s/%s",paddpath,pcmdfname);
	else strncpy_s(pszpath,pcmdfname,_TRUNCATE);

	_snprintf_s(psztemp,_TRUNCATE,"mirv_movie_filename \"%s\"",pszpath);

	pEngfuncs->pfnClientCmd(psztemp);

	pEngfuncs->Cvar_SetValue("mirv_movie_fps",(float)atof(pEngfuncs->Cmd_Argv(2)));

	// command won't have finished here, so we do a evil hack:
	// we also can't simply free and alloc here, because Quake 1 / H-L piggy backs infos:
	cvar_t *pIamVictim = pEngfuncs->pfnGetCvarPointer("mirv_movie_filename");
	char *pIamEvil=pIamVictim->string;
	pIamVictim->string = pszpath;

	pEngfuncs->Con_DPrintf("game dir: %s\ncvar is: %s\n",pEngfuncs->pfnGetGameDirectory(),pEngfuncs->pfnGetCvarPointer("mirv_movie_filename")->string);
	g_Filming.Start();
	pIamVictim->string = pIamEvil;
}

void Hook_endmovie(void)
{
	g_Filming.Stop();
}

REGISTER_DEBUGCMD_FUNC(debug_cmdaddress)
{
	if(pEngfuncs->Cmd_Argc()!=2) return;

	char *parg = pEngfuncs->Cmd_Argv(1);

	void * paddr = (void *)g_CmdTools.GiveCommandFn(parg);
	pEngfuncs->Con_Printf("%s: 0x%08x\n",parg,paddr);
}

REGISTER_DEBUGCMD_FUNC(debug_spec)
{
	pEngfuncs->Con_Printf(
		"iuser1: %i\n"
		"iuser2: %i\n",
		ppmove->iuser1,
		ppmove->iuser2
	);

	if(pEngfuncs->Cmd_Argc()!=2) return;
	int idx = atoi(pEngfuncs->Cmd_Argv(1));

	cl_entity_t *e = pEngfuncs->GetEntityByIndex(idx);

	if (e)
		pEngfuncs->Con_Printf(" e");
	else
		return;
	if(e->player)
		pEngfuncs->Con_Printf(" player");
	else
		return;
	if(e->model)
		pEngfuncs->Con_Printf(" model");
	else
		return;
	if(!(e->curstate.effects & EF_NODRAW))
		pEngfuncs->Con_Printf(" NODRAW");
	else
		return;

	float flDeltaTime = fabs(pEngfuncs->GetClientTime() - e->curstate.msg_time);
	pEngfuncs->Con_Printf(" time:%f\n", flDeltaTime);

}


void HookGameLoaded()
{
	SCREENINFO screeninfo;

	// Re-Hook ClientFunctionTable:
	HookClientFunctions();

#ifdef AFX_SCRIPT
	g_Script_CanConsolePrint = true;
#endif // AFX_SCRIPT

	// Now is a good time to install OpenGl extensions:
	Install_All_Gl_Extensions();

#ifdef AFX_GUI
	// Install Gui
	AfxGui_Init();
#endif // AFX_GUI

	// Register commands amd Cvars (may not be used beforehand):
	AfxRegisterCommands();

	pEngfuncs->Con_Printf("Half-Life Advanced Effects (%s) loaded.\n\n", __DATE__);

	if(!g_Has_All_Gl_Extensions)
		pEngfuncs->Con_Printf("MDT WARNING: Could not install all OpenGL extensions. Some features might not work.\n");

	


	screeninfo.iSize = sizeof(SCREENINFO);
	pEngfuncs->pfnGetScreenInfo(&screeninfo);
	pEngfuncs->Con_DPrintf("ScreenRes: %dx%d\n", screeninfo.iWidth, screeninfo.iHeight);

	g_Filming.SupplySupportRenderer(g_pSupportRender);

	g_Filming.setScreenSize(screeninfo.iWidth,screeninfo.iHeight);

	// Init the CmdTools (might be a bit late here for hooking some funcs):
	g_CmdTools.Init(pEngfuncs);
	pEngfuncs->Con_DPrintf("CommandTree at: 0x%08x\n", g_CmdTools.GiveCommandTreePtr());

	// install some command hooks:
	if (!(g_Old_connect = g_CmdTools.HookCommand("connect", Hook_connect))) pEngfuncs->Con_Printf("HLAE warning: Failed hooking connect");
	if (!(OldClientCmdDemForceHltv = g_CmdTools.HookCommand("dem_forcehltv", NewClientCmdDemForceHltv))) pEngfuncs->Con_Printf("HLAE warning: Failed hooking dem_forcehltv");
	if (!(g_CmdTools.HookCommand("startmovie", Hook_startmovie))) pEngfuncs->Con_Printf("HLAE warning: Failed hooking startmovie");
	if (!(g_CmdTools.HookCommand("endmovie", Hook_endmovie))) pEngfuncs->Con_Printf("HLAE warning: Failed hooking endmovie");
}
