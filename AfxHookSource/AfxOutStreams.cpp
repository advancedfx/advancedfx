#include "stdafx.h"

#include "AfxOutStreams.h"
#include "AfxWriteFileLimiter.h"
#include "hlaeFolder.h"
#include "AfxStreams.h"

#include <shared/RawOutput.h>
#include <shared/OpenExrOutput.h>
#include <shared/StringTools.h>
#include <shared/FileTools.h>

#include <AfxHookSource/SourceInterfaces.h>

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

	bool isBgra = CAfxImageFormat::PF_ARGB == buffer.Format.PixelFormat;

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


void ReplaceAllW(std::wstring & str, const std::map<std::wstring, std::wstring> & replacements)
{
	size_t start_pos = 0;
	while (true)
	{
		size_t found_pos = std::wstring::npos;
		std::map<std::wstring, std::wstring>::const_iterator itFound = replacements.end();

		for (std::map<std::wstring, std::wstring>::const_iterator itR = replacements.begin(); itR != replacements.end(); ++itR)
		{
			if (itR->first.empty()) continue;

			size_t cur_found_pos = str.find(itR->first, start_pos);

			if (cur_found_pos != std::wstring::npos && (found_pos == std::wstring::npos || cur_found_pos < found_pos))
			{
				found_pos = cur_found_pos;
				itFound = itR;
			}
		}

		if (itFound != replacements.end())
		{
			str.replace(found_pos, itFound->first.length(), itFound->second);
			start_pos = found_pos + itFound->second.length();
		}
		else break;
	}
}

BOOL AfxOutFFMPEGVideoStream_CreatePipe(
	const char * pipeName,
	OUT LPHANDLE lpReadPipe,
	OUT LPHANDLE lpWritePipe,
	IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
	DWORD nSize,
	DWORD timeOutMs,
	DWORD dwReadMode,
	DWORD dwWriteMode)
{
	HANDLE ReadPipeHandle, WritePipeHandle;
	DWORD dwError;

	// Only one valid OpenMode flag - FILE_FLAG_OVERLAPPED

	if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (nSize == 0) {
		nSize = 4096;
	}

	ReadPipeHandle = CreateNamedPipeA(
		pipeName,
		PIPE_ACCESS_INBOUND | dwReadMode,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1, // Number of pipes
		nSize, // Out buffer size
		nSize, // In buffer size
		timeOutMs, // Timeout in ms
		lpPipeAttributes
	);

	if (!ReadPipeHandle) {
		return FALSE;
	}

	WritePipeHandle = CreateFileA(
		pipeName,
		GENERIC_WRITE,
		0, // No sharing
		lpPipeAttributes,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | dwWriteMode,
		NULL                       // Template file
	);

	if (INVALID_HANDLE_VALUE == WritePipeHandle) {
		dwError = GetLastError();
		CloseHandle(ReadPipeHandle);
		SetLastError(dwError);
		return FALSE;
	}

	*lpReadPipe = ReadPipeHandle;
	*lpWritePipe = WritePipeHandle;
	return(TRUE);
}

CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream(const CAfxImageFormat & imageFormat, const std::wstring & path, const std::wstring & ffmpegOptions, float frameRate)
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

		std::wostringstream ffmpegArgs;

		ffmpegArgs << L"\"" << ffmpegExe << L"\"";
		ffmpegArgs << L" -f rawvideo -pixel_format ";

		switch (imageFormat.PixelFormat)
		{
		case CAfxImageFormat::PF_RGB:
			ffmpegArgs << L"rgb24";
			break;
		case CAfxImageFormat::PF_ARGB:
			ffmpegArgs << L"argb";
			break;
		case CAfxImageFormat::PF_A:
			ffmpegArgs << L"gray";
			break;
		default:
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::CAfxOutFFMPEGVideoStream: Unsupported image format.");
			return;
		}

		ffmpegArgs << " -framerate " << frameRate;
		ffmpegArgs << " -video_size " << imageFormat.Width << "x" << imageFormat.Height;
		ffmpegArgs << " -i pipe:0 -vf vflip,setsar=sar=1/1";

		std::wstring myFFMPEGOptions(ffmpegOptions);

		// breaking change: // myPath.append(L"\\");

		std::map<std::wstring, std::wstring> replacements;
		replacements[L"{AFX_STREAM_PATH}"] = myPath;
		replacements[L"{QUOTE}"] = L"\"";
		replacements[L"\\{"] = L"{";
		replacements[L"\\}"] = L"}";
		replacements[L"\\\\"] = L"\\";

		ReplaceAllW(myFFMPEGOptions, replacements);

		ffmpegArgs << " " << myFFMPEGOptions;
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

		// Create STDOUT:

		std::ostringstream stOutPipeNameStream;
		stOutPipeNameStream << "\\\\.\\pipe\\AfxHookSource_FFMPEG_Out_" << GetCurrentProcessId() << "_" << (void*)this;
		std::string stdOutPipeName(stOutPipeNameStream.str());

		if (TRUE != AfxOutFFMPEGVideoStream_CreatePipe(
			stdOutPipeName.c_str(),
			&m_hChildStd_OUT_Rd,
			&m_hChildStd_OUT_Wr,
			&saAttr,
			0,
			20 * 1000,
			0, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream: Could not create STDOUT.\n");
		}

		if (INVALID_HANDLE_VALUE != m_hChildStd_OUT_Rd && !SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:STDOUT SetHandleInformation.\n");
			CloseHandle(m_hChildStd_OUT_Rd);
			m_hChildStd_OUT_Rd = INVALID_HANDLE_VALUE;
		}

		/*
		if (INVALID_HANDLE_VALUE == (m_OverlappedStdout.hEvent = CreateEventA(NULL, true, true, NULL)))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:STDOUT CreateEventA.\n");
		}
		*/

		// Create STDERR:

		std::ostringstream stErrPipeNameStream;
		stErrPipeNameStream << "\\\\.\\pipe\\AfxHookSource_FFMPEG_Err_" << GetCurrentProcessId() << "_" << (void*)this;
		std::string stdErrPipeName(stErrPipeNameStream.str());

		if (TRUE != AfxOutFFMPEGVideoStream_CreatePipe(
			stdErrPipeName.c_str(),
			&m_hChildStd_ERR_Rd,
			&m_hChildStd_ERR_Wr,
			&saAttr,
			0,
			20 * 1000,
			0, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream: Could not create STDERR.\n");
		}

		if (INVALID_HANDLE_VALUE != m_hChildStd_ERR_Rd && !SetHandleInformation(m_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:STDERR SetHandleInformation.\n");
			CloseHandle(m_hChildStd_ERR_Rd);
			m_hChildStd_ERR_Rd = INVALID_HANDLE_VALUE;
		}

		/*
		if (INVALID_HANDLE_VALUE == (m_OverlappedStderr.hEvent = CreateEventA(NULL, true, true, NULL)))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:STDERR CreateEventA.\n");
		}
		*/

		// Create STDIN:

		std::ostringstream stdInPipeNameStream;
		stdInPipeNameStream << "\\\\.\\pipe\\AfxHookSource_FFMPEG_In_" << GetCurrentProcessId() << "_" << (void *)this;
		std::string stdInPipeName(stdInPipeNameStream.str());

		if (TRUE != AfxOutFFMPEGVideoStream_CreatePipe(
			stdInPipeName.c_str(),
			&m_hChildStd_IN_Rd,
			&m_hChildStd_IN_Wr,
			&saAttr,
			1024 * 1024,
			20 * 1000,
			0, FILE_FLAG_OVERLAPPED))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream: Could not create STDIN.\n");
		}

		if (INVALID_HANDLE_VALUE != m_hChildStd_IN_Wr && !SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:STDIN SetHandleInformation.\n");
			CloseHandle(m_hChildStd_IN_Wr);
			m_hChildStd_IN_Wr = INVALID_HANDLE_VALUE;
		}

		if (INVALID_HANDLE_VALUE == (m_OverlappedStdin.hEvent = CreateEventA(NULL, true, true, NULL)))
		{
			Tier0_Warning("AFXERROR: CAfxERRFFMPEGVideoStream::CAfxERRFFMPEGVideoStream:STDIN CreateEventA.\n");
		}

		//

		ZeroMemory(&m_ProcessInfo, sizeof(m_ProcessInfo));

		m_Okay = INVALID_HANDLE_VALUE != m_hChildStd_IN_Rd
			&& INVALID_HANDLE_VALUE != m_hChildStd_IN_Wr
			&& INVALID_HANDLE_VALUE != m_hChildStd_ERR_Rd
			&& INVALID_HANDLE_VALUE != m_hChildStd_ERR_Wr
			&& INVALID_HANDLE_VALUE != m_hChildStd_OUT_Rd
			&& INVALID_HANDLE_VALUE != m_hChildStd_OUT_Wr
			? TRUE : FALSE;

		if (FALSE != m_Okay)
		{
			startupInfo.hStdInput = m_hChildStd_IN_Rd;
			startupInfo.hStdError = m_hChildStd_ERR_Wr;
			startupInfo.hStdOutput = m_hChildStd_OUT_Wr;

			m_Okay = CreateProcessW(
				ffmpegExe.c_str(),
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
	if (INVALID_HANDLE_VALUE != m_hChildStd_IN_Wr)
	{
		CloseHandle(m_hChildStd_IN_Wr);
		m_hChildStd_IN_Wr = INVALID_HANDLE_VALUE;
	}

	if (FALSE != m_Okay)
	{
		DWORD waitCode = WAIT_TIMEOUT;
		//for (int i = 0; i < 20000 && WAIT_TIMEOUT == waitCode; ++i)
		while(WAIT_TIMEOUT == waitCode)
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

	if (INVALID_HANDLE_VALUE != m_hChildStd_IN_Rd)
	{
		CloseHandle(m_hChildStd_IN_Rd);
		m_hChildStd_IN_Rd = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_hChildStd_OUT_Rd)
	{
		CloseHandle(m_hChildStd_OUT_Rd);
		m_hChildStd_OUT_Rd = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_hChildStd_OUT_Wr)
	{
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_hChildStd_ERR_Rd)
	{
		CloseHandle(m_hChildStd_ERR_Rd);
		m_hChildStd_ERR_Rd = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_hChildStd_ERR_Wr)
	{
		CloseHandle(m_hChildStd_ERR_Wr);
		m_hChildStd_ERR_Wr = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_OverlappedStdin.hEvent)
	{
		CloseHandle(m_OverlappedStdin.hEvent);

		m_OverlappedStdin.hEvent = INVALID_HANDLE_VALUE;
	}
	/*
	if (INVALID_HANDLE_VALUE != m_OverlappedStdout.hEvent)
	{
		CloseHandle(m_OverlappedStdout.hEvent);

		m_OverlappedStdout.hEvent = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_OverlappedStderr.hEvent)
	{
		CloseHandle(m_OverlappedStderr.hEvent);

		m_OverlappedStderr.hEvent = INVALID_HANDLE_VALUE;
	}
	*/
}

bool CAfxOutFFMPEGVideoStream::SupplyVideoData(const CAfxImageBuffer & buffer)
{
	/*
	static DWORD frames = 0;
	static DWORD firstTickCount = GetTickCount();
	
	++frames;
	DWORD delta = GetTickCount() - firstTickCount;

	if (1000 < delta)
	{
		float fps = delta ? (float)frames / (delta / 1000.0f): 0;
		Tier0_Msg("FPS: %f\n",fps);
		frames = 0;
		firstTickCount = GetTickCount();
	}

	return true;
	*/

	if (TRUE != m_Okay) return false;

	if (!(buffer.Format == m_ImageFormat))
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::SupplyVideoData: Format mismatch.\n");
		Close();
		return false;
	}

	//DWORD lastTickCount = GetTickCount();

	DWORD offset = 0;
	DWORD length = (DWORD)buffer.Format.Bytes;

	if (WriteFile(m_hChildStd_IN_Wr, (LPVOID) & (((char*)buffer.Buffer)[offset]), length, NULL, &m_OverlappedStdin)) return true;
	
	if(ERROR_IO_PENDING != GetLastError())
		return false;

	bool completed = false;

	char chBuf[251];
	DWORD bytesAvail;

	while (!completed)
	{
		if (PeekNamedPipe(m_hChildStd_ERR_Rd, chBuf, 250, NULL, &bytesAvail, NULL))
		{
			while (0 < bytesAvail)
			{
				DWORD dwBytesRead;
				if (ReadFile(m_hChildStd_ERR_Rd, chBuf, min(bytesAvail, 250), &dwBytesRead, NULL))
				{
					chBuf[dwBytesRead] = 0;
					Tier0_Warning("%s", chBuf);
					bytesAvail -= dwBytesRead;
				}
				else
				{
					Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr ReadFile.\n");
					return false;
				}
			}
		}
		else
		{
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr PeekNamedPipe.\n");
			return false;
		}

		if (PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &bytesAvail, NULL))
		{
			while (0 < bytesAvail)
			{
				DWORD dwBytesRead;
				if (ReadFile(m_hChildStd_OUT_Rd, chBuf, min(bytesAvail, 250), &dwBytesRead, NULL))
				{
					chBuf[dwBytesRead] = 0;
					Tier0_Msg("%s", chBuf);
					bytesAvail -= dwBytesRead;
				}
				else
				{
					Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdOut ReadFile.\n");
					return false;
				}
			}
		}
		else
		{
			Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdOut PeekNamedPipe.\n");
			return false;
		}

		DWORD result = WaitForSingleObject(m_OverlappedStdin.hEvent, 0);
		switch (result)
		{
		case WAIT_OBJECT_0:
			completed = true;
			break;
		case WAIT_TIMEOUT:
			break;
		default:
			return false;
		}
	}

	DWORD bytesWritten;
	if (!GetOverlappedResult(m_hChildStd_IN_Wr, &m_OverlappedStdin, &bytesWritten, FALSE) || bytesWritten != length)
		return false;

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
		while (0 < bytesAvail)
		{
			DWORD dwBytesRead;
			if (ReadFile(m_hChildStd_ERR_Rd, chBuf, min(bytesAvail, 250), &dwBytesRead, NULL))
			{
				chBuf[dwBytesRead] = 0;
				Tier0_Warning("%s", chBuf);
				bytesAvail -= dwBytesRead;
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr ReadFile.\n");
				return false;
			}
		}
	}
	else
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdErr PeekNamedPipe.\n");
		return false;
	}

	if (PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &bytesAvail, NULL))
	{
		while (0 < bytesAvail)
		{
			DWORD dwBytesRead;
			if (ReadFile(m_hChildStd_OUT_Rd, chBuf, min(bytesAvail, 250), &dwBytesRead, NULL))
			{
				chBuf[dwBytesRead] = 0;
				Tier0_Msg("%s", chBuf);
				bytesAvail -= dwBytesRead;
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdOut ReadFile.\n");
				return false;
			}
		}
	}
	else
	{
		Tier0_Warning("AFXERROR: CAfxOutFFMPEGVideoStream::HandleOutAndErr: StdOut PeekNamedPipe.\n");
		return false;
	}

	// Check if FFMPEG exited:
	if (WAIT_TIMEOUT != WaitForSingleObject(m_ProcessInfo.hProcess, 0))
	{
		return false;
	}

	return true;
}

