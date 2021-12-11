#pragma once

#include "../ClientTools.h"

class CClientToolsCsgo : public CClientTools
{
public:
	static inline CClientToolsCsgo * Instance(void)
	{
		return m_Instance;
	}

	static inline SOURCESDK::C_BasePlayer_csgo* GetLocalPlayer(void) {
		if(m_Instance) {
			if(auto iface = m_Instance->m_ClientTools) {
				return reinterpret_cast<SOURCESDK::C_BasePlayer_csgo *>(iface->GetLocalPlayer());
			}
		}
		return nullptr;
	}	

	CClientToolsCsgo(SOURCESDK::CSGO::IClientTools * clientTools);

	virtual ~CClientToolsCsgo();

	virtual void OnPostToolMessage(void * hEntity, void * msg);

	virtual void OnBeforeFrameRenderStart(void);

	virtual void OnAfterSetupEngineView(void) override;

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

	virtual bool SupportsRecordPlayerCameras() override {
		return true;
	}

	virtual bool SupportsRecordViewModelMultiple() override {
		return true;
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
};
