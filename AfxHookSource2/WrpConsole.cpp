#include "WrpConsole.h"

#include <list>

bool g_bComandsRegisterred = false;
std::list<class SOURCESDK::CS2::CCmd *> m_Commands;

CWrpAddCommand::CWrpAddCommand(class SOURCESDK::CS2::CCmd * command) {
    m_Commands.emplace_back(command);
    if(SOURCESDK::CS2::g_pCVar) {
        SOURCESDK::CS2::g_pCVar->RegisterConCommand(command);
    }
}

void WrpRegisterCommands() {
    if(g_bComandsRegisterred) return;

    if(g_bComandsRegisterred = (nullptr != SOURCESDK::CS2::g_pCVar)) {
        for(auto it = m_Commands.begin(); it != m_Commands.end(); it++) {
            SOURCESDK::CS2::g_pCVar->RegisterConCommand(*it);
        }
    }
}
