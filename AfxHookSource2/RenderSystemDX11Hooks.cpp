#include "stdafx.h"

#include "RenderSystemDX11Hooks.h"

#include "CampathDrawer.h"
#include "ReShadeAdvancedfx.h"
#include "WrpConsole.h"

#include "../shared/AfxDetours.h"
#include "../shared/binutils.h"
#include "../shared/Captures.h"
#include "../shared/FileTools.h"
#include "../shared/ImageBufferPoolThreadSafe.h"
#include "../shared/ImageTransformer.h"
#include "../shared/RecordingSettings.h"
#include "../shared/RefCountedThreadSafe.h"
#include "../shared/StringTools.h"

#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
#include "../deps/release/prop/cs2/sdk_src/public/icvar.h"

#include <d3d11.h>

#include <map>
#include <queue>
#include <list>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <functional>

#include <dxgi.h>
#include <dxgi1_4.h>

extern advancedfx::CThreadPool * g_pThreadPool;
extern advancedfx::CImageBufferPoolThreadSafe * g_pImageBufferPoolThreadSafe;

extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;
extern SOURCESDK::CS2::ICvar * SOURCESDK::CS2::g_pCVar;

std::mutex g_SwapChainMutex;
std::queue<std::function<bool(ID3D11DeviceContext * pDeviceContext, ID3D11Texture2D * pTexture)>> g_SwapchainBeforePresentQueue;
std::queue<std::function<bool(ID3D11DeviceContext * pDeviceContext)>> g_SwapchainAfterPresentQueue;

bool g_bEnableReShade = true;

class CAfxCpuTexture
: public advancedfx::CRefCountedThreadSafe
, public advancedfx::ICapture
, public advancedfx::IImageBuffer
{
public:
    virtual void AddRef() override {
        advancedfx::CRefCountedThreadSafe::AddRef();
    }

    virtual void Release() override {
        advancedfx::CRefCountedThreadSafe::Release();
    }

    virtual const advancedfx::CImageFormat * GetImageBufferFormat() const {
        return &m_ImageFormat;
    }

    virtual const void * GetImageBufferData() const {
        return m_MappedResource.pData;
    }

	virtual const IImageBuffer* GetBuffer() const {
        return this;
    }

    CAfxCpuTexture(ID3D11Device * pDevice)
    : m_pDevice(pDevice)
    {
        m_pDevice->AddRef();
        m_MappedResource.pData = nullptr;
        m_Condition.notify_one();
    }

    void GpuCopyResource(ID3D11DeviceContext * pContext, ID3D11Texture2D * pTexture) {
        if(m_pCpuTexture == nullptr && pTexture) {
            D3D11_TEXTURE2D_DESC desc;
            pTexture->GetDesc(&desc);
            bool bMultiSampled = 1 < desc.SampleDesc.Count;
            desc.BindFlags = 0;
            desc.MiscFlags = 0;
            desc.MipLevels = 1;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            advancedfx::ImageFormat format = advancedfx::ImageFormat::Unkown;
            switch(desc.Format) {
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT:
               format = advancedfx::ImageFormat::RGBA;
               desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
               break;
            default:
                advancedfx::Warning("AFXERROR: GpuCopyResource - unspported DXGI_FORMAT: %i\n",desc.Format);
            }
            if(bMultiSampled) {
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.CPUAccessFlags = 0;
                m_pDevice->CreateTexture2D(&desc, nullptr, &m_pResolveTexture);
            }
            desc.Usage = D3D11_USAGE_STAGING;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            m_ImageFormat = advancedfx::CImageFormat(format, desc.Width, desc.Height);
            m_Format = desc.Format;
            m_pDevice->CreateTexture2D(&desc, nullptr, &m_pCpuTexture);
        }

        if(m_pCpuTexture && pTexture) {
            if(m_pResolveTexture) {
                pContext->ResolveSubresource(m_pResolveTexture, 0, pTexture, 0, m_Format);
                pContext->CopyResource(m_pCpuTexture, m_pResolveTexture);
            } else {
                pContext->CopyResource(m_pCpuTexture, pTexture);
            }
        }
    }

    void CpuSignalDone() {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_bCpuDone = true;
        m_Condition.notify_one();
    }     

    class advancedfx::IImageBuffer * CpuBeginAccess(ID3D11DeviceContext * pContext) {
        m_MappedResource.pData = nullptr;
        if(m_pCpuTexture) {
            if(SUCCEEDED(pContext->Map(m_pCpuTexture, 0, D3D11_MAP_READ , 0, &m_MappedResource))) {
                m_ImageFormat = advancedfx::CImageFormat(m_ImageFormat.Format, m_ImageFormat.Width, m_ImageFormat.Height, m_MappedResource.RowPitch);
                return this;
            }
        }
        return nullptr;
    }

    void IfAcccessedWaitForCpuAndEndAccess(ID3D11DeviceContext * pContext) {        
        if(m_MappedResource.pData) {
            WaitForCpu();
            pContext->Unmap(m_pCpuTexture, 0);    
            m_MappedResource.pData = nullptr;        
        }
    }

protected:
    virtual ~CAfxCpuTexture() {
        if(m_pCpuTexture) {
            m_pCpuTexture->Release();
        }
        if(m_pResolveTexture){
            m_pResolveTexture->Release();
        }
        m_pDevice->Release();
    }

private:
    ID3D11Device * m_pDevice;
    ID3D11Texture2D * m_pCpuTexture = nullptr;
    ID3D11Texture2D * m_pResolveTexture = nullptr;
    DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    bool m_bCpuDone = true;
    advancedfx::CImageFormat m_ImageFormat;
    D3D11_MAPPED_SUBRESOURCE m_MappedResource;


    void WaitForCpu() {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Condition.wait(lock, [this] { return m_bCpuDone; });
        m_bCpuDone = false;
    }
};

