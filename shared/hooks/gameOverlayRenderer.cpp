#include "stdafx.h"

#include "gameOverlayRenderer.h"

#include "../AfxDetours.h"
#include "../binutils.h"

#include <Windows.h>

using namespace Afx::BinUtils;

bool GameOverlay_Enable(bool value)
{
	static bool firstRun = true;
	static DWORD addrPaintEvent = 0;
	static DWORD oldAddrPaintEvent;
	static bool wasEnabled = true;

	if(firstRun)
	{
		firstRun = false;

		HMODULE hGameOverlayRenderer = GetModuleHandleA("GameOverlayRenderer.dll");

		if(!hGameOverlayRenderer)
			return false;

		DWORD addrPaintEventFailedText = 0;
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
			MemRange result = FindBytes(baseRange, (char const *)&addrPaintEventFailedText, sizeof(addrPaintEventFailedText));
			if(result.IsEmpty())
				return false;

			addrPaintEvent = *(DWORD *)(result.Start - 0x9);
		}
	}

	if(!addrPaintEvent)
		return false;

	if(wasEnabled == value)
		return true;

	wasEnabled = value;

	if(value)
	{
		*(DWORD *)addrPaintEvent = oldAddrPaintEvent;
	}
	else
	{
		oldAddrPaintEvent = *(DWORD *)addrPaintEvent;
		*(DWORD *)addrPaintEvent = 0;
	}

	return true;
}
