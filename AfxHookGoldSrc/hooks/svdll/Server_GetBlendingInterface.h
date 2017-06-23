#pragma once

#include <windows.h>

/// <returns>new proc</returns>
FARPROC Hook_ServerGetBlendingInterface(FARPROC oldProc);
