#pragma once

#include "WrpGlobals.h"
#include "WrpConsole.h"
#include "../shared/CamIO.h"
#include <shared/CamPath.h>
#include "../shared/MirvInput.h"

#include <list>

class Hook_VClient_RenderView;

// global singelton instance:
extern Hook_VClient_RenderView g_Hook_VClient_RenderView;


// Hook_VClient_RenderView /////////////////////////////////////////////////////

class Hook_VClient_RenderView
	: private IMirvInputDependencies
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
		Override_MirvPgl = 9,
		Override_Interop = 10
	};

	CamPath m_CamPath;

	bool ViewOverrideReset = true;
	bool ForceViewOverride = true;
	bool ForceViewOverrideHltv = false;

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

	void Console_MirvInput(IWrpCommandArgs * args) {
		m_MirvInput->ConCommand(args);
	}

	MirvInput * m_MirvInput;

private:
	float m_LastFrameTime = 0;

	bool m_Export;
	bool m_FovOverride;
	double m_FovValue;
	WrpGlobals * m_Globals;
	bool m_Import;
	float m_ImportBaseTime;
	bool m_IsInstalled;

	CamExport * m_CamExport = 0;
	CamImport * m_CamImport = 0;

	Override m_Overrides[11];

	virtual bool GetSuspendMirvInput() override;
	virtual void GetLastCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) override;
	virtual void GetGameCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) override;
	virtual double GetInverseScaledFov(double fov) override;

	void SetDefaultOverrides();

	bool OverrideFromString(const char * value, Override & outOverride);
};

