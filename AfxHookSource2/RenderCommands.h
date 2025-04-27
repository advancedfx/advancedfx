#pragma once

#include <d3d11.h>

#include <functional>
#include <atomic>
#include <queue>
#include <mutex>

class IRenderPassCommands {
public:
    virtual ID3D11DeviceContext* GetContext() = 0;
    virtual bool GetSkipFrame() = 0;
};

class CRenderCommands {
public:
    CRenderCommands();
    ~CRenderCommands();

    typedef std::function<void()> Fn;
    typedef std::function<void(IRenderPassCommands* pRenderPassCommands)> FnContext;
    typedef std::function<void(IRenderPassCommands* pRenderPassCommands, ID3D11Texture2D * pTexture)> FnContextTexture;
    typedef std::function<void(IRenderPassCommands* pRenderPassCommands, ID3D11RenderTargetView * pTarget)> FnContextTarget;
    //typedef std::function<void(ID3D11DeviceContext * pDeviceContext, ID3D11DepthStencilView * pDepthStencil)> FnContextDepthStencil;

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

        virtual ID3D11DeviceContext* GetContext() {
            return Context;
        }
        virtual bool GetSkipFrame() {
            return SkipFrame;
        }

        ID3D11DeviceContext * Context = nullptr;
        bool SkipFrame = false;

        void Clear() {
            //AfterSmokeDepth.Clear();
            BeforeUi.Clear();
            BeforeUi2.Clear();
            BeforePresent.Clear();
            AfterPresent.Clear();
        }

        void Finalize(){
            BeginReliable.Clear();
            Clear();
            if(Context) {
                OnAfterPresentOrContextLossReliable();
                Context = nullptr;
            } else {
                AfterPresentOrContextLossReliable.Clear();
            }
            while(!FinalizeReliable.Empty()) {
                FinalizeReliable.Front()();
                FinalizeReliable.Pop();
            }
            SkipFrame = false;
        }

        CQueue<FnContext> BeginReliable;

        //CQueue<FnContextDepthStencil> AfterSmokeDepth;

        CQueue<FnContextTexture> BeforeUi;
        
        CQueue<FnContextTarget> BeforeUi2;

        CQueue<FnContextTexture> BeforePresent;

        CQueue<FnContext> AfterPresent;

        /// This might be called from a different thread, but should be safe with regards to concurrency on the device context passsed.
        CQueue<FnContext> AfterPresentOrContextLossReliable;

        CQueue<Fn> FinalizeReliable;

        void OnBeforeUi(ID3D11Texture2D * pTexture) {
            while(!BeforeUi.Empty()) {
                BeforeUi.Front()(this, pTexture);
                BeforeUi.Pop();
            }
        }

        void OnBeforeUi2(ID3D11RenderTargetView * pTarget) {
            while(!BeforeUi2.Empty()) {
                BeforeUi2.Front()(this, pTarget);
                BeforeUi2.Pop();
            }
        }        

        void OnBeforePresent(ID3D11Texture2D * pTexture) {
            while(!BeforePresent.Empty()) {
                BeforePresent.Front()(this, pTexture);
                BeforePresent.Pop();
            }
        }

        void OnAfterPresent() {
            while(!AfterPresent.Empty()) {
                AfterPresent.Front()(this);
                AfterPresent.Pop();
            }            
        }

        void OnAfterPresentOrContextLossReliable() {    
            while(!AfterPresentOrContextLossReliable.Empty()) {
                AfterPresentOrContextLossReliable.Front()(this);
                AfterPresentOrContextLossReliable.Pop();
            }
            if(Context) Context->Release();
            Context = nullptr;
        }
    };

    CRenderPassCommands & EngineThread_GetCommands();

    void EngineThread_BeginFrame();

    void EngineThread_BeforePresent();
    void EngineThread_AfterPresent(bool presented);

    void RenderThread_BeginFrame(ID3D11DeviceContext* pContext);

    CRenderPassCommands * RenderThread_GetCommands();

    void RenderThread_EndFrame(ID3D11DeviceContext* pContext);

private:  
    std::mutex m_CommandsQueueMutex;
    std::queue<CRenderPassCommands *> m_CommandsQueue;
    bool m_QueuedCommands = false;
    std::mutex m_ReusableMutex;
    std::queue<CRenderPassCommands *> m_Reusable;
    CRenderPassCommands * m_EngineThreadCommands = nullptr;
    CRenderPassCommands * m_RenderThreadCommands = nullptr;
    bool m_EngineThread_FrameBegun = false;
    bool m_RenderThread_FrameBegun = false;
    int m_SkipFrames = 0;
};
 