#include "stdafx.h"

#include "csgo_vphysics.h"

#include <shared/binutils.h>
#include <shared/AfxDetours.h>

double g_csgo_vphsyiscs_frametime_lowerlimit;

// Description:
//
// We basically get at the limit in IPhysicsEnvironment::Simulate using a pattern search for the checks on the function argument.
bool Hook_csgo_vphsyics_frametime_lowerlimit(void)
{
	static bool firstRun = true;
	static bool firstResult = false;

	if (!firstRun) return firstResult;
	firstRun = false;

	HMODULE h_vphysics = GetModuleHandleA("vphysics.dll");
	if (h_vphysics)
	{
		Afx::BinUtils::ImageSectionsReader sections(h_vphysics);
		if (!sections.Eof())
		{
			Afx::BinUtils::MemRange search = sections.GetMemRange();

			Afx::BinUtils::MemRange result;

			int numResults = 0;

			if (!search.IsEmpty())
			{
				result = FindPatternString(search, "F3 0F 10 05 ?? ?? ?? ?? F3 0F 10 4D 08 0F 2F C1 0F 82 ?? ?? ?? ?? 0F 5A C1 66 0F 2F 05 ?? ?? ?? ??");

				if (!result.IsEmpty())
				{
					double ** constAddr = (double **)(result.End - 4);

					// 1. store initial value:
					g_csgo_vphsyiscs_frametime_lowerlimit = **constAddr;

					// 2. make it point use our variable instead:
					MdtMemBlockInfos mbis;
					MdtMemAccessBegin(constAddr, 4, &mbis);

					*constAddr = &g_csgo_vphsyiscs_frametime_lowerlimit;

					MdtMemAccessEnd(&mbis);

					firstResult = true;
				}
			}
		}
	}

	return firstResult;
}

void csgo_vphysics_SetMaxFps(double value)
{
	value = value + 1;
	g_csgo_vphsyiscs_frametime_lowerlimit = value ? 1.0 / value : 0.0;
}

double csgo_vphysics_GetMaxFps(void)
{
	return g_csgo_vphsyiscs_frametime_lowerlimit ? (1.0 / g_csgo_vphsyiscs_frametime_lowerlimit) - 1 : 0;
}
