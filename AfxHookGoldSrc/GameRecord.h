#pragma once

#include <hlsdk.h>

#include <shared/AfxGameRecord.h>

class CGameRecord
{
public:
	bool RecordCamera = true;

	bool HooksValid();

	bool GetRecording();

	bool StartRecording(const char* fileName);

	void EndRecording();

	void OnFrameBegin();

	void OnFrameEnd(float view_origin[3], float view_angles[3], float fov);

	void RecordCurrentEntity();

	void WriteVector(float  value[3]);
	void WriteQAngle(float value[3]);
	void WriteQuaternion(float value[4]);

private:
	advancedfx::CAfxGameRecord m_AfxGameRecord;
};

extern class CGameRecord g_GameRecord;