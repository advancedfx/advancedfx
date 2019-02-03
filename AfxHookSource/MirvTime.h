#pragma once

class CMirvTime
{
public:
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

private:
	TimeMode m_TimeMode = TimeMode_CurTime;
	float m_PausedTime = 0;
	bool m_IsPaused = false;
	int m_FrameCount = 0;

};

extern CMirvTime g_MirvTime;
