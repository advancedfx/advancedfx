#include "stdafx.h"

#include "AfxHookSourceInput.h"

#include "WrpVEngineClient.h"


extern WrpVEngineClient * g_VEngineClient;

AfxHookSourceInput g_AfxHookSourceInput;

AfxHookSourceInput::AfxHookSourceInput()
: m_CameraControlMode(false)
, m_CamResetView(false)
, m_CamForward(0.0)
, m_CamForwardI(0.0)
, m_CamLeft(0.0)
, m_CamLeftI(0.0)
, m_CamUp(0.0)
, m_CamUpI(0.0)
, m_CamFov(0.0)
, m_CamFovI(0.0)
, m_CamPitch(0.0)
, m_CamPitchI(0.0)
, m_CamPitchM(0.0)
, m_CamYaw(0.0)
, m_CamYawI(0.0)
, m_CamYawM(0.0)
, m_CamRoll(0.0)
, m_CamRollI(0.0)
, m_CamSpeed(1.0)
, m_Focus(true)
, m_IgnoreKeyUp(false)
, m_IgnoreNextKey(false)
, m_MouseSens(1.0/10)
, m_KeyboardSens(1.0)
, m_KeyboardForwardSpeed(320.0)
, m_KeyboardBackwardSpeed(320.0)
, m_KeyboardLeftSpeed(320.0)
, m_KeyboardRightSpeed(320.0)
, m_KeyboardUpSpeed(320.0)
, m_KeyboardDownSpeed(320.0)
, m_KeyboardPitchPositiveSpeed(180.0)
, m_KeyboardPitchNegativeSpeed(180.0)
, m_KeyboardYawPositiveSpeed(180.0)
, m_KeyboardYawNegativeSpeed(180.0)
, m_KeyboardRollPositiveSpeed(180.0)
, m_KeyboardRollNegativeSpeed(180.0)
, m_KeyboardFovPositiveSpeed(10.0)
, m_KeyboardFovNegativeSpeed(10.0)
, m_MouseYawSpeed(180.0)
, m_MousePitchSpeed(180.0)
, m_FirstGetCursorPos(true)
, m_LastCursorX(0)
, m_LastCursorY(0)
, m_MouseUpSpeed(320.0)
, m_MouseDownSpeed(320.0)
, m_MouseForwardSpeed(320.0)
, m_MouseBackwardSpeed(320.0)
, m_MouseLeftSpeed(320.0)
, m_MouseRightSpeed(320.0)
, m_MLDown(false)
, m_MRDown(false)
{
}

void AfxHookSourceInput::DoCamSpeedDecrease(void)
{
	m_CamSpeed = m_CamSpeed / 2;
	if(m_CamSpeed < 1.0/256) m_CamSpeed = 1.0/256;
}

void AfxHookSourceInput::DoCamSpeedIncrease(void)
{
	m_CamSpeed = m_CamSpeed * 2;
	if(m_CamSpeed > 256) m_CamSpeed = 256;
}

bool AfxHookSourceInput::GetCamResetView(void)
{
	bool result = m_CamResetView;
	m_CamResetView = false;
	return result;
}

double AfxHookSourceInput::GetCamDForward(void)
{
	return m_CamSpeed * (m_CamForward -m_CamForwardI);
}

double AfxHookSourceInput::GetCamDLeft(void)
{
	return m_CamSpeed * (m_CamLeft -m_CamLeftI);
}

double AfxHookSourceInput::GetCamDUp(void)
{
	return m_CamSpeed * (m_CamUp -m_CamUpI);
}

double AfxHookSourceInput::GetCamDPitch(void)
{
	return m_CamSpeed * (m_CamPitch -m_CamPitchI +m_CamPitchM);
}

double AfxHookSourceInput::GetCamDYaw(void)
{
	return m_CamSpeed * (m_CamYaw -m_CamYawI +m_CamYawM);
}

double AfxHookSourceInput::GetCamDRoll(void)
{
	return m_CamSpeed * (m_CamRoll -m_CamRollI);
}

double AfxHookSourceInput::GetCamDFov(void)
{
	return m_CamSpeed * (m_CamFov -m_CamFovI);
}

bool AfxHookSourceInput::GetCameraControlMode(void)
{
	return m_CameraControlMode;
}

void AfxHookSourceInput::SetCameraControlMode(bool enable)
{
	m_CameraControlMode = enable;
}

double AfxHookSourceInput::GetKeyboardSensitivty(void)
{
	return m_KeyboardSens;
}