class CAfxCapture {
public:
    CAfxCapture(class advancedfx::COutVideoStreamCreator* pOutVideoStreamCreator)
     : m_pOutVideoStreamCreator(pOutVideoStreamCreator)
    {
        m_pOutVideoStreamCreator->AddRef();
        m_ProcessingThread = std::thread(&CAfxCapture::ProcessingThreadFunc, this);
    }

    void Finish(ID3D11DeviceContext * pDeviceContext) {
        while(m_CpuTextures[0] || m_CpuTextures[1]) {
            if(m_CpuTextures[m_Index]) {
                m_CpuTextures[m_Index]->IfAcccessedWaitForCpuAndEndAccess(pDeviceContext);
                m_CpuTextures[m_Index]->Release();
                m_CpuTextures[m_Index] = nullptr;
            }
            m_Index = (m_Index + 1) % 2;
        }
    }

    ~CAfxCapture() {
        if(m_CpuTextures[0]) {
            m_CpuTextures[0]->Release();
        }
        if(m_CpuTextures[1]) {
            m_CpuTextures[1]->Release();
        }
        {
            std::unique_lock<std::mutex> lock(m_ProcessingThreadMutex);
            m_ShutDown = true;
            m_ProcessingThreadCv.notify_one();
        }
        m_ProcessingThread.join();
    }

    void OnBeforeGpuPresent(ID3D11DeviceContext * pDeviceContext, ID3D11Texture2D * pTexture) {
        if(m_CpuTextures[m_Index] == nullptr) {
            ID3D11Device * pDevice = nullptr;
            pDeviceContext->GetDevice(&pDevice);
            if(pDevice) {
                auto pCpuTexture = new CAfxCpuTexture(pDevice);
                pCpuTexture->AddRef();
                m_CpuTextures[m_Index] = pCpuTexture;
                pDevice->Release();
            }
        } else {
            m_CpuTextures[m_Index]->IfAcccessedWaitForCpuAndEndAccess(pDeviceContext);
        }
        m_CpuTextures[m_Index]->GpuCopyResource(pDeviceContext, pTexture);
    }

    void OnAfterGpuPresent(ID3D11DeviceContext * pDeviceContext) {
        m_Index = (m_Index + 1) % 2;
        if(m_CpuTextures[m_Index]) {
            m_CpuTextures[m_Index]->CpuBeginAccess(pDeviceContext);
            StartProcess(0,m_CpuTextures[m_Index]);
        }
    }

private:

    void StartProcess(size_t index, CAfxCpuTexture * pCpuTexture) {
        std::unique_lock<std::mutex> lock(m_ProcessingThreadMutex);
        if(index == 0) {
            m_In.push_back(new CBuffers(m_Streams_size));
        }
        (*m_In.rbegin())->SetAt(index, pCpuTexture);
        if (index + 1  >= m_Streams_size) {
            m_ProcessingThreadCv.notify_one();
        }
    }

    size_t m_Index = 0;
    CAfxCpuTexture * m_CpuTextures[2]={nullptr,nullptr};

    const size_t m_Streams_size = 1;
    class advancedfx::COutVideoStreamCreator* m_pOutVideoStreamCreator;
	advancedfx::COutVideoStream* m_OutVideoStream = nullptr;

	class CBuffers {
	public:
		CBuffers(size_t size) : m_Buffers(size) {
		}

		size_t GetSize() const {
			return m_Buffers.size();
		}

		class CAfxCpuTexture * GetAt(size_t index) const {
			return m_Buffers[index];
		}

		void SetAt(size_t index, class CAfxCpuTexture * value) {
			m_Buffers[index] = value;
		}

	private:
		std::vector<class CAfxCpuTexture *> m_Buffers;
	};

	std::atomic_int m_CapturesLeft = 0;

	std::mutex m_ProcessingThreadMutex;
	std::condition_variable m_ProcessingThreadCv;
	std::thread m_ProcessingThread;
	bool m_ShutDown = false;

	std::list<class CBuffers*> m_In;

