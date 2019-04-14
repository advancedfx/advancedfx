#include "stdafx.h"

//
// These hooks mainly hook into
// CViewRender::SetUpView inside the client.dll
//



#include "RenderView.h"

#include "MirvTime.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <shared/bvhimport.h>
#include <shared/bvhexport.h>

#include "addresses.h"
#include "WrpVEngineClient.h"
#include "AfxHookSourceInput.h"
#include "aiming.h"
#include "MirvCam.h"
#include "AfxInterop.h"


BvhExport * g_BvhExport = NULL;

BvhImport g_BvhImport;

// Create singelton instance:
Hook_VClient_RenderView g_Hook_VClient_RenderView;


unsigned int g_OfsCvarFloatValue;

float GetCvarFloat(void * pcvar)
{
	float * pf = (float *)(*(unsigned char **)pcvar +g_OfsCvarFloatValue);

	float f = *pf;

	return f;
}

void SetCvarFloat(void * pcvar, float value)
{
	float * pf = (float *)(*(unsigned char **)pcvar +g_OfsCvarFloatValue);

	*pf = value;
}


// Hook_VClient_RenderView /////////////////////////////////////////////////////

Hook_VClient_RenderView::Hook_VClient_RenderView()
: m_Globals(0)
, handleZoomEnabled(false)
, handleZoomMinUnzoomedFov(90.0)
{
	m_Export = false;
	m_FovOverride = false;
	m_FovValue = 0.0;
	m_Import = false;
	m_ImportBaseTime = 0;
	m_IsInstalled = false;

	LastCameraOrigin[0] = 0.0;
	LastCameraOrigin[1] = 0.0;
	LastCameraOrigin[2] = 0.0;
	LastCameraAngles[0] = 0.0;
	LastCameraAngles[1] = 0.0;
	LastCameraAngles[2] = 0.0;

	LastCameraFov = 90.0;
}


Hook_VClient_RenderView::~Hook_VClient_RenderView() {
	ExportEnd();
	ImportEnd();

	delete m_CamExport;
	delete m_CamImport;
}

bool Hook_VClient_RenderView::ExportBegin(wchar_t const *fileName, double frameTime) {
	ExportEnd();

	g_BvhExport = new BvhExport(fileName, "MdtCam", frameTime);
	m_Export = true;

	return m_Export;
}

void Hook_VClient_RenderView::ExportEnd() {

	delete g_BvhExport;
	g_BvhExport = 0;
	m_Export = false;
}


bool Hook_VClient_RenderView::GetFovOverride(double &outValue)
{
	outValue = m_FovValue;

	return m_FovOverride;
}

void Hook_VClient_RenderView::FovOverride(double value)
{
	m_FovValue = value;
	m_FovOverride = true;
}

void Hook_VClient_RenderView::FovDefault()
{
	m_FovOverride = false;
}

float Hook_VClient_RenderView::GetImportBasteTime() {
	return m_ImportBaseTime;
}

bool Hook_VClient_RenderView::ImportBegin(wchar_t const *fileName)
{
	ImportEnd();

	m_Import = g_BvhImport.LoadMotionFile(fileName);

	return m_Import;
}

void Hook_VClient_RenderView::ImportEnd() {
	g_BvhImport.CloseMotionFile();

	m_Import = false;
}

void Hook_VClient_RenderView::Install(WrpGlobals * globals)
{
	if(m_IsInstalled)
		return;

	m_Globals = globals;

	m_IsInstalled = true;
}

WrpGlobals * Hook_VClient_RenderView::GetGlobals()
{
	return m_Globals;
}

bool Hook_VClient_RenderView::IsInstalled(void) {
	return m_IsInstalled;
}

void TrySetAbsOriginAndAngles(float Tx, float Ty, float Tz, float Rx, float Ry, float Rz)
{
	static bool firstRun = true;
	static SOURCESDK::CSGO::IServerTools * serverTools = nullptr;

	if (firstRun)
	{
		firstRun = false;
		
		if (HMODULE hServerModule = GetModuleHandleA("server"))
		{
			if (SOURCESDK::CreateInterfaceFn createInterfaceFn = (SOURCESDK::CreateInterfaceFn)GetProcAddress(hServerModule, "CreateInterface"))
			{
				int returnCode = 0;

				serverTools = (SOURCESDK::CSGO::IServerTools *)createInterfaceFn(SOURCESDK_CSGO_VSERVERTOOLS_INTERFACE_VERSION, &returnCode);
			}
		}

		if (!serverTools)
		{
			Tier0_Warning(
				"AFXERROR: Could not get %s interface required for server view override.\n"
				, SOURCESDK_CSGO_VSERVERTOOLS_INTERFACE_VERSION);
		}
	}

	if (serverTools)
	{
		SOURCESDK::Vector origin;
		SOURCESDK::QAngle angles;

		origin.x = Tx;
		origin.y = Ty;
		origin.z = Tz;

		angles.x = Rx;
		angles.y = Ry;
		angles.z = Rz;

		serverTools->MoveEngineViewTo(origin, angles);
	}
}


