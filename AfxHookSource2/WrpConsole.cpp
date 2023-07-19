#include "WrpConsole.h"

#include <list>

bool g_bComandsRegisterred = false;



std::list<class SOURCESDK::CS2::CCmd *> & GetCommands() {
    static std::list<class SOURCESDK::CS2::CCmd *> m_Commands;
    return m_Commands;
}

CWrpAddCommand::CWrpAddCommand(class SOURCESDK::CS2::CCmd * command) {
    GetCommands().emplace_back(command);
    if(SOURCESDK::CS2::g_pCVar) {
        SOURCESDK::CS2::g_pCVar->RegisterConCommand(command);
    }
}

void WrpRegisterCommands() {
    if(g_bComandsRegisterred) return;

    if(g_bComandsRegisterred = (nullptr != SOURCESDK::CS2::g_pCVar)) {
        for(auto it =  GetCommands().begin(); it !=  GetCommands().end(); it++) {
            SOURCESDK::CS2::g_pCVar->RegisterConCommand(*it);
        }
    }
}
