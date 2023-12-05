#include "stdafx.h"

#include "RenderSystemDX11Hooks.h"

#include "CampathDrawer.h"

#include "../shared/AfxDetours.h"

#include <d3d11.h>

#include <map>
#include <list>
#include <shared_mutex>
#include <atomic>

#include <dxgi.h>
#include <dxgi1_4.h>

IDXGISwapChain * g_pSwapChain = nullptr;
ID3D11Device * g_pDevice = nullptr;
ID3D11DeviceContext * g_pImmediateContext = nullptr;
ID3D11DeviceContext * g_pDeferredContext = nullptr;
ID3D11RenderTargetView * g_pRTView = nullptr;
ID3D11Resource * g_pRTResource = nullptr;
D3D11_TEXTURE2D_DESC g_RTDesc;
size_t g_ClearCount = 0;
size_t g_RTCount = 0;
int g_iDraw = 0;

extern void ErrorBox(char const * messageText);

typedef HRESULT (STDMETHODCALLTYPE * CreateRenderTargetView_t)( void * This,
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView);

CreateRenderTargetView_t g_Old_CreateRenderTargetView = nullptr;

HRESULT STDMETHODCALLTYPE New_CreateRenderTargetView(  void * This,
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView) {
    
    HRESULT result = g_Old_CreateRenderTargetView(This, pResource, pDesc, ppRTView);

    if(SUCCEEDED(result) && ppRTView && *ppRTView) {
        ID3D11Texture2D * pTexture = nullptr;
        HRESULT result2 = g_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D), (void**)&pTexture);
        if(SUCCEEDED(result2)) {
            if(pResource == pTexture) {
                pTexture->GetDesc(&g_RTDesc);
                g_pRTResource = pResource;
                g_pRTView = *ppRTView;
                g_ClearCount = 0;
                g_iDraw = 0;
            }
            pTexture->Release();
        }
    }

    return result;
}

typedef void (STDMETHODCALLTYPE * ClearDepthStencilView_t)( ID3D11DeviceContext * This, 
    /* [annotation] */ 
    _In_  ID3D11DepthStencilView *pDepthStencilView,
    /* [annotation] */ 
    _In_  UINT ClearFlags,
    /* [annotation] */ 
    _In_  FLOAT Depth,
    /* [annotation] */ 
    _In_  UINT8 Stencil);

ClearDepthStencilView_t g_Old_ClearDepthStencilView = nullptr;

ID3D11Resource * g_pCurrentRenderTargetViewResource = nullptr;
ID3D11RenderTargetView * g_pCurrentRenderTargetView = nullptr;
ID3D11DepthStencilView * g_pCurrentDepthStencilView = nullptr;
D3D11_VIEWPORT g_ViewPort;

void STDMETHODCALLTYPE New_ClearDepthStencilView( ID3D11DeviceContext * This, 
    /* [annotation] */ 
    _In_  ID3D11DepthStencilView *pDepthStencilView,
    /* [annotation] */ 
    _In_  UINT ClearFlags,
    /* [annotation] */ 
    _In_  FLOAT Depth,
    /* [annotation] */ 
    _In_  UINT8 Stencil) {

    if(This == g_pImmediateContext && g_iDraw == 2 && (
        pDepthStencilView == g_pCurrentDepthStencilView && (ClearFlags & D3D11_CLEAR_DEPTH)
        || pDepthStencilView != g_pCurrentDepthStencilView
    )) {
        g_iDraw = 3;
        g_CampathDrawer.OnRenderThread_Draw(g_pImmediateContext, &g_ViewPort, g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);
        g_CampathDrawer.OnRenderThread_Present();
    }

    g_Old_ClearDepthStencilView(This, pDepthStencilView, ClearFlags, Depth, Stencil);

    if(This == g_pImmediateContext && pDepthStencilView && (ClearFlags & D3D11_CLEAR_DEPTH)) {

        D3D11_DEPTH_STENCIL_VIEW_DESC desc;
        pDepthStencilView->GetDesc(&desc);

        if(desc.Flags == 0 && desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT
            && (desc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2D
                || desc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2DMS
            ))
        {
            ID3D11Resource * pResource = nullptr;
            pDepthStencilView->GetResource(&pResource);
            if(pResource) {
                ID3D11Texture2D * pTexture = nullptr;
                if(SUCCEEDED(pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture))) {
                    D3D11_TEXTURE2D_DESC desc;
                    pTexture->GetDesc(&desc);

                    if(/*desc.Width == g_RTDesc.Width && desc.Height == g_RTDesc.Height && desc.MipLevels == g_RTDesc.MipLevels*/ true) {
                        
                        if(g_ClearCount == g_RTCount) {
                            g_pCurrentDepthStencilView = pDepthStencilView;
                            g_iDraw = 1;
                        }

                        g_ClearCount++;
                    }

                    pTexture->Release();
                }

                pResource->Release();
            }
        }
    }
}

