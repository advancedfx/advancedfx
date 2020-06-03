#pragma once

#include "WrpConsole.h"

class CMirvTime
{
public:
	~CMirvTime() {
		delete m_HostFrameRate;
	}

	enum TimeMode
	{
		TimeMode_CurTime,
		TimeMode_ResumePaused
	};

	void SetMode(TimeMode value);
	TimeMode GetMode();

	bool IsPaused();

	float GetTime();

	float GetPausedTime();

	void SetPausedTime(float value);

	// Raw time:

	float GetAbsoluteFrameTime();

	int GetFrameCount();

	float GetCurTime();

	float GetFrameTime();

	//
	// State setters:

	void OnFrameRenderStart();

	void OnLevelInitPreEntity();

	float GetDriveTimeFactor() {
		return m_DriveTimeFactor;
	}

	void SetDriveTimeFactor(float value) {
		m_DriveTimeFactor = value;
	}

	bool GetDriveTimeEnabled() { return m_DriveTimeEnabled; }

	void SetDriveTimeEnabled(bool value);

	bool GetDrivingByFps() { return m_DrivingByFps; }

private:
	TimeMode m_TimeMode = TimeMode_CurTime;
	float m_PausedTime = 0;
	bool m_IsPaused = false;
	int m_FrameCount = 0;
	float m_DriveTimeFactor = 1;
	bool m_DriveTimeEnabled = false;
	WrpConVarRef* m_HostFrameRate = nullptr;
	bool m_FirstDriveScale = true;
	bool m_FirstDriveFps = true;
	bool m_FirstDriveMirvSndTimeScale = true;
	bool m_DrivingByFps = false;
	float m_FirstTimescale = 1;
	float m_FirstFps = 60;
	float m_FirstMirvSndTimeScale = 1;
};

extern CMirvTime g_MirvTime;