	void ProcessingThreadFunc() {
		std::unique_lock<std::mutex> lock(m_ProcessingThreadMutex);
		while (!m_ShutDown || 0 < m_CapturesLeft || !m_In.empty()) {
			if (!m_In.empty()) {
				class CBuffers* buffers = m_In.front();
				if(buffers->GetSize() >= m_Streams_size) {
					class CBuffers* task = buffers;
					m_In.pop_front();
					lock.unlock();

                    size_t taskSize = task->GetSize();
                    for(size_t i=0; i < taskSize; i++) {
                        auto pTexture = task->GetAt(i);
                        if(pTexture) {
                            advancedfx::ICapture* noAlphaCapture = advancedfx::ImageTransformer::RgbaToBgr(g_pThreadPool,g_pImageBufferPoolThreadSafe,pTexture);
                            if (noAlphaCapture) {
                                if (const advancedfx::IImageBuffer* buffer = noAlphaCapture->GetBuffer()) {                            
                                    if (m_OutVideoStream == nullptr) {
                                        m_OutVideoStream = m_pOutVideoStreamCreator->CreateOutVideoStream(*buffer->GetImageBufferFormat());
                                        if (nullptr == m_OutVideoStream)
                                        {
                                            advancedfx::Warning("AFXERROR: Failed to create image stream for screen recording.\n");
                                        }
                                        else
                                        {
                                            m_OutVideoStream->AddRef();
                                        }
                                    }
                                    if (nullptr != m_OutVideoStream && !m_OutVideoStream->SupplyImageBuffer(buffer))
                                    {
                                        advancedfx::Warning("AFXERROR: Failed writing image for screen recording.\n");
                                    }
                                }
                                else {
                                   advancedfx::Warning("AFXERROR: Could not get capture buffer for screen recording.\n");
                                }
                                noAlphaCapture->Release();
                                noAlphaCapture = nullptr; 
                            }                               
                            pTexture->CpuSignalDone();
                        }
                    }

					delete task;
					task = nullptr;
					lock.lock();
				} else {
					m_ProcessingThreadCv.wait(lock);
				}
			} else {
				m_ProcessingThreadCv.wait(lock);
			}		
        }
        if(m_OutVideoStream) m_OutVideoStream->Release();
        m_pOutVideoStreamCreator->Release();
	}
};

IDXGISwapChain * g_pSwapChain = nullptr;
ID3D11Device * g_pDevice = nullptr;
ID3D11DeviceContext * g_pImmediateContext = nullptr;
int g_iDraw = 0;

extern void ErrorBox(char const * messageText);

typedef HRESULT (STDMETHODCALLTYPE * CreateRenderTargetView_t)( ID3D11Device * This,
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView);

CreateRenderTargetView_t g_Old_CreateRenderTargetView = nullptr;

