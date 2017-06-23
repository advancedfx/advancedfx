#pragma once

#include <shared/bvhimport.h>

class CCamImport
{
public:
	CCamImport();
	~CCamImport();

	bool IsActive();

	bool LoadMotionFile(wchar_t const * fileName);
	void CloseMotionFile();

	// outformat: see BvhImport::BvhChannel_t
	// return: truew if successful
	bool GetCamPositon(double fTimeOfs, double outCamdata[6]);

	double GetBaseTime();

	void SetBaseTime(double fBaseTime);

private:
	double m_BaseTime;
	BvhImport m_BvhImport;
};

extern CCamImport g_CamImport;
