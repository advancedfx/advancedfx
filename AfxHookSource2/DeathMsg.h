#pragma once

#include <windows.h>

void HookDeathMsg(HMODULE clientDll);
void HookPanorama(HMODULE panoramaDll);

struct currentGameCamera {
	double origin[3];
	double angles[3];
	float time;
};

extern currentGameCamera g_CurrentGameCamera;

namespace CS2 {
// https://github.com/danielkrupinski/Osiris
	namespace PanoramaUIPanel {
		extern ptrdiff_t getAttributeString;
		extern ptrdiff_t setAttributeString;

		// TODO: get these from pattern matching
		constexpr ptrdiff_t panelId = 0x10;
		constexpr ptrdiff_t children = 0x28;
		constexpr ptrdiff_t panelStyle = 0x68;
		constexpr ptrdiff_t panelFlags = 0x11c;

		constexpr ptrdiff_t k_EPanelFlag_HasOwnLayoutFile = 0x40;
	}

	namespace PanoramaPanelStyle {
		extern ptrdiff_t setPanelStyleProperty;
	}

	namespace PanoramaUIEngine {
		extern ptrdiff_t makeSymbol;
	}
};
