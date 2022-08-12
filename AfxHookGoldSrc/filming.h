#pragma once

#include <windows.h>

#include <gl\gl.h>

#include <list>
#include <string>

#include <shared/EasySampler.h>
#include "film_sound.h"
#include "mdt_media.h"
#include "supportrender.h"

#include <shared/CamPath.h>
#include <shared/AfxOutStreams.h>

void LinearizeFloatDepthBuffer(GLfloat *pBuffer, unsigned int count, GLdouble zNear, GLdouble zFar);
void InverseFloatDepthBuffer(GLfloat *pBuffer, unsigned int count, GLdouble zNear, GLdouble zFar);
void LogarithmizeDepthBuffer(GLfloat *pBuffer, unsigned int count, GLdouble zNear, GLdouble zFar);
void DebugDepthBuffer(GLfloat *pBuffer, unsigned int count);
void SliceDepthBuffer(GLfloat *pBuffer, unsigned int count, GLfloat sliceLo, GLfloat sliceHi);

// Constraints: 
// - assumes the GLfloat buffer to contain values in [0.0f,1.0f] v
// - assumes GLfloat to conform with IEEE 754-2008 binary32
// - componentBytes \in 1,2,3
void GLfloatArrayToXByteArray(GLfloat *pBuffer, unsigned int width, unsigned int height, unsigned char componentBytes);


enum FILMING_BUFFER { FB_COLOR, FB_DEPTH, FB_ALPHA };
enum FILMING_DEPTHFN { FD_INV, FD_LINEAR, FD_LOG };


class FilmingStream :
	private IFramePrinter,
	private IFloatFramePrinter
{
public:
	/// <param name="sampleDuration">&lt;= 0: no sampling, sample duration (1/sps) otherwise</param>
	/// <remarks>Depth buffers can't be sampled atm.</remarks>
	FilmingStream(
		wchar_t const * takePath, wchar_t const * name,
		FILMING_BUFFER buffer,
		double samplingFrameDuration,
		bool TASMode,
		int x, int y, int width, int height,
		const wchar_t * ffMpegOptions = nullptr
	);
	~FilmingStream();

	void Capture(double time, CMdt_Media_RAWGLPIC * usePic, float spsHint);

private:
	bool m_Bmp;
	FILMING_BUFFER m_Buffer;
	unsigned char m_BytesPerPixel;
	bool m_DepthDebug;
	FILMING_DEPTHFN m_DepthFn;
	float m_DepthSliceLo;
	float m_DepthSliceHi;
	bool m_DirCreated;
	int m_FrameCount;
	GLenum m_GlBuffer;
	GLenum m_GlType;
	int m_Height;
	std::wstring m_Path;
	EasyByteSampler * m_Sampler;
	EasyFloatSampler * m_SamplerFloat;
	int m_Pitch;
	int m_Width;
	int m_X;
	int m_Y;
	bool m_TASMode;
	CMdt_Media_RAWGLPIC m_PreviousFrame;
	double m_NextFrameIsAt;
	advancedfx::COutFFMPEGVideoStream* m_FfmpegOutStream = nullptr;
	std::wstring m_FfmpegOptions;

	void WriteFrame(CMdt_Media_RAWGLPIC& frame, double time);

	/// <summary>Implements IFramePrinter.</summary>
	virtual void Print(unsigned char const * data);

	/// <summary>Implements IFloatFramePrinter.</summary>
	virtual void Print(float const * data);

	virtual void PrintExr(float const* data);
};


// Filming /////////////////////////////////////////////////////////////////////


class Filming
{
public:
	enum DRAW_RESULT { DR_NORMAL, DR_HIDE, DR_MASK };
	enum STEREO_STATE { STS_LEFT, STS_RIGHT };
	enum HUD_REQUEST_STATE { HUDRQ_NORMAL,HUDRQ_CAPTURE_COLOR,HUDRQ_CAPTURE_ALPHA };
	enum MATTE_STAGE { MS_ALL, MS_WORLD, MS_ENTITY };
	enum MATTE_METHOD { MM_KEY, MM_ALPHA };

	Filming();
	~Filming();

	void FovOverride(double value);
	void FovDefault();
	
	void RollOverride(double value);
	void RollDefault();

	// used in OpenGl32Hooks.cpp
	void FullClear();

	CamPath * GetCamPath()
	{
		return &m_CamPath;
	}

	MATTE_METHOD GetMatteMethod();
	CFilmSound * GetFilmSound() { return &_FilmSound; }

	void On_CL_Disconnect(void);

	void SupplySupportRenderer(CHlaeSupportRender *pSupportRender)
	{
		_pSupportRender = pSupportRender;
	}

	DRAW_RESULT shouldDraw(GLenum mode);
	void Start();
	void Stop();
	bool recordBuffers(HDC hSwapHDC,BOOL *bSwapRes);	// call to record from the currently selected buffers, returns true if it already swapped itself, in this case also bSwapRes is the result of SwapBuffers

	void setScreenSize(GLint w, GLint h);

	bool isFilming() { return (m_iFilmingState != FS_INACTIVE); }
	bool isFinished() { return !_bRecordBuffers_FirstCall; }
	bool checkClear(GLbitfield mask);

	Filming::DRAW_RESULT doWireframe(GLenum mode);

	::std::list<int> matt_entities_ids;

