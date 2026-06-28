#pragma once

#include <Windows.h>
#include <cstddef>

bool MirvPanorama_InitStyleProperties(HMODULE panoramaDll);
void MirvPanorama_SetHudPanel(void** hudPanel);
bool Panorama_SetPanelOpacity(void* panel, float value);
bool Panorama_SetPanelVisible(void* panel, bool value);

namespace CS2 {
	namespace PanoramaUIPanel {
		extern ptrdiff_t getAttributeString;
		extern ptrdiff_t setAttributeString;

		extern void** hudPanel;

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