typedef void (STDMETHODCALLTYPE * ResolveSubresource_t)( ID3D11DeviceContext * This,
    /* [annotation] */ 
    _In_  ID3D11Resource *pDstResource,
    /* [annotation] */ 
    _In_  UINT DstSubresource,
    /* [annotation] */ 
    _In_  ID3D11Resource *pSrcResource,
    /* [annotation] */ 
    _In_  UINT SrcSubresource,
    /* [annotation] */ 
    _In_  DXGI_FORMAT Format);

ResolveSubresource_t g_Old_ResolveSubresource = nullptr;

void STDMETHODCALLTYPE New_ResolveSubresource( ID3D11DeviceContext * This,
    /* [annotation] */ 
    _In_  ID3D11Resource *pDstResource,
    /* [annotation] */ 
    _In_  UINT DstSubresource,
    /* [annotation] */ 
    _In_  ID3D11Resource *pSrcResource,
    /* [annotation] */ 
    _In_  UINT SrcSubresource,
    /* [annotation] */ 
    _In_  DXGI_FORMAT Format) {

    if(This == g_pImmediateContext && g_iDraw == 2 &&  pSrcResource == g_pCurrentRenderTargetViewResource) {
        g_iDraw = 3;
        g_CampathDrawer.OnRenderThread_Draw(g_pImmediateContext, &g_ViewPort, g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);        
        g_CampathDrawer.OnRenderThread_Present();
    }     

    g_Old_ResolveSubresource(This, pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
 }

typedef void (STDMETHODCALLTYPE * OMSetRenderTargets_t)( ID3D11DeviceContext * This,
            /* [annotation] */ 
            _In_range_( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            _In_opt_  ID3D11DepthStencilView *pDepthStencilView);

OMSetRenderTargets_t g_Old_OMSetRenderTargets = nullptr;

void STDMETHODCALLTYPE New_OMSetRenderTargets( ID3D11DeviceContext * This,
            /* [annotation] */ 
            _In_range_( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            _In_opt_  ID3D11DepthStencilView *pDepthStencilView) {

    if(This == g_pImmediateContext) {
        if(g_iDraw == 1 && pDepthStencilView == g_pCurrentDepthStencilView && ppRenderTargetViews && ppRenderTargetViews[0]) {
            g_iDraw = 2;
            UINT numViewPorts = 1;
            g_pImmediateContext->RSGetViewports(&numViewPorts,&g_ViewPort);
            g_pCurrentRenderTargetView = ppRenderTargetViews[0];
            g_pCurrentRenderTargetViewResource = nullptr;
            if(g_pCurrentRenderTargetView) {
                g_pCurrentRenderTargetView->GetResource(&g_pCurrentRenderTargetViewResource);
                if(g_pCurrentRenderTargetViewResource) g_pCurrentRenderTargetViewResource->Release();
            }
        }
        else if (g_iDraw == 2 && pDepthStencilView && pDepthStencilView != g_pCurrentDepthStencilView && ppRenderTargetViews && ppRenderTargetViews[0]) {
            g_iDraw = 3;
            g_CampathDrawer.OnRenderThread_Draw(g_pImmediateContext, &g_ViewPort, g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);
            g_CampathDrawer.OnRenderThread_Present();
        }
    }
    
    g_Old_OMSetRenderTargets(This, NumViews, ppRenderTargetViews, pDepthStencilView);
}

void Hook_Context(ID3D11DeviceContext * pDeviceContext) {
    static void **last_vtable = nullptr;
    void **vtable = *(void***)pDeviceContext;

    // (We can not use vtable detours here, becuse something writes them back after we did that.)

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    if(last_vtable) {
        DetourDetach(&(PVOID&)g_Old_OMSetRenderTargets, New_OMSetRenderTargets);
        DetourDetach(&(PVOID&)g_Old_ClearDepthStencilView, New_ClearDepthStencilView);
        DetourDetach(&(PVOID&)g_Old_ResolveSubresource, New_ResolveSubresource);
        if(NO_ERROR != DetourTransactionCommit()) {
            ErrorBox("Failed detaching on ID1D11RenderContext.");
        }
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
    }
    g_Old_OMSetRenderTargets = (OMSetRenderTargets_t)vtable[33];
    g_Old_ClearDepthStencilView = (ClearDepthStencilView_t)vtable[53];
    g_Old_ResolveSubresource = (ResolveSubresource_t)vtable[57];
    DetourAttach(&(PVOID&)g_Old_OMSetRenderTargets, New_OMSetRenderTargets);
    DetourAttach(&(PVOID&)g_Old_ClearDepthStencilView, New_ClearDepthStencilView);
    DetourAttach(&(PVOID&)g_Old_ResolveSubresource, New_ResolveSubresource);
    if(NO_ERROR != DetourTransactionCommit()) {
        ErrorBox("Failed attaching on ID1D11RenderContext.");
    }

    last_vtable = vtable;   
}

typedef void (STDMETHODCALLTYPE * ID3D11Device_GetImmediateContext_t)(ID3D11Device * This, 
    /* [annotation] */ 
    _Outptr_  ID3D11DeviceContext **ppImmediateContext);

ID3D11Device_GetImmediateContext_t g_Old_ID3D11Device_GetImmediateContext = nullptr;


void STDMETHODCALLTYPE New_ID3D11Device_GetImmediateContext(ID3D11Device * This, 
    /* [annotation] */ 
    _Outptr_  ID3D11DeviceContext **ppImmediateContext) {
    g_Old_ID3D11Device_GetImmediateContext(This, ppImmediateContext);
    if(ppImmediateContext && *ppImmediateContext){
        Hook_Context(*ppImmediateContext);
        g_pImmediateContext = *ppImmediateContext;
    }
}

typedef HRESULT(WINAPI* D3D11CreateDevice_t)(IDXGIAdapter*,D3D_DRIVER_TYPE,HMODULE,UINT,const D3D_FEATURE_LEVEL*,UINT,UINT,ID3D11Device**,D3D_FEATURE_LEVEL*,ID3D11DeviceContext**);
D3D11CreateDevice_t g_Old_D3D11CreateDevice = nullptr;

HRESULT WINAPI New_D3D11CreateDevice(
    IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext
    ) {
#ifdef _DEBUG
    Flags = Flags | D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = g_Old_D3D11CreateDevice(
        pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

    if(SUCCEEDED(result) && ppDevice && *ppDevice) {
        if(g_Old_CreateRenderTargetView && g_pDevice) {
            // Unhook wrong one (happens with D3D11_CREATE_DEVICE_DEBUG).
            void **vtable = *(void***)g_pDevice;
            AfxDetourPtr(&(vtable[9]),g_Old_CreateRenderTargetView,nullptr);
            g_Old_CreateRenderTargetView = nullptr;
        }
        if(g_Old_ID3D11Device_GetImmediateContext && g_pDevice) {
            // Unhook wrong one (happens with D3D11_CREATE_DEVICE_DEBUG).
            void **vtable = *(void***)g_pDevice;
            AfxDetourPtr(&(vtable[40]),g_Old_ID3D11Device_GetImmediateContext,nullptr);
            g_Old_ID3D11Device_GetImmediateContext = nullptr;
        }     
        if(g_pDevice) {
            g_CampathDrawer.EndDevice();
            g_pDevice = nullptr;
        }
        g_pDevice = *ppDevice;
        if(nullptr == g_Old_CreateRenderTargetView) {
            void **vtable = *(void***)*ppDevice;
            AfxDetourPtr(&(vtable[9]),New_CreateRenderTargetView,(PVOID*)&g_Old_CreateRenderTargetView);
        }
        if(nullptr == g_Old_ID3D11Device_GetImmediateContext) {
            void **vtable = *(void***)*ppDevice;
            AfxDetourPtr(&(vtable[40]),New_ID3D11Device_GetImmediateContext,(PVOID*)&g_Old_ID3D11Device_GetImmediateContext);
        }
        g_CampathDrawer.BeginDevice(*ppDevice);
    }
    if(SUCCEEDED(result) && ppImmediateContext && *ppImmediateContext) {
        Hook_Context(*ppImmediateContext);
        g_pImmediateContext = *ppImmediateContext;
    }

    return result;
}

typedef HRESULT (STDMETHODCALLTYPE * Present_t)( ID3D11DeviceContext * This,
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags);

Present_t g_OldPresent = nullptr;

HRESULT STDMETHODCALLTYPE New_Present( ID3D11DeviceContext * This,
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags) {
    
    if (This == g_pImmediateContext) {
        if (g_iDraw == 2) {
            g_CampathDrawer.OnRenderThread_Draw(g_pImmediateContext, &g_ViewPort, g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);
        }
        if (g_iDraw <= 2) {
            g_CampathDrawer.OnRenderThread_Present();
        }
    }
    HRESULT result = g_OldPresent(This, SyncInterval, Flags);

    g_ClearCount = 0;
    g_iDraw = 0;

    return result;
}


typedef HRESULT (STDMETHODCALLTYPE * CreateSwapChain_t)( void * This,
            /* [annotation][in] */ 
            _In_  IUnknown *pDevice,
            /* [annotation][in] */ 
            _In_  DXGI_SWAP_CHAIN_DESC *pDesc,
            /* [annotation][out] */ 
            _COM_Outptr_  IDXGISwapChain **ppSwapChain);

CreateSwapChain_t g_OldCreateSwapChain = nullptr;

HRESULT STDMETHODCALLTYPE New_CreateSwapChain( void * This,
            /* [annotation][in] */ 
            _In_  IUnknown *pDevice,
            /* [annotation][in] */ 
            _In_  DXGI_SWAP_CHAIN_DESC *pDesc,
            /* [annotation][out] */ 
            _COM_Outptr_  IDXGISwapChain **ppSwapChain) {
    HRESULT result = g_OldCreateSwapChain(This, pDevice, pDesc, ppSwapChain);

    if(SUCCEEDED(result) && ppSwapChain && *ppSwapChain) {
        g_pSwapChain = *ppSwapChain;
        if(nullptr == g_OldPresent) {
            void **vtable = *(void***)*ppSwapChain;
            g_OldPresent = (Present_t)vtable[8];
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)g_OldPresent, New_Present);
            if(NO_ERROR != DetourTransactionCommit()) ErrorBox("Failed to detour IDXGISwapChain::Present.");
        }
    }

    return result;
}

HRESULT WINAPI New_CreateDXGIFactory(REFIID riid, _COM_Outptr_ void **ppFactory);

CAfxImportFuncHook<HRESULT(WINAPI*)(REFIID riid, _COM_Outptr_ void **ppFactory)> g_Import_rendersystemdx11_dxgi_CreateDXGIFactory("CreateDXGIFactory", &New_CreateDXGIFactory);
CAfxImportDllHook g_Import_rendersystemdx11_dxgi("dxgi.dll", CAfxImportDllHooks({
	&g_Import_rendersystemdx11_dxgi_CreateDXGIFactory
    }));

HRESULT WINAPI New_CreateDXGIFactory(REFIID riid, _COM_Outptr_ void **ppFactory) {
    HRESULT result = g_Import_rendersystemdx11_dxgi_CreateDXGIFactory.GetTrueFuncValue()(riid, ppFactory);

    if(SUCCEEDED(result) && ppFactory && *ppFactory
        && ( __uuidof(IDXGIFactory4) == riid || __uuidof(IDXGIFactory) == riid )
        && nullptr == g_OldCreateSwapChain) {
            void **vtable = *(void***)*ppFactory;
            AfxDetourPtr(&(vtable[10]),New_CreateSwapChain,(PVOID*)&g_OldCreateSwapChain);
        }
        
    return result;
}

CAfxImportsHook g_Import_rendersystemdx11(CAfxImportsHooks({
	&g_Import_rendersystemdx11_dxgi
    }));

bool Hook_RenderSystemDX11(void * hModule) {
    static bool firstResult = false;
    static bool firstRun = true;

    if(firstRun) {
        firstRun = false;

        if(g_Import_rendersystemdx11.Apply((HMODULE)hModule)) {
            // We have to detour it in-place, because it can be called from elsewhere:
            if(HMODULE hD3D11Dll = GetModuleHandleA("d3d11.dll")) {
                if(g_Old_D3D11CreateDevice = (D3D11CreateDevice_t)GetProcAddress(hD3D11Dll,"D3D11CreateDevice")) {
                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());
                    DetourAttach(&(PVOID&)g_Old_D3D11CreateDevice, New_D3D11CreateDevice);
                    if(NO_ERROR == DetourTransactionCommit()) {
                        firstResult = true;
                    } else ErrorBox("Failed to hook D3D11CreateDevice.");
                } else ErrorBox("Failed to get D3D11CreateDevice address.");
            } else ErrorBox("Failed to get d3d11.dll module handle.");
        } else ErrorBox("Failed rendersystemdx11 import hooks.");
    }

    return firstResult;
}
