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
ID3D11RenderTargetView * g_pRTView = nullptr;

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
                g_pRTView = *ppRTView;
            }

            pTexture->Release();
        }
    }

    return result;
}

typedef void (STDMETHODCALLTYPE * ID3D11Device_GetImmediateContext_t)(ID3D11Device * This, 
    /* [annotation] */ 
    _Outptr_  ID3D11DeviceContext **ppImmediateContext);

ID3D11Device_GetImmediateContext_t g_Old_ID3D11Device_GetImmediateContext = nullptr;

void STDMETHODCALLTYPE New_ID3D11Device_GetImmediateContext_t(ID3D11Device * This, 
    /* [annotation] */ 
    _Outptr_  ID3D11DeviceContext **ppImmediateContext) {
    g_Old_ID3D11Device_GetImmediateContext(This, ppImmediateContext);
    if(ppImmediateContext && *ppImmediateContext){
        //*ppImmediateContext = new CID3D11DeviceContextHook(This,*ppImmediateContext);
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
   // Flags = Flags | D3D11_CREATE_DEVICE_DEBUG;
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
            AfxDetourPtr(&(vtable[40]),New_ID3D11Device_GetImmediateContext_t,(PVOID*)&g_Old_ID3D11Device_GetImmediateContext);
        }
        g_CampathDrawer.BeginDevice(*ppDevice);
    }
    if(SUCCEEDED(result) && ppImmediateContext && *ppImmediateContext) {
        g_pImmediateContext = *ppImmediateContext;
        //*ppImmediateContext = new CID3D11DeviceContextHook(*ppDevice,*ppImmediateContext); // They will just release it shortly there-after, which is why we detour ID3D11Device::GetImmediateContext.
    }

    return result;
}

typedef HRESULT (STDMETHODCALLTYPE * Present_t)( void * This,
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags);

Present_t g_OldPresent = nullptr;

HRESULT STDMETHODCALLTYPE New_Present( void * This,
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags) {

    g_CampathDrawer.OnRenderThread_Present(g_pImmediateContext, g_pRTView);

    HRESULT result = g_OldPresent(This, SyncInterval, Flags);
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
            AfxDetourPtr(&(vtable[8]),New_Present,(PVOID*)&g_OldPresent);
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

extern void ErrorBox(char const * messageText);

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