void AfxHookSourceInput::SetKeyboardSensitivity(double value)
{
	m_KeyboardSens = value;
}

double AfxHookSourceInput::GetMouseSensitivty(void)
{
	return m_MouseSens;
}

void AfxHookSourceInput::SetMouseSensitivity(double value)
{
	m_MouseSens = value;
}

double AfxHookSourceInput::KeyboardForwardSpeed_get(void)
{
	return m_KeyboardForwardSpeed;
}

void AfxHookSourceInput::KeyboardForwardSpeed_set(double value)
{
	m_KeyboardForwardSpeed = value;
}
	
double AfxHookSourceInput::KeyboardBackwardSpeed_get(void)
{
	return m_KeyboardBackwardSpeed;
}

void AfxHookSourceInput::KeyboardBackwardSpeed_set(double value)
{
	m_KeyboardBackwardSpeed = value;
}
	
double AfxHookSourceInput::KeyboardLeftSpeed_get(void)
{
	return m_KeyboardLeftSpeed;
}

void AfxHookSourceInput::KeyboardLeftSpeed_set(double value)
{
	m_KeyboardLeftSpeed = value;
}
	
double AfxHookSourceInput::KeyboardRightSpeed_get(void)
{
	return m_KeyboardRightSpeed;
}

void AfxHookSourceInput::KeyboardRightSpeed_set(double value)
{
	m_KeyboardRightSpeed = value;
}
	
double AfxHookSourceInput::KeyboardUpSpeed_get(void)
{
	return m_KeyboardUpSpeed;
}

void AfxHookSourceInput::KeyboardUpSpeed_set(double value)
{
	m_KeyboardUpSpeed = value;
}
	
double AfxHookSourceInput::KeyboardDownSpeed_get(void)
{
	return m_KeyboardDownSpeed;
}

void AfxHookSourceInput::KeyboardDownSpeed_set(double value)
{
	m_KeyboardDownSpeed = value;
}
	
double AfxHookSourceInput::KeyboardPitchPositiveSpeed_get(void)
{
	return m_KeyboardPitchPositiveSpeed;
}
void AfxHookSourceInput::KeyboardPitchPositiveSpeed_set(double value)
{
	m_KeyboardPitchPositiveSpeed = value;
}
	
double AfxHookSourceInput::KeyboardPitchNegativeSpeed_get(void)
{
	return m_KeyboardPitchNegativeSpeed;
}

void AfxHookSourceInput::KeyboardPitchNegativeSpeed_set(double value)
{
	m_KeyboardPitchNegativeSpeed = value;
}
	
double AfxHookSourceInput::KeyboardYawPositiveSpeed_get(void)
{
	return m_KeyboardYawPositiveSpeed;
}

void AfxHookSourceInput::KeyboardYawPositiveSpeed_set(double value)
{
	m_KeyboardYawPositiveSpeed = value;
}
	
double AfxHookSourceInput::KeyboardYawNegativeSpeed_get(void)
{
	return m_KeyboardYawNegativeSpeed;
}

void AfxHookSourceInput::KeyboardYawNegativeSpeed_set(double value)
{
	m_KeyboardYawNegativeSpeed = value;
}
	
double AfxHookSourceInput::KeyboardRollPositiveSpeed_get(void)
{
	return m_KeyboardRollPositiveSpeed;
}

void AfxHookSourceInput::KeyboardRollPositiveSpeed_set(double value)
{
	m_KeyboardRollPositiveSpeed = value;
}
	
double AfxHookSourceInput::KeyboardRollNegativeSpeed_get(void)
{
	return m_KeyboardRollNegativeSpeed;
}

void AfxHookSourceInput::KeyboardRollNegativeSpeed_set(double value)
{
	m_KeyboardRollNegativeSpeed = value;
}
	
double AfxHookSourceInput::KeyboardFovPositiveSpeed_get(void)
{
	return m_KeyboardFovPositiveSpeed;
}
void AfxHookSourceInput::KeyboardFovPositiveSpeed_set(double value)
{
	m_KeyboardFovPositiveSpeed = value;
}
	
double AfxHookSourceInput::KeyboardFovNegativeSpeed_get(void)
{
	return m_KeyboardFovNegativeSpeed;
}

void AfxHookSourceInput::KeyboardFovNegativeSpeed_set(double value)
{
	m_KeyboardFovNegativeSpeed = value;
}
	
double AfxHookSourceInput::MouseYawSpeed_get(void)
{
	return m_MouseYawSpeed;
}

