#pragma once

#include <windows.h>

BOOL  WINAPI NewSetPixelFormat(__in HDC hdc, __in int format, __in CONST PIXELFORMATDESCRIPTOR * ppfd);