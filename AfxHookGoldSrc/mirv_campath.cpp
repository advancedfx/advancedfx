#include "stdafx.h"

#include "cmdregister.h"
#include "filming.h"
#include "hooks/DemoPlayer/DemoPlayer.h"
#include "hooks/HookHw.h"
#include <shared/StringTools.h>

REGISTER_CMD_FUNC(campath)
{
	int argc = pEngfuncs->Cmd_Argc();

	if(2 <= argc)
	{
		char * subcmd = pEngfuncs->Cmd_Argv(1);

		if(!_stricmp("add", subcmd) && 2 == argc)
		{
			g_Filming.GetCamPath()->Add(
				g_DemoPlayer->GetDemoTime(),
				CamPathValue(
					g_Filming.LastCameraOrigin[0],
					g_Filming.LastCameraOrigin[1],
					g_Filming.LastCameraOrigin[2],
					g_Filming.LastCameraAngles[PITCH],
					g_Filming.LastCameraAngles[YAW],
					g_Filming.LastCameraAngles[ROLL],
					g_Filming.LastCameraFov
				)
			);

			return;
		}
		else if(!_stricmp("enable", subcmd) && 3 == argc)
		{
			bool enable = 0 != atoi(pEngfuncs->Cmd_Argv(2));
			g_Filming.GetCamPath()->Enabled_set(
				enable
			);

			if(enable && !g_Filming.GetCamPath()->CanEval())
				pEngfuncs->Con_Printf(
					"Warning: Campath enabled but can not be evaluated yet.\n"
					"Did you add enough points?\n"
				);

			return;
		}
		else if(!_stricmp("clear", subcmd) && 2 == argc)
		{
			g_Filming.GetCamPath()->Clear();

			return;
		}
		else if(!_stricmp("print", subcmd) && 2 == argc)
		{
			pEngfuncs->Con_Printf("passed id: time -> (x,y,z) fov (pitch,yaw,roll)\n");
			
			double curtime = g_DemoPlayer->GetDemoTime();

			int i=0;
			for(CamPathIterator it = g_Filming.GetCamPath()->GetBegin(); it != g_Filming.GetCamPath()->GetEnd(); ++it)
			{
				double vieworigin[3];
				double viewangles[3];
				double fov;

				double time = it.GetTime();
				CamPathValue val = it.GetValue();
				QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

				vieworigin[0] = val.X;
				vieworigin[1] = val.Y;
				vieworigin[2] = val.Z;
				viewangles[PITCH] = ang.Pitch;
				viewangles[YAW] = ang.Yaw;
				viewangles[ROLL] =  ang.Roll;
				fov = val.Fov;

				pEngfuncs->Con_Printf(
					"%s %i: %f -> (%f,%f,%f) %f (%f,%f,%f)\n",
					time <= curtime ? "Y" : "n",
					i, time,
					vieworigin[0],vieworigin[1],vieworigin[2],
					fov,
					viewangles[PITCH],viewangles[YAW],viewangles[ROLL]
				);

				i++;
			}
			pEngfuncs->Con_Printf("---- Current time: %f\n", curtime);

			return;
		}
		else if(!_stricmp("remove", subcmd) && 3 == argc)
		{
			int idx = atoi(pEngfuncs->Cmd_Argv(2));
			int i=0;
			for(CamPathIterator it = g_Filming.GetCamPath()->GetBegin(); it != g_Filming.GetCamPath()->GetEnd(); ++it)
			{
				if(i == idx)
				{
					double time = it.GetTime();
					g_Filming.GetCamPath()->Remove(time);
					break;
				}
				i++;
			}

			return;
		}
		else if(!_stricmp("load", subcmd) && 3 == argc)
		{	
			std::wstring wideString;
			bool bOk = AnsiStringToWideString(pEngfuncs->Cmd_Argv(2), wideString)
				&& g_Filming.GetCamPath()->Load(wideString.c_str())
			;

			pEngfuncs->Con_Printf("Loading campath: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if(!_stricmp("save", subcmd) && 3 == argc)
		{	
			std::wstring wideString;
			bool bOk = AnsiStringToWideString(pEngfuncs->Cmd_Argv(2), wideString)
				&& g_Filming.GetCamPath()->Save(wideString.c_str())
			;

			pEngfuncs->Con_Printf("Saving campath: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if(!_stricmp("edit", subcmd) && 3 <= argc)
		{	
			const char * arg2 = pEngfuncs->Cmd_Argv(2);
			
			if(!_stricmp("start", arg2))
			{
				g_Filming.GetCamPath()->SetStart(
					g_DemoPlayer->GetDemoTime()
				);

				return;
			}
			else
			if(!_stricmp("duration", arg2) && 4 <= argc)
			{
				double duration = atof(pEngfuncs->Cmd_Argv(3));

				g_Filming.GetCamPath()->SetDuration(
					duration
				);

				return;
			}
		}
	}

	pEngfuncs->Con_Printf(
		PREFIX "campath add - adds current demotime and view as keyframe\n"
		PREFIX "campath enable 0|1 - set whether the camera splines are active or not. Please note that currently at least 4 Points are required to make it active successfully!\n"
		PREFIX "campath clear - removes all keyframes\n"
		PREFIX "campath print - prints keyframes\n"
		PREFIX "campath remove <id> - removes a keyframe\n"
		PREFIX "campath load <fileName> - loads the campath from the file (XML format)\n"
		PREFIX "campath save <fileName> - saves the campath to the file (XML format)\n"
		PREFIX "campath edit start - set current demotime as new start time for the path you created\n"
		PREFIX "campath edit duration <dValue> - set floating point value <dValue> as new duration for the path you created (in seconds). Please see remarks in HLAE manual.\n"
		"Please note: you might want to use " PREFIX "fx_xtendvis with " PREFIX "campath!\n"
	);
	return;
}
