#pragma once

#include <shared/AfxMath.h>

#include <stdio.h>
#include <fstream>
#include <map>

double AlienSwarm_FovScaling(double width, double height, double fov);

double AlienSwarm_InverseFovScaling(double width, double height, double fov);

class CamIO
{
public:
	enum ScaleFov
	{
		SF_None = 0,
		SF_AlienSwarm
	};

	struct CamData
	{
		double Time = 0;
		double XPosition = 0;
		double YPosition = 0;
		double ZPosition = 0;
		double XRotation = 0;
		double YRotation = 0;
		double ZRotation = 0;
		double Fov = 90;
	};

protected:
	ScaleFov m_ScaleFov = SF_None;

	double DoFovScaling(double width, double height, double fov);
	double UndoFovScaling(double width, double height, double fov);
};


class CamExport : public CamIO
{
public:
	CamExport(const wchar_t * fileName, ScaleFov scaleFov);

	~CamExport();

	void WriteFrame(
		double width, double height
		, const CamData & camData
	);

private:
	std::ofstream m_Ofs;
};

/// <remarks>Going back in time is very inefficient, you should avoid that or improve code for that.</remarks>
class CamImport : public CamIO
{
public:
	CamImport(char const * fileName, double startTime);

	~CamImport();

	void SetStart(double startTime);

	/// <remarks>If the function fails outCamData content is undefined.</remarks>
	bool GetCamData(double time, double width, double height, CamData & outCamData);

	bool IsBad() { return m_Ifs.bad(); }

private:
	bool m_FileStartOk;
	std::ifstream m_Ifs;
	double m_StartTime;
	std::streampos m_DataStart;
	double m_FirstFrameTime;
	bool m_HasLastFrame = false;
	bool m_LastFrameTime;
	double m_FinalFrameTime;
	bool m_HasFinalFrame = false;
	CamData m_LastFrame;
	Afx::Math::Quaternion m_LastQuat;
	CamData m_NextFrame;
	Afx::Math::Quaternion m_NextQuat;

	bool ReadDataLine(CamData & outCamData);
};
