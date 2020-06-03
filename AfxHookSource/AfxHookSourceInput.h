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

	enum OffsetMode
	{
		OffsetMode_Last,
		OffsetMode_OwnLast,
		OffsetMode_Game,
		OffsetMode_Current
	};

	AfxHookSourceInput();

	bool Override(float & Tx, float &Ty, float & Tz, float & Rx, float & Ry, float & Rz, float & Fov);

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

	double MouseFovPositiveSpeed_get(void);
	void MouseFovPositiveSpeed_set(double value);

	double MouseFovNegativeSpeed_get(void);
	void MouseFovNegativeSpeed_set(double value);


	bool GetEnableMouseMove() { return m_MMove; }
	void SetEnableMouseMove(bool value) { m_MMove = value; }

	double GetMouseUpSpeed() { return m_MouseUpSpeed; }
	double GetMouseDownSpeed() { return m_MouseDownSpeed; }
	double GetMouseLeftSpeed() { return m_MouseLeftSpeed;  }
	double GetMouseRightSpeed() { return m_MouseRightSpeed;  }
	double GetMouseForwardSpeed() { return m_MouseForwardSpeed; }
	double GetMouseBackwardSpeed() { return m_MouseBackwardSpeed; }

	void SetMouseUpSpeed(double value) { m_MouseUpSpeed = value; }
	void SetMouseDownSpeed(double value) { m_MouseDownSpeed = value; }
	void SetMouseLeftSpeed(double value) { m_MouseLeftSpeed = value;  }
	void SetMouseRightSpeed(double value) { m_MouseRightSpeed = value; }
	void SetMouseForwardSpeed(double value) { m_MouseForwardSpeed = value; }
	void SetMouseBackwardSpeed(double value) { m_MouseBackwardSpeed = value; }

	bool Supply_CharEvent(WPARAM wParam, LPARAM lParam);
	bool Supply_KeyEvent(KeyState keyState, WPARAM wParam, LPARAM lParam);
	bool Supply_MouseEvent(DWORD uMsg, WPARAM & wParam, LPARAM & lParam);
	bool Supply_RawMouseMotion(int dX, int dY);
	void Supply_GetCursorPos(LPPOINT lpPoint);
	void Supply_SetCursorPos(int x, int y);
	void Supply_MouseFrameEnd(void);
	void Supply_Focus(bool hasFocus);

	OffsetMode GetOffsetMode() { return m_OffsetMode; }
	void SetOffsetMode(OffsetMode value) { m_OffsetMode = value; }

	void SetTx(float value)
	{
		m_SetTx = true;
		m_SetTxValue = value;
	}

	void SetTy(float value)
	{
		m_SetTy = true;
		m_SetTyValue = value;
	}

	void SetTz(float value)
	{
		m_SetTz = true;
		m_SetTzValue = value;
	}

	void SetRx(float value)
	{
		m_SetRx = true;
		m_SetRxValue = value;
	}

	void SetRy(float value)
	{
		m_SetRy = true;
		m_SetRyValue = value;
	}

	void SetRz(float value)
	{
		m_SetRz = true;
		m_SetRzValue = value;
	}

	void SetFov(float value)
	{
		m_SetFov = true;
		m_SetFovValue = value;
	}

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
	double m_MouseUpSpeed;
	double m_MouseDownSpeed;
	double m_MouseForwardSpeed;
	double m_MouseBackwardSpeed;
	double m_MouseLeftSpeed;
	double m_MouseRightSpeed;
	double m_MouseFovPositiveSpeed;
	double m_MouseFovNegativeSpeed;

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
	bool m_MMove = false;
	bool m_MLDown;
	bool m_MLWasDown = false;
	bool m_MRDown;
	bool m_MRWasDown = false;
	bool m_MouseFov = true;
	bool m_CameraControlMode;
	bool m_Focus;
	bool m_IgnoreKeyUp;
	bool m_IgnoreNextKey;
	LONG m_LastCursorX;
	LONG m_LastCursorY;

	bool m_SetTx = false;
	bool m_SetTy = false;
	bool m_SetTz = false;
	bool m_SetRx = false;
	bool m_SetRy = false;
	bool m_SetRz = false;
	bool m_SetFov = false;

	float m_SetTxValue;
	float m_SetTyValue;
	float m_SetTzValue;
	float m_SetRxValue;
	float m_SetRyValue;
	float m_SetRzValue;
	float m_SetFovValue;

	OffsetMode m_OffsetMode = OffsetMode_Last;

	bool GetConsoleOpen(void);

	void DoCamSpeedDecrease(void);
	void DoCamSpeedIncrease(void);
};

extern AfxHookSourceInput g_AfxHookSourceInput;
