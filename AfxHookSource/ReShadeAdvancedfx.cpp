#include "stdafx.h"

#include "ReShadeAdvancedfx.h"

#include <Windows.h>

CReShadeAdvancedfx g_ReShadeAdvancedfx;

CReShadeAdvancedfx::AdvancedfxRenderEffects_t CReShadeAdvancedfx::m_AdvancedfxRenderEffects = nullptr;

bool CReShadeAdvancedfx::IsConnected() {
	return nullptr != m_AdvancedfxRenderEffects;
}

void CReShadeAdvancedfx::Connect() {
	if (IsConnected()) return;

	HMODULE hModule = GetModuleHandleW(L"ReShade_advancedfx.addon");
	if (hModule != nullptr) {
		m_AdvancedfxRenderEffects = (AdvancedfxRenderEffects_t)GetProcAddress(hModule, "AdvancedfxRenderEffects");
	}
}

void CReShadeAdvancedfx::AdvancedfxRenderEffects(void* pRenderTargetView, void* pDepthTextureResource) {
	if (nullptr == m_AdvancedfxRenderEffects) return;
	m_AdvancedfxRenderEffects(pRenderTargetView, pDepthTextureResource);
	m_Rendered = true;
}

bool CReShadeAdvancedfx::HasRendered() {
	return m_Rendered;
}

void CReShadeAdvancedfx::ResetHasRendered() {
	m_Rendered = false;
}