HRESULT STDMETHODCALLTYPE New_CreateRenderTargetView(  ID3D11Device * This,
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView) {
    
    HRESULT result = g_Old_CreateRenderTargetView(This, pResource, pDesc, ppRTView);

    if (SUCCEEDED(result) && ppRTView && *ppRTView
    &&g_pSwapChain // can be nullptr e.g. when people forget to "disable service" on FACEIT anti cheat.
    ) {
        ID3D11Texture2D * pTexture = nullptr;
        HRESULT result2 = g_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D), (void**)&pTexture);
        if(SUCCEEDED(result2)) {
            if(pResource == pTexture) {
                if(g_pDevice) {
                    g_CampathDrawer.EndDevice();
                    g_pDevice = nullptr;
                }
                g_iDraw = 0;

                g_pDevice = This;
                g_CampathDrawer.BeginDevice(This);
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

void DrawReShade(ID3D11RenderTargetView * pRendertargetView, ID3D11DepthStencilView * pDepthStencilView) {
    if(g_ReShadeAdvancedfx.IsConnected() && !g_ReShadeAdvancedfx.HasRendered()) {
        ID3D11Resource * pRenderTargetResource = nullptr;
        ID3D11Resource * pDepthStencilResource = nullptr;

        if(pRendertargetView) { pRendertargetView->GetResource(&pRenderTargetResource); }
        if(pDepthStencilView) { pDepthStencilView->GetResource(&pDepthStencilResource); }

        g_ReShadeAdvancedfx.AdvancedfxRenderEffects(pRenderTargetResource, pDepthStencilResource);

        if(pDepthStencilResource) pDepthStencilResource->Release();
        if(pRenderTargetResource) pRenderTargetResource->Release();
    }
}

void STDMETHODCALLTYPE New_ClearDepthStencilView( ID3D11DeviceContext * This, 
    /* [annotation] */ 
    _In_  ID3D11DepthStencilView *pDepthStencilView,
    /* [annotation] */ 
    _In_  UINT ClearFlags,
    /* [annotation] */ 
    _In_  FLOAT Depth,
    /* [annotation] */ 
    _In_  UINT8 Stencil) {

    g_Old_ClearDepthStencilView(This, pDepthStencilView, ClearFlags, Depth, Stencil);

    if (g_pImmediateContext == This && pDepthStencilView == g_pCurrentDepthStencilView && (ClearFlags && D3D11_CLEAR_DEPTH) && g_iDraw == 1) {
        g_iDraw = 2;
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

    if(This->GetType() == D3D11_DEVICE_CONTEXT_IMMEDIATE) {
        if(g_iDraw == 0 && pDepthStencilView && ppRenderTargetViews && ppRenderTargetViews[0]) {
            g_iDraw = 2;
            g_pImmediateContext = This;
            g_pCurrentDepthStencilView = pDepthStencilView;
            g_pCurrentRenderTargetView = ppRenderTargetViews[0];
            g_pCurrentRenderTargetViewResource = nullptr;
            if(g_pCurrentRenderTargetView) {
                g_pCurrentRenderTargetView->GetResource(&g_pCurrentRenderTargetViewResource);
                if(g_pCurrentRenderTargetViewResource) g_pCurrentRenderTargetViewResource->Release();
            }
        }
        else if (g_iDraw == 2 && pDepthStencilView == nullptr && ppRenderTargetViews && ppRenderTargetViews[0] && ppRenderTargetViews[0] == g_pCurrentRenderTargetView) {
            g_iDraw = 3;
            UINT numViewPorts = 1;
            g_pImmediateContext->RSGetViewports(&numViewPorts, &g_ViewPort);
            g_CampathDrawer.OnRenderThread_Draw(g_pImmediateContext, &g_ViewPort, g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);
        }
        else if (g_iDraw == 3 && pDepthStencilView == nullptr && ppRenderTargetViews && ppRenderTargetViews[0]) {
            g_iDraw = 4;
            if (g_bEnableReShade) DrawReShade(g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);
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
    //Flags = Flags | D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT result = g_Old_D3D11CreateDevice(
        pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

    if(SUCCEEDED(result) && ppDevice && *ppDevice) {
        static void **last_vtable = nullptr;
        void **vtable = *(void***)*ppDevice;
        // (We can not use vtable detours here, becuse something writes them back after we did that.)
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        if(last_vtable) {
            DetourDetach(&(PVOID&)g_Old_CreateRenderTargetView, New_CreateRenderTargetView);
            DetourDetach(&(PVOID&)g_Old_ID3D11Device_GetImmediateContext, New_ID3D11Device_GetImmediateContext);
            if(NO_ERROR != DetourTransactionCommit()) {
                ErrorBox("Failed detaching in D3D11CreateDevice.");
            }
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
        }
        g_Old_CreateRenderTargetView = (CreateRenderTargetView_t)vtable[9];
        g_Old_ID3D11Device_GetImmediateContext = (ID3D11Device_GetImmediateContext_t)vtable[40];
        DetourAttach(&(PVOID&)g_Old_CreateRenderTargetView, New_CreateRenderTargetView);
        DetourAttach(&(PVOID&)g_Old_ID3D11Device_GetImmediateContext, New_ID3D11Device_GetImmediateContext);
        if(NO_ERROR != DetourTransactionCommit()) {
            ErrorBox("Failed attaching in D3D11CreateDevice.");
        }
        last_vtable = vtable;
    }
    if(SUCCEEDED(result) && ppImmediateContext && *ppImmediateContext) {
        Hook_Context(*ppImmediateContext);
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
    
    if (g_iDraw == 2) {
        g_iDraw = 3;
        g_CampathDrawer.OnRenderThread_Draw(g_pImmediateContext, &g_ViewPort, g_pCurrentRenderTargetView, g_pCurrentDepthStencilView);
    }
    g_CampathDrawer.OnRenderThread_Present();

    DrawReShade(nullptr, nullptr);

    {
        std::unique_lock<std::mutex> lock(g_SwapChainMutex);

        if(!g_SwapchainBeforePresentQueue.empty()) {
            ID3D11Texture2D * pTexture = nullptr;
            if(g_pSwapChain) g_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D), (void**)&pTexture);
            while(!g_SwapchainBeforePresentQueue.empty()) {
                bool bBreak = g_SwapchainBeforePresentQueue.front()(g_pImmediateContext,pTexture);
                g_SwapchainBeforePresentQueue.pop();
                if(bBreak) break;
            }
            if(pTexture) pTexture->Release();
        }
    }

    HRESULT result = g_OldPresent(This, SyncInterval, Flags);


    {
        std::unique_lock<std::mutex> lock(g_SwapChainMutex);
        
        while(!g_SwapchainAfterPresentQueue.empty()) {
            bool bBreak = g_SwapchainAfterPresentQueue.front()(g_pImmediateContext);
            g_SwapchainAfterPresentQueue.pop();
            if(bBreak) break;
        }
    }

	g_ReShadeAdvancedfx.ResetHasRendered();        

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
HRESULT WINAPI New_CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void **ppFactory);

CAfxImportFuncHook<HRESULT(WINAPI*)(REFIID riid, _COM_Outptr_ void **ppFactory)> g_Import_rendersystemdx11_dxgi_CreateDXGIFactory("CreateDXGIFactory", &New_CreateDXGIFactory);
CAfxImportFuncHook<HRESULT(WINAPI*)(REFIID riid, _COM_Outptr_ void **ppFactory)> g_Import_rendersystemdx11_dxgi_CreateDXGIFactory1("CreateDXGIFactory1", &New_CreateDXGIFactory1);

CAfxImportDllHook g_Import_rendersystemdx11_dxgi("dxgi.dll", CAfxImportDllHooks({
	&g_Import_rendersystemdx11_dxgi_CreateDXGIFactory,
    &g_Import_rendersystemdx11_dxgi_CreateDXGIFactory1
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

HRESULT WINAPI New_CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void **ppFactory) {
    HRESULT result = g_Import_rendersystemdx11_dxgi_CreateDXGIFactory1.GetTrueFuncValue()(riid, ppFactory);

    if(SUCCEEDED(result) && ppFactory && *ppFactory
        && ( __uuidof(IDXGIFactory4) == riid || __uuidof(IDXGIFactory) == riid )
        && nullptr == g_OldCreateSwapChain) {
            void **vtable = *(void***)*ppFactory;
            AfxDetourPtr(&(vtable[10]),New_CreateSwapChain,(PVOID*)&g_OldCreateSwapChain);
        }
        
    return result;
}

typedef bool (__fastcall * CRenderDeviceBase_Present_t)(
    void * This, void * Rdx, void * R8d);

CRenderDeviceBase_Present_t g_Old_CRenderDeviceBase_Present = nullptr;


CAfxCapture * g_ActiveCapture = nullptr;

bool __fastcall New_CRenderDeviceBase_Present(
    void * This, void * Rdx, void * R8) {
    
    g_CampathDrawer.OnEngineThread_EndFrame();

    {
        std::unique_lock<std::mutex> lock(g_SwapChainMutex);
        if(g_ActiveCapture) {
            CAfxCapture * capture = g_ActiveCapture;
            g_SwapchainBeforePresentQueue.push([capture](ID3D11DeviceContext * pDeviceContext, ID3D11Texture2D * pTexture){
                capture->OnBeforeGpuPresent(pDeviceContext, pTexture);
                return false;
            });        
            g_SwapchainAfterPresentQueue.push([capture](ID3D11DeviceContext * pDeviceContext){
                capture->OnAfterGpuPresent(pDeviceContext);
                return false;
            });
        }

        g_SwapchainBeforePresentQueue.push([](ID3D11DeviceContext * pDeviceContext, ID3D11Texture2D * pTexture){
            return true;
        });
        g_SwapchainAfterPresentQueue.push([](ID3D11DeviceContext * pDeviceContext){
            return true;
        });    
    }

    bool result = g_Old_CRenderDeviceBase_Present(This, Rdx, R8);

    return result;
}

CAfxImportsHook g_Import_rendersystemdx11(CAfxImportsHooks({
	&g_Import_rendersystemdx11_dxgi
    }));

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

void Hook_RenderSystemDX11(void * hModule) {
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
                    } else ErrorBox("Failed to hook D3D11CreateDevice.");
                } else ErrorBox("Failed to get D3D11CreateDevice address.");
            } else ErrorBox("Failed to get d3d11.dll module handle.");
        } else ErrorBox("Failed rendersystemdx11 import hooks.");

        Afx::BinUtils::MemRange textRange = Afx::BinUtils::MemRange::FromEmpty();
        Afx::BinUtils::MemRange dataRange = Afx::BinUtils::MemRange::FromEmpty();
        {
            Afx::BinUtils::ImageSectionsReader sections((HMODULE)hModule);
            if(!sections.Eof()) {
                textRange = sections.GetMemRange();
                sections.Next();
                if(!sections.Eof()){
                    dataRange = sections.GetMemRange();
                }
            }
        }

        // CRenderDeviceBase::Present
        //
        // Function jmps into a function that references string "CRenderDeviceBase::Present(640):".
        if(void ** vtable = (void**)Afx::BinUtils::FindClassVtable((HMODULE)hModule,".?AVCRenderDeviceDx11@@", 0, 0x0)) {
            AfxDetourPtr(&(vtable[16]),New_CRenderDeviceBase_Present,(PVOID*)&g_Old_CRenderDeviceBase_Present);
        } else ErrorBox(MkErrStr(__FILE__, __LINE__));	              
    }

}

void EndCapture() {
    if(g_ActiveCapture) {
        CAfxCapture * capture = g_ActiveCapture;
        {
            std::unique_lock<std::mutex> lock(g_SwapChainMutex);
            g_SwapchainAfterPresentQueue.push([capture](ID3D11DeviceContext * pDeviceContext){
                capture->Finish(pDeviceContext);
                delete capture;
                return false;
            });
        }
        g_ActiveCapture = nullptr;
    }
}

void CreateCapture(class advancedfx::COutVideoStreamCreator* pOutVideoStreamCreator) {
    EndCapture();
    g_ActiveCapture = new CAfxCapture(pOutVideoStreamCreator);
}

advancedfx::CImageBufferPoolThreadSafe g_ImageBufferPool;

class CAfxStreams : public advancedfx::IRecordStreamSettings {
public:
    CAfxStreams() {
        m_RecordScreen = new CRecordScreen(false, advancedfx::CRecordingSettings::GetDefault());        
    }

	void ShutDown(void) {
        if(m_Shutdown) return;
        m_Shutdown = true;
		delete m_RecordScreen;
    }    

    ~CAfxStreams(){
        ShutDown();
    }

    const char* GetRecordName() {
        return m_RecordName.c_str();
    }

    void SetRecordName(const char * value) {
        m_RecordName = value;
    }

    bool GetStartMovieWav() {
        return m_StartMovieWav;
    }

    void SetStartMovieWav(bool value) {
        m_StartMovieWav = value;
    }

    bool GetOverrideFps() {
        return m_OverrideFps;
    }

    void SetOverrideFps(bool value) {
        m_OverrideFps = value;
    }

    float GetOverrideFpsValue() {
        return m_OverrideFpsValue;
    }

    void SetOverrideFpsValue(float value) {
        m_OverrideFpsValue = value;
    }

	virtual bool GetStreamFolder(std::wstring& outFolder) const {
        outFolder = m_TakeDir;
        return true;
    }

    const wchar_t * GetTakeDir(void) const {
        return m_TakeDir.c_str();
    }

	virtual advancedfx::StreamCaptureType GetCaptureType() const {
        return m_StreamCaptureType;
    }

    virtual advancedfx::IImageBufferPool * GetImageBufferPool() const {
        return &g_ImageBufferPool;
    }

    virtual bool GetFormatBmpNotTga() const {        
        return m_FormatBmpAndNotTga;
    }

    void SetFormatBmpNotTga(bool value) {
        m_FormatBmpAndNotTga = value;
    }

    void Console_RecordScreen(advancedfx::ICommandArgs* args);

    void RecordStart();
    void RecordEnd();

    bool GetRecording() { return m_Recording; }

private:
    bool m_Shutdown = false;
    advancedfx::StreamCaptureType m_StreamCaptureType = advancedfx::StreamCaptureType::Normal;
    bool m_FormatBmpAndNotTga = false;
    bool m_StartMovieWav = true;
    bool m_OverrideFps = false;
    float m_OverrideFpsValue = 60.0;
    std::string m_RecordName = "untitled_rec";
    
	class CRecordScreen {
	public:
		bool Enabled;
		advancedfx::CRecordingSettings* Settings;

		CRecordScreen(bool enabled, advancedfx::CRecordingSettings* settings)
			: Enabled(enabled)
			, Settings(settings)
		{
			settings->AddRef();
		}

		CRecordScreen(const CRecordScreen& other)
		: Enabled(other.Enabled)
		, Settings(other.Settings) {
			Settings->AddRef();
		}

		~CRecordScreen() {
			Settings->Release();
		}
	};
	CRecordScreen* m_RecordScreen;

    bool m_Recording = false;
    std::wstring m_TakeDir;
    bool m_StartMovieWavUsed = false;
    float m_StartHostFrameRateValue = 60.0;
    
    bool m_UsedHostFramerRateValue = false;
    float m_OldValue_host_framerate;
    bool m_OldValue_r_always_render_all_windows;
    int m_OldValue_engine_no_focus_sleep;
} g_AfxStreams;

void CAfxStreams::Console_RecordScreen(advancedfx::ICommandArgs* args) {
	int argC = args->ArgC();
	char const* arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		char const* arg1 = args->ArgV(1);
		if (0 == _stricmp(arg1, "enabled")) {
			if (3 <= argC) {
				m_RecordScreen->Enabled = 0 != atoi(args->ArgV(2));
				return;
			}

			advancedfx::Message(
				"%s enabled 0|1 - Disable (0, default) or enable (1) game screen recording.\n"
				"Current value: %i\n"
				, arg0
				, (m_RecordScreen->Enabled?1:0)
			);
			return;
		}
		if (0 == _stricmp(arg1, "settings")) {
			if (3 <= argC)
			{
				char const* arg2 = args->ArgV(2);

				if (advancedfx::CRecordingSettings* settings = advancedfx::CRecordingSettings::GetByName(arg2))
				{
					settings->AddRef();
					m_RecordScreen->Settings->Release();
					m_RecordScreen->Settings = settings;
				}
				else
				{
					advancedfx::Warning("AFXERROR: There is no recording setting named %s\n", arg2);
				}

				return;
			}

			advancedfx::Message(
				"%s settings <name> - Set recording settings to use from mirv_streams settings.\n"
				"Current value: %s\n"
				, arg0
				, m_RecordScreen->Settings->GetName()
			);

			return;
		}
	}

	advancedfx::Message(
		"%s enabled [...] - Enables / disables screen recording.\n"
		"%s settings [...] - Controls recording settings.\n"
		, arg0
		, arg0
	);
}

