#ifndef ADVANCEDFX_LOCALE_H
#define ADVANCEDFX_LOCALE_H

typedef unsigned long AdvancedfxLocaleId;

struct AdvancedfxILocale {

	const char* (*GetLanguage)(struct AdvancedfxILocale * This);
	const char* (*GetRegion)(struct AdvancedfxILocale* This);
};

#endif
