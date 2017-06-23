#pragma once

#include <windows.h>

class AfxHookSourceInput
{
public:
	enum KeyState
	{
		KS_DOWN,
		KS_UP
	};

	AfxHookSourceInput();

	bool GetCamResetView(void);
	double GetCamDForward(void);
	double GetCamDLeft(void);
	double GetCamDUp(void);
	double GetCamDPitch(void);
	double GetCamDYaw(void);
	double GetCamDRoll(void);
	double GetCamDFov(void);

	bool GetCameraControlMode(void);
	void SetCameraControlMode(bool enable);

	double GetKeyboardSensitivty(void);
	void SetKeyboardSensitivity(double value);

	double GetMouseSensitivty(void);
	void SetMouseSensitivity(double value);

	double KeyboardForwardSpeed_get(void);
	void KeyboardForwardSpeed_set(double value);
	
	double KeyboardBackwardSpeed_get(void);
	void KeyboardBackwardSpeed_set(double value);
	
	double KeyboardLeftSpeed_get(void);
	void KeyboardLeftSpeed_set(double value);
	
	double KeyboardRightSpeed_get(void);
	void KeyboardRightSpeed_set(double value);
	
	double KeyboardUpSpeed_get(void);
	void KeyboardUpSpeed_set(double value);
	
	double KeyboardDownSpeed_get(void);
	void KeyboardDownSpeed_set(double value);
	
	double KeyboardPitchPositiveSpeed_get(void);
	void KeyboardPitchPositiveSpeed_set(double value);
	
	double KeyboardPitchNegativeSpeed_get(void);
	void KeyboardPitchNegativeSpeed_set(double value);
	
	double KeyboardYawPositiveSpeed_get(void);
	void KeyboardYawPositiveSpeed_set(double value);
	
	double KeyboardYawNegativeSpeed_get(void);
	void KeyboardYawNegativeSpeed_set(double value);
	
	double KeyboardRollPositiveSpeed_get(void);
	void KeyboardRollPositiveSpeed_set(double value);
	
	double KeyboardRollNegativeSpeed_get(void);
	void KeyboardRollNegativeSpeed_set(double value);
	
	double KeyboardFovPositiveSpeed_get(void);
	void KeyboardFovPositiveSpeed_set(double value);
	
	double KeyboardFovNegativeSpeed_get(void);
	void KeyboardFovNegativeSpeed_set(double value);
	
	double MouseYawSpeed_get(void);
	void MouseYawSpeed_set(double value);
	
	double MousePitchSpeed_get(void);
	void MousePitchSpeed_set(double value);

	bool Supply_CharEvent(WPARAM wParam, LPARAM lParam);
	bool Supply_KeyEvent(KeyState keyState, WPARAM wParam, LPARAM lParam);
	bool Supply_RawMouseMotion(int dX, int dY);
	void Supply_GetCursorPos(LPPOINT lpPoint);
	void Supply_SetCursorPos(int x, int y);
	void Supply_MouseFrameEnd(void);
	void Supply_Focus(bool hasFocus);

private:
	static const double m_CamSpeedFacMove;
	static const double m_CamSpeedFacRotate;
	static const double m_CamSpeedFacZoom;
	
	double m_FirstGetCursorPos;
	double m_MouseSens;
	double m_KeyboardSens;

	double m_KeyboardForwardSpeed;
	double m_KeyboardBackwardSpeed;
	double m_KeyboardLeftSpeed;
	double m_KeyboardRightSpeed;
	double m_KeyboardUpSpeed;
	double m_KeyboardDownSpeed;
	double m_KeyboardPitchPositiveSpeed;
	double m_KeyboardPitchNegativeSpeed;
	double m_KeyboardYawPositiveSpeed;
	double m_KeyboardYawNegativeSpeed;
	double m_KeyboardRollPositiveSpeed;
	double m_KeyboardRollNegativeSpeed;
	double m_KeyboardFovPositiveSpeed;
	double m_KeyboardFovNegativeSpeed;
	double m_MouseYawSpeed;
	double m_MousePitchSpeed;

	bool m_CamResetView;
	double m_CamForward;
	double m_CamForwardI;
	double m_CamLeft;
	double m_CamLeftI;
	double m_CamUp;
	double m_CamUpI;
	double m_CamFov;
	double m_CamFovI;
	double m_CamPitch;
	double m_CamPitchI;
	double m_CamPitchM;
	double m_CamYaw;
	double m_CamYawI;
	double m_CamYawM;
	double m_CamRoll;
	double m_CamRollI;
	double m_CamSpeed;
	bool m_CameraControlMode;
	bool m_Focus;
	bool m_IgnoreKeyUp;
	bool m_IgnoreNextKey;
	LONG m_LastCursorX;
	LONG m_LastCursorY;

	bool GetConsoleOpen(void);

	void DoCamSpeedDecrease(void);
	void DoCamSpeedIncrease(void);
};

extern AfxHookSourceInput g_AfxHookSourceInput;
