#pragma once

#include <hlsdk.h>

#include <shared/AfxGameRecord.h>

#include <list>

class CGameRecord
{
public:
	bool RecordCamera = true;

	bool HooksValid();

	bool GetRecording();

	bool StartRecording(const char* fileName);

	void EndRecording();

	void OnFrameBegin();

	void BeforeHostFrame();

	void OnFrameEnd(float view_origin[3], float view_angles[3], float fov);

	void RecordCurrentEntity();

private:
	std::list<int> m_Indexes;

	advancedfx::CAfxGameRecord m_AfxGameRecord;

	void WriteVector(float  value[3]);
	void WriteQAngle(float value[3]);
	void WriteQuaternion(float value[4]);
};

extern class CGameRecord g_GameRecord;