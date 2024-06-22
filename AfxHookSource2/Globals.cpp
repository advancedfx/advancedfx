#include "Globals.h"

void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookSource2", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}