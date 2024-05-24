#include "stdafx.h"

#include "MirvInput.h"

#include "AfxConsole.h"
#include "StringTools.h"

#include "../deps/release/rapidxml/rapidxml.hpp"
#include "../deps/release/rapidxml/rapidxml_print.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#undef min
#undef max

void MirvInput::Mem::Console(advancedfx::ICommandArgs * args)
{
	int argc = args->ArgC();

	char const * cmd0 = 0 < argc ? args->ArgV(0) : "n/a";

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("store", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			double x,y,z,rx,ry,rz,fov;
			m_Input->m_Dependencies->GetLastCameraData(x,y,z,rx,ry,rz,fov);

			m_Map[std::string(name)] = CData(
				x
				, y
				, z
				, rx
				, ry
				, rz
				, fov
			);

			return;
		}
		else if (!_stricmp("use", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			bool useOrigin = false;
			bool useAngles = false;
			bool useFov = false;

			for (int i = 3; i < argc; ++i)
			{
				char const * opt = args->ArgV(i);

				if (!_stricmp("origin", opt))
					useOrigin = true;
				if (!_stricmp("angles", opt))
					useAngles = true;
				if (!_stricmp("fov", opt))
					useFov = true;
			}

			if (!(useOrigin || useAngles || useFov))
				useOrigin = useAngles = useFov = true;

			std::map<std::string, CData>::iterator it = m_Map.find(std::string(name));

			if (it != m_Map.end())
			{
				CData & data = it->second;

				if (useOrigin)
				{
					m_Input->SetTx(data.Origin[0]);
					m_Input->SetTy(data.Origin[1]);
					m_Input->SetTz(data.Origin[2]);
				}
				if (useAngles)
				{
					m_Input->SetRx(data.Angles[0]);
					m_Input->SetRy(data.Angles[1]);
					m_Input->SetRz(data.Angles[2]);
				}
				if (useFov)
				{
					m_Input->SetFov(data.Fov);
				}

				return;
			}
			advancedfx::Message("Error: There is no state with name %s.\n", name);
			return;
		}
		else if (!_stricmp("remove", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			std::map<std::string, CData>::iterator it = m_Map.find(std::string(name));

			if (it != m_Map.end())
			{
				m_Map.erase(it);

				return;
			}
			advancedfx::Message("Error: There is no state with name %s.\n", name);
			return;
		}
		else if (!_stricmp("print", cmd1))
		{
			advancedfx::Message("name: x y z | xRoll yPitch zYaw | fov\n");
			for (std::map<std::string, CData>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
			{
				char const * name = it->first.c_str();
				CData & data = it->second;

				advancedfx::Message(
					"%s: %f %f %f | %f %f %f | %f\n"
					, name
					, data.Origin[0], data.Origin[1], data.Origin[2]
					, data.Angles[1], data.Angles[0], data.Angles[2]
					, data.Fov
				);
			}

			return;
		}
		else if (!_stricmp("clear", cmd1))
		{
			m_Map.clear();
			return;
		}
		else if (!_stricmp("save", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			std::wstring wideString;
			bool bOk = UTF8StringToWideString(name, wideString)
				&& Save(wideString.c_str());

			advancedfx::Message("Saving: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if (!_stricmp("load", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			std::wstring wideString;
			bool bOk = UTF8StringToWideString(name, wideString)
				&& Load(wideString.c_str());

			advancedfx::Message("Loading: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}

	}

	advancedfx::Message(
		"%s store <name> - Store current view state.\n"
		"%s use <name> [origin] [angles] [fov] - Restore view state, if only name is given, then all properties are restored, otherwise only the given properties.\n"
		"%s remove <name> - Remove stored state.\n"
		"%s print - Print currently stored states.\n"
		"%s clear - Clear all stored states.\n"
		"%s save <fileName> - Save all states to file in XML format.\n"
		"%s load <fileName> - Load states from file (adding to existing states / overwriting states with same name).\n"
		, cmd0
		, cmd0
		, cmd0
		, cmd0
		, cmd0
		, cmd0
		, cmd0
	);
}

char * MirvInput_double2xml(rapidxml::xml_document<> & doc, double value)
{
	char szTmp[196];
	_snprintf_s(szTmp, _TRUNCATE,"%f", value);
	return doc.allocate_string(szTmp);
}

bool MirvInput::Mem::Save(wchar_t const * fileName)
{
	rapidxml::xml_document<> doc;

	rapidxml::xml_node<> * decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	rapidxml::xml_node<> * mirvInput = doc.allocate_node(rapidxml::node_element, "mirvInput");
	doc.append_node(mirvInput);

	rapidxml::xml_node<> * states = doc.allocate_node(rapidxml::node_element, "states");
	mirvInput->append_node(states);

	for (std::map<std::string, CData>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		char const * name = it->first.c_str();
		CData & data = it->second;

		rapidxml::xml_node<> * state = doc.allocate_node(rapidxml::node_element, "s");
		state->append_attribute(doc.allocate_attribute("name", name));
		state->append_attribute(doc.allocate_attribute("x", MirvInput_double2xml(doc, data.Origin[0])));
		state->append_attribute(doc.allocate_attribute("y", MirvInput_double2xml(doc, data.Origin[1])));
		state->append_attribute(doc.allocate_attribute("z", MirvInput_double2xml(doc, data.Origin[2])));
		state->append_attribute(doc.allocate_attribute("rx", MirvInput_double2xml(doc, data.Angles[1])));
		state->append_attribute(doc.allocate_attribute("ry", MirvInput_double2xml(doc, data.Angles[0])));
		state->append_attribute(doc.allocate_attribute("rz", MirvInput_double2xml(doc, data.Angles[2])));
		state->append_attribute(doc.allocate_attribute("fov", MirvInput_double2xml(doc, data.Fov)));

		states->append_node(state);
	}

	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	FILE * pFile = 0;
	_wfopen_s(&pFile, fileName, L"wb");

	if (0 != pFile)
	{
		fputs(xmlString.c_str(), pFile);
		fclose(pFile);
		return true;
	}

	return false;
}

bool MirvInput::Mem::Load(wchar_t const * fileName)
{
	bool bOk = false;

	FILE * pFile = 0;

	_wfopen_s(&pFile, fileName, L"rb");

	if (!pFile)
		return false;

	fseek(pFile, 0, SEEK_END);
	size_t fileSize = ftell(pFile);
	rewind(pFile);

	char * pData = new char[fileSize + 1];
	pData[fileSize] = 0;

	size_t readSize = fread(pData, sizeof(char), fileSize, pFile);
	bOk = readSize == fileSize;
	if (bOk)
	{
		try
		{
			do
			{
				rapidxml::xml_document<> doc;
				doc.parse<0>(pData);

				rapidxml::xml_node<> * cur_node = doc.first_node("mirvInput");
				if (!cur_node) break;

				cur_node = cur_node->first_node("states");
				if (!cur_node) break;

				for (cur_node = cur_node->first_node("s"); cur_node; cur_node = cur_node->next_sibling("s"))
				{
					rapidxml::xml_attribute<> * nameAttr = cur_node->first_attribute("name");
					if (!nameAttr) continue;

					rapidxml::xml_attribute<> * xA = cur_node->first_attribute("x");
					rapidxml::xml_attribute<> * yA = cur_node->first_attribute("y");
					rapidxml::xml_attribute<> * zA = cur_node->first_attribute("z");
					rapidxml::xml_attribute<> * rxA = cur_node->first_attribute("rx");
					rapidxml::xml_attribute<> * ryA = cur_node->first_attribute("ry");
					rapidxml::xml_attribute<> * rzA = cur_node->first_attribute("rz");
					rapidxml::xml_attribute<> * fovA = cur_node->first_attribute("fov");

					double dX = xA ? atof(xA->value()) : 0.0;
					double dY = yA ? atof(yA->value()) : 0.0;
					double dZ = zA ? atof(zA->value()) : 0.0;
					double dRX = rxA ? atof(rxA->value()) : 0.0;
					double dRY = ryA ? atof(ryA->value()) : 0.0;
					double dRZ = rzA ? atof(rzA->value()) : 0.0;
					double dFov = fovA ? atof(fovA->value()) : 90.0;

					m_Map[std::string(nameAttr->value())] = CData(
						dX, dY, dZ
						, dRY, dRX, dRZ
						, dFov
					);
				}
			} while (false);
		}
		catch (rapidxml::parse_error &)
		{
			bOk = false;
		}
	}

	delete pData;

	fclose(pFile);

	return bOk;
}

MirvInput::MirvInput(IMirvInputDependencies * dependencies)
: m_Dependencies(dependencies)
, m_CameraControlMode(false)
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
, m_CamYaw(0.0)
, m_CamYawI(0.0)
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
, m_MouseFovPositiveSpeed(45.0)
, m_MouseFovNegativeSpeed(45.0)
{
	m_Mem.Connect(this);
}

void MirvInput::DoCamSpeedDecrease(void)
{
	m_CamSpeed = m_CamSpeed / m_CamSpeedBasis;
	if(m_CamSpeed < 1.0/256) m_CamSpeed = 1.0/256;
}

void MirvInput::DoCamSpeedIncrease(void)
{
	m_CamSpeed = m_CamSpeed * m_CamSpeedBasis;
	if(m_CamSpeed > 256) m_CamSpeed = 256;
}

bool MirvInput::GetCamResetView(void)
{
	bool result = m_CamResetView;
	m_CamResetView = false;
	return result;
}

double MirvInput::GetCamDForward(void)
{
	return m_CamSpeed * (m_CamForward -m_CamForwardI +m_MouseInput.GetForward());
}

double MirvInput::GetCamDLeft(void)
{
	return m_CamSpeed * (m_CamLeft -m_CamLeftI +m_MouseInput.GetLeft());
}

double MirvInput::GetCamDUp(void)
{
	return m_CamSpeed * (m_CamUp -m_CamUpI +m_MouseInput.GetUp());
}

double MirvInput::GetCamDPitch(void)
{
	return m_CamSpeed * (m_CamPitch -m_CamPitchI +m_MouseInput.GetPitch());
}

double MirvInput::GetCamDYaw(void)
{
	return m_CamSpeed * (m_CamYaw -m_CamYawI +m_MouseInput.GetYaw());
}

double MirvInput::GetCamDRoll(void)
{
	return m_CamSpeed * (m_CamRoll -m_CamRollI);
}

double MirvInput::GetCamDFov(void)
{
	return m_CamSpeed * (m_CamFov -m_CamFovI +m_MouseInput.GetFov());
}

bool MirvInput::GetCameraControlMode(void)
{
	return m_CameraControlMode;
}

void MirvInput::SetCameraControlMode(bool enable)
{
	m_CameraControlMode = enable;
}

double MirvInput::GetKeyboardSensitivty(void)
{
	return m_KeyboardSens;
}

void MirvInput::SetKeyboardSensitivity(double value)
{
	m_KeyboardSens = value;
}

double MirvInput::GetMouseSensitivty(void)
{
	return m_MouseSens;
}

void MirvInput::SetMouseSensitivity(double value)
{
	m_MouseSens = value;
}

double MirvInput::KeyboardForwardSpeed_get(void)
{
	return m_KeyboardForwardSpeed;
}

void MirvInput::KeyboardForwardSpeed_set(double value)
{
	m_KeyboardForwardSpeed = value;
}
	
double MirvInput::KeyboardBackwardSpeed_get(void)
{
	return m_KeyboardBackwardSpeed;
}

void MirvInput::KeyboardBackwardSpeed_set(double value)
{
	m_KeyboardBackwardSpeed = value;
}
	
double MirvInput::KeyboardLeftSpeed_get(void)
{
	return m_KeyboardLeftSpeed;
}

void MirvInput::KeyboardLeftSpeed_set(double value)
{
	m_KeyboardLeftSpeed = value;
}
	
double MirvInput::KeyboardRightSpeed_get(void)
{
	return m_KeyboardRightSpeed;
}

void MirvInput::KeyboardRightSpeed_set(double value)
{
	m_KeyboardRightSpeed = value;
}
	
double MirvInput::KeyboardUpSpeed_get(void)
{
	return m_KeyboardUpSpeed;
}

void MirvInput::KeyboardUpSpeed_set(double value)
{
	m_KeyboardUpSpeed = value;
}
	
double MirvInput::KeyboardDownSpeed_get(void)
{
	return m_KeyboardDownSpeed;
}

void MirvInput::KeyboardDownSpeed_set(double value)
{
	m_KeyboardDownSpeed = value;
}
	
double MirvInput::KeyboardPitchPositiveSpeed_get(void)
{
	return m_KeyboardPitchPositiveSpeed;
}
void MirvInput::KeyboardPitchPositiveSpeed_set(double value)
{
	m_KeyboardPitchPositiveSpeed = value;
}
	
double MirvInput::KeyboardPitchNegativeSpeed_get(void)
{
	return m_KeyboardPitchNegativeSpeed;
}

void MirvInput::KeyboardPitchNegativeSpeed_set(double value)
{
	m_KeyboardPitchNegativeSpeed = value;
}
	
double MirvInput::KeyboardYawPositiveSpeed_get(void)
{
	return m_KeyboardYawPositiveSpeed;
}

void MirvInput::KeyboardYawPositiveSpeed_set(double value)
{
	m_KeyboardYawPositiveSpeed = value;
}
	
double MirvInput::KeyboardYawNegativeSpeed_get(void)
{
	return m_KeyboardYawNegativeSpeed;
}

void MirvInput::KeyboardYawNegativeSpeed_set(double value)
{
	m_KeyboardYawNegativeSpeed = value;
}
	
double MirvInput::KeyboardRollPositiveSpeed_get(void)
{
	return m_KeyboardRollPositiveSpeed;
}

void MirvInput::KeyboardRollPositiveSpeed_set(double value)
{
	m_KeyboardRollPositiveSpeed = value;
}
	
double MirvInput::KeyboardRollNegativeSpeed_get(void)
{
	return m_KeyboardRollNegativeSpeed;
}

void MirvInput::KeyboardRollNegativeSpeed_set(double value)
{
	m_KeyboardRollNegativeSpeed = value;
}
	
double MirvInput::KeyboardFovPositiveSpeed_get(void)
{
	return m_KeyboardFovPositiveSpeed;
}
void MirvInput::KeyboardFovPositiveSpeed_set(double value)
{
	m_KeyboardFovPositiveSpeed = value;
}
	
double MirvInput::KeyboardFovNegativeSpeed_get(void)
{
	return m_KeyboardFovNegativeSpeed;
}

void MirvInput::KeyboardFovNegativeSpeed_set(double value)
{
	m_KeyboardFovNegativeSpeed = value;
}
	
double MirvInput::MouseYawSpeed_get(void)
{
	return m_MouseYawSpeed;
}

void MirvInput::MouseYawSpeed_set(double value)
{
	m_MouseYawSpeed = value;
}
	
double MirvInput::MousePitchSpeed_get(void)
{
	return m_MousePitchSpeed;
}

void MirvInput::MousePitchSpeed_set(double value)
{
	m_MousePitchSpeed = value;
}

double MirvInput::MouseFovPositiveSpeed_get(void)
{
	return m_MouseFovPositiveSpeed;
}

void MirvInput::MouseFovPositiveSpeed_set(double value)
{
	m_MouseFovPositiveSpeed = value;
}


double MirvInput::MouseFovNegativeSpeed_get(void)
{
	return m_MouseFovNegativeSpeed;
}

void MirvInput::MouseFovNegativeSpeed_set(double value)
{
	m_MouseFovNegativeSpeed = value;
}

bool MirvInput::Supply_CharEvent(WPARAM wParam, LPARAM lParam)
{
	if(!m_Focus)
		return false;

	if(m_Dependencies->GetSuspendMirvInput())
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

bool MirvInput::Supply_KeyEvent(KeyState keyState, WPARAM wParam, LPARAM lParam)
{
	if(!m_Focus)
		return false;

	if(m_Dependencies->GetSuspendMirvInput())
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

bool  MirvInput::Supply_MouseEvent(DWORD uMsg, WPARAM & wParam, LPARAM & lParam)
{
	if (!m_Focus)
		return false;

	if (m_Dependencies->GetSuspendMirvInput())
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
			m_MouseInput.Normal.LeftButtonDown = true;
			m_MNormalLeftButtonWasDown = true;
			return true;
		case WM_LBUTTONUP:
			m_MouseInput.Normal.LeftButtonDown = false;
			return true;
		case WM_RBUTTONDBLCLK:
			return true;
		case WM_RBUTTONDOWN:
			m_MouseInput.Normal.RightButtonDown = true;
			m_MNormalRightButtonWasDown = false;
			return true;
		case WM_RBUTTONUP:
			m_MouseInput.Normal.RightButtonDown = false;
			return true;
		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON)
			{
				m_MouseInput.Normal.LeftButtonDown = true;
				m_MNormalLeftButtonWasDown = true;
				wParam &= ~(WPARAM)MK_LBUTTON;
			} else {
				m_MouseInput.Normal.LeftButtonDown = false;
			}
			if (wParam & MK_RBUTTON)
			{
				m_MouseInput.Normal.RightButtonDown = true;
				m_MNormalRightButtonWasDown = true;
				wParam &= ~(WPARAM)MK_RBUTTON;
			} else {
				m_MouseInput.Normal.RightButtonDown = false;
			}
			break;
		case WM_MOUSEWHEEL:
			{
				signed short delta = GET_WHEEL_DELTA_WPARAM(wParam);

				m_MouseInput.Normal.Fov += m_MouseSens * (0 < delta ? m_MouseFovNegativeSpeed : m_MouseFovPositiveSpeed) * -delta;
			}
			return true;
		}
	}

	return false;
}

void MirvInput::ProcessRawInputData(PRAWINPUT pData) {
	switch (pData->header.dwType) {
	case RIM_TYPEMOUSE: {
		RAWMOUSE* rawmouse = &(pData->data.mouse);
		int dX = 0;
		int dY = 0;
		int wheelDelta = 0;

		if ((rawmouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
		{
			dX = rawmouse->lLastX;
			dY = rawmouse->lLastY;

			if (m_CameraControlMode) {
				rawmouse->lLastX = 0;
				rawmouse->lLastY = 0;
			}
		}
		else
		{
			static bool initial = true;
			static LONG lastX = 0;
			static LONG lastY = 0;

			if (initial)
			{
				initial = false;
				lastX = rawmouse->lLastX;
				lastY = rawmouse->lLastY;
			}

			dX = rawmouse->lLastX - lastX;
			dY = rawmouse->lLastY - lastY;

			lastX = rawmouse->lLastX;
			lastY = rawmouse->lLastY;

			if (m_CameraControlMode) {
				rawmouse->lLastX -= dX;
				rawmouse->lLastY -= dY;
			}
		}

		m_MouseInput.Raw.LeftButtonDown = m_MouseInput.Raw.LeftButtonDown || (rawmouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN);
		m_MouseInput.Raw.RightButtonDown = m_MouseInput.Raw.RightButtonDown || (rawmouse->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN);


		if (m_CameraControlMode) {
			if (!(m_MMove && (m_MouseInput.Raw.LeftButtonDown || m_MouseInput.Raw.RightButtonDown)))
			{
				m_MouseInput.Raw.Yaw += m_MouseSens * m_MouseYawSpeed * -dX;
				m_MouseInput.Raw.Pitch += m_MouseSens * m_MousePitchSpeed * dY;
			}
			else
			{
				m_MouseInput.Raw.Left += m_MouseSens * (dX < 0 ? m_MouseLeftSpeed : m_MouseRightSpeed) * -dX;

				if (m_MouseInput.Raw.LeftButtonDown)
				{
					m_MouseInput.Raw.Forward += m_MouseSens * (dY < 0 ? m_MouseForwardSpeed : m_MouseBackwardSpeed) * -dY;
				}
				if (m_MouseInput.Raw.RightButtonDown)
				{
					m_MouseInput.Raw.Up += m_MouseSens * (dY < 0 ? m_MouseUpSpeed : m_MouseDownSpeed) * -dY;
				}
			}

			if (m_MMove) {
				if (rawmouse->usButtonFlags & RI_MOUSE_HWHEEL) {
					float delta = (float)(short)rawmouse->usButtonData;
					m_MouseInput.Raw.Fov = m_MouseSens * (0 < delta ? m_MouseFovNegativeSpeed : m_MouseFovPositiveSpeed) * -delta;
					rawmouse->usButtonData = 0;
				}
			}
		}

		if (rawmouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) m_MouseInput.Raw.LeftButtonDown = false;
		if (rawmouse->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) m_MouseInput.Raw.RightButtonDown = false;

		if (m_CameraControlMode && m_MMove) {
			rawmouse->usButtonFlags &= ~(USHORT)(RI_MOUSE_LEFT_BUTTON_DOWN | RI_MOUSE_RIGHT_BUTTON_DOWN | RI_MOUSE_LEFT_BUTTON_UP | RI_MOUSE_RIGHT_BUTTON_UP | RI_MOUSE_HWHEEL);
		}
	} break;
	case RIM_TYPEKEYBOARD: {
		if (m_CameraControlMode) {
			RAWKEYBOARD* rawkeyboard = &(pData->data.keyboard);
			if(!(m_IgnoreNextKey || m_IgnoreKeyUp && (rawkeyboard->Flags & RI_KEY_BREAK))) {
				bool bIgnoreKey = false;
				switch(rawkeyboard->VKey) {
				case VK_ESCAPE:
				case VK_CONTROL:
				case VK_HOME:
				case VK_NUMPAD5:
				case 0x57: // W key
				case VK_NUMPAD8:
				case 0x53: // S key
				case VK_NUMPAD2:
				case 0x41: // A key
				case VK_NUMPAD4:
				case 0x44: // D key
				case VK_NUMPAD6:
				case 0x52: // R key
				case VK_NUMPAD9:
				case 0x46: // F key
				case VK_NUMPAD3:
				case VK_NUMPAD1:
				case VK_NEXT:
				case VK_NUMPAD7:
				case VK_PRIOR:
				case 0x58: // X key
				case VK_DECIMAL:
				case 0x5A: // Z key
				case VK_NUMPAD0:
				case VK_DOWN:
				case VK_UP:
				case VK_LEFT:
				case VK_RIGHT:
					bIgnoreKey = true;
					break;
				}
				if(bIgnoreKey) {
					rawkeyboard->MakeCode = 0;
					rawkeyboard->VKey = 0;
					rawkeyboard->Message = 0;
					rawkeyboard->ExtraInformation = 0;
				}
			}
		}
	} break;
	}
}

UINT MirvInput::Supply_RawInputBuffer(UINT result, _Out_writes_bytes_opt_(*pcbSize) PRAWINPUT pData, _Inout_ PUINT pcbSize, _In_ UINT cbSizeHeader) {
	if(!m_Focus)
		return result;

	if(m_Dependencies->GetSuspendMirvInput())
		return result;

	if(0 < result && nullptr != pData && sizeof(RAWINPUTHEADER) == cbSizeHeader && pData && pcbSize && sizeof(RAWINPUT)*result <= *pcbSize) {
		for(UINT i = 0; i < result; i++) {
			ProcessRawInputData(&pData[i]);
		}
	}

	return result;
}

UINT MirvInput::Supply_RawInputData(UINT result, _In_ HRAWINPUT hRawInput, _In_ UINT uiCommand, _Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData, _Inout_ PUINT pcbSize, _In_ UINT cbSizeHeader)
{
	if(!m_Focus)
		return result;

	if(m_Dependencies->GetSuspendMirvInput())
		return result;

	if(-1 != result && nullptr != hRawInput && uiCommand == RID_INPUT && sizeof(RAWINPUTHEADER) <= cbSizeHeader && pData && pcbSize && sizeof(RAWINPUT) <= *pcbSize) {

		ProcessRawInputData((RAWINPUT *)pData);

		return result;
	}

	return result;
}

void MirvInput::Supply_GetCursorPos(LPPOINT lpPoint)
{
	if(!lpPoint)
		return;

	if(!m_Focus)
		return;

	if(m_Dependencies->GetSuspendMirvInput())
		return;

	if(m_CameraControlMode)
	{
		if(m_FirstGetCursorPos)
		{
			m_FirstGetCursorPos = false;
		}

		// this will not work correctly if there was no SetCursorPos call prior to this call:
		LONG dX = lpPoint->x -m_LastCursorX;
		LONG dY = lpPoint->y -m_LastCursorY;

		if (!(m_MMove && (m_MNormalLeftButtonWasDown || m_MouseInput.Normal.RightButtonDown)))
		{
			m_MouseInput.Normal.Yaw += m_MouseSens * m_MouseYawSpeed * -dX;
			m_MouseInput.Normal.Pitch += m_MouseSens * m_MousePitchSpeed * dY;
		}
		else
		{
			m_MouseInput.Normal.Left += m_MouseSens * (dX < 0 ? m_MouseLeftSpeed : m_MouseRightSpeed) * -dX;

			if (m_MNormalLeftButtonWasDown)
			{
				m_MouseInput.Normal.Forward +=  m_MouseSens * (dY < 0 ? m_MouseForwardSpeed : m_MouseBackwardSpeed) * -dY;
			}
			if (m_MNormalRightButtonWasDown)
			{
				m_MouseInput.Normal.Up += m_MouseSens * (dY < 0 ? m_MouseUpSpeed : m_MouseDownSpeed) * -dY;
			}
		}

		// pretend we didn't move from last SetCursorPos call:
		lpPoint->x = m_LastCursorX;
		lpPoint->y = m_LastCursorY;
	}

	m_MNormalLeftButtonWasDown = m_MouseInput.Normal.LeftButtonDown;
	m_MNormalRightButtonWasDown = m_MouseInput.Normal.RightButtonDown;		
}

void MirvInput::Supply_SetCursorPos(int x, int y)
{
	m_LastCursorX = x;
	m_LastCursorY = y;
}

void MirvInput::Supply_MouseFrameEnd(void)
{
	if(!m_Focus)
		return;

	if(m_Dependencies->GetSuspendMirvInput())
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

		m_FirstGetCursorPos = true;
	}

	m_MouseInput.Clear();
	m_MNormalLeftButtonWasDown = m_MouseInput.Normal.LeftButtonDown;
	m_MNormalRightButtonWasDown = m_MouseInput.Normal.RightButtonDown;	
}

void MirvInput::Supply_Focus(bool hasFocus)
{
	m_Focus = hasFocus;
}

bool MirvInput::Override(float deltaT, float & Tx, float &Ty, float & Tz, float & Rx, float & Ry, float & Rz, float & Fov)
{
	bool overriden = false;

	if (GetCameraControlMode())
	{
		overriden = true;

		if (!m_InputOn)
		{
			m_InputOn = true;

			m_Dependencies->GetLastCameraData(m_InputX,m_InputY,m_InputZ,m_InputRx,m_InputRy,m_InputRz,m_InputFov);
		}

		switch (GetOffsetMode())
		{
		case MirvInput::OffsetMode_Last:
			m_Dependencies->GetLastCameraData(m_InputX,m_InputY,m_InputZ,m_InputRx,m_InputRy,m_InputRz,m_InputFov);
			break;
		case MirvInput::OffsetMode_Game:
			m_Dependencies->GetGameCameraData(m_InputX,m_InputY,m_InputZ,m_InputRx,m_InputRy,m_InputRz,m_InputFov);
			break;
		case MirvInput::OffsetMode_Current:
			m_InputX = Tx;
			m_InputY = Ty;
			m_InputZ = Tz;
			m_InputRx = Rx;
			m_InputRy = Ry;
			m_InputRz = Rz;
			m_InputFov = Fov;
			break;
		}

		double dT = deltaT;
		double dForward = dT * GetCamDForward();
		double dLeft = dT * GetCamDLeft();
		double dUp = dT * GetCamDUp();
		double dPitch = dT * GetCamDPitch();
		double dRoll = dT * GetCamDRoll();
		double dYaw = dT * GetCamDYaw();
		double dFov = dT * GetCamDFov();
		double forward[3], right[3], up[3];

		if(m_RotLocalSpace) {
			Afx::Math::Quaternion inputQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(dPitch, dYaw, dRoll))).Normalized();
			Afx::Math::Quaternion currentQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(m_InputRx, m_InputRy, m_InputRz))).Normalized();

			/*
			// Take shortest path:
			double dotProduct = Afx::Math::DotProduct(inputQuat, currentQuat);
			if (dotProduct < 0)
			{
				inputQuat = -1.0 * inputQuat;
			}*/

			Afx::Math::QEulerAngles newAngles = (currentQuat * inputQuat).ToQREulerAngles().ToQEulerAngles();

			Rx = (float)newAngles.Pitch;
			Ry = (float)newAngles.Yaw;
			Rz = (float)newAngles.Roll;
		} else {
			Rx = (float)(m_InputRx + dPitch);
			Ry = (float)(m_InputRy + dYaw);
			Rz = (float)(m_InputRz + dRoll);
		}

		Fov = (float)(m_InputFov + dFov);

		// limit fov to sane values:
		if (Fov < 1) Fov = 1;
		else if (Fov > 179) Fov = 179;

		if (GetCamResetView())
		{
			Rx = 0;
			Ry = 0;
			Rz = 0;
			Fov = 90.0;
		}

		Afx::Math::MakeVectors(Rz, Rx, Ry, forward, right, up);

		Tx = (float)(m_InputX + dForward * forward[0] - dLeft * right[0] + dUp * up[0]);
		Ty = (float)(m_InputY + dForward * forward[1] - dLeft * right[1] + dUp * up[1]);
		Tz = (float)(m_InputZ + dForward * forward[2] - dLeft * right[2] + dUp * up[2]);
	}
	else
	{
		m_InputOn = false;
	}

	if (m_SetTx)
	{
		m_SetTx = false;
		Tx = m_SetTxValue;
		overriden = true;
	}
	if (m_SetTy)
	{
		m_SetTy = false;
		Ty = m_SetTyValue;
		overriden = true;
	}
	if (m_SetTz)
	{
		m_SetTz = false;
		Tz = m_SetTzValue;
		overriden = true;
	}

	if (m_SetRx)
	{
		m_SetRx = false;
		Rx = m_SetRxValue;
		overriden = true;
	}
	if (m_SetRy)
	{
		m_SetRy = false;
		Ry = m_SetRyValue;
		overriden = true;
	}
	if (m_SetRz)
	{
		m_SetRz = false;
		Rz = m_SetRzValue;
		overriden = true;
	}
	if (m_SetFov)
	{
		m_SetFov = false;
		Fov = m_SetFovValue;
		overriden = true;
	}	

	if(m_InputOn) {
		m_InputX = Tx;
		m_InputY = Ty;
		m_InputZ = Tz;
		m_InputRx = Rx;
		m_InputRy = Ry;
		m_InputRz = Rz;
		m_InputFov = Fov;
	}

	if(!m_InputOn || !m_SmoothEnabled || !m_SmoothWasEnabled) {
		m_LastX = Tx;
		m_LastY = Ty;
		m_LastZ = Tz;
		m_LastFov = Fov;
		m_LastOutQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(Rx, Ry, Rz)));

		if(m_InputOn && m_SmoothEnabled) {
			m_SmoothWasEnabled = true;
		} else {
			m_SmoothWasEnabled = false;
		}
	} else {
		if (m_HalfTimeAng)
		{
			double t = deltaT / m_HalfTimeAng;

			Afx::Math::Quaternion targetQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(Rx, Ry, Rz))).Normalized();

			if (m_RotShortestPath)
			{
				// Take shortest path:
				double dotProduct = Afx::Math::DotProduct(targetQuat, m_LastOutQuat);
				if (dotProduct < 0)
				{
					targetQuat = -1.0 * targetQuat;
				}
			}

			double targetAngle = m_LastOutQuat.GetAng(targetQuat, Afx::Math::Vector3()) * 180.0 / M_PI;
			double angle = CalcDeltaExpSmooth(t, targetAngle);

			if (abs(angle) > AFX_MATH_EPS)
			{
				m_LastOutQuat = m_LastOutQuat.Slerp(targetQuat, angle / targetAngle).Normalized();
			}

			Afx::Math::QEulerAngles newAngles = m_LastOutQuat.ToQREulerAngles().ToQEulerAngles();

			Rx = (float)newAngles.Pitch;
			Ry = (float)newAngles.Yaw;
			Rz = (float)newAngles.Roll;

			overriden = true;
		}
		else
		{
			m_LastOutQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(Rx, Ry, Rz)));
		}

		if (m_HalfTimeVec)
		{
			double t = deltaT / m_HalfTimeVec;

			m_LastX = Tx = (float)CalcExpSmooth(t, m_LastX, Tx);
			m_LastY = Ty = (float)CalcExpSmooth(t, m_LastY, Ty);
			m_LastZ = Tz = (float)CalcExpSmooth(t, m_LastZ, Tz);

			overriden = true;
		}
		else
		{
			m_LastX = Tx;
			m_LastY = Ty;
			m_LastZ = Tz;
		}

		if (m_HalfTimeFov)
		{
			double t = deltaT / m_HalfTimeFov;

			double oldOpposite = 2 * tan(0.5 * m_LastFov * M_PI / 180.0);
			double targetOpposite = 2 * tan(0.5 * Fov * M_PI / 180.0);

			double newOppposite = CalcExpSmooth(t, oldOpposite, targetOpposite);
			m_LastFov = Fov = (float)(2 * atan(0.5 * newOppposite) * 180.0 / M_PI);

			overriden = true;
		}
		else
		{
			m_LastFov = Fov;
		}
	}

	return overriden;
}