void AfxHookSourceInput::MouseYawSpeed_set(double value)
{
	m_MouseYawSpeed = value;
}
	
double AfxHookSourceInput::MousePitchSpeed_get(void)
{
	return m_MousePitchSpeed;
}

void AfxHookSourceInput::MousePitchSpeed_set(double value)
{
	m_MousePitchSpeed = value;
}


bool AfxHookSourceInput::Supply_CharEvent(WPARAM wParam, LPARAM lParam)
{
	if(!m_Focus)
		return false;

	if(GetConsoleOpen())
		return false;

	if(m_IgnoreNextKey)
	{
		return false;
	}

	if(m_CameraControlMode)
	{
		switch(wParam)
		{
		case '+':
			{
				for(int i=0; i<=(lParam&0xFFFF); i++) DoCamSpeedIncrease();
			}
			return true;
		case '-':
			{
				for(int i=0; i<=(lParam&0xFFFF); i++) DoCamSpeedDecrease();
			}
			return true;
		}
		return true;
	}

	return false;
}

bool AfxHookSourceInput::Supply_KeyEvent(KeyState keyState, WPARAM wParam, LPARAM lParam)
{
	if(!m_Focus)
		return false;

	if(GetConsoleOpen())
	{
		m_IgnoreKeyUp = KS_DOWN == keyState;
		return false;
	}

	if(m_IgnoreKeyUp && KS_UP == keyState)
	{
		m_IgnoreKeyUp = false;
		return false;
	}

	if(m_IgnoreNextKey)
	{
		if(KS_UP == keyState)
			m_IgnoreNextKey = false;

		return false;
	}

	if(m_CameraControlMode)
	{
		switch(wParam)
		{
		case VK_ESCAPE:
			if(KS_UP == keyState) m_CameraControlMode = false;
			return true;
		case VK_CONTROL:
			if(KS_UP == keyState) m_IgnoreNextKey = true;
			return true;
		case VK_HOME:
		case VK_NUMPAD5:
			if(KS_DOWN == keyState)
			{
				m_CamResetView = true;
				m_CamSpeed = 1.0;
			}
			return true;
		case 0x57: // W key
		case VK_NUMPAD8:
			m_CamForward = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardForwardSpeed : 0.0;
			return true;
		case 0x53: // S key
		case VK_NUMPAD2:
			m_CamForwardI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardBackwardSpeed : 0.0;
			return true;
		case 0x41: // A key
		case VK_NUMPAD4:
			m_CamLeft = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardLeftSpeed : 0.0;
			return true;
		case 0x44: // D key
		case VK_NUMPAD6:
			m_CamLeftI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardRightSpeed : 0.0;
			return true;
		case 0x52: // R key
		case VK_NUMPAD9:
			m_CamUp = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardUpSpeed : 0.0;
			return true;
		case 0x46: // F key
		case VK_NUMPAD3:
			m_CamUpI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardDownSpeed : 0.0;
			return true;
		case VK_NUMPAD1:
		case VK_NEXT:
			m_CamFov = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardFovPositiveSpeed : 0.0;
			return true;
		case VK_NUMPAD7:
		case VK_PRIOR:
			m_CamFovI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardFovNegativeSpeed : 0.0;
			return true;
		case 0x58: // X key
		case VK_DECIMAL:
			m_CamRoll = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardRollPositiveSpeed : 0.0;
			return true;
		case 0x5A: // Z key
		case VK_NUMPAD0:
			m_CamRollI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardRollNegativeSpeed : 0.0;
			return true;
		case VK_DOWN:
			m_CamPitch = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardPitchPositiveSpeed : 0.0;
			return true;
		case VK_UP:
			m_CamPitchI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardPitchNegativeSpeed : 0.0;
			return true;
		case VK_LEFT:
			m_CamYaw = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardYawPositiveSpeed : 0.0;
			return true;
		case VK_RIGHT:
			m_CamYawI = KS_DOWN == keyState ? m_KeyboardSens * m_KeyboardYawNegativeSpeed : 0.0;
			return true;
		}
	}

	return false;
}