	// WH fx:
	void DoWorldFxBegin(GLenum mode); // begin and end have to maintain the order
	void DoWorldFxEnd(); // .

	// lightmap fx:
	void DoWorldFx2(GLenum mode);

	float m_MatteColour[3] = {0,0,0};

	void setMatteColour(float r, float g, float b)
	{
		m_MatteColour[0] = r;
		m_MatteColour[1] = g;
		m_MatteColour[2] = b;

		bRequestingMatteTextUpdate=true;
	}

	void setWhTintColor(float r, float g, float b);

	//
	HUD_REQUEST_STATE giveHudRqState();

	MATTE_STAGE GetMatteStage();

	bool GetSimulate2() { return _bSimulate2; }

	bool bRequestingMatteTextUpdate = false;

	double LastCameraOrigin[3] = { 0,0,0 };
	double LastCameraAngles[3] = { 0,0,0 };
	double LastCameraFov = 90;

	bool m_HandleZoomEnabled = false;
	float m_HandleZoomMinUnzoomedFov = 90.0f;

	void GetCameraOfs(float &right, float &up, float &forward); // will copy the current camera ofs to the supplied addresses
	void GetCameraAngs(float& pitch, float& yaw, float& roll);
	float GetStereoOffset(); // returns current stereoofs
	bool bEnableStereoMode();
	STEREO_STATE GetStereoState();

	void OnHudBeginEvent(); // called by Hud Begin tour
	bool OnHudEndEvent(); // called by Hud End tour, if pDoLoop is true the toor will cause an loop, otherwise it will continue normal HL code operation


	void SetCameraOfs(float right, float up, float forward); // you can set an static cameraofs here, however during stereomode it should be 0
	void SetCameraAngs(float pitch, float yaw, float roll);
	void SetStereoOfs(float left_and_rightofs); // will be used in stereo mode to displace the camera left and right, suggested values are between 1.0 - 1.4, value should be positive, otherewise you would switch left and right cam

	void OnR_RenderView(float vieworg[3], float viewangles[3], float & fov);

	double GetDebugClientTime();

	void SupplyZClipping(GLdouble zNear, GLdouble zFar);

	GLdouble GetZNear() { return m_ZNear; }
	GLdouble GetZFar() { return m_ZFar; }

	void EnableDebugCapture(bool bEnable) {
		m_DebugCapture = bEnable;
	}
	void DoCanDebugCapture(void);


private:
	enum FILMING_STATE { FS_INACTIVE, FS_STARTING, FS_ACTIVE };

	CamPath m_CamPath;
	bool m_CaptureEarly = false;
	bool m_DebugCapture = false;
	bool m_EnableStereoMode = false;
	bool m_FovOverride = false;
	bool m_RollOverride = false;
	double m_FovValue = 90.0;
	double m_RollValue = 0.0;
	int m_Height = 480;
	unsigned int m_HostFrameCount = 0;
	bool m_HudDrawnInFrame = false;
	unsigned int m_LastCamFrameMid = 0;
	unsigned int m_LastCamFrameLeft = 0;
	unsigned int m_LastCamFrameRight = 0;
	MATTE_METHOD m_MatteMethod = MM_KEY;
	double m_StartClientTime = 0;
	DWORD m_StartTickCount = 0;
	std::wstring m_TakeDir;
	int m_Width = 640;
	GLdouble m_ZFar = 1;
	GLdouble m_ZNear = 0;
	float m_fps = 0;
	double m_time = 0;
	double m_LastHostTime = 0;
	bool m_TASMode = false;


	CHlaeSupportRender *_pSupportRender = nullptr;

	CFilmSound _FilmSound; // our sound filming class
	bool _bExportingSound = false;

	unsigned int m_nFrames = 0;

	CMdt_Media_RAWGLPIC m_GlRawPic;

	MATTE_STAGE m_iMatteStage = MS_ALL;

	FILMING_STATE m_iFilmingState = FS_INACTIVE;

	bool m_bInWireframe = false;
	GLenum m_iLastMode = 0;

	struct _cameraofs_s { float right; float up; float forward; float pitch; float yaw; float roll; } _cameraofs = { 0,0,0,0,0,0 };
	float	_fStereoOffset = (float)1.27;

	STEREO_STATE _stereo_state = STS_LEFT;

	// it is very important to understand this and the things connected to it right:
	bool _bRecordBuffers_FirstCall = false; 
	// On the one hand Filming::recordBuffres() can get called because the engine advanced in time and rendered an new frame, in this case _bRecordBuffers_FirstCall is true
	// On the other hand we might have triggered a new frame our self by doing an manual call to R_RenderView, in that case _bRecordBuffers_FirstCall is false!!
	// The second case usually can only happen when we have the R_RenderView hook and therefore the code connected to it enabled (which is the defualt).

	HUD_REQUEST_STATE _HudRqState = HUDRQ_NORMAL;

	bool _bSimulate2 = false;

	bool _bWorldFxDisableBlend = false;
	bool _bWorldFxEnableDepth = false;

	float _fx_whRGBf[3] = {0.0f, 0.5f, 1.0f};

	bool _InMatteEntities(int iid);

	void clearBuffers();	// call this (i.e. after Swapping) when we can prepare (clear) our buffers for the next frame
};

extern Filming g_Filming;