// CAfxOutSamplingStream ///////////////////////////////////////////////////////

CAfxOutSamplingStream::CAfxOutSamplingStream(const CAfxImageFormat & imageFormat, CAfxOutVideoStream * outVideoStream, float frameRate, EasySamplerSettings::Method method, double frameDuration, double exposure, float frameStrength)
	: CAfxOutVideoStream(imageFormat)
	, m_OutVideoStream(outVideoStream)
	, m_Time(0.0)
	, m_InputFrameDuration(frameRate ? 1.0 / frameRate : 0.0)
{
	if(m_OutVideoStream) m_OutVideoStream->AddRef();

	unsigned int bytesPerPixel = 1;

	switch (imageFormat.PixelFormat)
	{
	case CAfxImageFormat::PF_RGB:
		m_EasySampler.Byte = new EasyByteSampler(EasySamplerSettings(
			imageFormat.Width * 3,
			imageFormat.Height,
			method,
			frameDuration,
			m_Time,
			exposure,
			frameStrength
		), (int)imageFormat.Pitch, this);
		break;
	case CAfxImageFormat::PF_ARGB:
		m_EasySampler.Byte = new EasyByteSampler(EasySamplerSettings(
			imageFormat.Width * 4,
			imageFormat.Height,
			method,
			frameDuration,
			m_Time,
			exposure,
			frameStrength
		), (int)imageFormat.Pitch, this);
		break;
	case CAfxImageFormat::PF_A:
		m_EasySampler.Byte = new EasyByteSampler(EasySamplerSettings(
			imageFormat.Width * 1,
			imageFormat.Height,
			method,
			frameDuration,
			m_Time,
			exposure,
			frameStrength
		), (int)imageFormat.Pitch, this);
		break;
	case CAfxImageFormat::PF_ZFloat:
		m_EasySampler.Float = new EasyFloatSampler(EasySamplerSettings(
			imageFormat.Width,
			imageFormat.Height,
			method,
			frameDuration,
			m_Time,
			exposure,
			frameStrength
		), this);
		break;
	default:
		Tier0_Warning("AFXERROR: CAfxOutSamplingStream::CAfxOutSamplingStream: Unspoported image format.");
	}
}

