#pragma once

#ifdef AFX_INTEROP

#include "SourceInterfaces.h"
#include <d3d9.h>
#include <d3d9Hooks.h>

namespace AfxInterop {

	void DllProcessAttach();

	bool Enabled();

	//
	// Engine thread:

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeFrameStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeFrameRenderStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeHud(const SOURCESDK::CViewSetup_csgo & view);

	/// <remarks>Must be called from engine thread only.</remarks>
	void Shutdown();

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelInitPreEntity(char const* pMapName);

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelShutdown();

	int GetFrameCount();

	//
	// Drawing thread:

	void DrawingThreadPrepareDraw(int frameCount);

	void DrawingThreadFinished();

	bool GetDoingHud(void);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThreadBeforeHud();

	void OnCreatedSurface(IAfxInteropSurface * surface);

	void OnReleaseSurface(IAfxInteropSurface * surface);

	/// <param name="info">can be nullptr</param>
	void OnSetRenderTarget(DWORD RenderTargetIndex, IAfxInteropSurface * surface);
}

#endif
