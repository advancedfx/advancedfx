#include "stdafx.h"

#include "camexport.h"

#include <hlsdk.h>

#include "cmdregister.h"
#include "filming.h"

#include <string>

using namespace std;

extern cl_enginefuncs_s *pEngfuncs;
extern Filming g_Filming;

CCamExport g_CamExport;


REGISTER_CMD_FUNC(camexport_timeinfo)
{
	pEngfuncs->Con_Printf("Current (filming)ClientTime: %f\n", g_Filming.GetDebugClientTime());
}


// CCamExport //////////////////////////////////////////////////////////////////

CCamExport::CCamExport() {
	m_BvhFileLeft = NULL;
	m_BvhFileMain = NULL;
	m_BvhFileRight = NULL;
}

CCamExport::~CCamExport() {
	EndFiles();
}

bool CCamExport::HasFileLeft() {
	return NULL != m_BvhFileLeft;
}

bool CCamExport::HasFileMain() {
	return NULL != m_BvhFileMain;
}

bool CCamExport::HasFileRight() {
	return NULL != m_BvhFileRight;
}

void CCamExport::EndFile(BvhExport * & bvhFile) {
	if(NULL != bvhFile) delete bvhFile;
	bvhFile = NULL;
}

BvhExport * CCamExport::BeginFile(wchar_t const * folder, wchar_t const * fileName, char const * rootName, double frameTime) {
	
	std::wstring strFileName(folder);
	if(folder && 0 < wcslen(folder))
		strFileName.append(L"\\");
	strFileName.append(fileName);

	BvhExport * bvhFile = new BvhExport(
		strFileName.c_str(),
		rootName,
		frameTime
	);

	return bvhFile;
}

void CCamExport::BeginFileLeft(wchar_t const * folder, double frameTime) {
	m_BvhFileLeft = BeginFile(folder, L"motion_left.bvh", "MdtCamLeft", frameTime);
}

void CCamExport::BeginFileMain(wchar_t const * folder, double frameTime) {
	m_BvhFileMain = BeginFile(folder, L"motion.bvh", "MdtCam", frameTime);
}

void CCamExport::BeginFileRight(wchar_t const * folder, double frameTime) {
	m_BvhFileRight = BeginFile(folder, L"motion_right.bvh", "MdtCamRight", frameTime);
}


void CCamExport::EndFileLeft() {
	EndFile(m_BvhFileLeft);
}

void CCamExport::EndFileMain() {
	EndFile(m_BvhFileMain);
}

void CCamExport::EndFileRight() {
	EndFile(m_BvhFileRight);
}


void CCamExport::EndFiles() {
	EndFileLeft();
	EndFileMain();
	EndFileRight();
}


void CCamExport::WriteFrame(BvhExport * bvhFile, double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation) {
	if(bvhFile)
		bvhFile->WriteFrame(Xposition, Yposition, Zposition, Zrotation, Xrotation, Yrotation);
}

void CCamExport::WriteLeftFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation) {
	WriteFrame(m_BvhFileLeft, Xposition, Yposition, Zposition, Zrotation, Xrotation, Yrotation);
}

void CCamExport::WriteMainFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation) {
	WriteFrame(m_BvhFileMain, Xposition, Yposition, Zposition, Zrotation, Xrotation, Yrotation);
}

void CCamExport::WriteRightFrame(double Xposition, double Yposition, double Zposition, double Zrotation, double Xrotation, double Yrotation) {
	WriteFrame(m_BvhFileRight, Xposition, Yposition, Zposition, Zrotation, Xrotation, Yrotation);
}


