#include <RenderCommands.h>

CRenderCommands::CRenderCommands() {
    m_EngineThreadCommands = new CRenderPassCommands();
}

CRenderCommands::~CRenderCommands() {
    {
        std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
        while(!m_CommandsQueue.empty()) {
            delete m_CommandsQueue.front();
            m_CommandsQueue.pop();
        }
    }
    {
        std::unique_lock<std::mutex> lock(m_ReusableMutex);
        while(!m_Reusable.empty()) {
            delete m_Reusable.front();
            m_Reusable.pop();
        }
    }    
}

CRenderCommands::CRenderPassCommands & CRenderCommands::EngineThread_GetCommands() {
    return *m_EngineThreadCommands;
}

void CRenderCommands::EngineThread_BeginFrame() {
    if (m_EngineThread_FrameBegun) return;
    m_EngineThread_FrameBegun = true;

    CRenderPassCommands* pRenderPassCommands = m_EngineThreadCommands;
    m_EngineThreadCommands = nullptr;

    if (m_SkipFrames) {
        pRenderPassCommands->SkipFrame = true;
        m_SkipFrames--;
    }
    else {
        pRenderPassCommands->SkipFrame = false;
    }

    {
        std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
        m_CommandsQueue.emplace(pRenderPassCommands);
    }
}

void CRenderCommands::EngineThread_BeforePresent() {
    if (!m_EngineThread_FrameBegun) {
        // Late to the party on the engine thread.
        EngineThread_BeginFrame();
    }
}

void CRenderCommands::EngineThread_AfterPresent(bool presented) {
    {
        std::unique_lock<std::mutex> lock(m_ReusableMutex);
        if (!m_Reusable.empty()) {
            m_EngineThreadCommands = m_Reusable.front();
            m_Reusable.pop();
        }
    }

    if (m_EngineThreadCommands == nullptr) m_EngineThreadCommands = new CRenderPassCommands();

    if (!presented) {
        //m_SkipFrames++;
    }

    m_EngineThread_FrameBegun = false;
    
}

void CRenderCommands::RenderThread_BeginFrame(ID3D11DeviceContext* pContext) {
    if (m_RenderThread_FrameBegun) return;
    m_RenderThread_FrameBegun = true;

    {
        std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
        if (m_CommandsQueue.empty()) m_RenderThreadCommands = nullptr;
        else {
            m_RenderThreadCommands = m_CommandsQueue.front();
            m_CommandsQueue.pop();
        }
    }
    if (m_RenderThreadCommands) {
        m_RenderThreadCommands->Context = pContext;
        if (pContext) {
            pContext->AddRef();
            auto& beginFrameReliable = m_RenderThreadCommands->BeginReliable;
            while (!beginFrameReliable.Empty()) {
                beginFrameReliable.Front()(m_RenderThreadCommands);
                beginFrameReliable.Pop();
            }
        }
    }
}

CRenderCommands::CRenderPassCommands * CRenderCommands::RenderThread_GetCommands() {
    return m_RenderThreadCommands;
}

void CRenderCommands::RenderThread_EndFrame(ID3D11DeviceContext* pContext) {
    if (!m_RenderThread_FrameBegun) {
        // Late to the party on the render thread.
        RenderThread_BeginFrame(pContext);
    }

    if(m_RenderThreadCommands) {
        m_RenderThreadCommands->Finalize();
        {
            std::unique_lock<std::mutex> lock(m_ReusableMutex);
            m_Reusable.emplace(m_RenderThreadCommands);
        }
        m_RenderThreadCommands = nullptr;
    }

    m_RenderThread_FrameBegun = false;
}
