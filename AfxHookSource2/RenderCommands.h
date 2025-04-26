#pragma once

#include <d3d11.h>

#include <functional>
#include <atomic>
#include <queue>
#include <deque>
#include <mutex>
#include <shared_mutex>

class CRenderCommands {
public:
    CRenderCommands();
    ~CRenderCommands();

    typedef std::function<void()> Fn;
    typedef std::function<void(ID3D11DeviceContext * pDeviceContext)> FnContext;
    typedef std::function<void(ID3D11DeviceContext * pDeviceContext, ID3D11Texture2D * pTexture)> FnContextTexture;
    typedef std::function<void(ID3D11DeviceContext * pDeviceContext, ID3D11RenderTargetView * pTarget)> FnContextTarget;
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
    
    class CRenderPassCommands {
    public:
        ~CRenderPassCommands() {
            Finalize();
        }

        ID3D11DeviceContext * Context = nullptr;

        void Clear() {
            //AfterSmokeDepth.Clear();
            BeforeUi.Clear();
            BeforeUi2.Clear();
            BeforePresent.Clear();
            AfterPresent.Clear();

            Context = nullptr;
        }

        void Finalize(){
            BeginReliable.Clear();
            Clear();
            if(Context) {
                OnAfterPresentOrContextLossReliable();
            } else {
                AfterPresentOrContextLossReliable.Clear();
            }
            while(!FinalizeReliable.Empty()) {
                FinalizeReliable.Front()();
                FinalizeReliable.Pop();
            }
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
                BeforeUi.Front()(Context, pTexture);
                BeforeUi.Pop();
            }
        }

        void OnBeforeUi2(ID3D11RenderTargetView * pTarget) {
            while(!BeforeUi2.Empty()) {
                BeforeUi2.Front()(Context, pTarget);
                BeforeUi2.Pop();
            }
        }        

        void OnBeforePresent(ID3D11Texture2D * pTexture) {
            while(!BeforePresent.Empty()) {
                BeforePresent.Front()(Context, pTexture);
                BeforePresent.Pop();
            }
        }

        void OnAfterPresent() {
            while(!AfterPresent.Empty()) {
                AfterPresent.Front()(Context);
                AfterPresent.Pop();
            }            
        }

        void OnAfterPresentOrContextLossReliable() {    
            while(!AfterPresentOrContextLossReliable.Empty()) {
                AfterPresentOrContextLossReliable.Front()(Context);
                AfterPresentOrContextLossReliable.Pop();
            }
            if(Context) Context->Release();
            Context = nullptr;
        }
    };

    CRenderPassCommands & EngineThread_GetCommands();

    void EngineThread_Finish();

    void EngineThread_Present();

    CRenderPassCommands * RenderThread_GetCommands(ID3D11DeviceContext *pContext);

    void RenderThread_Present();

private:  
    bool m_CommandsQueuedBeforePresent = false;
    std::shared_timed_mutex m_CommandsQueueMutex;
    std::deque<CRenderPassCommands *> m_CommandsQueue;
    std::mutex m_ReusableMutex;
    std::queue<CRenderPassCommands *> m_Reusable;
    CRenderPassCommands * m_EngineThreadCommands = nullptr;
    CRenderPassCommands * m_RenderThreadCommands = nullptr;
    bool m_AquiredCommands = false;
};
 