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
	void BeforeHud();

	/// <remarks>Must be called from engine thread only.</remarks>
	void Shutdown();

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelInitPreEntity(char const* pMapName);

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelShutdown();

	int GetFrameCount();

	bool GetFrameInfoSent();

	//
	// Drawing thread:

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThreadBeforeHud(int frameCount, bool frameInfoSent);

	void OnCreatedSharedSurface(ISharedSurfaceInfo * surface);

	void OnReleaseSharedSurface(ISharedSurfaceInfo * surface);

	/// <param name="info">can be nullptr</param>
	void OnSetSharedRenderTarget(DWORD RenderTargetIndex, ISharedSurfaceInfo * surface);

	/// <param name="info">can be nullptr</param>
	void OnSetSharedDepthStencilSurface(ISharedSurfaceInfo * surface);
}

#endif
