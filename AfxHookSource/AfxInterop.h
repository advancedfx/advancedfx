#pragma once

#ifdef AFX_INTEROP

#include "SourceInterfaces.h"
#include <d3d9.h>

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
	void Shutdown();

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelInitPreEntity(char const* pMapName);

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelShutdown();

	//
	// Drawing thread:

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThreadBeforeHud(void);

	bool CreateTexture(const char * textureName, const char * textureGroup, IDirect3DDevice9 * device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, HRESULT & result);

	HRESULT OnSetRenderTarget(IDirect3DDevice9 * device, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

	HRESULT OnSetDepthStencilSurface(IDirect3DDevice9 * device, IDirect3DSurface9* pNewZStencil);

	HRESULT OnSetSexture(IDirect3DDevice9 * device, DWORD Stage, IDirect3DBaseTexture9* pTexture);

	HRESULT OnCreateRenderTarget(IDirect3DDevice9 * device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);

	HRESULT OnCreateDepthStencilSurface(IDirect3DDevice9 * device, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
}

#endif