void CAfxStreams::RecordStart()
{
	RecordEnd();

	advancedfx::Message("Starting recording ... ");
	
	if(UTF8StringToWideString(m_RecordName.c_str(), m_TakeDir)
		&& (m_TakeDir.append(L"\\take"), SuggestTakePath(m_TakeDir.c_str(), 4, m_TakeDir))
		&& CreatePath(m_TakeDir.c_str(), m_TakeDir)
	)
	{
		m_Recording = true;
		m_StartMovieWavUsed = false;

		std::string utf8TakeDir;
		bool utf8TakeDirOk = WideStringToUTF8String(m_TakeDir.c_str(), utf8TakeDir);
        SOURCESDK::CS2::Cvar_s * handle_host_framerate = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("host_framerate", false).Get());
        SOURCESDK::CS2::Cvar_s * handle_engine_no_focus_sleep = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("engine_no_focus_sleep", false).Get());
        SOURCESDK::CS2::Cvar_s * handle_r_always_render_all_windows = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("r_always_render_all_windows", false).Get());
        
        m_UsedHostFramerRateValue = GetOverrideFps();

        if(m_UsedHostFramerRateValue && handle_host_framerate) {
            m_OldValue_host_framerate = handle_host_framerate->m_Value.m_flValue;
            handle_host_framerate->m_Value.m_flValue = GetOverrideFpsValue();
        }

        if(handle_engine_no_focus_sleep) {
            m_OldValue_engine_no_focus_sleep = handle_engine_no_focus_sleep->m_Value.m_i32Value;
            handle_engine_no_focus_sleep->m_Value.m_i32Value = 0;
        }

        if(handle_r_always_render_all_windows) {
            m_OldValue_r_always_render_all_windows = handle_r_always_render_all_windows->m_Value.m_bValue;
            handle_r_always_render_all_windows->m_Value.m_bValue = true;
        }

		float host_framerate = m_OverrideFps ? m_OverrideFpsValue : (handle_host_framerate != nullptr ? m_OldValue_host_framerate : 0);
		double frameTime;
		if (1.0 <= host_framerate) {
			m_StartHostFrameRateValue = host_framerate;
			frameTime = 1.0 / host_framerate;
		}
		else {
			m_StartHostFrameRateValue = host_framerate ? 1.0f / host_framerate : 0.0f;
			frameTime = host_framerate;
		}

		if (0 == frameTime) {
			advancedfx::Warning("You probably forgot to set host_framerate to the FPS you want to record.\n");
			if (nullptr == handle_host_framerate) {
				advancedfx::Warning("You probably forgot to set mirv_streams record fps to the FPS you want to record.\n");
			}
		}

		if(m_RecordScreen->Enabled) {
			CreateCapture(
				m_RecordScreen->Settings->CreateOutVideoStreamCreator(
					*this,
					*this,
					m_StartHostFrameRateValue,
					""
				)
			);
		}

		advancedfx::Message("done.\n");

		advancedfx::Message("Recording to \"%s\".\n", utf8TakeDirOk ? utf8TakeDir.c_str() : "?");

		m_StartMovieWavUsed = m_StartMovieWav;

		if (m_StartMovieWavUsed)
		{
            SOURCESDK::CS2::ConCommandHandle handle_startmovie = SOURCESDK::CS2::g_pCVar->FindCommand( "startmovie", false );
            if(handle_startmovie.IsValid()) {
                const char * pszArgs[3] = {"startmovie",ADVNACEDFX_STARTMOIVE_WAV_KEY,"wav"};
                SOURCESDK::CS2::g_pCVar->DispatchConCommand(handle_startmovie, SOURCESDK::CS2::CCommandContext(SOURCESDK::CS2::CT_FIRST_SPLITSCREEN_CLIENT,0), SOURCESDK::CS2::CCommand(3,pszArgs));
            } else advancedfx::Warning("AFXERROR: startmovie command not found, wav recording not possible.");
		}
	}
	else
	{
		advancedfx::Message("FAILED");
		advancedfx::Warning("Error: Failed to create directories for \"%s\".\n", m_RecordName.c_str());
	}

}

