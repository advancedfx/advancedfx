#pragma once

#ifdef AFX_INTEROP

#include "SourceInterfaces.h"
#include "MatRenderContextHook.h"
#include <d3d9.h>
#include <d3d9Hooks.h>

namespace AfxInterop {

	typedef struct EnabledFeatures_s {
		bool BeforeTranslucentShadow = false;
		bool AfterTranslucentShadow = false;
		bool BeforeTranslucent = false;
		bool AfterTranslucent = false;
		bool BeforeHud = false;
		bool AfterHud = false;

		bool GetEnabled() {
			return BeforeTranslucentShadow
				|| AfterTranslucentShadow
				|| BeforeTranslucent
				|| AfterTranslucent
				|| BeforeHud
				|| AfterHud;
		}

		bool GetDepthRequired() {
			return BeforeTranslucentShadow
				|| AfterTranslucentShadow
				|| BeforeTranslucent
				|| AfterTranslucent
				|| BeforeHud;
		}

		void Clear()
		{
			BeforeTranslucentShadow = false;
			AfterTranslucentShadow = false;
			BeforeTranslucent = false;
			AfterTranslucent = false;
			BeforeHud = false;
			AfterHud = false;
		}

	} EnabledFeatures_t;

	void DllProcessAttach();

	bool Enabled();

	//
	// Engine thread:

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeFrameStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeFrameRenderStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void AfterFrameRenderStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void OnRenderView(const SOURCESDK::CViewSetup_csgo & view, EnabledFeatures_t & outEnabled);

	/// <remarks>Must be called from engine thread only.</remarks>
	void OnRenderViewEnd();

	/// <remarks>Must be called from engine thread only.</remarks>
	void Shutdown();

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelInitPreEntity(char const* pMapName);

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelShutdown();

	/// <remarks>Must be called from engine thread only.</remarks>
	int GetFrameCount();

	/// <param name="Rx">Pitch</param>
	/// <param name="Ry">Yaw</param>
	/// <param name="Rz">Roll</param>
	/// <returns>true if values are overriden.</returns>
	/// <remarks>Must be called from engine thread only.</remarks>
	bool OnViewOverride(float & Tx, float & Ty, float & Tz, float & Rx, float & Ry, float & Rz, float & Fov);

	/// <remarks>Must be called from engine thread only.</remarks>
	void On_DrawTranslucentRenderables(SOURCESDK::CSGO::CRendering3dView * rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall);

	//
	// Drawing thread:

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThreadPrepareDraw(int frameCount);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThread_On_DrawTranslucentRenderables(bool bInSkybox, bool bShadowDepth, bool afterCall);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThread_BeforeHud(IAfxMatRenderContextOrg * context);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThread_AfterHud(IAfxMatRenderContextOrg * context);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void OnCreatedSurface(IAfxInteropSurface * surface);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void OnReleaseSurface(IAfxInteropSurface * surface);

	/// <param name="info">can be nullptr</param>
	/// <remarks>Must be called from drawing thread only.</remarks>
	void OnSetRenderTarget(DWORD RenderTargetIndex, IAfxInteropSurface * surface);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThread_OnRenderViewEnd();
}

#endif
