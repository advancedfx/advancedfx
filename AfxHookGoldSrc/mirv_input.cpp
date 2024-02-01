#include "stdafx.h"

#include "mirv_input.h"

#include "cmdregister.h"
#include "MirvCommandArgs.h"
#include "filming.h"
#include "hooks/HookHw.h"


#include "../shared/MirvInput.h"

extern bool g_b_SDL2_ShowCursor;
extern bool g_b_SDL2_RelativeMouseMode;

class MirvInputEx : private IMirvInputDependencies
{
public:
	MirvInputEx() {
		m_MirvInput = new MirvInput(this);
	}

	~MirvInputEx() {
		delete m_MirvInput;
	}

	MirvInput* m_MirvInput;

private:
	virtual bool GetSuspendMirvInput() override {
		return g_b_SDL2_ShowCursor && !g_b_SDL2_RelativeMouseMode;
	}

	virtual void GetLastCameraData(double& x, double& y, double& z, double& rX, double& rY, double& rZ, double& fov) override {
		x = g_Filming.LastCameraOrigin[0];
		y = g_Filming.LastCameraOrigin[1];
		z = g_Filming.LastCameraOrigin[2];
		rX = g_Filming.LastCameraAngles[0];
		rY = g_Filming.LastCameraAngles[1];
		rZ = g_Filming.LastCameraAngles[2];
		fov = g_Filming.LastCameraFov;
	}

	virtual void GetGameCameraData(double& x, double& y, double& z, double& rX, double& rY, double& rZ, double& fov) override {
		x = g_Filming.GameCameraOrigin[0];
		y = g_Filming.GameCameraOrigin[1];
		z = g_Filming.GameCameraOrigin[2];
		rX = g_Filming.GameCameraAngles[0];
		rY = g_Filming.GameCameraAngles[1];
		rZ = g_Filming.GameCameraAngles[2];
		fov = g_Filming.GameCameraFov;
	}

	virtual double GetInverseScaledFov(double fov) override {
		return fov;
	}

private:

} g_MirvInputEx;

class MirvInput * MirvInput_Get() {
	return g_MirvInputEx.m_MirvInput;
}

bool MirvInput_Override(float deltaT, float& Tx, float& Ty, float& Tz, float& Rx, float& Ry, float& Rz, float& Fov) {
	return g_MirvInputEx.m_MirvInput->Override(deltaT, Tx, Ty, Tz, Rx, Ry, Rz, Fov);
}

void MirvInput_SupplyMouseFrameEnd() {
	g_MirvInputEx.m_MirvInput->Supply_MouseFrameEnd();
}

REGISTER_CMD_FUNC(input)
{
	CMirvCommandArgs args(pEngfuncs);
	g_MirvInputEx.m_MirvInput->ConCommand(&args);
}