void CAfxStreams::RecordEnd()
{
	if(m_Recording)
	{
		advancedfx::Message("Finishing recording ... ");
		if (m_StartMovieWavUsed)
		{
            SOURCESDK::CS2::ConCommandHandle handle_endmovie = SOURCESDK::CS2::g_pCVar->FindCommand( "endmovie", false );
            if(handle_endmovie.IsValid()) {
                const char * pszArgs[1] = {"endmovie"};
                SOURCESDK::CS2::g_pCVar->DispatchConCommand(handle_endmovie, SOURCESDK::CS2::CCommandContext(SOURCESDK::CS2::CT_FIRST_SPLITSCREEN_CLIENT,0), SOURCESDK::CS2::CCommand(1,pszArgs));
            } else advancedfx::Warning("AFXERROR: endmovie command not found, stopping the wav recording not possible.");

		}

		if(m_RecordScreen->Enabled) {
            EndCapture();
		}

        SOURCESDK::CS2::Cvar_s * handle_host_framerate = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("host_framerate", false).Get());
        SOURCESDK::CS2::Cvar_s * handle_engine_no_focus_sleep = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("engine_no_focus_sleep", false).Get());
        SOURCESDK::CS2::Cvar_s * handle_r_always_render_all_windows = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("r_always_render_all_windows", false).Get());

        if(m_UsedHostFramerRateValue && handle_host_framerate) {
            handle_host_framerate->m_Value.m_flValue = m_OldValue_host_framerate;
        }

        if(handle_engine_no_focus_sleep) {
            handle_engine_no_focus_sleep->m_Value.m_i32Value = m_OldValue_engine_no_focus_sleep;
        }

        if(handle_r_always_render_all_windows) {
            handle_r_always_render_all_windows->m_Value.m_bValue = m_OldValue_r_always_render_all_windows;
        }

		advancedfx::Message("done.\n");
	}

	m_Recording = false;
}

