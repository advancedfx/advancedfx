#pragma once

#include "../ClientTools.h"
#include <cssV34/sdk_src/public/toolframework/itoolframework.h>

class CClientToolsCssV34 : public CClientTools
{
public:
	static inline CClientToolsCssV34 * Instance(void)
	{
		return m_Instance;
	}

	CClientToolsCssV34(SOURCESDK::CSSV34::IClientTools * clientTools);

	virtual ~CClientToolsCssV34();

	virtual void OnPostToolMessage(void * hEntity, void * msg);

	virtual void OnBeforeFrameRenderStart(void);

	virtual void OnAfterFrameRenderEnd(void);

	virtual void EnableRecordingMode_set(bool value);

	virtual bool EnableRecordingMode_get();

	virtual void StartRecording(wchar_t const * fileName);

	virtual void EndRecording();

protected:
	using CClientTools::Write;

private:
	static CClientToolsCssV34 * m_Instance;

	SOURCESDK::CSSV34::IClientTools * m_ClientTools;
	std::map<SOURCESDK::CSSV34::HTOOLHANDLE, bool> m_TrackedHandles;

	void Write(SOURCESDK::CSSV34::CBoneList const * value);

	void OnPostToolMessageCssV34(SOURCESDK::CSSV34::HTOOLHANDLE hEntity, SOURCESDK::CSSV34::KeyValues * msg);
};
