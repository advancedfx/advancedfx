#include "stdafx.h"

#include "CamIO.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <sstream>
#include <share.h>

double AlienSwarm_FovScaling(double width, double height, double fov)
{
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	double halfAngle = 0.5 * fov * (2.0 * M_PI / 360.0);
	double t = ratio * tan(halfAngle);
	return 2.0 * atan(t) / (2.0 * M_PI / 360.0);
}

double AlienSwarm_InverseFovScaling(double width, double height, double fov)
{
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	double t = tan(0.5 * fov * (2.0 * M_PI / 360.0));
	double halfAngle = atan(t / ratio);
	return 2.0 * halfAngle / (2.0 * M_PI / 360.0);
}

double CamIO::DoFovScaling(double width, double height, double fov)
{
	if (SF_AlienSwarm == m_ScaleFov)
		return AlienSwarm_FovScaling(width, height, fov);

	return fov;
}

double CamIO::UndoFovScaling(double width, double height, double fov)
{
	if (SF_AlienSwarm == m_ScaleFov)
		return AlienSwarm_InverseFovScaling(width, height, fov);

	return fov;
}


CamExport::CamExport(const wchar_t * fileName, ScaleFov scaleFov)
	: m_Ofs(fileName, std::ofstream::out | std::ofstream::trunc)
{
	m_ScaleFov = scaleFov;

	m_Ofs << "advancedfx Cam" << std::endl;
	m_Ofs << "version 1" << std::endl;
	m_Ofs << "scaleFov " << (m_ScaleFov == SF_AlienSwarm ? "alienSwarm" : "none") << std::endl;
	m_Ofs << "channels time xPosition yPosition zPositon xRotation yRotation zRotation fov" << std::endl;
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


CamImport::CamImport(char const * fileName, double startTime)
	: m_Ifs(fileName, std::ifstream::in, _SH_DENYWR)
	, m_StartTime(startTime)
{
	int version = 0;
	m_ScaleFov = SF_None;

	char magic[15] = { 0 };

	m_Ifs.getline(magic, sizeof(magic)/sizeof(char));
	if (0 != strcmp(magic, "advancedfx Cam")) m_Ifs.clear(m_Ifs.rdstate() | std::ifstream::badbit);
	

	while (m_Ifs.good())
	{
		std::string line;

		std::getline(m_Ifs, line);

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
				m_ScaleFov = SF_AlienSwarm;
		}
	}

	if(1 != version) m_Ifs.setstate(m_Ifs.rdstate() | std::ifstream::badbit);

	m_DataStart = m_Ifs.tellg();
	m_FileStartOk = !m_Ifs.fail();
}

CamImport::~CamImport()
{
	m_Ifs.close();
}

void CamImport::SetStart(double startTime)
{
	m_StartTime = startTime;
	if (m_FileStartOk)
	{
		m_HasLastFrame = false;
		m_Ifs.clear();
		m_Ifs.seekg(m_DataStart);
	}
}

bool CamImport::GetCamData(double time, double width, double height, CamData & outCamData)
{
	if (m_Ifs.bad())
		return false;

	if (m_HasLastFrame && time - m_StartTime < m_LastFrame.Time - m_FirstFrameTime)
	{
		m_Ifs.seekg(m_DataStart);
		m_HasLastFrame = false;
	}

	if (!m_HasLastFrame)
	{
		m_HasLastFrame = ReadDataLine(m_LastFrame);
		if (!m_HasLastFrame)
			return false;
		m_FirstFrameTime = m_LastFrame.Time;
		m_LastQuat = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(m_LastFrame.YRotation, m_LastFrame.ZRotation, m_LastFrame.XRotation)));
		m_NextFrame = m_LastFrame;
		m_NextQuat = m_LastQuat;
	}
	
	while (m_NextFrame.Time - m_FirstFrameTime < time - m_StartTime)
	{
		m_LastFrame = m_NextFrame;
		m_LastQuat = m_NextQuat;
		if (!ReadDataLine(m_NextFrame))
			return false;
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
	outCamData.Fov = (1 - t) * UndoFovScaling(width, height, m_LastFrame.Fov) + t * UndoFovScaling(width, height, m_NextFrame.Fov); // this maybe won't result in linear perception, maybe fix it when time.

	return true;
}

bool CamImport::ReadDataLine(CamData & outCamData)
{
	std::string line;

	if (!std::getline(m_Ifs, line))
		return false;

	std::istringstream iss(line);

	iss >> outCamData.Time >> outCamData.XPosition >> outCamData.YPosition >> outCamData.ZPosition >> outCamData.XRotation >> outCamData.YRotation >> outCamData.ZRotation >> outCamData.Fov;

	return !iss.fail();
}
