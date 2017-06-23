#pragma once

#include <stdio.h>
#include <windows.h>


// BvhExport ///////////////////////////////////////////////////////////////////

class BvhExport
{
public:
	/// <summary> Creates a new BVH file </summary>
	BvhExport(wchar_t const * fileName, char const * rootName, double frameTime);

	/// </summary> Closes the BVH file </summary>
	~BvhExport();

	void WriteFrame(
		double Xposition, double Yposition, double Zposition,
		double Zrotation, double Xrotation, double Yrotation
	);

private:
	unsigned int m_FrameCount;
	FILE * m_pMotionFile;
	long m_lMotionTPos;

	void BeginContent(FILE *pFile ,char const * pRootName, double frameTime, long &ulTPos);

};


