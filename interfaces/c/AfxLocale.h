#ifndef ADVANCEDFX_LOCALE_H
#define ADVANCEDFX_LOCALE_H
#include "AfxTypes.h"

typedef unsigned long AdvancedfxLocaleId;


struct AdvancedfxILocaleVtable
{
	//
	// Implement AdvancedfxIReferenceVtable:

	void(*Delete)(struct AdvancedfxILocale* This);

	//
	// Own:

	const char* (*GetLanguage)(struct AdvancedfxILocale* This);

	const char* (*GetRegion)(struct AdvancedfxILocale* This);
};

struct AdvancedfxILocale
{
	//
	// Implement AdvancedfxIReference:

	size_t RefCountValid;
	struct AdvancedfxILocaleVtable* Vtable;

	//
	// Own:
};

#endif
