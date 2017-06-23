#include "stdafx.h"

#include "bvhexport.h"

#include <sstream>

using namespace std;


// BvhExport //////////////////////////////////////////////////////////////////

BvhExport::BvhExport(wchar_t const * fileName, char const * rootName, double frameTime)
{
	m_FrameCount = 0;
	_wfopen_s(&m_pMotionFile, fileName, L"wb");

	if (m_pMotionFile != NULL)
		BeginContent(m_pMotionFile, rootName, frameTime, m_lMotionTPos);
}

BvhExport::~BvhExport()
{
	char pTmp[100];

	if (m_pMotionFile) {
		fseek(m_pMotionFile, m_lMotionTPos, SEEK_SET);
		_snprintf_s(pTmp, _TRUNCATE, "Frames: %11i", m_FrameCount);
		fputs(pTmp, m_pMotionFile);
		fclose(m_pMotionFile);
		m_pMotionFile = NULL;
	}
}

void BvhExport::BeginContent(FILE *pFile, char const * pRootName, double frameTime, long &ulTPos)
{
	char szTmp[1024];

	fputs("HIERARCHY\n",pFile);

	fputs("ROOT ",pFile);
	fputs(pRootName,pFile);
	fputs("\n{\n\tOFFSET 0.00 0.00 0.00\n\tCHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n\tEnd Site\n\t{\n\t\tOFFSET 0.00 0.00 -1.00\n\t}\n}\n",pFile);

	fputs("MOTION\n",pFile);
	ulTPos = ftell(pFile);
	fputs("Frames: 0123456789A\n",pFile);

	_snprintf_s(szTmp, _TRUNCATE,"Frame Time: %f\n", frameTime);
	fputs(szTmp,pFile);
}

void BvhExport::WriteFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation) {
	char pszT[1024];

	_snprintf_s(pszT, _TRUNCATE, "%f %f %f %f %f %f\n", Xposition, Yposition, Zposition, Zrotation, Xrotation, Yrotation);

	if (m_pMotionFile) fputs(pszT, m_pMotionFile);

	m_FrameCount++;
}