void Hook_VClient_RenderView::OnViewOverride(float &Tx, float &Ty, float &Tz, float &Rx, float &Ry, float &Rz, float &Fov)
{
	bool originOrAnglesOverriden = false;

	float curTime = g_MirvTime.GetTime();

	GameCameraOrigin[0] = Tx;
	GameCameraOrigin[1] = Ty;
	GameCameraOrigin[2] = Tz;
	GameCameraAngles[0] = Rx;
	GameCameraAngles[1] = Ry;
	GameCameraAngles[2] = Rz;
	GameCameraFov = Fov;

	if (g_MirvCam.ApplySource(Tx, Ty, Tz, Rz, Rx, Ry)) originOrAnglesOverriden = true;

	if(m_CamPath.Enabled_get() && m_CamPath.CanEval())
	{
		// no extrapolation:
		if(m_CamPath.GetLowerBound() <= curTime && curTime <= m_CamPath.GetUpperBound())
		{
			CamPathValue val = m_CamPath.Eval( curTime );
			QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

			//Tier0_Msg("================",curTime);
			//Tier0_Msg("currenTime = %f",curTime);
			//Tier0_Msg("vCp = %f %f %f\n", val.X, val.Y, val.Z);

			originOrAnglesOverriden = true;

			Tx = (float)val.X;
			Ty = (float)val.Y;
			Tz = (float)val.Z;

			Rx = (float)ang.Pitch;
			Ry = (float)ang.Yaw;
			Rz = (float)ang.Roll;

			Fov = (float)val.Fov;
		}
	}

	if(m_Import) {
		double Tf[6];

		if(g_BvhImport.GetCamPosition(
			curTime -m_ImportBaseTime,
			Tf
		)) {
			originOrAnglesOverriden = true;

			Ty = (float)(-Tf[0]);
			Tz = (float)(+Tf[1]);
			Tx = (float)(-Tf[2]);
			Rz = (float)(-Tf[3]);
			Rx = (float)(-Tf[4]);
			Ry = (float)(+Tf[5]);
		}
	}

	if (m_CamImport)
	{
		CamIO::CamData camData;

		if (m_CamImport->GetCamData(curTime, LastWidth, LastHeight, camData))
		{
			originOrAnglesOverriden = true;

			Tx = (float)camData.XPosition;
			Ty = (float)camData.YPosition;
			Tz = (float)camData.ZPosition;
			Rx = (float)camData.YRotation;
			Ry = (float)camData.ZRotation;
			Rz = (float)camData.XRotation;
			Fov = (float)camData.Fov;
		}
	}

	if(m_FovOverride && (!handleZoomEnabled || handleZoomMinUnzoomedFov <= Fov)) Fov = (float)m_FovValue;

	if(g_AfxHookSourceInput.GetCameraControlMode() && m_Globals)
	{
		originOrAnglesOverriden = true;

		double dT = m_Globals->absoluteframetime_get();
		double dForward = dT * g_AfxHookSourceInput.GetCamDForward();
		double dLeft = dT * g_AfxHookSourceInput.GetCamDLeft();
		double dUp = dT * g_AfxHookSourceInput.GetCamDUp();
		double dPitch = dT * g_AfxHookSourceInput.GetCamDPitch();
		double dRoll = dT * g_AfxHookSourceInput.GetCamDRoll();
		double dYaw = dT * g_AfxHookSourceInput.GetCamDYaw();
		double dFov = dT * g_AfxHookSourceInput.GetCamDFov();
		double forward[3], right[3], up[3];

		Rx = (float)(LastCameraAngles[0] +dPitch);
		Ry = (float)(LastCameraAngles[1] +dYaw);
		Rz = (float)(LastCameraAngles[2] +dRoll);
		Fov = (float)(LastCameraFov +dFov);

		if(g_AfxHookSourceInput.GetCamResetView())
		{
			Rx = 0;
			Ry = 0;
			Rz = 0;
			Fov = 90.0;
		}

		MakeVectors(Rz, Rx, Ry, forward, right, up);

		Tx = (float)(LastCameraOrigin[0] + dForward*forward[0] -dLeft*right[0] +dUp*up[0]);
		Ty = (float)(LastCameraOrigin[1] + dForward*forward[1] -dLeft*right[1] +dUp*up[1]);
		Tz = (float)(LastCameraOrigin[2] + dForward*forward[2] -dLeft*right[2] +dUp*up[2]);
	}

	// limit fov to sane values:
	if(Fov<1) Fov = 1;
	else if(Fov>179) Fov = 179;

	if(m_Globals)
	{
		double dRx = Rx;
		double dRy = Ry;
		double dRz = Rz;

		if(g_Aiming.Aim(m_Globals->absoluteframetime_get(), Vector3(Tx, Ty, Tz), dRx, dRy, dRz))
		{
			originOrAnglesOverriden = true;
			
			Rx = (float)dRx;
			Ry = (float)dRy;
			Rz = (float)dRz;
		}
	}

	if (g_MirvCam.ApplyOffset(Tx, Ty, Tz, Rz, Rx, Ry)) originOrAnglesOverriden = true;

	g_MirvCam.ApplyFov(Fov);

#ifdef AFX_INTEROP
	if(AfxInterop::OnRenderView(Tx, Ty, Tz, Rx, Ry, Rz, Fov)) originOrAnglesOverriden = true;
#endif

	if (originOrAnglesOverriden && this->ForceViewOverride)
	{
		TrySetAbsOriginAndAngles(Tx, Ty, Tz, Rz, Rx, Ry);
	}


	if(m_Export) {
		g_BvhExport->WriteFrame(
			-Ty, +Tz, -Tx,
			-Rz, -Rx, +Ry
		);
	}

	if (m_CamExport)
	{
		CamIO::CamData camData;

		camData.Time = curTime;
		camData.XPosition = Tx;
		camData.YPosition = Ty;
		camData.ZPosition = Tz;
		camData.YRotation = Rx;
		camData.ZRotation = Ry;
		camData.XRotation = Rz;
		camData.Fov = Fov;

		m_CamExport->WriteFrame(LastWidth, LastHeight, camData);
	}

	LastCameraOrigin[0] = Tx;
	LastCameraOrigin[1] = Ty;
	LastCameraOrigin[2] = Tz;
	LastCameraAngles[0] = Rx;
	LastCameraAngles[1] = Ry;
	LastCameraAngles[2] = Rz;
	LastCameraFov = Fov;

	g_AfxHookSourceInput.Supply_MouseFrameEnd();

	//Tier0_Msg("Hook_VClient_RenderView::OnViewOverride: curTime = %f, LastCameraOrigin=%f,%f,%f\n",curTime,LastCameraOrigin[0],LastCameraOrigin[1],LastCameraOrigin[2]);
}

