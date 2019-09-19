#ifndef ADVANCEDFX_LOCALE_H
#define ADVANCEDFX_LOCALE_H
#include "AfxTypes.h"

typedef unsigned long AdvancedfxLocaleId;


struct AdvancedfxILocaleVtable
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxILibrary* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxILibrary* This);

	const char* (*GetLanguage)(struct AdvancedfxILocale* This);

	const char* (*GetRegion)(struct AdvancedfxILocale* This);
};

struct AdvancedfxILocale
{
	AdvancedfxILocaleVtable* Vtable;
};

#endif
