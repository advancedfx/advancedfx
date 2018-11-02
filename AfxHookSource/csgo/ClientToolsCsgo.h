#pragma once

#include "../ClientTools.h"

class CClientToolsCsgo : public CClientTools
{
public:
	static inline CClientToolsCsgo * Instance(void)
	{
		return m_Instance;
	}

	CClientToolsCsgo(SOURCESDK::CSGO::IClientTools * clientTools);

	virtual ~CClientToolsCsgo();

	virtual void OnPostToolMessage(void * hEntity, void * msg);

	virtual void OnBeforeFrameRenderStart(void);

	virtual void OnAfterFrameRenderEnd(void);

	virtual bool SuppotsAutoEnableRecordingMode(void) {
		return true;
	}

	virtual bool EnableRecordingMode_get() {
		return true;
	}

	virtual void StartRecording(wchar_t const * fileName);

	virtual void EndRecording();

	void DebugEntIndex(int index);

	SOURCESDK::CSGO::IClientTools * GetClientToolsInterface()
	{
		return m_ClientTools;
	}

protected:
	virtual float ScaleFov(int width, int height, float fov);

	using CClientTools::Write;

private:
	static CClientToolsCsgo * m_Instance;

	SOURCESDK::CSGO::IClientTools * m_ClientTools;
	std::map<SOURCESDK::CSGO::HTOOLHANDLE, bool> m_TrackedHandles;

	void Write(SOURCESDK::CSGO::CBoneList const * value);

	void OnPostToolMessageCsgo(SOURCESDK::CSGO::HTOOLHANDLE hEntity, SOURCESDK::CSGO::KeyValues * msg);

	bool IsViewmodel(SOURCESDK::CSGO::HTOOLHANDLE hEntity);

	void DebugEntity(SOURCESDK::CSGO::HTOOLHANDLE hEntity);
};
