#include "stdafx.h"

#include "gdi32Hooks.h"

#include "../AfxSettings.h"

void MbPrintPixelFormatDescriptor(char const * title, PIXELFORMATDESCRIPTOR const * pfd)
{
	char szTmp[1000];
	_snprintf_s(szTmp, _TRUNCATE,
		"ColorBits: %u\nDepthBits: %u\nRedBits: %u, RedShift: %u\nGreenBits: %u, GreenShift: %u\nBlueBits: %u, BlueShift: %u\nAlphaBits: %u, AlphaShift: %u",
		pfd->cColorBits,
		pfd->cDepthBits,
		pfd->cRedBits, pfd->cRedShift,
		pfd->cGreenBits, pfd->cGreenShift,
		pfd->cBlueBits, pfd->cBlueShift,
		pfd->cAlphaBits, pfd->cAlphaShift
	);

	MessageBox(0, szTmp, title, MB_OK|MB_ICONINFORMATION);
}

BOOL WINAPI NewSetPixelFormat(__in HDC hdc, __in int format, __in CONST PIXELFORMATDESCRIPTOR* ppfd);
CAfxImportFuncHook<BOOL(WINAPI*)(HDC, int, CONST PIXELFORMATDESCRIPTOR*)> g_Import_GDI32_SetPixelFormat("SetPixelFormat", NewSetPixelFormat);
CAfxImportFuncHookBase* g_pImport_GDI32_SetPixelFormat = &g_Import_GDI32_SetPixelFormat;
BOOL WINAPI NewSetPixelFormat(__in HDC hdc, __in int format, __in CONST PIXELFORMATDESCRIPTOR * ppfd)
{
	if (!g_AfxSettings.ForceAlpha8_get())
		return SetPixelFormat(hdc, format, ppfd);

	PIXELFORMATDESCRIPTOR *myppfd = const_cast<PIXELFORMATDESCRIPTOR *>(ppfd);

	// we intentionally void the const paradigm here:
	myppfd->cAlphaBits = 8; // request alpha bit planes (generic implementation doesn't support that)
	myppfd->cAlphaShift = 24;

	return g_Import_GDI32_SetPixelFormat.TrueFunc(hdc, ChoosePixelFormat(hdc, myppfd), myppfd);
}