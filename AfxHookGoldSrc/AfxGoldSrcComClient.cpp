#include "stdafx.h"

// Copyright (c) by advancedfx.org
//
// Last changes:
// 2010-03-20 dominik.matrixstorm.com
//
// First changes
// 2010-03-20 dominik.matrixstorm.com

#include "AfxGoldSrcComClient.h"

#include "hooks/user32Hooks.h"


AfxGoldSrcComClient g_AfxGoldSrcComClient;

HANDLE g_ReadPipe = NULL;
HANDLE g_WritePipe = NULL;

void GetAfxPipes()
{
	static bool firstRun = true;
	if(!firstRun)
		return;
	firstRun = false;

	char tmp[33];
	tmp[32] = 0;

	LPSTR cmdLine = GetCommandLine();

	char const * strRead = strstr(cmdLine, "-afxpipes ");
	if(!strRead)
		return;
	strRead += strlen("-afxpipes ");

	char const * strWrite = strstr(strRead, ",");
	if(!strWrite)
		return;
	strWrite += strlen(",");

	char const * strEnd = strstr(strWrite, " ");
	if(!strEnd)
		strEnd = cmdLine +strlen(cmdLine);

	memset(tmp, 0, 33);
	strncpy_s(tmp, strRead, min(32, strWrite -strRead -1));
	g_ReadPipe = (HANDLE)atoi(tmp);

	memset(tmp, 0, 33);
	strncpy_s(tmp, strWrite, min(32, strEnd -strWrite));
	g_WritePipe = (HANDLE)atoi(tmp);
}

HANDLE GetClientReadPipe()
{
	GetAfxPipes();

	return g_ReadPipe;
}

HANDLE GetClientWritePipe()
{
	GetAfxPipes();

	return g_WritePipe;
}


// AfxGoldSrcComClient /////////////////////////////////////////////////////////

AfxGoldSrcComClient::AfxGoldSrcComClient()
: PipeCom(GetClientReadPipe(), GetClientWritePipe())
, m_Connected(false)
{
	ComUInt32 version;
	bool ok = false;

	m_Connected = false;

	version = this->ReadUInt32();

	if(COM_VERSION != version)
	{
		this->Write((ComBoolean)ok);
		throw "AfxGoldSrcComVersion mismatch";
		return;
	}

	ok = true;
	this->Write((ComBoolean)ok);

	m_Width = this->ReadInt32();
	m_Height = this->ReadInt32();
	m_ForceAlpha8 = this->ReadBoolean();
	m_FullScreen = this->ReadBoolean();
	m_OptimizeCaptureVis = this->ReadBoolean();
	m_ParentWindow = this->ReadInt32();
	m_RenderMode = (ComRenderMode)this->ReadInt32();

	m_Connected = ok;
}

AfxGoldSrcComClient::~AfxGoldSrcComClient()
{
	Close();
}


void AfxGoldSrcComClient::Close()
{
	if(!m_Connected)
		return;

	SendMessage(CLM_Closed);
}

bool AfxGoldSrcComClient::GetFullScreen()
{
	return m_FullScreen;
}


bool AfxGoldSrcComClient::GetForceAlpha8()
{
	return m_ForceAlpha8;
}

int AfxGoldSrcComClient::GetHeight()
{
	return m_Height;
}


HWND AfxGoldSrcComClient::GetParentWindow()
{
	return (HWND)m_ParentWindow;
}


ComRenderMode AfxGoldSrcComClient::GetRenderMode()
{
	return m_RenderMode;
}

bool AfxGoldSrcComClient::GetOptimizeCaptureVis()
{
	return m_OptimizeCaptureVis;
}

int AfxGoldSrcComClient::GetWidth()
{
	return m_Width;
}


ServerMessage AfxGoldSrcComClient::RecvMessage(void)
{
	return (ServerMessage)this->ReadInt32();
}

void AfxGoldSrcComClient::SendMessage(ClientMessage message)
{
	this->Write((ComInt32)message);
}


void AfxGoldSrcComClient::OnRecordStarting()
{
	SendMessage(CLM_OnRecordStarting);
}

void AfxGoldSrcComClient::OnRecordEnded()
{
	SendMessage(CLM_OnRecordEnded);
}

void AfxGoldSrcComClient::OnHostFrame()
{
	SendMessage(CLM_OnHostFrame);

	//
	// Pull messages from server:

	ServerMessage serverMessage;

	while(SVM_EOT != (serverMessage = RecvMessage()))
	{
		switch(serverMessage)
		{
		case SVM_Close:
			CloseGameWindow();
			break;
		}
	}
}


void AfxGoldSrcComClient::UpdateWindowSize(int x, int y)
{
	SendMessage(CLM_UpdateWindowSize);

	this->Write((ComInt32)x);
	this->Write((ComInt32)y);
}