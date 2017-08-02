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

	virtual void StartRecording(wchar_t const * fileName);

	virtual void EndRecording();

	void DebugEntIndex(int index);

protected:
	using CClientTools::Write;

private:
	static CClientToolsCsgo * m_Instance;

	SOURCESDK::CSGO::IClientTools * m_ClientTools;
	std::map<SOURCESDK::CSGO::HTOOLHANDLE, bool> m_TrackedHandles;

	void Write(SOURCESDK::CSGO::CBoneList const * value);

	void OnPostToolMessageCsgo(SOURCESDK::CSGO::HTOOLHANDLE hEntity, SOURCESDK::CSGO::KeyValues * msg);
};
