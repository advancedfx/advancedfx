#pragma once

#include <windows.h>

/// <returns>new proc</returns>
FARPROC Hook_GiveFnptrsToDll(FARPROC oldProc);
