#pragma once

class AfxSettings
{
public:
	enum RenderMode
	{
		RenderMode_Standard = 0,
		RenderMode_FrameBufferObject = 1,
		RenderMode_MemoryDC = 2
	};

	AfxSettings();

	bool FullScreen_get(void)
	{
		return m_FullScreen;
	}

	bool ForceAlpha8_get(void)
	{
		return m_ForceAlpha8;
	}

	int Height_get(void)
	{
		return m_Height;
	}

	int Width_get(void)
	{
		return m_Width;
	}

	/// <summary> Desired render mode. </summary>
	RenderMode RenderMode_get(void)
	{
		return m_RenderMode;
	}

	/// <summary> Whether to optimize window visibility for capturing (to 
	/// make as much pixels visible as possible to prevent undefined pixel
	/// content. </summary>
	bool OptimizeCaptureVis_get(void)
	{
		return m_OptimizeCaptureVis;
	}

private:
	bool m_FullScreen = false;
	bool m_ForceAlpha8 = false;
	int m_Width = 1280;
	int m_Height = 720;
	RenderMode m_RenderMode = RenderMode_Standard;
	bool m_OptimizeCaptureVis = true;
};

extern AfxSettings g_AfxSettings;
