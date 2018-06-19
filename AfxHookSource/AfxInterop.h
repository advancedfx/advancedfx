#pragma once

#ifdef AFX_INTEROP

#include "SourceInterfaces.h"

namespace AfxInterop {

	void DllProcessAttach();

	bool Enabled();

	//
	// Engine thread:

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeFrameStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void BeforeFrameRenderStart();

	/// <remarks>Must be called from engine thread only.</remarks>
	void Shutdown();

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelInitPreEntity(char const* pMapName);

	/// <remarks>Must be called from engine thread only.</remarks>
	void LevelShutdown();

	//
	// Drawing thread:

	/// <remarks>Must be called from drawing thread only.</remarks>
	void DrawingThreadBeforeHud(void);

}

#endif
