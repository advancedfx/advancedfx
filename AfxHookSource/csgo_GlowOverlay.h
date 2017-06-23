#pragma once

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-02-09 dominik.matrixstorm.com
//
// First changes:
// 2017-02-09 dominik.matrixstorm.com

class CAfxRenderViewStream;

class ICsgoCGlowOverlayFix abstract
{
public:
	/// <summary>Call this when the Main view is about to be rendered.</summary>
	virtual void OnMainViewRenderBegin() = 0;

	/// <summary>Call this before a stream is about to be rendered, assumes that MainView has been rendered first!</summary>
	virtual void OnStreamRenderViewBegin(CAfxRenderViewStream * idPtr) = 0;

	/// <summary>Call this, when the stream is not going to be rendered again in reasonable time or is removed.</summary>
	virtual void OnStreamFinished(CAfxRenderViewStream * idPtr) = 0;

};

ICsgoCGlowOverlayFix * GetCsgoCGlowOverlayFix(void);
