#include "MirvTime.h"

#include "RenderView.h"


CMirvTime g_MirvTime;


void CMirvTime::SetMode(TimeMode value)
{
	m_TimeMode = value;
}

CMirvTime::TimeMode CMirvTime::GetMode()
{
	return m_TimeMode;
}

bool CMirvTime::IsPaused()
{
	return m_IsPaused;
}

float CMirvTime::GetTime()
{
	return m_PausedTime + GetCurTime();
}


float CMirvTime::GetPausedTime()
{
	return m_PausedTime;
}

void CMirvTime::SetPausedTime(float value)
{
	m_PausedTime = value;
}


float CMirvTime::GetAbsoluteFrameTime()
{
	if (WrpGlobals * globals = g_Hook_VClient_RenderView.GetGlobals())
	{
		return globals->absoluteframetime_get();
	}

	return 0.0f;
}

int CMirvTime::GetFrameCount()
{
	if (WrpGlobals * globals = g_Hook_VClient_RenderView.GetGlobals())
	{
		return globals->framecount_get();
	}

	return 0;
}

float CMirvTime::GetCurTime()
{
	if (WrpGlobals * globals = g_Hook_VClient_RenderView.GetGlobals())
	{
		return globals->curtime_get();
	}

	return 0.0f;
}

float CMirvTime::GetFrameTime()
{
	if (WrpGlobals * globals = g_Hook_VClient_RenderView.GetGlobals())
	{
		return globals->frametime_get();
	}

	return 0.0f;
}


void CMirvTime::OnFrameRenderStart()
{
	m_IsPaused = 0 == GetFrameTime();

	if (TimeMode_ResumePaused == m_TimeMode && m_IsPaused) m_PausedTime += GetAbsoluteFrameTime();
}

void CMirvTime::OnLevelInitPreEntity()
{
	m_PausedTime = 0;
}