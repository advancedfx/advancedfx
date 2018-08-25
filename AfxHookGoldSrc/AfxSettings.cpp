#include "stdafx.h"

#include "AfxSettings.h"

AfxSettings g_AfxSettings;

AfxSettings::AfxSettings()
{
	LPWSTR *szArglist;
	int nArgs;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	for (int i = 1; i < nArgs; ++i)
	{
		if (!wcscmp(L"-full", szArglist[i]) && nArgs - i > 1)
		{
			m_FullScreen = true;
			++i;
		}
		else if (!wcscmp(L"-window", szArglist[i]) && nArgs - i > 1)
		{
			m_FullScreen = false;
			++i;
		}
		else if (!wcscmp(L"-afxForceAlpha8", szArglist[i]) && nArgs - i > 1)
		{
			m_ForceAlpha8 = 0 != _wtoi(szArglist[i + 1]);
			++i;
		}
		else if (!wcscmp(L"-w", szArglist[i]) && nArgs - i > 1)
		{
			m_Width = _wtoi(szArglist[i + 1]);
			++i;
		}
		else if (!wcscmp(L"-h", szArglist[i]) && nArgs - i > 1)
		{
			m_Height = _wtoi(szArglist[i + 1]);
			++i;
		}
		else if (!wcscmp(L"-afxRenderMode", szArglist[i]) && nArgs - i > 1)
		{
			if (!_wcsicmp(L"standard", szArglist[i + 1]))
			{
				m_RenderMode = RenderMode_Standard;
			}
			else if (!_wcsicmp(L"fBO", szArglist[i + 1]))
			{
				m_RenderMode = RenderMode_FrameBufferObject;
			}
			else if (!_wcsicmp(L"memoryDC", szArglist[i + 1]))
			{
				m_RenderMode = RenderMode_MemoryDC;
			}
			++i;
		}
		else if (!wcscmp(L"-afxOptimizeCaptureVis", szArglist[i]) && nArgs - i > 1)
		{
			m_OptimizeCaptureVis = 0 != _wtoi(szArglist[i + 1]);
			++i;
		}
	}

	LocalFree(szArglist);
}
