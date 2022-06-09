#pragma once

class CReShadeAdvancedfx {
public:
	bool IsConnected();

	void Connect();

	void AdvancedfxRenderEffects(void* pRenderTargetView, void* pDepthTextureResource);

	bool HasRendered();

	void ResetHasRendered();

private:
	typedef bool (*AdvancedfxRenderEffects_t)(void* pRenderTargetView, void* pDepthTextureResource);
	static AdvancedfxRenderEffects_t m_AdvancedfxRenderEffects;
	bool m_Rendered = false;
};

extern CReShadeAdvancedfx g_ReShadeAdvancedfx;
