#include "stdafx.h"

#include "MirvPanorama.h"

#include "Globals.h"
#include "WrpConsole.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/utlmap.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/utlstring.h"
#include "../shared/binutils.h"

#include "addresses.h"

#include <Windows.h>
#include <stdint.h>
#include <string.h>

static void* g_CStylePropertyOpacity_vtable = 0;
static void* g_CStylePropertyVisible_vtable = 0;

typedef void(__fastcall *g_CPanelStyleSetStyleProperty_t)(void* This, void* property, bool transition);
static g_CPanelStyleSetStyleProperty_t g_CPanelStyleSetStyleProperty = nullptr;

struct StylePropertySymbolMap {
	uint8_t findSymbol(const char* stylePropertyName) {
		if(!symbols) return 0xFF;

		for(int i = 0; i < symbols->numElements; ++i) {
			if(strcmp(symbols->memory[i].key.Get(), stylePropertyName) == 0) return symbols->memory[i].value;
		}

		return 0xFF;
	}

	SOURCESDK::CS2::CUtlMap<SOURCESDK::CS2::CUtlString, uint8_t>* symbols;
} g_PanoramaStylePropertySymbols;

CON_COMMAND(__mirv_panorama_dump_style_symbols, "") {
	auto symbols = g_PanoramaStylePropertySymbols.symbols;
	if(!symbols) return;

	for(int i = 0; i < symbols->numElements; ++i) {
		auto node = symbols->memory[i];
		advancedfx::Message("%i: %s\n", node.value, node.key.Get());
	}
}

struct StylePropertyOpacity {
	void* vtable;
	uint8_t id;
	bool disallowTransition = false;
	u_char pad[0x6];
	float value;

	StylePropertyOpacity() {}

	StylePropertyOpacity(void* vt, uint8_t i, float v)
		: vtable(vt), id(i), value(v) {}
};

struct StylePropertyVisible {
	void* vtable;
	uint8_t id;
	bool disallowTransition = false;
	u_char pad[0x6];
	uint16_t value;

	StylePropertyVisible() {}

	StylePropertyVisible(void* vt, uint8_t i, bool v)
		: vtable(vt), id(i), value(v ? 0x0101 : 0x0001) {}
};

static bool makeOpacityProperty(StylePropertyOpacity* out, float value) {
	auto id = g_PanoramaStylePropertySymbols.findSymbol("opacity");
	if(g_CStylePropertyOpacity_vtable == nullptr || id == 0xFF) return false;

	*out = StylePropertyOpacity { g_CStylePropertyOpacity_vtable, id, value };

	return true;
}

static bool makeVisibleProperty(StylePropertyVisible* out, bool value) {
	auto id = g_PanoramaStylePropertySymbols.findSymbol("visibility");
	if(g_CStylePropertyVisible_vtable == nullptr || id == 0xFF) return false;

	*out = StylePropertyVisible { g_CStylePropertyVisible_vtable, id, value };

	return true;
}

struct CUIPanel {
	bool setOpacity(float value) {
		auto style = (u_char*)(this + CS2::PanoramaUIPanel::panelStyle);

		StylePropertyOpacity styleProp;
		if(!makeOpacityProperty(&styleProp, value)) return false;

		g_CPanelStyleSetStyleProperty(style, &styleProp, true);

		return true;
	}

	bool setVisible(bool value) {
		auto style = (u_char*)(this + CS2::PanoramaUIPanel::panelStyle);

		StylePropertyVisible styleProp;
		if(!makeVisibleProperty(&styleProp, value)) return false;

		g_CPanelStyleSetStyleProperty(style, &styleProp, true);

		return true;
	}
};

namespace CS2 {
	namespace PanoramaUIPanel {
		ptrdiff_t getAttributeString = 0;
		ptrdiff_t setAttributeString = 0;
		void** hudPanel = nullptr;
	}

	namespace PanoramaPanelStyle {
		ptrdiff_t setPanelStyleProperty = 0;
	}

	namespace PanoramaUIEngine {
		ptrdiff_t makeSymbol = 0;
	}
};

void MirvPanorama_SetHudPanel(void** value) {
	CS2::PanoramaUIPanel::hudPanel = value;
}

bool Panorama_SetPanelOpacity(void* panel, float value) {
	if(!panel) return false;
	return ((CUIPanel*)panel)->setOpacity(value);
}

bool Panorama_SetPanelVisible(void* panel, bool value) {
	if(!panel) return false;
	return ((CUIPanel*)panel)->setVisible(value);
}

bool MirvPanorama_InitStyleProperties(HMODULE panoramaDll) {
	{
		g_CStylePropertyOpacity_vtable = (void**)Afx::BinUtils::FindClassVtable(panoramaDll, ".?AVCStylePropertyOpacity@panorama@@", 0, 0);
		if(nullptr == g_CStylePropertyOpacity_vtable) {
			ErrorBox(MkErrStr(__FILE__, __LINE__));
			return false;
		}
	}

	{
		g_CStylePropertyVisible_vtable = (void**)Afx::BinUtils::FindClassVtable(panoramaDll, ".?AVCStylePropertyVisible@panorama@@", 0, 0);
		if(nullptr == g_CStylePropertyVisible_vtable) {
			ErrorBox(MkErrStr(__FILE__, __LINE__));
			return false;
		}
	}

	{
		auto addr = getAddress(panoramaDll, "7F ?? 48 8D 05 ?? ?? ?? ?? 48 83 C4 ?? C3");
		if(0 == addr) {
			ErrorBox(MkErrStr(__FILE__, __LINE__));
			return false;
		}

		auto out = addr + 7 + *(int32_t*)(addr + 5);
		g_PanoramaStylePropertySymbols.symbols = (SOURCESDK::CS2::CUtlMap<SOURCESDK::CS2::CUtlString, uint8_t>*)(out + 8 + 2);
	}

	{
		auto addr = getAddress(panoramaDll, "E8 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 45 ?? EB");
		if(!addr) {
			ErrorBox(MkErrStr(__FILE__, __LINE__));
			return false;
		}

		g_CPanelStyleSetStyleProperty = (g_CPanelStyleSetStyleProperty_t)(addr + 5 + *(int32_t*)(addr + 1));
	}

	return true;
}
