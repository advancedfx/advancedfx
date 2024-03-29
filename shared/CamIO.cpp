#include "stdafx.h"

#include "CamIO.h"

#include <string>
#include <sstream>
#include <share.h>

double CamIO::DoFovScaling(double width, double height, double fov)
{
	if(SF_New == m_ScaleFov)
		return Auto_FovScaling(width, height, fov);

	if (SF_OldAlienSwarm == m_ScaleFov)
		return AlienSwarm_FovScaling(width, height, fov);

	return fov;
}

double CamIO::UndoFovScaling(double width, double height, double fov)
{
	if(SF_New == m_ScaleFov)
		return Auto_InverseFovScaling(width, height, fov);

	if (SF_OldAlienSwarm == m_ScaleFov)
		return AlienSwarm_InverseFovScaling(width, height, fov);

	return fov;
}


CamExport::CamExport(const wchar_t * fileName)
	: m_Ofs(fileName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc)
{
	m_ScaleFov = SF_New;

	m_Ofs << "advancedfx Cam" << std::endl;
	m_Ofs << "version 2" << std::endl;
	//m_Ofs << "scaleFov " << (m_ScaleFov == SF_AlienSwarm ? "alienSwarm" : "none") << std::endl;
	m_Ofs << "channels time xPosition yPosition zPosition xRotation yRotation zRotation fov" << std::endl;
	m_Ofs << "DATA" << std::endl;

	m_Ofs << std::fixed;
	m_Ofs.precision(6);
}

CamExport::~CamExport()
{
	m_Ofs.close();
}

void CamExport::WriteFrame(double width, double height, const CamData & camData)
{
	m_Ofs << camData.Time << " " << camData.XPosition << " " << camData.YPosition << " " << camData.ZPosition << " " << camData.XRotation << " " << camData.YRotation << " " << camData.ZRotation << " " << DoFovScaling(width, height, camData.Fov) << std::endl;
}

void afx_std_string_remove_CR(std::string & inOutValue) {
	if (!inOutValue.empty() && '\r' == *(--inOutValue.end()))
		inOutValue.erase(--inOutValue.end());
}

CamImport::CamImport(char const * fileName, double startTime)
	: m_Ifs(fileName, std::ifstream::in | std::ofstream::binary, _SH_DENYWR)
	, m_StartTime(startTime)
{
	int version = 0;
	m_ScaleFov = SF_New;

	char magic[14 + 1 + 1] = { 0 };

	m_Ifs.getline(magic, sizeof(magic) / sizeof(char), '\n'); if ('\r' == magic[14]) magic[14] = '\0';
	if (0 != strcmp(magic, "advancedfx Cam"))m_Ifs.clear(m_Ifs.rdstate() | std::ifstream::badbit);


	while (m_Ifs.good())
	{
		std::string line;

		std::getline(m_Ifs, line, '\n'); afx_std_string_remove_CR(line);

		std::istringstream iss(line);

		std::string verb;

		iss >> verb;

		if (0 == verb.compare("DATA"))
			break;
		else if (0 == verb.compare("version"))
		{
			iss >> version;
		}
		else if (0 == verb.compare("scaleFov"))
		{
			std::string arg;

			iss >> arg;

			if (0 == arg.compare("alienSwarm"))
				m_ScaleFov = SF_OldAlienSwarm;
			else
				m_ScaleFov = SF_OldNone;
		}
	}

	if (version < 1 || version > 2) m_Ifs.setstate(m_Ifs.rdstate() | std::ifstream::badbit);

	m_DataStart = m_Ifs.tellg();
}

CamImport::~CamImport()
{
	m_Ifs.close();
}

void CamImport::SetStart(double startTime)
{
	m_StartTime = startTime;
}

