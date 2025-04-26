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

void CRenderCommands::EngineThread_Present() {
    CRenderPassCommands* pRenderPassCommands = m_EngineThreadCommands;
    m_EngineThreadCommands = nullptr;

    {
        std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
        m_CommandsQueue.emplace(pRenderPassCommands);
    }

    {
        std::unique_lock<std::mutex> lock(m_ReusableMutex);
        if (!m_Reusable.empty()) m_EngineThreadCommands = m_Reusable.front();
    }

    if(m_EngineThreadCommands == nullptr) m_EngineThreadCommands = new CRenderPassCommands();
}

CRenderCommands::CRenderPassCommands * CRenderCommands::RenderThread_GetCommands(ID3D11DeviceContext *pContext) {
    if(!m_AquiredCommands) {
        {
            std::unique_lock<std::mutex> lock(m_CommandsQueueMutex);
            if (m_CommandsQueue.empty()) m_RenderThreadCommands = nullptr;
            else {
                m_RenderThreadCommands = m_CommandsQueue.front();
                m_CommandsQueue.pop();
            }
        }
        if(m_RenderThreadCommands) {
            m_RenderThreadCommands->Context = pContext;
            if(pContext) {
                auto & beginFrameReliable = m_RenderThreadCommands->BeginReliable;
                while(!beginFrameReliable.Empty()) {
                    beginFrameReliable.Front()(pContext);
                    beginFrameReliable.Pop();
                }
            }
        }
        m_AquiredCommands = true;
    }
    return m_RenderThreadCommands;
}

void CRenderCommands::RenderThread_Present() {
    if(m_RenderThreadCommands) {
        m_RenderThreadCommands->Finalize();
        {
            std::unique_lock<std::mutex> lock(m_ReusableMutex);
            m_Reusable.emplace(m_RenderThreadCommands);
        }
        m_RenderThreadCommands = nullptr;
    }
    m_AquiredCommands = false;
}
