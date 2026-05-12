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

void CRenderCommands::EngineThread_EndFrame() {

    // Submit current frame:

    CRenderPassCommands* pRenderPassCommands = m_EngineThreadCommands;
    m_EngineThreadCommands = nullptr;

    {
        std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
        m_CommandsQueue.emplace(pRenderPassCommands);
    }

    // Prepare next frame:

    {
        std::unique_lock<std::mutex> lock(m_ReusableMutex);
        if (!m_Reusable.empty()) {
            m_EngineThreadCommands = m_Reusable.front();
            m_Reusable.pop();
        }
    }

    if (m_EngineThreadCommands == nullptr) m_EngineThreadCommands = new CRenderPassCommands();
}

void CRenderCommands::RenderThread_BeginFrame() {
    m_RenderThread_FrameBegun = true;
    m_CurrentThreadId = GetCurrentThreadId();

    {
        std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
        if (m_CommandsQueue.empty()) m_RenderThreadCommands = nullptr;
        else {
            m_RenderThreadCommands = m_CommandsQueue.front();
            m_CommandsQueue.pop();
        }
    }
    if (m_RenderThreadCommands) {
        auto& beginFrameReliable = m_RenderThreadCommands->BeginReliable;
        while (!beginFrameReliable.Empty()) {
            beginFrameReliable.Front()();
            beginFrameReliable.Pop();
        }
    }
}

CRenderCommands::CRenderPassCommands * CRenderCommands::RenderThread_GetCommands() {
    return m_RenderThreadCommands;
}

bool CRenderCommands::RenderThread_FrameBegun() {
    return m_RenderThread_FrameBegun;
}

void CRenderCommands::RenderThread_EndFrame() {
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

bool CRenderCommands::CurrentThreadIsRenderThread() const {
    return m_RenderThread_FrameBegun &&  GetCurrentThreadId() == m_CurrentThreadId;
}