#pragma once

#include <hlsdk.h>

#include <shared/AfxGameRecord.h>

#include <map>

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

	void SetRenderModel(struct model_s* model);
	void StudioSetHeader(void * header);

private:
	int m_Index = 0;	
	std::map<cl_entity_t*, std::map<void*, int>> m_Indexes;

	struct model_s* m_Model = nullptr;
	void* m_Header = nullptr;
	int m_RenderCount = 0;

	advancedfx::CAfxGameRecord m_AfxGameRecord;

	void WriteVector(float  value[3]);
	void WriteQAngle(float value[3]);
	void WriteQuaternion(float value[4]);
	void WriteMatrix3x4(float matrix[3][4]);

	void RecordModel(cl_entity_t* entity, struct model_s* model, void* v_header);
};

extern class CGameRecord g_GameRecord;