double MirvInput::CalcExpSmooth(double deltaT, double oldVal, double newVal)
{
	const double limitTime = 19.931568569324174087221916576936;

	if (deltaT < 0)
		return oldVal;
	else if (limitTime < deltaT)
		return newVal;

	const double halfTime = 0.69314718055994530941723212145818;

	double x = 1 / exp(deltaT * halfTime);

	return x * oldVal +  (1 - x) * newVal;
}

double MirvInput::CalcDeltaExpSmooth(double deltaT, double deltaVal)
{
	const double limitTime = 19.931568569324174087221916576936;

	if (deltaT < 0)
		return 0;
	else if (limitTime < deltaT)
		return deltaVal;

	const double halfTime = 0.69314718055994530941723212145818;

	double x = 1 / exp(deltaT * halfTime);

	return (1 - x) * deltaVal;
}

void MirvInput::ConCommand(advancedfx::ICommandArgs * args)
{
	const char * arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("camera", arg1))
		{
			SetCameraControlMode(true);
			return;
		}
		else
		if(0 == _stricmp("cfg", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("msens", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseSensitivity(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetMouseSensitivty());
					return;
				}
				else
				if(0 == _stricmp("ksens", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetKeyboardSensitivity(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetKeyboardSensitivty());
					return;
				}
				else
				if(0 == _stricmp("kForwardSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardForwardSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardForwardSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kBackwardSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardBackwardSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardBackwardSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kLeftSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardLeftSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardLeftSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kRightSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardRightSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardRightSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kUpSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardUpSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardUpSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kDownSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardDownSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardDownSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kPitchPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardPitchPositiveSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardPitchPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kPitchNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardPitchNegativeSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardPitchNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kYawPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardYawPositiveSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardYawPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kYawNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardYawNegativeSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardYawNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kRollPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardRollPositiveSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardRollPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kRollNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardRollNegativeSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardRollNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kFovPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardFovPositiveSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardFovPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kFovNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						KeyboardFovNegativeSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", KeyboardFovNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("mYawSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						MouseYawSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", MouseYawSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("mPitchSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						MousePitchSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", MousePitchSpeed_get());
					return;
				}
				else if(0 == _stricmp("mFovPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						MouseFovPositiveSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", MouseFovPositiveSpeed_get());
					return;
				}
				else if (0 == _stricmp("mFovNegativeSpeed", arg2))
				{
					if (4 <= argc)
					{
						double value = atof(args->ArgV(3));
						MouseFovNegativeSpeed_set(value);
						return;
					}
					advancedfx::Message("Value: %f\n", MouseFovNegativeSpeed_get());
					return;
				}
				else if(0 == _stricmp("mLeftSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseLeftSpeed(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetMouseLeftSpeed());
					return;
				}
				else if (0 == _stricmp("mRightSpeed", arg2))
				{
					if (4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseRightSpeed(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetMouseRightSpeed());
					return;
				}
				else if (0 == _stricmp("mForwardSpeed", arg2))
				{
					if (4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseForwardSpeed(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetMouseForwardSpeed());
					return;
				}
				else if (0 == _stricmp("mBackSpeed", arg2))
				{
					if (4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseBackwardSpeed(value);
						return;
					}
					advancedfx::Message("Value: %\nf", GetMouseBackwardSpeed());
					return;
				}
				else if (0 == _stricmp("mUpSpeed", arg2))
				{
					if (4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseUpSpeed(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetMouseUpSpeed());
					return;
				}
				else if (0 == _stricmp("mDownSpeed", arg2))
				{
					if (4 <= argc)
					{
						double value = atof(args->ArgV(3));
						SetMouseDownSpeed(value);
						return;
					}
					advancedfx::Message("Value: %f\n", GetMouseDownSpeed());
					return;
				}
				else if (0 == _stricmp("mouseMoveSupport", arg2))
				{
					if (4 <= argc)
					{
						bool value = 0 != atoi(args->ArgV(3));
						SetEnableMouseMove(value);
						return;
					}
					advancedfx::Message("Value: %i\n", GetEnableMouseMove() ? 1 : 0);
					return;
				}
				else if (0 == _stricmp("offsetMode", arg2))
				{
					if (4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						if (0 == _stricmp("last", arg3))
						{
							SetOffsetMode(MirvInput::OffsetMode_Last);
						}
						else if (0 == _stricmp("ownLast", arg3))
						{
							SetOffsetMode(MirvInput::OffsetMode_OwnLast);
						}
						else if (0 == _stricmp("game", arg3))
						{
							SetOffsetMode(MirvInput::OffsetMode_Game);
						}
						else if (0 == _stricmp("current", arg3))
						{
							SetOffsetMode(MirvInput::OffsetMode_Current);
						}
						else
						{
							advancedfx::Warning("AFXERROR: %s is not a valid offset mode.\n", arg3);
						}

						return;
					}

					const char * szOffsetMode = "[n/a]";

					switch(GetOffsetMode())
					{
					case MirvInput::OffsetMode_Last:
						szOffsetMode = "last";
						break;
					case MirvInput::OffsetMode_OwnLast:
						szOffsetMode = "ownLast";
						break;
					case MirvInput::OffsetMode_Game:
						szOffsetMode = "game";
						break;
					case MirvInput::OffsetMode_Current:
						szOffsetMode = "current";
						break;
					}

					advancedfx::Message("Value: %s\n", szOffsetMode);
					return;
				}
				else if(0 == _stricmp("smooth",arg2)) {

					if (4 <= argc)
					{
						char const * arg3 = args->ArgV(3);

						if (0 == _stricmp("enabled", arg3))
						{
							if (5 <= argc)
							{
								m_SmoothEnabled = 0 != atoi(args->ArgV(4));
								return;
							}

							advancedfx::Message(
								"%s cfg smooth enabled 0|1 - Disable / enable smoothing.\n"
								"Current value: %i\n"
								, arg0
								, (m_SmoothEnabled?1:0)
							);
							return;
						}
						else if (0 == _stricmp("halfTime", arg3))
						{
							if (5 <= argc)
							{
								m_HalfTimeVec = m_HalfTimeAng = m_HalfTimeFov = atof(args->ArgV(4));
								return;
							}

							advancedfx::Message(
								"%s cfg smooth halfTime <fHalfTimeSecs> - Set new value for vec,ang,fov.\n"
								, arg0
							);
							return;
						}
						else if (0 == _stricmp("halfTimeVec", arg3))
						{
							if (5 <= argc)
							{
								m_HalfTimeVec = atof(args->ArgV(4));
								return;
							}

							advancedfx::Message(
								"%s cfg smooth halfTimeVec <fHalfTimeSecs> - Set new value.\n"
								"Current value: %f\n"
								, arg0
								, m_HalfTimeVec
							);
							return;
						}
						else if (0 == _stricmp("halfTimeAng", arg3))
						{
							if (5 <= argc)
							{
								m_HalfTimeAng = atof(args->ArgV(4));
								return;
							}

							advancedfx::Message(
								"%s cfg smooth halfTimeAng <fHalfTimeSecs> - Set new value.\n"
								"Current value: %f\n"
								, arg0
								, m_HalfTimeAng
							);
							return;
						}
						else if (0 == _stricmp("halfTimeFov", arg3))
						{
							if (5 <= argc)
							{
								m_HalfTimeFov = atof(args->ArgV(4));
								return;
							}

							advancedfx::Message(
								"%s cfg smooth halfTimeFov <fHalfTimeSecs> - Set new value.\n"
								"Current value: %f\n"
								, arg0
								, m_HalfTimeFov
							);
							return;
						}
						else if (0 == _stricmp("rotShortestPath", arg3))
						{
							if (5 <= argc)
							{
								m_RotShortestPath = 0 != atoi(args->ArgV(4));
								return;
							}

							advancedfx::Message(
								"%s cfg smooth rotShortestPath 0|1 - Set new value.\n"
								"Current value: %i\n"
								, arg0
								, m_RotShortestPath ? 1 : 0
							);
							return;
						}
					}

					advancedfx::Message(
						"%s cfg smooth enabled [...]\n"
						"%s cfg smooth halfTime [...]\n"
						"%s cfg smooth halfTimeVec [...]\n"
						"%s cfg smooth halfTimeAng [...]\n"
						"%s cfg smooth halfTimeFov [...]\n"
						"%s cfg smooth rotShortestPath [...] - If to rotate shortest path.\n"
						, arg0
						, arg0
						, arg0
						, arg0
						, arg0
						, arg0
					);

					return;
				}
				else if(0 == _stricmp("rotLocalSpace",arg2)) {

					if (4 <= argc)
					{
						m_RotLocalSpace = 0 != atoi(args->ArgV(3));
						return;
					}

					advancedfx::Message(
						"%s cfg rotLocalSpace 0|1\n"
						"Current value: %i\n",
						arg0,
						(m_RotLocalSpace ? 1 : 0)
					);

					return;
				}
				else if(0 == _stricmp("stepFactor",arg2)) {

					if (4 <= argc)
					{
						SetCamSpeedBasis(atof(args->ArgV(3)));
						return;
					}

					advancedfx::Message(
						"%s cfg stepFactor <fVal> - Where 1 <= <fVal> <= 8\n"
						"Current value: %f\n",
						arg0,
						GetCamSpeedBasis()
					);

					return;
				}				
			}

			advancedfx::Message(
				"Usage:\n"
				"%s cfg mouseMoveSupport - Get value.\n"
				"%s cfg mouseMoveSupport 0|1 - Disable / Enable mouse move support (use left / right mouse button).\n"
				"%s cfg offsetMode - Get value.\n"
				"%s cfg offsetMode last|ownLast|game|current - Default: current, last = old method (last outputted), ownLast = as outputted by %s, game = as outputted by game, current = as outputted by current overrides.\n"
				"%s cfg msens - Get mouse sensitivity.\n"
				"%s cfg msens <dValue> - Set mouse sensitivity.\n"
				"%s cfg ksens - Get keyboard sensitivity.\n"
				"%s cfg ksens <dValue> - Set keyboard sensitivity.\n"
				,arg0,
				arg0,
				arg0,
				arg0, arg0,
				arg0,
				arg0,
				arg0,
				arg0
			);
			advancedfx::Message(
				"%s cfg kForwardSpeed - Get value.\n"
				"%s cfg kForwardSpeed <dValue> - Set value.\n"
				"%s cfg kBackwardSpeed - Get value.\n"
				"%s cfg kBackwardSpeed <dValue> - Set value.\n"
				"%s cfg kLeftSpeed - Get value.\n"
				"%s cfg kLeftSpeed <dValue> - Set value.\n"
				"%s cfg kRightSpeed - Get value.\n"
				"%s cfg kRightSpeed <dValue> - Set value.\n"
				"%s cfg kUpSpeed - Get value.\n"
				"%s cfg kUpSpeed <dValue> - Set value.\n"
				"%s cfg kDownSpeed - Get value.\n"
				"%s cfg kDownSpeed <dValue> - Set value.\n"
				"%s cfg kPitchPositiveSpeed - Get value.\n"
				"%s cfg kPitchPositiveSpeed <dValue> - Set value.\n"
				"%s cfg kPitchNegativeSpeed - Get value.\n"
				"%s cfg kPitchNegativeSpeed <dValue> - Set value.\n"
				"%s cfg kYawPositiveSpeed - Get value.\n"
				"%s cfg kYawPositiveSpeed <dValue> - Set value.\n"
				"%s cfg kYawNegativeSpeed - Get value.\n"
				"%s cfg kYawNegativeSpeed <dValue> - Set value.\n"
				"%s cfg kRollPositiveSpeed - Get value.\n"
				"%s cfg kRollPositiveSpeed <dValue> - Set value.\n"
				"%s cfg kRollNegativeSpeed - Get value.\n"
				"%s cfg kRollNegativeSpeed <dValue> - Set value.\n"
				"%s cfg kFovPositiveSpeed - Get value.\n"
				"%s cfg kFovPositiveSpeed <dValue> - Set value.\n"
				"%s cfg kFovNegativeSpeed - Get value.\n"
				"%s cfg kFovNegativeSpeed <dValue> - Set value.\n"
				,arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0,
				arg0
			);
			advancedfx::Message(
				"%s cfg mYawSpeed - Get value.\n"
				"%s cfg mYawSpeed <dValue> - Set value.\n"
				"%s cfg mPitchSpeed - Get value.\n"
				"%s cfg mPitchSpeed <dValue> - Set value.\n"
				"%s cfg mFovPositiveSpeed - Get value.\n"
				"%s cfg mFovPositiveSpeed <dValue> - Set value.\n"
				"%s cfg mFovNegativeSpeed - Get value.\n"
				"%s cfg mFovNegativeSpeed <dValue> - Set value.\n"
				"%s cfg mForwardSpeed - Get value.\n"
				"%s cfg mForwardSpeed <dValue> - Set value.\n"
				"%s cfg mBackwardSpeed - Get value.\n"
				"%s cfg mBackwardSpeed <dValue> - Set value.\n"
				"%s cfg mLeftSpeed - Get value.\n"
				"%s cfg mLeftSpeed <dValue> - Set value.\n"
				"%s cfg mRightSpeed - Get value.\n"
				"%s cfg mRightSpeed <dValue> - Set value.\n"
				"%s cfg mUpSpeed - Get value.\n"
				"%s cfg mUpSpeed <dValue> - Set value.\n"
				"%s cfg mDownSpeed - Get value.\n"
				"%s cfg mDownSpeed <dValue> - Set value.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			advancedfx::Message(
				"%s cfg smooth [...]\n"
				, arg0
			);
			advancedfx::Message(
				"%s cfg rotLocalSpace [...] - Enable rotating in local space instead of global space\n"
				, arg0
			);
			advancedfx::Message(
				"%s cfg stepFactor [...] - The factor to use for the speed increase/decrase.\n"
				, arg0
			);
			return;
		}
		else
		if(0 == _stricmp("end", arg1))
		{
			SetCameraControlMode(false);
			return;
		}
		else
		if(0 == _stricmp("position", arg1))
		{
			if(5 == argc)
			{
				char const * arg2 = args->ArgV(2);
				char const * arg3 = args->ArgV(3);
				char const * arg4 = args->ArgV(4);
	
				if(0 != _stricmp("*", arg2)) SetTx((float)atof(arg2));
				if (0 != _stricmp("*", arg3)) SetTy((float)atof(arg3));
				if (0 != _stricmp("*", arg4)) SetTz((float)atof(arg4));
				return;
			}

			double x,y,z,rx,ry,rz,fov;
			m_Dependencies->GetLastCameraData(x,y,z,rx,ry,rz,fov);			

			advancedfx::Message(
				"%s position <x> <y> <z> - Set new position (only useful in camera input mode), use * where you don't want changes.\n"
				"Current value: %f %f %f\n"
				, arg0
				, x
				, y
				, z
			);
			return;
		}
		else
		if(0 == _stricmp("angles", arg1))
		{
			if(5 == argc)
			{
				char const * arg2 = args->ArgV(2);
				char const * arg3 = args->ArgV(3);
				char const * arg4 = args->ArgV(4);
	
				if (0 != _stricmp("*", arg2)) SetRx((float)atof(arg2));
				if (0 != _stricmp("*", arg3)) SetRy((float)atof(arg3));
				if (0 != _stricmp("*", arg4)) SetRz((float)atof(arg4));
				return;
			}

			double x,y,z,rx,ry,rz,fov;
			m_Dependencies->GetLastCameraData(x,y,z,rx,ry,rz,fov);

			advancedfx::Message(
				"%s angles <yPitch> <xRoll> <zYaw> - Set new angles (only useful in camera input mode), use * where you don't want changes.\n"
				"Current value: %f %f %f\n"
				, arg0
				, rx
				, ry
				, rz
			);
			return;
		}
		else
		if(0 == _stricmp("fov", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (4 <= argc) {
					char const * arg3 = args->ArgV(3);

					if (0 == _stricmp("real", arg2)) {
						SetFov((float)(m_Dependencies->GetInverseScaledFov(atof(arg3))));
						return;
					}
				}
				else {
					SetFov((float)atof(arg2));
					return;
				}
			}

			double x,y,z,rx,ry,rz,fov;
			m_Dependencies->GetLastCameraData(x,y,z,rx,ry,rz,fov);

			advancedfx::Message(
				"%s fov [real] <fov> - Set new fov (only useful in camera input mode).\n"
				"Current value: %f\n"
				, arg0
				, fov
			);
			return;
		}
		else if (0 == _stricmp("mem", arg1))
		{
			advancedfx::CSubCommandArgs subArgs(args, 2);

			m_Mem.Console(&subArgs);

			return;
		}
	}

	advancedfx::Message(
		"Usage:\n"
		"%s camera - Enable camera input mode, see HLAE manual for keybinds etc.\n"
		"%s cfg [...] - Control input mode configuration.\n"
		"%s end - End input mode(s).\n"
		"%s position [...]\n"
		"%s angles [...]\n"
		"%s fov [...]\n"
		"%s mem [...] - Store, use, save and load %s view states.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0, arg0
	);
}