void Hook_VClient_RenderView::SetImportBaseTime(float value) {
	m_ImportBaseTime = value;
}

bool Hook_VClient_RenderView::ImportToCamPath(bool adjustInterp, double fov)
{
	if(!m_Import)
		return false;

	bool bOk = g_BvhImport.CopyToCampath(m_ImportBaseTime, fov, m_CamPath);

	if(bOk)
	{
		ImportEnd();

		if(adjustInterp)
		{
			m_CamPath.PositionInterpMethod_set(CamPath::DI_LINEAR);
			m_CamPath.RotationInterpMethod_set(CamPath::QI_SLINEAR);
			m_CamPath.FovInterpMethod_set(CamPath::DI_LINEAR);
		}

		m_CamPath.Enabled_set(true);
	}

	return bOk;
}

void Hook_VClient_RenderView::Console_CamIO(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * cmd0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (0 == _stricmp("export", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (0 == _stricmp("start", cmd2) && 5 <= argc)
				{
					if (0 != m_CamExport)
					{
						delete m_CamExport;
						m_CamExport = 0;
					}

					std::wstring fileName(L"");

					if (UTF8StringToWideString(args->ArgV(3), fileName))
					{
						m_CamExport = new CamExport(fileName.c_str(), 0 == _stricmp("alienSwarm", args->ArgV(4)) ? CamExport::SF_AlienSwarm : CamExport::SF_None);
					}
					else
						Tier0_Warning("Error: Can not convert \"%s\" from UTF-8 to WideString.\n", args->ArgV(3));


					return;
				}
				else if (0 == _stricmp("end", cmd2))
				{
					if (0 != m_CamExport)
					{
						delete m_CamExport;
						m_CamExport = 0;
					}
					else
						Tier0_Warning("No cam export was active.");

					return;
				}
			}

			Tier0_Msg(
				"%s export start <fileName> <fovScaling> - Starts exporting to file <fileName>, <fovScaling> can be \"none\" for engine FOV or \"alienSwarm\" for scaling like Alien Swarm SDK (i.e. CS:GO).\n"
				"%s export end - Stops exporting.\n"
				, cmd0
				, cmd0
			);
			return;
		}
		else if (0 == _stricmp("import", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (0 == _stricmp("start", cmd2) && 4 <= argc)
				{
					if (0 != m_CamImport)
					{
						delete m_CamImport;
						m_CamImport = 0;
					}

					m_CamImport = new CamImport(args->ArgV(3), g_MirvTime.GetTime());
					if (m_CamImport->IsBad()) Tier0_Warning("Error importing CAM file \"%s\"\n", args->ArgV(3));
					return;
				}
				else if (0 == _stricmp("end", cmd2))
				{
					delete m_CamImport;
					m_CamImport = 0;
					return;
				}

			}

			Tier0_Msg(
				"%s import start <fileName> - Starts importing cam from file <fileName>.\n"
				"%s import end - Stops importing.\n"
				, cmd0
				, cmd0
			);
			return;
		}
	}

	Tier0_Msg(
		"%s export [...] - Controls export of new camera motion data.\n"
		"%s import [...] - Controls import of new camera motion data.\n"
		, cmd0
		, cmd0
	);
}
