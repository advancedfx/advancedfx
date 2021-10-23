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
		bool AfterRenderView = false;

		bool GetEnabled();

		bool GetDepthRequired();

		void Clear()
		{
			BeforeTranslucentShadow = false;
			AfterTranslucentShadow = false;
			BeforeTranslucent = false;
			AfterTranslucent = false;
			BeforeHud = false;
			AfterHud = false;
			AfterRenderView = false;
		}

		void Or(const EnabledFeatures_s& other)
		{
			BeforeTranslucentShadow = other.BeforeTranslucentShadow;
			AfterTranslucentShadow = other.AfterTranslucentShadow;
			BeforeTranslucent = other.BeforeTranslucent;
			AfterTranslucent = other.AfterTranslucent;
			BeforeHud = other.BeforeHud;
			AfterHud = other.AfterHud;
			AfterRenderView = other.AfterRenderView;
		}

	} EnabledFeatures_t;

	void DllProcessAttach();

	bool Enabled();

	bool MainEnabled();

	bool Active();

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
	void On_DrawTranslucentRenderables(IAfxMatRenderContext * ctx, SOURCESDK::CSGO::CRendering3dView * rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall);

	void OnBeforeHud(IAfxMatRenderContext* ctx);

	void OnAfterHud(IAfxMatRenderContext* ctx);

	//
	// Drawing thread:

	/// <remarks>Must be called from drawing thread only.</remarks>
	void OnCreatedSurface(IAfxInteropSurface * surface);

	/// <remarks>Must be called from drawing thread only.</remarks>
	void OnReleaseSurface(IAfxInteropSurface * surface);

	/// <param name="info">can be nullptr</param>
	/// <remarks>Must be called from drawing thread only.</remarks>
	void OnSetRenderTarget(DWORD RenderTargetIndex, IAfxInteropSurface * surface);

	void DrawingThread_DeviceLost();

	void DrawingThread_DeviceRestored();

	//
	// Different Threads:

	void On_Materialysystem_FlushQueue();
}

#endif