CAfxOutSamplingStream::~CAfxOutSamplingStream()
{
	switch (m_ImageFormat.PixelFormat)
	{
	case CAfxImageFormat::PF_RGB:
	case CAfxImageFormat::PF_ARGB:
	case CAfxImageFormat::PF_A:
		delete m_EasySampler.Byte;
		break;
	case CAfxImageFormat::PF_ZFloat:
		delete m_EasySampler.Float;
	};

	if (m_OutVideoStream) m_OutVideoStream->Release();
}

bool CAfxOutSamplingStream::SupplyVideoData(const CAfxImageBuffer & buffer)
{
	if (nullptr == m_OutVideoStream) return false;

	if (!(buffer.Format == m_ImageFormat))
	{
		Tier0_Warning("AFXERROR: CAfxOutSamplingStream::SupplyVideoData: Format mismatch.\n");
		return false;
	}

	switch (m_ImageFormat.PixelFormat)
	{
	case CAfxImageFormat::PF_RGB:
	case CAfxImageFormat::PF_ARGB:
	case CAfxImageFormat::PF_A:
		m_EasySampler.Byte->Sample((const unsigned char*)buffer.Buffer, m_Time);
		break;
	case CAfxImageFormat::PF_ZFloat:
		m_EasySampler.Float->Sample((const float*)buffer.Buffer, m_Time);
	};

	m_Time += m_InputFrameDuration;

	return true;
}

void CAfxOutSamplingStream::Print(unsigned char const * data)
{
	if (CAfxImageBuffer * buffer = g_AfxStreams.ImageBufferPool.AquireBuffer())
	{
		buffer->AutoRealloc(m_ImageFormat);
		memcpy(buffer->Buffer, data, buffer->Format.Bytes);
		m_OutVideoStream->SupplyVideoData(*buffer);
		buffer->Release();
	}
}

void CAfxOutSamplingStream::Print(float const * data)
{
	if (CAfxImageBuffer * buffer = g_AfxStreams.ImageBufferPool.AquireBuffer())
	{
		buffer->AutoRealloc(m_ImageFormat);
		memcpy(buffer->Buffer, data, buffer->Format.Bytes);
		m_OutVideoStream->SupplyVideoData(*buffer);
		buffer->Release();
	}
}
