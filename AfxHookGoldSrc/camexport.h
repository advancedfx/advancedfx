#pragma once

#include <shared/bvhexport.h>


// CCamExport //////////////////////////////////////////////////////////////////

class CCamExport {
public:
	CCamExport();
	~CCamExport();

	void BeginFileLeft(wchar_t const * folder, double frameTime);
	void BeginFileMain(wchar_t const * folder, double frameTime);
	void BeginFileRight(wchar_t const * folder, double frameTime);

	void EndFileLeft();
	void EndFileMain();
	void EndFileRight();
	void EndFiles();

	bool HasFileLeft();
	bool HasFileMain();
	bool HasFileRight();

	void WriteLeftFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation);
	void WriteMainFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation);
	void WriteRightFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation);

private:
	BvhExport * m_BvhFileLeft;
	BvhExport * m_BvhFileMain;
	BvhExport * m_BvhFileRight;
	unsigned int m_FrameCount;

	BvhExport * BeginFile(wchar_t const * folder, wchar_t const * fileName, char const * rootName, double frameTime);

	void EndFile(BvhExport * & bvhFile);

	void WriteFrame(BvhExport * bvhFile, double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation);
};

extern CCamExport g_CamExport;

