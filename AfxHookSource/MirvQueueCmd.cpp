#include "stdafx.h"

#include "MirvQueueCmd.h"

#include "WrpConsole.h"
#include "WrpVEngineClient.h"

#include <queue>

extern WrpVEngineClient * g_VEngineClient;

std::queue<std::function<void()>> g_MirvQueueCmd;

void MirvQueueCmd(std::function<void()> fn) {
    if(g_VEngineClient) {
        g_MirvQueueCmd.push(fn);
        g_VEngineClient->ExecuteClientCmd("__mirv_queue");
    } else {
        fn();
    }
}

CON_COMMAND(__mirv_queue, "Internal queueing command.")
{
    if(!g_MirvQueueCmd.empty()) {
        g_MirvQueueCmd.front()();
        g_MirvQueueCmd.pop();
    }
}
