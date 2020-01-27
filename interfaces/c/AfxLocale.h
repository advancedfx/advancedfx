#ifndef ADVANCEDFX_LOCALE_H
#define ADVANCEDFX_LOCALE_H
#include "AfxTypes.h"



struct AdvancedfxILocaleVtable
{
	struct AdvancedfxIReferenced* (*GetAsReferenced)(struct AdvancedfxILocale* This);

	struct AdvancedfxIInterface* (*GetAsInterface)(struct AdvancedfxILocale* This);

	const char* (*GetLanguage)(struct AdvancedfxILocale* This);

	const char* (*GetRegion)(struct AdvancedfxILocale* This);
};



#endif
