#include "stdafx.h"

#include "UnityInterop.h"

#include <Windows.h>

namespace UnityInterop {

	static bool m_Enabled = false;
	static int m_Frame = 0;

	void DllProcessAttach() {
		m_Enabled = wcsstr(GetCommandLineW(), L"-afxUnityInterop");
	}

	bool Enabled() {
		return m_Enabled;
	}

}