#pragma once

#include <d3d11.h>

#include <functional>
#include <atomic>
#include <queue>
#include <mutex>

class IRenderPassCommands {
public:
};

class CRenderCommands {
public:
    CRenderCommands();
    ~CRenderCommands();

    typedef std::function<void()> Fn;
    typedef std::function<void(ID3D11DeviceContext * pDeviceContext)> FnContext;
    typedef std::function<void(ID3D11DeviceContext * pDeviceContext, ID3D11Texture2D * pTexture)> FnContextTexture;
    typedef std::function<void(ID3D11DeviceContext * pDeviceContext, ID3D11RenderTargetView * pTarget)> FnContextTarget;

    template<class T> class CQueue {
    public:
        ~CQueue() {
            Clear();
        }

        void Clear() {
           while(!Empty()) Pop();
        }

        void Push(T &&value) {
            m_Queue.push(value);
        }

        bool Empty() {
            return m_Queue.empty();
        }

        T & Front() {
            return m_Queue.front();
        }

        void Pop() {
            m_Queue.pop();
        }

    private:
        std::queue<T> m_Queue;
    };
    
    class CRenderPassCommands :public IRenderPassCommands {
    public:
        ~CRenderPassCommands() {
            Finalize();
        }

        ID3D11DeviceContext * Context = nullptr;

        void Finalize(){
            BeginReliable.Clear();
            BeforeUi.Clear();
            BeforeUi2.Clear();
            BeforePresent.Clear();
            AfterPresent.Clear();

            OnAfterPresentOrContextLossReliable();

            while(!FinalizeReliable.Empty()) {
                FinalizeReliable.Front()();
                FinalizeReliable.Pop();
            }
        }

        CQueue<Fn> BeginReliable;

        CQueue<FnContextTexture> BeforeUi;
        
        CQueue<FnContextTarget> BeforeUi2;

        CQueue<FnContextTexture> BeforePresent;

        CQueue<FnContext> AfterPresent;

        CQueue<FnContext> AfterPresentOrContextLossReliable;

        CQueue<Fn> FinalizeReliable;

        void OnBeforeUi(ID3D11Texture2D * pTexture) {
            if(Context && pTexture) {
                while(!BeforeUi.Empty()) {
                    BeforeUi.Front()(Context, pTexture);
                    BeforeUi.Pop();
                }
            } else {
                BeforeUi.Clear();
            }
        }

        void OnBeforeUi2(ID3D11RenderTargetView * pTarget) {
            if(Context && pTarget) {
                while(!BeforeUi2.Empty()) {
                    BeforeUi2.Front()(Context, pTarget);
                    BeforeUi2.Pop();
                }
            } else {
                BeforeUi2.Clear();
            }
        }        
        
        void OnBeforePresent(ID3D11Texture2D * pTexture) {
            if(Context && pTexture) {
                while(!BeforePresent.Empty()) {
                    BeforePresent.Front()(Context, pTexture);
                    BeforePresent.Pop();
                }
            } else {
                BeforePresent.Clear();
            }
        }

        void OnAfterPresent() {
            if(Context) {
                while(!AfterPresent.Empty()) {
                    AfterPresent.Front()(Context);
                    AfterPresent.Pop();
                }
            } else {
                AfterPresent.Clear();
            }
        }

        void OnAfterPresentOrContextLossReliable() {
            if(Context) {
                while(!AfterPresentOrContextLossReliable.Empty()) {
                    AfterPresentOrContextLossReliable.Front()(Context);
                    AfterPresentOrContextLossReliable.Pop();
                }
            } else {
                AfterPresentOrContextLossReliable.Clear();
            }
            SetContext(nullptr);
        }

        void SetContext(ID3D11DeviceContext* pContext) {
            if(Context) Context->Release();
            Context = pContext;
            if(pContext) pContext->AddRef();
        }

        ID3D11DeviceContext* GetContext() const {
            return Context;
        }        
    };

    CRenderPassCommands & EngineThread_GetCommands();
   
    void EngineThread_EndFrame();

    CRenderPassCommands * RenderThread_GetCommands();

    bool RenderThread_FrameBegun();

    void RenderThread_BeginFrame();

    void RenderThread_EndFrame();

    void RenderThread_SetContext(ID3D11DeviceContext* pContext) {
        if(m_RenderThreadCommands) m_RenderThreadCommands->SetContext(pContext);
    }

    ID3D11DeviceContext* RenderThread_GetContext() const {
        if(m_RenderThreadCommands) return m_RenderThreadCommands->GetContext();
        return nullptr;
    }

    bool CurrentThreadIsRenderThread() const;

private:  
    std::mutex m_CommandsQueueMutex;
    std::queue<CRenderPassCommands *> m_CommandsQueue;
    bool m_QueuedCommands = false;
    std::mutex m_ReusableMutex;
    std::queue<CRenderPassCommands *> m_Reusable;
    CRenderPassCommands * m_EngineThreadCommands = nullptr;
    CRenderPassCommands * m_RenderThreadCommands = nullptr;
    bool m_RenderThread_FrameBegun = false;
    DWORD m_CurrentThreadId=0;
};
 