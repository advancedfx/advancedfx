#pragma once

#include "WrpGlobals.h"
#include "WrpConsole.h"
#include "CamIO.h"
#include <shared/CamPath.h>

#include <list>

class Hook_VClient_RenderView;

// global singelton instance:
extern Hook_VClient_RenderView g_Hook_VClient_RenderView;


// Hook_VClient_RenderView /////////////////////////////////////////////////////

class Hook_VClient_RenderView
{
public:
	enum Override
	{
		Override_CamSource = 0,
		Override_Campath = 1,
		Override_Bvh = 2,
		Override_Camio = 3,
		Override_Fov = 4,
		Override_Input = 5,
		Override_Aim = 6,
		Override_CamOffset = 7,
		Override_CamFov = 8,
		Override_Interop = 9
	};

	CamPath m_CamPath;

	bool ViewOverrideReset = true;
	bool ForceViewOverride = true;

	float GameCameraOrigin[3] = { 0.0f, 0.0f, 0.0f };
	float GameCameraAngles[3] = { 0.0f, 0.0f, 0.0f };
	float GameCameraFov = 90.0f;

	double CurrentCameraOrigin[3];
	double CurrentCameraAngles[3];
	double CurrentCameraFov;

	double LastCameraOrigin[3];
	double LastCameraAngles[3];
	double LastCameraFov;

	int LastWidth = 1280;
	int LastHeight = 720;

	bool handleZoomEnabled;
	double handleZoomMinUnzoomedFov;

	Hook_VClient_RenderView();
	~Hook_VClient_RenderView();

	bool ExportBegin(wchar_t const *fileName, double frameTime);
	void ExportEnd();

	bool GetFovOverride(double &outValue);

	void FovOverride(double value);
	void FovDefault();

	float GetImportBasteTime();

	bool ImportBegin(wchar_t const *fileName);
	void ImportEnd();

	bool ImportToCamPath(bool adjustInterp, double fov);

	void Install(WrpGlobals * globals);
	WrpGlobals * GetGlobals();

	bool IsInstalled(void);

	void OnAdjustEngineViewport(int& x, int& y, int& width, int& height)
	{
		LastWidth = width;
		LastHeight = height;
	}

	void OnViewOverride(float &Tx, float &Ty, float &Tz, float &Rx, float &Ry, float &Rz, float &Fov);

	void SetImportBaseTime(float value);

	void Console_CamIO(IWrpCommandArgs * args);

	void Console_Overrides(IWrpCommandArgs * args);

private:
	bool m_Export;
	bool m_FovOverride;
	double m_FovValue;
	WrpGlobals * m_Globals;
	bool m_Import;
	float m_ImportBaseTime;
	bool m_IsInstalled;

	bool m_InputOn = false;
	double InputCameraOrigin[3];
	double InputCameraAngles[3];
	double InputCameraFov;

	CamExport * m_CamExport = 0;
	CamImport * m_CamImport = 0;

	Override m_Overrides[10];

	void SetDefaultOverrides();

	bool OverrideFromString(const char * value, Override & outOverride);
};

