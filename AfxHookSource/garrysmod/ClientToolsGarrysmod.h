#pragma once

#include "../ClientTools.h"
#include <garrysmod/sdk_src/public/toolframework/itoolframework.h>

class CClientToolsGarrysmod : public CClientTools
{
public:
	static inline CClientToolsGarrysmod * Instance(void)
	{
		return m_Instance;
	}

	CClientToolsGarrysmod(SOURCESDK::GARRYSMOD::IClientTools * clientTools);

	virtual ~CClientToolsGarrysmod();

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
	using CClientTools::Write;

private:
	static CClientToolsGarrysmod * m_Instance;

	SOURCESDK::GARRYSMOD::IClientTools * m_ClientTools;
	std::map<SOURCESDK::GARRYSMOD::HTOOLHANDLE, bool> m_TrackedHandles;

	void OnPostToolMessageGarrysmod(SOURCESDK::GARRYSMOD::HTOOLHANDLE hEntity, SOURCESDK::GARRYSMOD::KeyValues * msg);
};