bool AfxStreams_IsRcording() {
    return g_AfxStreams.GetRecording();
}
const wchar_t * AfxStreams_GetTakeDir() {
    return g_AfxStreams.GetTakeDir();
}

CON_COMMAND(mirv_streams, "Access to streams system.")
{
    int argC = args->ArgC();
    const char * cmd0 = args->ArgV(0);

    if(2 <= argC)
    {
        const char * cmd1 = args->ArgV(1);
		if(0 == _stricmp(cmd1, "record"))
		{
			if(3 <= argC)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "name"))
				{
					if(4 <= argC)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.SetRecordName(cmd3);
						return;
					}

					advancedfx::Message(
						"mirv_streams record name <name> - Set record name to <name>.\n"
						"Current value: %s.\n",
						g_AfxStreams.GetRecordName()
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "start"))
				{
					g_AfxStreams.RecordStart();
					return;
				}
				else
				if(!_stricmp(cmd2, "end"))
				{
					g_AfxStreams.RecordEnd();
					return;
				}
				else
				if(!_stricmp(cmd2, "format"))
				{
					if(4 <= argC)
					{
						char const * cmd3 = args->ArgV(3);

                        if(0 == _stricmp(cmd3, "bmp")) g_AfxStreams.SetFormatBmpNotTga(true);
                        else if(0 == _stricmp(cmd3, "tga")) g_AfxStreams.SetFormatBmpNotTga(false);
                        else advancedfx::Warning("Error: Invalid format %s\n.", cmd3);

						return;
					}

					advancedfx::Message(
						"mirv_streams record format tga|bmp - Set record format to tga or bmp.\n"
						"Current value: %s.\n",
						g_AfxStreams.GetFormatBmpNotTga() ? "bmp" : "tga"
					);
					return;
				}
				else if (0 == _stricmp(cmd2, "screen"))
				{
					advancedfx::CSubCommandArgs subArgs(args, 3);

					g_AfxStreams.Console_RecordScreen(&subArgs);
					return;
				}
				else
				if (!_stricmp(cmd2, "startMovieWav"))
				{
					if (4 <= argC)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.SetStartMovieWav(0 != atoi(cmd3));
						return;
					}

					advancedfx::Message(
						"mirv_streams record startMovieWav 0|1 - Whether to record WAV audio (1) or not (0).\n"
						"Current value: %s.\n",
						g_AfxStreams.GetStartMovieWav() ? "1" : "0"
					);
					return;
				}
				else if (!_stricmp(cmd2, "fps")) {
					if (4 <= argC)
					{
						char const* cmd3 = args->ArgV(3);
						if (0 == _stricmp(cmd3, "default")) {
							g_AfxStreams.SetOverrideFps(false);
							return;
						}
						else if (!StringIsAlphas(cmd3)) {
							g_AfxStreams.SetOverrideFpsValue((float)atof(cmd3));
							g_AfxStreams.SetOverrideFps(true);
							return;
						}
					}

					advancedfx::Message(
						"mirv_streams record fps default|<fValue>\n"
					);
					if (g_AfxStreams.GetOverrideFps()) {
						advancedfx::Message(
							"Current value: %f\n", g_AfxStreams.GetOverrideFpsValue()
						);
					}
					else {
						advancedfx::Message(
							"Current value: default\n"
						);
					}
					return;
				}
			}

			advancedfx::Message(
				"mirv_streams record name [...] - Set/get record name.\n"
				"mirv_streams record start - Begin recording.\n"
				"mirv_streams record end - End recording.\n"
				"mirv_streams record format [...] - Set/get file format.\n"
				"mirv_streams record fps [...] - Allows to override input FPS for games where we can not detect it (not needed for CS:GO).\n"
			);
			advancedfx::Message(
				"mirv_streams record screen [...] - Controls capturing the game content drawn to screen right before being presented.\n"
				"mirv_streams record startMovieWav [...] - Controls WAV audio recording.\n"
			);
			return;
		}
		else if (0 == _stricmp("settings", cmd1))
		{
			advancedfx::CSubCommandArgs subArgs(args, 2);
			advancedfx::CRecordingSettings::Console(&subArgs);
			return;
		}        
    }

	advancedfx::Message(
		"mirv_streams record [...] - Recording control.\n"
	);

	advancedfx::Message(
		"mirv_streams settings [...] - Recording settings.\n"
    );
}

CON_COMMAND(mirv_reshade, "Control ReShade_advancedfx ReShade addon.")
{
    if (!g_ReShadeAdvancedfx.IsConnected()) {
        advancedfx::Warning("AFXERROR: ReShade or ReShade_advancedfx.addon not loaded.\n");
        return;
    }

    static bool bEnableReshade = true;
    int argc = args->ArgC();
    const char * cmd0 = args->ArgV(0);

    if (2 <= argc)
    {
        char const* cmd1 = args->ArgV(1);

        if (0 == _stricmp("enabled", cmd1)) {
            if (3 <= argc) {
                bool bDoEnableReShade = 0 != atoi(args->ArgV(2));
                bEnableReshade = bDoEnableReShade;
                g_SwapchainAfterPresentQueue.push([bDoEnableReShade](ID3D11DeviceContext * pDeviceContext){
                    g_bEnableReShade = bDoEnableReShade;
                    return false;
                });   
                return;
            }

            advancedfx::Message(
                "%s enabled 0|1 - Enable / disable reshade addon.\n"
                "Current value: %s\n"
                , cmd0
                , bEnableReshade ? "1" : "0"
            );
            return;
        }
    }

    advancedfx::Message(
        "%s enabled [...].\n"
        , cmd0
    );
}