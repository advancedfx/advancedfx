#pragma once

#include "../ClientTools.h"
#include <css/sdk_src/public/toolframework/itoolframework.h>

class CClientToolsCss : public CClientTools
{
public:
	static inline CClientToolsCss * Instance(void)
	{
		return m_Instance;
	}

	CClientToolsCss(SOURCESDK::CSS::IClientTools * clientTools);

	virtual ~CClientToolsCss();

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
	static CClientToolsCss * m_Instance;

	SOURCESDK::CSS::IClientTools * m_ClientTools;
	std::map<SOURCESDK::CSS::HTOOLHANDLE, bool> m_TrackedHandles;

	void OnPostToolMessageCss(SOURCESDK::CSS::HTOOLHANDLE hEntity, SOURCESDK::CSS::KeyValues * msg);
};
