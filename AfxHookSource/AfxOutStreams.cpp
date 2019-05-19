#include "stdafx.h"

#include "AfxOutStreams.h"
#include "AfxWriteFileLimiter.h"
#include "hlaeFolder.h"

#include <shared/RawOutput.h>
#include <shared/OpenExrOutput.h>
#include <shared/StringTools.h>
#include <shared/FileTools.h>

#include <prop/AfxHookSource/SourceInterfaces.h>

#include <string>
#include <sstream>
#include <iomanip>

bool CAfxOutImageStream::SupplyVideoData(const CAfxImageBuffer & buffer)
{
	CAfxWriteFileLimiterScope writeFileLimiterScope;

	std::wstring path;

	if (CAfxImageFormat::PF_ZFloat == buffer.Format.PixelFormat)
	{
		return CreateCapturePath(".exr", path) && WriteFloatZOpenExr(
			path.c_str(),
			(unsigned char*)buffer.Buffer,
			buffer.Format.Width,
			buffer.Format.Height,
			sizeof(float),
			buffer.Format.Pitch,
			m_IfZip ? WFZOEC_Zip : WFZOEC_None
		);
	}

	if (CAfxImageFormat::PF_A == buffer.Format.PixelFormat)
	{
		return m_IfBmpNotTga
			? CreateCapturePath(".bmp", path) && WriteRawBitmap((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, 8, buffer.Format.Pitch)
			: CreateCapturePath(".tga", path) && WriteRawTarga((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, 8, true, buffer.Format.Pitch, 0)
			;
	}

	bool isBgra = CAfxImageFormat::PF_BGRA == buffer.Format.PixelFormat;

	return m_IfBmpNotTga && !isBgra
		? CreateCapturePath(".bmp", path) && WriteRawBitmap((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, 24, buffer.Format.Pitch)
		: CreateCapturePath(".tga", path) && WriteRawTarga((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, isBgra ? 32 : 24, false, buffer.Format.Pitch, isBgra ? 8 : 0)
		;
}


bool CAfxOutImageStream::CreateCapturePath(const char * fileExtension, std::wstring &outPath)
{
	if (!m_TriedCreatePath)
	{
		m_TriedCreatePath = true;

		bool dirCreated = CreatePath(m_Path.c_str(), m_Path);
		if (dirCreated)
		{
			m_SucceededCreatePath = true;
		}
		else
		{
			std::string ansiString;
			if (!WideStringToUTF8String(m_Path.c_str(), ansiString)) ansiString = "[n/a]";

			Tier0_Warning("ERROR: could not create \"%s\"\n", ansiString.c_str());
		}
	}

	if (!m_SucceededCreatePath)
		return false;

	std::wostringstream os;
	os << m_Path << L"\\" << std::setfill(L'0') << std::setw(5) << m_FrameNumber << std::setw(0) << fileExtension;

	outPath = os.str();

	++m_FrameNumber;

	return true;
}


void ReplaceAllW(std::wstring & str, const std::wstring & from, const std::wstring & to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream(const CAfxImageFormat & imageFormat, const std::wstring & path, const std::wstring && ffmpegOptions, float frameRate)
	: CAfxOutVideoStream(imageFormat)
{
	std::wstring myPath(path);

	if (!m_TriedCreatePath)
	{
		m_TriedCreatePath = true;

		bool dirCreated = CreatePath(myPath.c_str(), myPath);
		if (dirCreated)
		{
			m_SucceededCreatePath = true;
		}
		else
		{
			std::string ansiString;
			if (!WideStringToUTF8String(myPath.c_str(), ansiString)) ansiString = "[n/a]";

			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream: could not create path \"%s\"\n", ansiString.c_str());
		}
	}

	if (m_SucceededCreatePath)
	{
		std::wstring ffmpegExe(GetHlaeFolderW());
		ffmpegExe.append(L"ffmpeg\\bin\\ffmpeg.exe");

		std::wostringstream ffmpegArgs(ffmpegExe);

		ffmpegArgs << L"-i pipe:0 -f rawvideo -pixelFormat ";

		switch (imageFormat.PixelFormat)
		{
		case CAfxImageFormat::PF_BGR:
			ffmpegArgs << L"bgr24";
			break;
		case CAfxImageFormat::PF_BGRA:
			ffmpegArgs << L"bgra";
			break;
		case CAfxImageFormat::PF_A:
			ffmpegArgs << L"gray";
			break;
		default:
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream: Unsupported image format.");
			return;
		}

		ffmpegArgs << " -framerate " << frameRate;
		ffmpegArgs << " -videosize " << imageFormat.Width << "x" << imageFormat.Height;

		std::wstring myFFMPEGOptions(ffmpegOptions);

		myPath.append(L"\\");

		ReplaceAllW(myFFMPEGOptions, L"{AFX_STREAM_PATH}", myPath);
		ReplaceAllW(myFFMPEGOptions, L"{QUOTE}", L"\"");
		ReplaceAllW(myFFMPEGOptions, L"\\{", L"{");
		ReplaceAllW(myFFMPEGOptions, L"\\}", L"}");

		ffmpegArgs << ffmpegOptions;
		ffmpegArgs << L"\0";

		std::wstring commandLine(ffmpegArgs.str());

		STARTUPINFOW startupInfo;

		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		startupInfo.dwFlags = STARTF_USESTDHANDLES;

		SECURITY_ATTRIBUTES saAttr;

		ZeroMemory(&saAttr, sizeof(SECURITY_ATTRIBUTES));
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		// Create a pipe for the child process's STDOUT.

		if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
		{
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream: StdoutRd CreatePipe.\n");
			m_hChildStd_OUT_Rd = NULL;
			m_hChildStd_OUT_Wr = NULL;
		}

		// Ensure the read handle to the pipe for STDOUT is not inherited.

		if (NULL != m_hChildStd_OUT_Rd && !SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		{
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream:Stdout SetHandleInformation.\n");
			CloseHandle(m_hChildStd_OUT_Rd);
			m_hChildStd_OUT_Rd = NULL;
		}

		// Create a pipe for the child process's STDERR.

		if (!CreatePipe(&m_hChildStd_ERR_Rd, &m_hChildStd_ERR_Wr, &saAttr, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream: StdErrRd CreatePipe.\n");
			m_hChildStd_ERR_Wr = NULL;
		}

		// Ensure the read handle to the pipe for STDERR is not inherited.

		if (NULL != m_hChildStd_ERR_Rd && !SetHandleInformation(m_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:StdErr SetHandleInformation.\n");
			CloseHandle(m_hChildStd_ERR_Rd);
			m_hChildStd_ERR_Rd = NULL;
		}

		// Create a pipe for the child process's STDIN. 

		if (!CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream: Stdin CreatePipe.\n");
			m_hChildStd_IN_Rd = NULL;
			m_hChildStd_IN_Wr = NULL;
		}

		// Ensure the write handle to the pipe for STDIN is not inherited. 

		if (NULL != m_hChildStd_IN_Wr && !SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:Stdin SetHandleInformation.\n");
			CloseHandle(m_hChildStd_IN_Wr);
			m_hChildStd_IN_Wr = NULL;
		}


		ZeroMemory(&m_ProcessInfo, sizeof(m_ProcessInfo));

		m_Okay = NULL != m_hChildStd_IN_Rd
			&& NULL != m_hChildStd_IN_Wr
			&& NULL != m_hChildStd_ERR_Rd
			&& NULL != m_hChildStd_ERR_Wr
			&& NULL != m_hChildStd_OUT_Rd
			&& NULL != m_hChildStd_OUT_Wr
			? TRUE : FALSE;

		if (FALSE != m_Okay)
		{
			m_Okay = CreateProcessW(
				myFFMPEGOptions.c_str(),
				&(commandLine[0]),
				NULL,
				NULL,
				TRUE,
				CREATE_NO_WINDOW,
				NULL,
				NULL,
				&startupInfo,
				&m_ProcessInfo
			);

			if (TRUE != m_Okay)
			{
				Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream: CreateProcessW.\n");
			}
		}
		if (m_Okay)
		{
		}
		else
		{
			Close();
		}
	}
	else m_Okay = FALSE;
}

void CAfxOutFFMPEGVideoStream::Close()
{
	if (m_hChildStd_IN_Wr)
	{
		CloseHandle(m_hChildStd_IN_Wr);
		m_hChildStd_IN_Wr = NULL;
	}

	if (FALSE != m_Okay)
	{
		DWORD waitCode = WAIT_TIMEOUT;
		for (int i = 0; i < 20000 && WAIT_TIMEOUT == waitCode; ++i)
		{
			if (HandleOutAndErr())
			{
				waitCode = WaitForSingleObject(m_ProcessInfo.hProcess, 1);

			}
			else waitCode = WAIT_FAILED;
		}

		if (WAIT_OBJECT_0 != waitCode)
		{
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::Close.\n");
		}

		CloseHandle(m_ProcessInfo.hProcess);
		CloseHandle(m_ProcessInfo.hThread);

		m_Okay = FALSE;
	}

	if (m_hChildStd_IN_Rd)
	{
		CloseHandle(m_hChildStd_IN_Rd);
		m_hChildStd_IN_Rd = NULL;
	}
	if (m_hChildStd_OUT_Rd)
	{
		CloseHandle(m_hChildStd_OUT_Rd);
		m_hChildStd_OUT_Rd = NULL;
	}
	if (m_hChildStd_OUT_Wr)
	{
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = NULL;
	}
	if (m_hChildStd_ERR_Rd)
	{
		CloseHandle(m_hChildStd_ERR_Rd);
		m_hChildStd_ERR_Rd = NULL;
	}
	if (m_hChildStd_ERR_Wr)
	{
		CloseHandle(m_hChildStd_ERR_Wr);
		m_hChildStd_ERR_Wr = NULL;
	}
}

bool CAfxOutFFMPEGVideoStream::SupplyVideoData(const CAfxImageBuffer & buffer)
{
	if (TRUE != m_Okay) return false;

	if (!(buffer.Format == m_ImageFormat))
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::SupplyVideoData: Format mismatch.\n");
		Close();
		return false;
	}

	if (!HandleOutAndErr()) return false;

	DWORD bytesWritten;

	if (TRUE != WriteFile(m_hChildStd_IN_Wr, buffer.Buffer, (DWORD)buffer.Format.Bytes, &bytesWritten, NULL) || bytesWritten != (DWORD)buffer.Format.Bytes)
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::SupplyVideoData: Could not write to pipe.\n");
		Close();
		return false;
	}

	if (!HandleOutAndErr()) return false;

	return true;
}

CAfxOutFFMPEGVideoStream::~CAfxOutFFMPEGVideoStream()
{
	Close();
}

bool CAfxOutFFMPEGVideoStream::HandleOutAndErr()
{
	if (!m_Okay) return false;

	CHAR chBuf[251];
	DWORD bytesAvail;

	if (PeekNamedPipe(m_hChildStd_ERR_Rd, NULL, 0, NULL, &bytesAvail, NULL))
	{
		bool hadError = 0 < bytesAvail;

		while (0 < bytesAvail)
		{
			DWORD dwBytesRead;
			if (ReadFile(m_hChildStd_ERR_Rd, chBuf, 250, &dwBytesRead, NULL))
			{
				chBuf[dwBytesRead] = 0;
				Tier0_Warning("%s", chBuf);
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr ReadFile.\n");
				Close();
				return false;
			}
		}

		if(hadError)
		{
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr had data.\n");
			Close();
			return false;
		}
	}
	else
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr PeekNamedPipe.\n");
		Close();
		return false;
	}

	if (PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &bytesAvail, NULL))
	{
		while (0 < bytesAvail)
		{
			DWORD dwBytesRead;
			if (ReadFile(m_hChildStd_OUT_Rd, chBuf, 250, &dwBytesRead, NULL))
			{
				chBuf[dwBytesRead] = 0;
				Tier0_Msg("%s", chBuf);
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdOut ReadFile.\n");
				Close();
				return false;
			}
		}
	}
	else
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdOut PeekNamedPipe.\n");
		Close();
		return false;
	}

	return true;
}
