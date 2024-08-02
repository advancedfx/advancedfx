#include "Globals.h"

void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookSource2", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}

size_t getAddress(HMODULE dll, char const* pattern)
{
	Afx::BinUtils::ImageSectionsReader sections((HMODULE)dll);
	Afx::BinUtils::MemRange textRange = sections.GetMemRange();
	Afx::BinUtils::MemRange result = FindPatternString(textRange, pattern);
	if (result.IsEmpty()) {
		advancedfx::Warning("Could not find address for pattern: %s\n", pattern);
		return 0;
	} else {
		return result.Start;
	}
};