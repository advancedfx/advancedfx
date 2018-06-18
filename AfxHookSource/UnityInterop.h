#pragma once

#ifdef AFX_UNITYINTEROP

#include "SourceInterfaces.h"

namespace UnityInterop {

	void DllProcessAttach();

	void BeforeFrameStart();

	void BeforeFrameRenderStart();

	void Shutdown();

	void LevelInitPreEntity(char const* pMapName);

	void LevelShutdown();

	bool Enabled();

}

#endif