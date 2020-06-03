#include "MirvTime.h"

#include "RenderView.h"
#include "AfxStreams.h"
#include "csgo_SndMixTimeScalePatch.h"
#include "WrpVEngineClient.h"


extern WrpVEngineClient* g_VEngineClient;


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
	m_DrivingByFps = false;

	if (TimeMode_ResumePaused == m_TimeMode && m_IsPaused) m_PausedTime += GetAbsoluteFrameTime();

	//

	if (m_DriveTimeEnabled)
	{
		if (nullptr == m_HostFrameRate) m_HostFrameRate = new WrpConVarRef("host_framerate");

		float hostFrameRate = m_HostFrameRate->GetFloat();
		
		if(hostFrameRate)
		{
			if (m_FirstDriveFps)
			{
				m_FirstDriveFps = false;
				m_FirstFps = m_HostFrameRate->GetFloat();
			}

			float targetVal = m_FirstFps / m_DriveTimeFactor;
			m_HostFrameRate->SetValue(targetVal);

			m_DrivingByFps = true;

			if (csgo_SndMixTimeScalePatch_enable)
			{
				if (m_FirstDriveMirvSndTimeScale)
				{
					m_FirstDriveMirvSndTimeScale = false;
					m_FirstMirvSndTimeScale = csgo_SndMixTimeScalePatch_value;
				}

				csgo_SndMixTimeScalePatch_value = m_FirstMirvSndTimeScale * m_DriveTimeFactor;
			}
		}
		else
		{
			float demoTimeScale = 1;
			WrpVEngineClientDemoInfoEx* di = g_VEngineClient ? g_VEngineClient->GetDemoInfoEx() : nullptr;
			if (di) demoTimeScale = di->GetDemoPlaybackTimeScale();

			if (m_FirstDriveScale)
			{
				m_FirstDriveScale = false;
				m_FirstTimescale = demoTimeScale;
			}

			float targetVal = m_FirstTimescale * m_DriveTimeFactor;

			if (demoTimeScale != targetVal && g_VEngineClient)
			{
				char cmd[100];
				sprintf_s(cmd, _TRUNCATE, "demo_timescale %f", targetVal);

				g_VEngineClient->ExecuteClientCmd(cmd);
			}

		}
	}
}

void CMirvTime::OnLevelInitPreEntity()
{
	m_PausedTime = 0;
}

void CMirvTime::SetDriveTimeEnabled(bool value)
{
	if (value != m_DriveTimeEnabled)
	{
		if (m_DriveTimeEnabled)
		{
			if (nullptr == m_HostFrameRate) m_HostFrameRate = new WrpConVarRef("host_framerate");

			float hostFrameRate = m_HostFrameRate->GetFloat();

			if (hostFrameRate)
			{
				if (!m_FirstDriveFps)
				{
					m_HostFrameRate->SetValue(m_FirstFps);

					if (csgo_SndMixTimeScalePatch_enable)
					{
						if (!m_FirstDriveMirvSndTimeScale) csgo_SndMixTimeScalePatch_value = 1;
					}
				}
			}
			else if(m_FirstDriveScale)
			{
				if (g_VEngineClient)
				{
					char cmd[100];
					sprintf_s(cmd, _TRUNCATE, "demo_timescale %f", m_FirstTimescale);

					g_VEngineClient->ClientCmd_Unrestricted(cmd);
				}
			}
		}
		m_DriveTimeEnabled = value;
		if (!value)
		{
			m_FirstDriveScale = true;
			m_FirstDriveFps = true;
			m_FirstDriveMirvSndTimeScale = true;
			m_DrivingByFps = false;
		}
	}
}
