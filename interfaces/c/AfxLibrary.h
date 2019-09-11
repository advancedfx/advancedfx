#ifndef ADVANCEDFX_LIBARY_H
#define ADVANCEDFX_LIBARY_H
#include "AfxTypes.h"

// AdvancedfxFactoryFn: AdvancedfxILibraryManager -> AdvancedfxILibrary

#define ADVANCEDFX_ILIBRARY_DEFINE_UUID(name) ADVANCEDX_DEFINE_UUID(0x3BBBD18B,0x0100,0x4761,0xA588,0x9D,0xF8,0x8F,0xF0,0x52,0xAE)

struct AdvancedfxILibraryManager {

	AdvancedfxLocaleId(*GetLocale)(void);
};

struct AdvancedfxILibrary {

	AdvancedfxUuid(*GetUuid)(void);

	AdvancedfxVersion (*GetVersion)(void);

	const char* (*GetLabelUtf8)(void);
};

#endif
