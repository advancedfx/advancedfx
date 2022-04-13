#pragma once

#include "../ClientTools.h"
#include <tf2/sdk_src/public/toolframework/itoolframework.h>

class CClientToolsTf2 : public CClientTools
{
public:
	static inline CClientToolsTf2 * Instance(void)
	{
		return m_Instance;
	}

	CClientToolsTf2(SOURCESDK::TF2::IClientTools * clientTools);

	virtual ~CClientToolsTf2();

	virtual void OnPostToolMessage(void * hEntity, void * msg);

	virtual void OnBeforeFrameRenderStart(void);

	virtual void OnAfterFrameRenderEnd(void);

	virtual void EnableRecordingMode_set(bool value);

	virtual bool EnableRecordingMode_get();

	virtual int GetAgrVersion() override {
		return 6;
	}

	virtual void StartRecording(wchar_t const * fileName);

	virtual void EndRecording();

protected:
	virtual float ScaleFov(int width, int height, float fov);

	using CClientTools::Write;

private:
	static CClientToolsTf2 * m_Instance;

	SOURCESDK::TF2::IClientTools * m_ClientTools;
	std::map<SOURCESDK::TF2::HTOOLHANDLE, bool> m_TrackedHandles;

	void Write(SOURCESDK::TF2::CBoneList const * value);

	void OnPostToolMessageTf2(SOURCESDK::TF2::HTOOLHANDLE hEntity, SOURCESDK::TF2::KeyValues * msg);
};
