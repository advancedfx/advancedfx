#pragma once

#include "Globals.h"

struct AfxBasicColor {
	const char* name;
	afxUtils::RGBA value;
};

std::vector<AfxBasicColor> afxBasicColors = {
	{ "red", {255, 0, 0, 255} },
	{ "green", {0, 255, 0, 255} },
	{ "blue", {0, 0, 255, 255} },
	{ "yellow", {255, 255, 0, 255} },
	{ "cyan", {0, 255, 255, 255} },
	{ "magenta", {255, 0, 255, 255} },
	{ "white", {255, 255, 255, 255} },
	{ "black", {0, 0, 0, 255} },
	{ "90black", {0, 0, 0, 230} },
	{ "75black", {0, 0, 0, 191} },
	{ "50black", {0, 0, 0, 128} },
	{ "25black", {0, 0, 0, 64} },
	{ "10black", {0, 0, 0, 25} },
	{ "transparent", {0, 0, 0, 0} },
};
