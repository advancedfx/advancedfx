#include <RenderCommands.h>

CRenderCommands::CRenderCommands() {
    m_EngineThreadCommands = new CRenderPassCommands();
    {
        std::unique_lock<std::shared_timed_mutex> lock(m_CommandsQueueMutex);
        m_CommandsQueue.emplace_back(m_EngineThreadCommands);
    }
}

CRenderCommands::~CRenderCommands() {
    {
        std::unique_lock<std::shared_timed_mutex> lock(m_CommandsQueueMutex);
        while(!m_CommandsQueue.empty()) {
            delete *(m_CommandsQueue.begin());
            m_CommandsQueue.pop_front();
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

void CRenderCommands::EngineThread_Finish() {
    CRenderPassCommands * renderPassCommands = nullptr;
    if(m_CommandsQueuedBeforePresent) {
        std::unique_lock<std::shared_timed_mutex> lock(m_CommandsQueueMutex);

            renderPassCommands = m_CommandsQueue.back();
            m_CommandsQueue.pop_back();
        } 
        
    if(renderPassCommands) {
        renderPassCommands->Clear();
        m_EngineThreadCommands = renderPassCommands;
    } else {
        std::unique_lock<std::mutex> lock(m_ReusableMutex);
        if(!m_Reusable.empty()) m_EngineThreadCommands = m_Reusable.front();
    }

    if(m_EngineThreadCommands == nullptr) m_EngineThreadCommands = new CRenderPassCommands();

    {
        std::unique_lock<std::shared_timed_mutex> lock(m_CommandsQueueMutex);
        m_CommandsQueue.emplace_back(m_EngineThreadCommands);
    }

    m_CommandsQueuedBeforePresent = true;
}

void CRenderCommands::EngineThread_Present() {
    m_CommandsQueuedBeforePresent = false;
}


CRenderCommands::CRenderPassCommands * CRenderCommands::RenderThread_GetCommands(ID3D11DeviceContext *pContext) {
    if(!m_AquiredCommands) {
        {
            std::shared_lock<std::shared_timed_mutex> lock(m_CommandsQueueMutex);
            m_RenderThreadCommands = m_CommandsQueue.empty()?nullptr:m_CommandsQueue.front();
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
    CRenderPassCommands * renderPassCommand = nullptr;
    {
        std::unique_lock<std::shared_timed_mutex> lock(m_CommandsQueueMutex);
        if(!m_CommandsQueue.empty()) {
            renderPassCommand = m_CommandsQueue.front();
            m_CommandsQueue.pop_front();
        }
    }
    if(renderPassCommand) {
        renderPassCommand->Finalize();
        {
            std::unique_lock<std::mutex> lock(m_ReusableMutex);
            m_Reusable.emplace(renderPassCommand);
        }
    }

    m_RenderThreadCommands = nullptr;
    m_AquiredCommands = false;
}
