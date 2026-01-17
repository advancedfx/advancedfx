#include "stdafx.h"

#include "gameOverlayRenderer.h"

#include "../AfxDetours.h"
#include "../binutils.h"

#include <Windows.h>

using namespace Afx::BinUtils;

bool GameOverlay_Enable(bool value)
{
	static bool firstRun = true;
	static size_t addrPaintEvent = 0;
	static size_t oldAddrPaintEvent;
	static bool wasEnabled = true;

	if(firstRun)
	{
		firstRun = false;

#ifndef _WIN64
		HMODULE hGameOverlayRenderer = GetModuleHandleA("GameOverlayRenderer.dll");
#else
		HMODULE hGameOverlayRenderer = GetModuleHandleA("GameOverlayRenderer64.dll");
#endif //#ifndef _WIN64

		if(!hGameOverlayRenderer)
			return false;

		size_t addrPaintEventFailedText = 0;
		{
			ImageSectionsReader sections(hGameOverlayRenderer);
			if(sections.Eof())
				return false;

			sections.Next();
			if(sections.Eof())
				return false;

			MemRange baseRange = sections.GetMemRange();
			MemRange result = FindCString(sections.GetMemRange(), "Failed creating paint event\n");

			if(result.IsEmpty())
				return false;

			addrPaintEventFailedText = result.Start;
		}

		if(addrPaintEventFailedText)
		{
			ImageSectionsReader sections(hGameOverlayRenderer);
			if(sections.Eof())
				return false;

			MemRange baseRange = sections.GetMemRange();
#ifndef _WIN64
			MemRange result = FindBytes(baseRange, (char const *)&addrPaintEventFailedText, sizeof(addrPaintEventFailedText));
#else
			MemRange result = FindAddrInt32OffsetRefInContext(baseRange, addrPaintEventFailedText, (int32_t)sizeof(int32_t), "48 8d 0d", nullptr);
#endif //#ifndef _WIN64			
			if(result.IsEmpty())
				return false;

#ifndef _WIN64
			addrPaintEvent = *(size_t *)(result.Start - 0x9);
#else
			addrPaintEvent = result.Start - 0xC + 4 + *(int32_t *)(result.Start - 0xC);
#endif //#ifndef _WIN64			
		}
	}

	if(!addrPaintEvent)
		return false;

	if(wasEnabled == value)
		return true;

	wasEnabled = value;

	if(value)
	{
		*(size_t *)addrPaintEvent = oldAddrPaintEvent;
	}
	else
	{
		oldAddrPaintEvent = *(size_t *)addrPaintEvent;
		*(size_t *)addrPaintEvent = 0;
	}

	return true;
}
