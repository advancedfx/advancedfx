#include "stdafx.h"

#include "camimport.h"

#include <stdio.h>
#include <windows.h>

#include <shared/StringTools.h>

#include <hlsdk.h>

#include "filming.h"

#include "cmdregister.h"

extern cl_enginefuncs_s *pEngfuncs;
extern Filming g_Filming;

CCamImport g_CamImport;

REGISTER_CMD_FUNC(camimport_load)
{
	int ic =  pEngfuncs->Cmd_Argc();

	if((2 == ic)||(3 == ic))
	{
		double fBase;

		if(ic==2)
			fBase = g_Filming.GetDebugClientTime();
		else
			fBase = atof(pEngfuncs->Cmd_Argv(2));

		std::wstring wideString;

		if(AnsiStringToWideString(pEngfuncs->Cmd_Argv(1), wideString)
			&& g_CamImport.LoadMotionFile(wideString.c_str()))
		{
			pEngfuncs->Con_Printf("BVH opened.\n");
			g_CamImport.SetBaseTime(fBase);
		}
		else
			pEngfuncs->Con_Printf("ERROR.\n");
	} else {
		pEngfuncs->Con_Printf("Usage:\n" PREFIX "camimport_load <bvhfile> [<basetime>]\n");
	}
}

REGISTER_CMD_FUNC(camimport_end)
{
	g_CamImport.CloseMotionFile();
}

REGISTER_CMD_FUNC(camimport_basetime)
{
	if(2 == pEngfuncs->Cmd_Argc())
	{
		if(!strcmp(pEngfuncs->Cmd_Argv(1),"current"))
			g_CamImport.SetBaseTime(g_Filming.GetDebugClientTime());
		else
			g_CamImport.SetBaseTime(atof(pEngfuncs->Cmd_Argv(1)));
	} else {
		pEngfuncs->Con_Printf("Usage:\n" PREFIX "camimport_basetime (<basetime>|current)\nCurrent: %f",g_CamImport.GetBaseTime());
	}
}

////////////////////////////////////////////////////////////////////////////////


// CCamImport //////////////////////////////////////////////////////////////////

CCamImport::CCamImport()
{

}

void CCamImport::CloseMotionFile()
{
	m_BvhImport.CloseMotionFile();
}

bool CCamImport::GetCamPosition(double fTimeOfs, double outCamdata[6])
{
	if(m_BvhImport.IsActive())
		return m_BvhImport.GetCamPosition(fTimeOfs -m_BaseTime, outCamdata);

	return false;
}


bool CCamImport::IsActive()
{
	return m_BvhImport.IsActive();
}

bool CCamImport::LoadMotionFile(wchar_t const * fileName)
{
	return m_BvhImport.LoadMotionFile(fileName);
}

double CCamImport::GetBaseTime()
{
	return m_BaseTime;
}

void CCamImport::SetBaseTime(double fBaseTime)
{
	m_BaseTime = fBaseTime;
}

CCamImport::~CCamImport()
{
	m_BvhImport.CloseMotionFile();
}