bool  AfxHookSourceInput::Supply_MouseEvent(DWORD uMsg, WPARAM & wParam, LPARAM & lParam)
{
	if (!m_Focus)
		return false;

	if (GetConsoleOpen())
	{
		return false;
	}

	if (m_CameraControlMode && m_MMove)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDBLCLK:
			return true;
		case WM_LBUTTONDOWN:
			m_MLDown = true;
			return true;
		case WM_LBUTTONUP:
			m_MLDown = false;
			return true;
		case WM_RBUTTONDBLCLK:
			return true;
		case WM_RBUTTONDOWN:
			m_MRDown = true;
			return true;
		case WM_RBUTTONUP:
			m_MRDown = false;
			return true;
		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON)
			{
				m_MLDown = true;
				wParam &= ~(WPARAM)MK_LBUTTON;
			}
			if (wParam & MK_RBUTTON)
			{
				m_MRDown = true;
				wParam &= ~(WPARAM)MK_RBUTTON;
			}
			break;
		}
	}

	return false;
}

bool AfxHookSourceInput::Supply_RawMouseMotion(int dX, int dY)
{
	if(!m_Focus)
		return false;

	if(GetConsoleOpen())
		return false;

	if(m_CameraControlMode)
	{
		if (!(m_MMove && (m_MLDown || m_MRDown)))
		{
			m_CamYawM += m_MouseSens * m_MouseYawSpeed * -dX;
			m_CamPitchM += m_MouseSens * m_MousePitchSpeed * dY;
		}
		else
		{
			if (0 <= dX) m_CamLeftI += m_MouseSens * m_MouseRightSpeed * dX;
			else m_CamLeft += m_MouseSens * m_MouseLeftSpeed * - dX;

			if (m_MLDown)
			{
				m_MLWasDown = true;
				if (0 <= dY) m_CamForwardI += m_MouseSens * m_MouseBackwardSpeed * dY;
				else m_CamForward += m_MouseSens * m_MouseForwardSpeed * -dY;
			}
			if (m_MRDown)
			{
				m_MRWasDown = true;
				if (0 <= dY) m_CamUpI += m_MouseSens * m_MouseDownSpeed * dY;
				else m_CamUp += m_MouseSens * m_MouseUpSpeed * -dY;
			}
		}

		return true;
	}

	return false;
}

void AfxHookSourceInput::Supply_GetCursorPos(LPPOINT lpPoint)
{
	if(!lpPoint)
		return;

	if(!m_Focus)
		return;

	if(GetConsoleOpen())
		return;

	if(m_CameraControlMode)
	{
		if(m_FirstGetCursorPos)
		{
			m_FirstGetCursorPos = false;

			// Clear anything from raw input, since we override it with CursorPos method:
			m_CamYawM = 0.0;
			m_CamPitchM = 0.0;
		}

		// this will not work correctly if there was no SetCursorPos call prior to this call:
		LONG dX = lpPoint->x -m_LastCursorX;
		LONG dY = lpPoint->y -m_LastCursorY;

		m_CamYawM += m_MouseSens * m_MouseYawSpeed * -dX;
		m_CamPitchM += m_MouseSens * m_MousePitchSpeed * dY;

		// pretend we didn't move from last SetCursorPos call:
		lpPoint->x = m_LastCursorX;
		lpPoint->y = m_LastCursorY;
	}
}

void AfxHookSourceInput::Supply_SetCursorPos(int x, int y)
{
	m_LastCursorX = x;
	m_LastCursorY = y;
}

void AfxHookSourceInput::Supply_MouseFrameEnd(void)
{
	if(!m_Focus)
		return;

	if(GetConsoleOpen())
		return;

	if(m_CameraControlMode)
	{
		if(m_FirstGetCursorPos)
		{
			// TODO: HACK: In POV-Demos only rawinput is available, because
			// they don't call SetCursorPos / GetCursorPos to read player mouse
			// input. We make room to move here (so the mouse won't leave
			// the window).
			SetCursorPos(m_LastCursorX, m_LastCursorY);
		}

		m_CamYawM = 0.0;
		m_CamPitchM = 0.0;
		m_FirstGetCursorPos = true;

		if (m_MLWasDown || m_MRWasDown)
		{
			m_CamLeft = 0.0;
			m_CamLeftI = 0.0;

			if (m_MLWasDown)
			{
				m_MLWasDown = false;
				m_CamForward = 0.0;
				m_CamForwardI = 0.0;
			}

			if (m_MRWasDown)
			{
				m_MRWasDown = false;
				m_CamUp = 0.0;
				m_CamUpI = 0.0;
			}
		}
	}
}

void AfxHookSourceInput::Supply_Focus(bool hasFocus)
{
	m_Focus = hasFocus;
}

bool AfxHookSourceInput::GetConsoleOpen(void)
{
	return
		g_VEngineClient
		&& g_VEngineClient->Con_IsVisible()
	;
}
