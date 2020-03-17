#pragma once

#include <d3d9.h>

#define FOURCC_INTZ ((D3DFORMAT)(MAKEFOURCC('I', 'N', 'T', 'Z')))

typedef IDirect3D9 * (WINAPI * Direct3DCreate9_t)(UINT SDKVersion);
typedef HRESULT (WINAPI * Direct3DCreate9Ex_t)(UINT SDKVersion, IDirect3D9Ex**);

extern Direct3DCreate9_t old_Direct3DCreate9;
extern Direct3DCreate9Ex_t old_Direct3DCreate9Ex;

IDirect3D9 * WINAPI new_Direct3DCreate9(UINT SDKVersion);
HRESULT WINAPI new_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3DDevice);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
bool AfxD3D9_Check_Supports_R32F_With_Blending(void);


class __declspec(novtable) ID3d9HooksModulationColorBlendOverride abstract
{
public:
	/// <remarks>color is in bgr</remarks>
	virtual void D3d9HooksModulationColorBlendOverride(float color[4]) = 0;
};

//
// Override state management:
//
// Initially there is already a current state pushed, which should not be popped.
// Make sure to maintain push and pop correctly.

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9PushOverrideState(bool clean);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9PopOverrideState(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_ModulationColorBlend(ID3d9HooksModulationColorBlendOverride* overrideFn);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_ModulationColorBlend(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_BLENDOP(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_BLENDOP(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_SRCBLEND(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_SRCBLEND(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_DESTBLEND(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_DESTBLEND(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_SRGBWRITEENABLE(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_SRGBWRITEENABLE(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_COLORWRITEENABLE(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_COLORWRITEENABLE(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_ZWRITEENABLE(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_ZWRITEENABLE(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideBegin_D3DRS_ALPHABLENDENABLE(DWORD value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9OverrideEnd_D3DRS_ALPHABLENDENABLE(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_ps_c0(float const values[4]);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_ps_c0(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_ps_c5(float const values[4]);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_ps_c5(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_ps_c12_y(float value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_ps_c12_y(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_ps_c29_w(float value);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_ps_c29_w(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_ps_c31(float const values[4]);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_ps_c31(void);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_SetVertexShader(IDirect3DVertexShader9 * overrideShader);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_SetVertexShader();

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideBegin_SetPixelShader(IDirect3DPixelShader9 * overrideShader);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_OverrideEnd_SetPixelShader();

//
// //

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_Block_Present(bool block);

/// <remarks>IDirect3D9Device only (i.e. CS:GO but not CSS).</remarks>
void AfxD3D9_Block_Clear(bool block);

extern bool g_bD3D9DumpVertexShader;
extern bool g_bD3D9DumpPixelShader;

bool AfxD3d9_DrawDepthSupported(void);

enum AfxDrawDepthMode
{
	AfxDrawDepthMode_Inverse,
	AfxDrawDepthMode_Linear,
	AfxDrawDepthMode_LogE,
	AfxDrawDepthMode_PyramidalLinear,
	AfxDrawDepthMode_PyramidalLogE
};

enum AfxDrawDepthEncode
{
	AfxDrawDepthEncode_Gray,
	AfxDrawDepthEncode_Rgb,
	AfxDrawDepthEncode_Rgba
};

void AfxIntzOverrideBegin();
void AfxIntzOverrideEnd();

/// <param name="projectionMatrix">Can currently be null if AfxDrawDepthMode_Inverse, otherwise has to be projection matrix from engine.</param>
void AfxDrawDepth(AfxDrawDepthEncode encode, AfxDrawDepthMode mode, bool clip, float depthVal, float depthValMax, int x, int y, int width, int height, float zNear, float zFar, bool drawToScreen, float projectionMatrix[4][4]);

void AfxDrawGuides(int x, int y, int width, int height, bool phiGrid, bool ruleOfThirds);

void AfxDrawRect(IDirect3DTexture9* texture, int x, int y, int width, int height, float x0, float y0, float x1, float y1);

//

#ifdef AFX_INTEROP

void AfxD3D_WaitForGPU();

class __declspec(novtable) IAfxInteropSurface abstract
{
public:
	virtual IDirect3DSurface9 * AfxGetSurface() = 0;

	virtual void AfxReplacementEnabled_set(bool value) = 0;

	virtual bool AfdxReplacementEnabled_get() = 0;

	virtual void AfxSetReplacement(IDirect3DSurface9 * surface) = 0;

	virtual IDirect3DSurface9 * AfxGetReplacement() = 0;

	virtual void AfxSetDepthSurface(IDirect3DSurface9 * surface) = 0;

	virtual IDirect3DSurface9 * AfxGetDepthSurface() = 0;

	virtual IDirect3DSurface9 * AfxGetCurrentSurface() = 0;
};

#endif

IDirect3DDevice9* AfxGetDirect3DDevice9();

IDirect3DDevice9Ex* AfxGetDirect3DDevice9Ex();

IDirect3DSurface9* AfxGetRenderTargetSurface();