bool CamImport::GetCamData(double time, double width, double height, CamData & outCamData)
{
	if (m_Ifs.bad())
		return false; // too broken

	if (time - m_StartTime < 0)
		return false; // too early

	if (m_HasFinalFrame && m_FinalFrameTime - m_FirstFrameTime < time - m_StartTime)
		return false; // too late

	if (!m_HasLastFrame) {

		m_CurPos = m_Ifs.tellg();
		m_HasLastFrame = ReadDataLine(m_LastFrame);
		if (!m_HasLastFrame)
			return false;

		// First frame
		m_FirstFrameTime = m_LastFrame.Time;

		m_LastQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(m_LastFrame.YRotation, m_LastFrame.ZRotation, m_LastFrame.XRotation)));
		m_NextFrame = m_LastFrame;
		m_NextQuat = m_LastQuat;
	}

	// Jump backwards if required:
	if (
		m_LastFrame.Time - m_FirstFrameTime > time - m_StartTime // time is before current interval
		)
	{
		size_t loPos = m_DataStart;
		size_t hiPos = m_CurPos;

		m_Ifs.clear(std::ios::eofbit);
		if (m_Ifs.seekg(m_DataStart, std::ios_base::beg).bad()) return false;
		if(!ReadDataLine(m_LastFrame))
			return false;
		
		CamData curFrame;
		while (loPos < hiPos) {
			size_t curPos = (loPos + hiPos) / 2;

			m_Ifs.clear(std::ios::eofbit);
			if (m_Ifs.seekg(curPos, std::ios_base::beg).bad()) return false;

			while (!m_Ifs.bad() && !m_Ifs.eof() && '\n' != m_Ifs.get()); // seek until next line

			if (m_Ifs.bad())
				return false;

			if (m_Ifs.eof()) {
				hiPos = curPos;
				continue;
			}

			if (ReadDataLine(curFrame)) {
				float sub = (curFrame.Time - m_FirstFrameTime) - (time - m_StartTime);

				if (sub == 0) {
					m_CurPos = curPos;
					m_LastFrame = curFrame;
					break;
				}
				else if (sub > 0) {
					hiPos = curPos;
				}
				else {
					loPos = curPos;
					m_CurPos = curPos;
					m_LastFrame = curFrame;
				}
			}
			
			if (m_Ifs.bad())
				return false;
			
			hiPos = curPos;
		}

		m_LastQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(m_LastFrame.YRotation, m_LastFrame.ZRotation, m_LastFrame.XRotation)));
		m_NextFrame = m_LastFrame;
		m_NextQuat = m_LastQuat;
	}

	// Read next frame and Seek forward if required

	while (m_NextFrame.Time - m_FirstFrameTime < time - m_StartTime)
	{
		m_LastFrame = m_NextFrame;
		m_LastQuat = m_NextQuat;
		
		m_CurPos = m_Ifs.tellg();
		if (!ReadDataLine(m_NextFrame)) {
			m_FinalFrameTime = m_LastFrame.Time;
			m_HasFinalFrame = true;
			return false;
		}
		m_NextQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(m_NextFrame.YRotation, m_NextFrame.ZRotation, m_NextFrame.XRotation)));

		// Make sure we will travel the short way:
		double dotProduct = DotProduct(m_NextQuat, m_LastQuat);
		if (dotProduct<0.0)
		{
			m_NextQuat = -1.0 * m_NextQuat;
		}
	}

	double orgTime = time - m_StartTime + m_FirstFrameTime;

	double delta = m_NextFrame.Time - m_LastFrame.Time;
	double t = delta ? ((orgTime - m_LastFrame.Time) / delta) : 0;

	Afx::Math::QEulerAngles rot = m_LastQuat.Slerp(m_NextQuat, t).ToQREulerAngles().ToQEulerAngles();

	outCamData.Time = time;
	outCamData.XPosition = (1 - t) * m_LastFrame.XPosition + t * m_NextFrame.XPosition;
	outCamData.YPosition = (1 - t) * m_LastFrame.YPosition + t * m_NextFrame.YPosition;
	outCamData.ZPosition = (1 - t) * m_LastFrame.ZPosition + t * m_NextFrame.ZPosition;
	outCamData.XRotation = rot.Roll;
	outCamData.YRotation = rot.Pitch;
	outCamData.ZRotation = rot.Yaw;
	outCamData.Fov = (1 - t) * UndoFovScaling(width, height, m_LastFrame.Fov) + t * UndoFovScaling(width, height, m_NextFrame.Fov); // TODO: this maybe won't result in linear perception, maybe fix it when time.

	return true;
}

bool CamImport::ReadDataLine(CamData & outCamData)
{
	std::string line;

	if (!std::getline(m_Ifs, line, '\n'))
		return false;

	afx_std_string_remove_CR(line);

	std::istringstream iss(line);

	iss >> outCamData.Time >> outCamData.XPosition >> outCamData.YPosition >> outCamData.ZPosition >> outCamData.XRotation >> outCamData.YRotation >> outCamData.ZRotation >> outCamData.Fov;

	return !iss.fail();
}
