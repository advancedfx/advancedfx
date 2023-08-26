#include "stdafx.h"

#include "MirvInput.h"

#include "AfxMath.h"
#include "AfxConsole.h"
#include "StringTools.h"

#include "../deps/release/rapidxml/rapidxml.hpp"
#include "../deps/release/rapidxml/rapidxml_print.hpp"


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
, m_MouseFovPositiveSpeed(45.0)
, m_MouseFovNegativeSpeed(45.0)
, m_MLDown(false)
, m_MRDown(false)
{
	m_Mem.Connect(this);
}

void MirvInput::DoCamSpeedDecrease(void)
{
	m_CamSpeed = m_CamSpeed / 2;
	if(m_CamSpeed < 1.0/256) m_CamSpeed = 1.0/256;
}

void MirvInput::DoCamSpeedIncrease(void)
{
	m_CamSpeed = m_CamSpeed * 2;
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
	return m_CamSpeed * (m_CamForward -m_CamForwardI);
}

double MirvInput::GetCamDLeft(void)
{
	return m_CamSpeed * (m_CamLeft -m_CamLeftI);
}

double MirvInput::GetCamDUp(void)
{
	return m_CamSpeed * (m_CamUp -m_CamUpI);
}

double MirvInput::GetCamDPitch(void)
{
	return m_CamSpeed * (m_CamPitch -m_CamPitchI +m_CamPitchM);
}

double MirvInput::GetCamDYaw(void)
{
	return m_CamSpeed * (m_CamYaw -m_CamYawI +m_CamYawM);
}

double MirvInput::GetCamDRoll(void)
{
	return m_CamSpeed * (m_CamRoll -m_CamRollI);
}

double MirvInput::GetCamDFov(void)
{
	return m_CamSpeed * (m_CamFov -m_CamFovI);
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
		case WM_MOUSEWHEEL:
			{
				m_MouseFov = true;

				signed short delta = GET_WHEEL_DELTA_WPARAM(wParam);

				if(0 <= delta) m_CamFovI += m_MouseSens * m_MouseFovNegativeSpeed * delta;
				else m_CamFov += m_MouseSens * m_MouseFovPositiveSpeed * -delta;
			}
			return true;
		}
	}

	return false;
}

bool MirvInput::Supply_RawMouseMotion(int dX, int dY)
{
	if(!m_Focus)
		return false;

	if(m_Dependencies->GetSuspendMirvInput())
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

		if (m_MouseFov)
		{
			m_MouseFov = false;
			m_CamFov = 0.0;
			m_CamFovI = 0.0;
		}
	}
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

		Rx = (float)(m_InputRx + dPitch);
		Ry = (float)(m_InputRy + dYaw);
		Rz = (float)(m_InputRz + dRoll);
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

	return overriden;
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
