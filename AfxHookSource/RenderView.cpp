#include "stdafx.h"

//
// These hooks mainly hook into
// CViewRender::SetUpView inside the client.dll
//



#include "RenderView.h"

#include "MirvTime.h"

#include <shared/AfxDetours.h>
#include <shared/StringTools.h>

#include <shared/bvhimport.h>
#include <shared/bvhexport.h>

#include "addresses.h"
#include "WrpVEngineClient.h"
#include "AfxHookSourceInput.h"
#include "aiming.h"
#include "MirvCam.h"
#include "AfxInterop.h"
#include "csgo/ClientToolsCSgo.h"
#include "MirvPgl.h"


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

	CurrentCameraOrigin[0] = 0.0;
	CurrentCameraOrigin[1] = 0.0;
	CurrentCameraOrigin[2] = 0.0;
	CurrentCameraAngles[0] = 0.0;
	CurrentCameraAngles[1] = 0.0;
	CurrentCameraAngles[2] = 0.0;
	CurrentCameraFov = 90.0;


	SetDefaultOverrides();
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

void TrySetView(float Tx, float Ty, float Tz, float Rx, float Ry, float Rz, float fov, float * outOrgRz, float * outOrgFov)
{
	static bool firstRun = true;
	static SOURCESDK::CSGO::IServerTools * serverTools = nullptr;

	if (firstRun)
	{
		firstRun = false;
		
		if (SourceSdkVer_CSGO == g_SourceSdkVer)
		{
			if (CClientToolsCsgo::Instance())
			{

				if (HMODULE hServerModule = GetModuleHandleA("server"))
				{
					if (SOURCESDK::CreateInterfaceFn createInterfaceFn = (SOURCESDK::CreateInterfaceFn)GetProcAddress(hServerModule, "CreateInterface"))
					{
						int returnCode = 0;

						serverTools = (SOURCESDK::CSGO::IServerTools *)createInterfaceFn(SOURCESDK_CSGO_VSERVERTOOLS_INTERFACE_VERSION, &returnCode);
					}
				}
			}

			if (!serverTools)
			{
				Tier0_Warning(
					"AFXERROR: Could not get interfaces required for server view override.\n");
			}
		}
	}

	if (serverTools)
	{
		if (SOURCESDK::C_BaseEntity_csgo * localPlayer = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(CClientToolsCsgo::Instance()->GetClientToolsInterface()->GetLocalPlayer()))
		{

			SOURCESDK::Vector origin;
			SOURCESDK::QAngle angles;

			if (outOrgRz)
			{
				serverTools->GetPlayerPosition(origin, angles, localPlayer);
				*outOrgRz = angles.z;
			}

			if (outOrgFov)
			{
				*outOrgFov = (float)serverTools->GetPlayerFOV(localPlayer);
			}

			origin.x = Tx;
			origin.y = Ty;
			origin.z = Tz;

			angles.x = Rx;
			angles.y = Ry;
			angles.z = Rz;

			serverTools->SnapPlayerToPosition(origin, angles, localPlayer);
			serverTools->SetPlayerFOV((int)fov, localPlayer);
		}
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

	for (int i = 0; i < 11; ++i)
	{
		CurrentCameraOrigin[0] = Tx;
		CurrentCameraOrigin[1] = Ty;
		CurrentCameraOrigin[2] = Tz;
		CurrentCameraAngles[0] = Rx;
		CurrentCameraAngles[1] = Ry;
		CurrentCameraAngles[2] = Rz;
		CurrentCameraFov = Fov;

		switch (m_Overrides[i])
		{
		case Override_CamSource:
			if (g_MirvCam.ApplySource(Tx, Ty, Tz, Rz, Rx, Ry)) originOrAnglesOverriden = true;
			break;

		case Override_Campath:
			if (m_CamPath.Enabled_get() && m_CamPath.CanEval())
			{
				double campathCurTime = curTime - m_CamPath.GetOffset();

				// no extrapolation:
				if (m_CamPath.GetLowerBound() <= campathCurTime && campathCurTime <= m_CamPath.GetUpperBound())
				{
					CamPathValue val = m_CamPath.Eval(campathCurTime);
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
			break;

		case Override_Bvh:
			if (m_Import)
			{
				double Tf[6];

				if (g_BvhImport.GetCamPosition(
					curTime - m_ImportBaseTime,
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
			break;

		case Override_Camio:
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
			break;

		case Override_Fov:
			if (m_FovOverride && (!handleZoomEnabled || handleZoomMinUnzoomedFov <= Fov)) Fov = (float)m_FovValue;
			break;

		case Override_Input:
			if (g_AfxHookSourceInput.GetCameraControlMode() && m_Globals)
			{
				originOrAnglesOverriden = true;

				if (!m_InputOn)
				{
					m_InputOn = true;

					InputCameraOrigin[0] = LastCameraOrigin[0];
					InputCameraOrigin[1] = LastCameraOrigin[1];
					InputCameraOrigin[2] = LastCameraOrigin[2];
					InputCameraAngles[0] = LastCameraAngles[0];
					InputCameraAngles[1] = LastCameraAngles[1];
					InputCameraAngles[2] = LastCameraAngles[2];
					InputCameraFov = LastCameraFov;
				}

				switch (g_AfxHookSourceInput.GetOffsetMode())
				{
				case AfxHookSourceInput::OffsetMode_Last:
					InputCameraOrigin[0] = LastCameraOrigin[0];
					InputCameraOrigin[1] = LastCameraOrigin[1];
					InputCameraOrigin[2] = LastCameraOrigin[2];
					InputCameraAngles[0] = LastCameraAngles[0];
					InputCameraAngles[1] = LastCameraAngles[1];
					InputCameraAngles[2] = LastCameraAngles[2];
					InputCameraFov = LastCameraFov;
					break;
				case AfxHookSourceInput::OffsetMode_Game:
					InputCameraOrigin[0] = GameCameraOrigin[0];
					InputCameraOrigin[1] = GameCameraOrigin[1];
					InputCameraOrigin[2] = GameCameraOrigin[2];
					InputCameraAngles[0] = GameCameraAngles[0];
					InputCameraAngles[1] = GameCameraAngles[1];
					InputCameraAngles[2] = GameCameraAngles[2];
					InputCameraFov = GameCameraFov;
					break;
				case AfxHookSourceInput::OffsetMode_Current:
					InputCameraOrigin[0] = Tx;
					InputCameraOrigin[1] = Ty;
					InputCameraOrigin[2] = Tz;
					InputCameraAngles[0] = Rx;
					InputCameraAngles[1] = Ry;
					InputCameraAngles[2] = Rz;
					InputCameraFov = Fov;
					break;
				}

				double dT = 0 == m_LastFrameTime ? 1 : m_LastFrameTime;
				double dForward = dT * g_AfxHookSourceInput.GetCamDForward();
				double dLeft = dT * g_AfxHookSourceInput.GetCamDLeft();
				double dUp = dT * g_AfxHookSourceInput.GetCamDUp();
				double dPitch = dT * g_AfxHookSourceInput.GetCamDPitch();
				double dRoll = dT * g_AfxHookSourceInput.GetCamDRoll();
				double dYaw = dT * g_AfxHookSourceInput.GetCamDYaw();
				double dFov = dT * g_AfxHookSourceInput.GetCamDFov();
				double forward[3], right[3], up[3];

				Rx = (float)(InputCameraAngles[0] + dPitch);
				Ry = (float)(InputCameraAngles[1] + dYaw);
				Rz = (float)(InputCameraAngles[2] + dRoll);
				Fov = (float)(InputCameraFov + dFov);

				// limit fov to sane values:
				if (Fov < 1) Fov = 1;
				else if (Fov > 179) Fov = 179;

				if (g_AfxHookSourceInput.GetCamResetView())
				{
					Rx = 0;
					Ry = 0;
					Rz = 0;
					Fov = 90.0;
				}

				MakeVectors(Rz, Rx, Ry, forward, right, up);

				Tx = (float)(InputCameraOrigin[0] + dForward * forward[0] - dLeft * right[0] + dUp * up[0]);
				Ty = (float)(InputCameraOrigin[1] + dForward * forward[1] - dLeft * right[1] + dUp * up[1]);
				Tz = (float)(InputCameraOrigin[2] + dForward * forward[2] - dLeft * right[2] + dUp * up[2]);

				g_AfxHookSourceInput.Override(Tx, Ty, Tz, Rx, Ry, Rz, Fov);

				InputCameraOrigin[0] = Tx;
				InputCameraOrigin[1] = Ty;
				InputCameraOrigin[2] = Tz;
				InputCameraAngles[0] = Rx;
				InputCameraAngles[1] = Ry;
				InputCameraAngles[2] = Rz;

				InputCameraFov = Fov;
			}
			else
			{
				m_InputOn = false;
			}
			break;

		case Override_Aim:
			if (m_Globals)
			{
				double dRx = Rx;
				double dRy = Ry;
				double dRz = Rz;

				if (g_Aiming.Aim(m_Globals->absoluteframetime_get(), Vector3(Tx, Ty, Tz), dRx, dRy, dRz))
				{
					originOrAnglesOverriden = true;

					Rx = (float)dRx;
					Ry = (float)dRy;
					Rz = (float)dRz;
				}
			}
			break;

		case Override_CamOffset:
			if (g_MirvCam.ApplyOffset(Tx, Ty, Tz, Rz, Rx, Ry)) originOrAnglesOverriden = true;
			break;

		case Override_CamFov:
			g_MirvCam.ApplyFov(Fov);
			break;

		case Override_MirvPgl:
			if (MirvPgl::OnViewOverride(Tx, Ty, Tz, Rx, Ry, Rz, Fov)) originOrAnglesOverriden = true;
			break;

		case Override_Interop:
#ifdef AFX_INTEROP
			if (AfxInterop::OnViewOverride(Tx, Ty, Tz, Rx, Ry, Rz, Fov)) originOrAnglesOverriden = true;
#endif
			break;
		}

		// limit fov to sane values:
		if (Fov < 1) Fov = 1;
		else if (Fov > 179) Fov = 179;
	}
	
	static bool viewOverriding = false;

	if (originOrAnglesOverriden && this->ForceViewOverride)
	{
		TrySetView(Tx, Ty, Tz, Rx, Ry, Rz, Fov, nullptr, nullptr);

		viewOverriding = true;
	}
	else if (viewOverriding)
	{
		viewOverriding = false;

		if (this->ViewOverrideReset)
		{
			TrySetView(Tx, Ty, Tz, Rx, Ry, 0.0f, 90.0f, nullptr, nullptr);
		}
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

	m_LastFrameTime = m_Globals->absoluteframetime_get();

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

				if (0 == _stricmp("start", cmd2) && 4 <= argc)
				{
					if (0 != m_CamExport)
					{
						delete m_CamExport;
						m_CamExport = 0;
					}

					std::wstring fileName(L"");

					if (UTF8StringToWideString(args->ArgV(3), fileName))
					{
						m_CamExport = new CamExport(fileName.c_str());
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
						Tier0_Warning("No cam export was active.\n");

					return;
				}
			}

			Tier0_Msg(
				"%s export start <fileName> - Starts exporting to file <fileName>.\n"
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

void Hook_VClient_RenderView::SetDefaultOverrides()
{
	m_Overrides[0] = Override_CamSource;
	m_Overrides[1] = Override_Campath;
	m_Overrides[2] = Override_Bvh;
	m_Overrides[3] = Override_Camio;
	m_Overrides[4] = Override_Fov;
	m_Overrides[5] = Override_Input;
	m_Overrides[6] = Override_Aim;
	m_Overrides[7] = Override_CamOffset;
	m_Overrides[8] = Override_CamFov;
	m_Overrides[9] = Override_MirvPgl;
	m_Overrides[10] = Override_Interop;
}

bool Hook_VClient_RenderView::OverrideFromString(const char * value, Override & outOverride)
{
	if (0 == _stricmp("camSource", value)) {
		outOverride = Override_CamSource;
		return true;
	}
	if (0 == _stricmp("camPath", value)) {
		outOverride = Override_Campath;
		return true;
	}
	if (0 == _stricmp("bvh", value)) {
		outOverride = Override_Bvh;
		return true;
	}
	if (0 == _stricmp("camIo", value)) {
		outOverride = Override_Camio;
		return true;
	}
	if (0 == _stricmp("fov", value)) {
		outOverride = Override_Fov;
		return true;
	}
	if (0 == _stricmp("input", value)) {
		outOverride = Override_Input;
		return true;
	}
	if (0 == _stricmp("aim", value)) {
		outOverride = Override_Aim;
		return true;
	}
	if (0 == _stricmp("camOffset", value)) {
		outOverride = Override_CamOffset;
		return true;
	}
	if (0 == _stricmp("camFov", value)) {
		outOverride = Override_CamFov;
		return true;
	}
	if (0 == _stricmp("mirvPgl", value)) {
		outOverride = Override_MirvPgl;
		return true;
	}
	if (0 == _stricmp("interop", value)) {
		outOverride = Override_Interop;
		return true;
	}

	return false;
}

void Hook_VClient_RenderView::Console_Overrides(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("default", arg1))
		{
			SetDefaultOverrides();
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			const char * text[11] = {
				"camSource (of mirv_cam)"
				, "camPath"
				, "bvh"
				, "camIo"
				, "fov (mirv_fov)"
				, "input (mirv_input)"
				, "aim (mirv_aim)"
				, "camOffset (of mirv_cam)"
				, "camFov (of mirv_cam)"
				, "mirvPgl"
				, "interop (afx_interop)"
			};

			for (int i = 0; i < 11; ++i)
			{
				int value = m_Overrides[i];

				if (0 <= value && value < 11)
				{
					Tier0_Msg("%i: %s\n", i, text[m_Overrides[i]]);
				}
				else Tier0_Msg("%i: [n/a]\n", i);
			}
			return;
		}
		else if (0 == _stricmp("move", arg1) && 4 == argC)
		{
			Override overrideVal;

			if (!OverrideFromString(args->ArgV(2), overrideVal))
			{
				Tier0_Warning("AFXERROR: %s is not a valid overide name.\n", args->ArgV(2));
				return;
			}

			int pos = atoi(args->ArgV(3));

			if (pos < 0) pos = 0;
			else if (9 < pos) pos = 9;

			if (m_Overrides[pos] != overrideVal)
			{
				int orgPos;

				for (int i = 0; i < 11; ++i)
				{
					if (m_Overrides[i] == overrideVal)
					{
						orgPos = i;
						break;
					}
				}

				if (pos < orgPos)
				{
					for (int i = orgPos; i > pos; --i)
					{
						m_Overrides[i] = m_Overrides[i - 1];
					}
				}
				else // if (pos > orgPos)
				{
					for (int i = orgPos; i < pos; ++i)
					{
						m_Overrides[i] = m_Overrides[i + 1];
					}
				}

				m_Overrides[pos] = overrideVal;
			}

			return;
		}
	}

	Tier0_Msg(
		"%s default - Restore default override order.\n"
		"%s print - Print current override order.\n"
		"%s move <sOverrideName> <iPos> - Move override <sOverrideName> to position <iPos>.\n"
		, arg0
		, arg0
		, arg0
	);
}