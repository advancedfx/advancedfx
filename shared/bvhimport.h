#pragma once

#include <stdio.h>
#include <windows.h>

#include "CamPath.h"

/// <remarks> not thread safe, due to ms_readbuff </remarks>
class BvhImport
{
public:
	BvhImport();
	~BvhImport();

	bool IsActive();

	bool LoadMotionFile(wchar_t const * fileName);
	void CloseMotionFile();

	// outformat: see BvhChannel_t
	// return: true on success, false otherwise
	bool GetCamPosition(double fTimeOfs, double outCamdata[6]);

	bool CopyToCampath(double timeOfs, double fov, CamPath & camPath);

private:
	enum BvhChannel_t { BC_Xposition=0, BC_Yposition, BC_Zposition, BC_Zrotation, BC_Xrotation, BC_Yrotation };

	int channelcode[6];
	bool m_Active;
	FILE * m_File;
	double m_Cache[6];
	int m_Frames;
	int m_LastFrame;
	double m_FrameTime;
	long m_MotionFPos;
	static char ms_readbuff[1024];

	int DecodeBvhChannel(char * pszRemainder, char * & aoutNewRemainder);
};
