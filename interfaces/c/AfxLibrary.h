#ifndef ADVANCEDFX_LIBARY_H
#define ADVANCEDFX_LIBARY_H
#include "AfxTypes.h"
#include "AfxLocale.h"

// AdvancedfxFactoryFn: ADVANCEDFX_ILIBRARY_UUID, AdvancedfxILocale -> AdvancedfxILibrary

#define ADVANCEDFX_ILIBRARY_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x3BBBD18B,0x0100,0x4761,0xA588,0x9D,0xF8,0x8F,0xF0,0x52,0xAE)

struct AdvancedfxILibrary
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxILibrary* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxILibrary* This);

	AdvancedfxUuid(*GetUuid)(struct AdvancedfxILibrary * This);

	AdvancedfxVersion (*GetVersion)(struct AdvancedfxILibrary* This);

	const char* (*GetLabelUtf8)(struct AdvancedfxILibrary* This);
};

#endif
