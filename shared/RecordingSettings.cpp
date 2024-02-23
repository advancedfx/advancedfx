#include "stdafx.h"
#include "RecordingSettings.h"
#include "StringTools.h"

namespace advancedfx {


// CRecordingSettings ///////////////////////////////////////////////////////

CRecordingSettings::CShared CRecordingSettings::m_Shared;

CRecordingSettings::CShared::CShared()
{
	CRecordingSettings * classicSettings = new CClassicRecordingSettings();
	m_NamedSettings.emplace(classicSettings->GetName(), classicSettings);

	m_DefaultSettings = new CDefaultRecordingSettings(classicSettings);
	m_DefaultSettings->AddRef();
	m_NamedSettings.emplace(m_DefaultSettings->GetName(), m_DefaultSettings);

	{
		CRecordingSettings * settings = new CFfmpegRecordingSettings("afxFfmpeg", true, "-c:v libx264 -preset slow -crf 22 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CRecordingSettings * settings = new CFfmpegRecordingSettings("afxFfmpegYuv420p", true, "-c:v libx264 -pix_fmt yuv420p -preset slow -crf 22 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CRecordingSettings * settings = new CFfmpegRecordingSettings("afxFfmpegLosslessFast", true, "-c:v libx264rgb -preset ultrafast -crf 0 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CRecordingSettings * settings = new CFfmpegRecordingSettings("afxFfmpegLosslessBest", true, "-c:v libx264rgb -preset veryslow -crf 0 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CRecordingSettings * settings = new CFfmpegRecordingSettings("afxFfmpegRaw", true, "-c:v rawvideo {QUOTE}{AFX_STREAM_PATH}\\\\video.avi{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CRecordingSettings * settings = new CFfmpegRecordingSettings("afxFfmpegHuffyuv", true, "-c:v huffyuv {QUOTE}{AFX_STREAM_PATH}\\\\video.avi{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CRecordingSettings * settings = new CSamplingRecordingSettings("afxSampler30", true, m_DefaultSettings, EasySamplerSettings::ESM_Trapezoid, 30.0f, 1.0f, 1.0f);
		m_NamedSettings.emplace(settings->GetName(), settings);
	}
}

CClassicRecordingSettings::CShared::~CShared()
{
	m_NamedSettings.clear();
	m_DefaultSettings->Release();
}

void CRecordingSettings::Console(ICommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("print", arg1))
		{
			for (auto it = m_Shared.m_NamedSettings.begin(); it != m_Shared.m_NamedSettings.end(); ++it)
			{
				advancedfx::Message("%s%s\n", it->second.Settings->GetName(), it->second.Settings->GetProtected() ? " (protected)" : "");
			}
			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argC)
		{
			const char * arg2 = args->ArgV(2);

			if (CRecordingSettings * setting = CRecordingSettings::GetByName(arg2))
			{
				CSubCommandArgs subArgs(args, 3);
				setting->Console_Edit(&subArgs);
			}
			else
			{
				advancedfx::Warning("AFXERROR: There is no recording setting named %s.\n", arg2);
			}
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argC)
		{
			const char * arg2 = args->ArgV(2);

			auto it = m_Shared.m_NamedSettings.find(arg2);

			if (it != m_Shared.m_NamedSettings.end())
			{
				if (it->second.Settings->GetProtected())
				{
					advancedfx::Warning("AFXERROR: Setting %s is protected and thus can not be deleted.\n", arg2);
				}
				else if (!m_Shared.DeleteIfUnrefrenced(it))
				{
					advancedfx::Warning("AFXERROR: Could not delete %s, because it has further references.\n", arg2);
				}
			}
			else
			{
				advancedfx::Warning("AFXERROR: There is no recording setting named %s.\n", arg2);
			}
			return;
		}
		else if (0 == _strcmpi("add", arg1))
		{
			if (5 == argC && 0 == _stricmp("ffmpeg", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					advancedfx::Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					advancedfx::Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CRecordingSettings * settings = new CFfmpegRecordingSettings(arg3, false, args->ArgV(4));
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}
			else if (5 == argC && 0 == _stricmp("ffmpegEx", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					advancedfx::Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					advancedfx::Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CRecordingSettings * settings = new CFfmpegExRecordingSettings(arg3, false, args->ArgV(4));
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}
			else if (4 == argC && 0 == _stricmp("sampler", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					advancedfx::Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					advancedfx::Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CRecordingSettings * settings = new CSamplingRecordingSettings(arg3, false, m_Shared.m_DefaultSettings, EasySamplerSettings::ESM_Trapezoid, 30.0f, 1.0f, 1.0f);
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}
			else if (4 == argC && 0 == _stricmp("multi", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					advancedfx::Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					advancedfx::Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CRecordingSettings * settings = new CMultiRecordingSettings(arg3, false);
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}

			advancedfx::Message(
				"%s add ffmpeg <name> \"<yourOptionsHere>\" - Adds an FFMPEG setting, <yourOptionsHere> are output options, use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }. For an example see one of the afxFfmpeg* templates (edit them).\n"
				"%s add ffmpegEx <name> \"<yourOptionsHere>\" - Adds an extended FFMPEG setting, <yourOptionsHere> are output options, use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }. Further variables: {FFMPEG_PATH} {PIXEL_FORMAT} {FRAMERATE} {WIDTH} {HEIGHT} - For an example see one of the afxFfmpeg* templates (edit them).\n"
				"%s add sampler <name> - Adds a sampler with 30 fps and default settings, edit it afterwards to change them.\n"
				"%s add multi <name> - Adds multi settings, edit it afterwards to add settings to it.\n"
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
	}

	advancedfx::Message(
		"%s print - List currently registered settings\n"
		"%s edit <name> - Edit setting.\n"
		"%s remove <name> - Remove setting.\n"
		"%s add [...] - Add a setting.\n"
		, arg0
		, arg0
		, arg0
		, arg0
	);
}


// CClassicRecordingSettings ////////////////////////////////////////////////

void CClassicRecordingSettings::Console_Edit(ICommandArgs * args)
{
	advancedfx::Message("%s (type classic) recording setting options:\n", m_Name.c_str());
	advancedfx::Warning("The classic settings are controlled through mirv_streams settings and can not be edited.\n");
}

advancedfx::COutVideoStreamCreator * CClassicRecordingSettings::CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float frameRate, const char * pathSuffix) const
{
	std::wstring capturePath;
	if (stream.GetStreamFolder(capturePath)) {
		std::wstring widePathSuffix;
		if (UTF8StringToWideString(pathSuffix, widePathSuffix))
		{
			capturePath.append(widePathSuffix);

			advancedfx::StreamCaptureType captureType = stream.GetCaptureType();

			return new advancedfx::CClassicRecordingSettingsCreator(capturePath, (captureType == advancedfx::StreamCaptureType::Depth24ZIP || captureType == advancedfx::StreamCaptureType::DepthFZIP), streams.GetFormatBmpNotTga());
		}
		else
		{
			advancedfx::Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", pathSuffix);
		}
	}

	return nullptr;
}

// CFfmpegRecordingSettings ////////////////////////////////////////////////

void CFfmpegRecordingSettings::Console_Edit(ICommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("options", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					advancedfx::Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_FfmpegOptions = args->ArgV(2);
				return;
			}

			advancedfx::Message(
				"%s options \"<yourOptionsHere>\" - Set output options, use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }.\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_FfmpegOptions.c_str()
			);
			return;
		}
	}

	advancedfx::Message("%s (type ffmpeg) recording setting options:\n", m_Name.c_str());
	advancedfx::Message(
		"%s options [...] - FFMPEG options.\n"
		, arg0
	);
}

advancedfx::COutVideoStreamCreator* CFfmpegRecordingSettings::CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float frameRate, const char * pathSuffix) const
{
	std::wstring widePathSuffix;
	if (UTF8StringToWideString(pathSuffix, widePathSuffix))
	{
		std::wstring wideOptions;
		if (UTF8StringToWideString(m_FfmpegOptions.c_str(), wideOptions))
		{
			std::wstring capturePath;
			if (stream.GetStreamFolder(capturePath)) {
				capturePath.append(widePathSuffix);

				advancedfx::StreamCaptureType captureType = stream.GetCaptureType();

				return new advancedfx::CFfmpegRecordingSettingsCreator(capturePath, std::wstring(L"{QUOTE}{FFMPEG_PATH}{QUOTE} -f rawvideo -pixel_format {PIXEL_FORMAT} -loglevel repeat+level+warning -framerate {FRAMERATE} -video_size {WIDTH}x{HEIGHT} -i pipe:0 -vf setsar=sar=1/1 ").append(wideOptions), frameRate);
			}
		}
		else
		{
			advancedfx::Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", m_FfmpegOptions.c_str());
		}
	}
	else
	{
		advancedfx::Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", pathSuffix);
	}

	return nullptr;
}



// CFfmpegRecordingSettings ////////////////////////////////////////////////

void CFfmpegExRecordingSettings::Console_Edit(ICommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("options", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					advancedfx::Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_FfmpegOptions = args->ArgV(2);
				return;
			}

			advancedfx::Message(
				"%s options \"<yourOptionsHere>\" - Set output options use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }. Further variables: {FFMPEG_PATH} {PIXEL_FORMAT} {FRAMERATE} {WIDTH} {HEIGHT}\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_FfmpegOptions.c_str()
			);
			return;
		}
	}

	advancedfx::Message("%s (type ffmpegEx) recording setting options:\n", m_Name.c_str());
	advancedfx::Message(
		"%s options [...] - FFMPEG options.\n"
		, arg0
	);
}

advancedfx::COutVideoStreamCreator* CFfmpegExRecordingSettings::CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float frameRate, const char * pathSuffix) const
{
	std::wstring widePathSuffix;
	if (UTF8StringToWideString(pathSuffix, widePathSuffix))
	{
		std::wstring wideOptions;
		if (UTF8StringToWideString(m_FfmpegOptions.c_str(), wideOptions))
		{
			std::wstring capturePath;
			if (stream.GetStreamFolder(capturePath)) {
				capturePath.append(widePathSuffix);

				advancedfx::StreamCaptureType captureType = stream.GetCaptureType();

				return new advancedfx::CFfmpegRecordingSettingsCreator(capturePath, wideOptions, frameRate);
			}
		}
		else
		{
			advancedfx::Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", m_FfmpegOptions.c_str());
		}
	}
	else
	{
		advancedfx::Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", pathSuffix);
	}

	return nullptr;
}


// CDefaultRecordingSettings ////////////////////////////////////////////////

void CDefaultRecordingSettings::Console_Edit(ICommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("settings", arg1))
		{
			if (3 == argC)
			{
				CRecordingSettings * settings = CRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					advancedfx::Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else if(settings->InheritsFrom(this))
				{
					advancedfx::Warning("AFXERROR: Can not assign a setting that depends on this setting.\n");
				}
				else
				{
					if (m_DefaultSettings) m_DefaultSettings->Release();
					m_DefaultSettings = settings;
					if (m_DefaultSettings) m_DefaultSettings->AddRef();
				}

				return;
			}

			advancedfx::Message(
				"%s settings <settingsName> - Use settings with name <settingsName> as default settings.\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_DefaultSettings ? m_DefaultSettings->GetName() : "[null]"
			);
			return;
		}
	}

	advancedfx::Message("%s (type default) recording setting options:\n", m_Name.c_str());
	advancedfx::Message(
		"%s settings [...] - Set default settings.\n"
		, arg0
	);
}

// CMultiRecordingSettings //////////////////////////////////////////////////


void CMultiRecordingSettings::Console_Edit(ICommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 == argC)
			{
				CRecordingSettings * settings = CRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					advancedfx::Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else if (settings->InheritsFrom(this))
				{
					advancedfx::Warning("AFXERROR: Can not assign a setting that depends on this setting.\n");
				}
				else
				{
					if (settings) settings->AddRef();
					m_Settings.push_back(settings);
				}

				return;
			}

			advancedfx::Message(
				"%s add <settingsName> - Add settings with name <settingsName>.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1))
		{
			if (3 == argC)
			{
				CRecordingSettings * settings = CRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					advancedfx::Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else
				{
					for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
					{
						CRecordingSettings * itSettings = *it;
						if (itSettings == settings)
						{
							it = m_Settings.erase(it);
							itSettings->Release();
						}
					}
				}

				return;
			}

			advancedfx::Message(
				"%s remove <settingsName> - Remove settings with name <settingsName>.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			int idx = 0;
			for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
			{
				CRecordingSettings * itSettings = *it;
				advancedfx::Message("%i: %s\n", idx, itSettings ? itSettings->GetName() : "[null]");
				++idx;
			}
			if (0 == idx) advancedfx::Message("[empty]\n");
			return;
		}
	}

	advancedfx::Message("%s (type multi) recording setting options:\n", m_Name.c_str());
	advancedfx::Message(
		"%s add <settingsName> - Add settings.\n"
		"%s remove <settingsName> - Remove settings.\n"
		"%s print <settingsName> - List settings.\n"
		, arg0
		, arg0
		, arg0
	);
}

// CSamplingRecordingSettings ///////////////////////////////////////////////



advancedfx::COutVideoStreamCreator* CSamplingRecordingSettings::CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float frameRate, const char * pathSuffix) const
{
	if (m_OutputSettings)
	{
		if (advancedfx::COutVideoStreamCreator* outVideoStreamCreator = m_OutputSettings->CreateOutVideoStreamCreator(streams, stream, m_OutFps, pathSuffix))
		{
			return new advancedfx::CSamplingRecordingSettingsCreator(outVideoStreamCreator, frameRate, m_Method, m_OutFps ? 1.0 / m_OutFps : 0.0, m_Exposure, m_FrameStrength, streams.GetImageBufferPool());
		}
	}

	return nullptr;
}

void CSamplingRecordingSettings::Console_Edit(ICommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("settings", arg1))
		{
			if (3 == argC)
			{
				CRecordingSettings * settings = CRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					advancedfx::Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else if (settings->InheritsFrom(this))
				{
					advancedfx::Warning("AFXERROR: Can not assign a setting that depends on this setting.\n");
				}
				else
				{
					if (m_OutputSettings) m_OutputSettings->Release();
					m_OutputSettings = settings;
					if (m_OutputSettings) m_OutputSettings->AddRef();
				}

				return;
			}

			advancedfx::Message(
				"%s settings <settingsName> - Use settings with name <settingsName> as output settings.\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_OutputSettings ? m_OutputSettings->GetName() : "[null]"
			);
			return;
		}
		else if (0 == _stricmp("fps", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					advancedfx::Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_OutFps = (float)atof(args->ArgV(2));
				return;
			}

			advancedfx::Message(
				"%s fps <fValue> - Output FPS, has to be greater than 0.\n"
				"Current value: %f\n"
				, arg0
				, m_OutFps
			);
			return;
		}
		else if (0 == _stricmp("method", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					advancedfx::Warning("This setting is protected and can not be changed.\n");
					return;
				}

				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp(arg2, "rectangle"))
				{
					m_Method = EasySamplerSettings::ESM_Rectangle;
				}
				else if (0 == _stricmp(arg2, "trapezoid"))
				{
					m_Method = EasySamplerSettings::ESM_Trapezoid;
				}
				else
				{
					advancedfx::Warning("AFXERROR: Invalid value.\n");
				}

				return;
			}

			const char * curMethod = "[n/a]";

			switch (m_Method)
			{
			case EasySamplerSettings::ESM_Rectangle:
				curMethod = "rectangle";
				break;
			case EasySamplerSettings::ESM_Trapezoid:
				curMethod = "trapezoid";
				break;
			};

			advancedfx::Message(
				"%s method rectangle|trapezoid.\n"
				"Current value: %s\n"
				, arg0
				, curMethod
			);
			return;
		}
		else if (0 == _stricmp("exposure", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					advancedfx::Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_Exposure = atof(args->ArgV(2));
				return;
			}

			advancedfx::Message(
				"%s exposure <fValue>.\n"
				"Current value: %f\n"
				, arg0
				, m_Exposure
			);
			return;
		}
		else if (0 == _stricmp("strength", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					advancedfx::Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_FrameStrength = (float)atof(args->ArgV(2));
				return;
			}

			advancedfx::Message(
				"%s strength <fValue>.\n"
				"Current value: %f\n"
				, arg0
				, m_FrameStrength
			);
			return;
		}
	}

	advancedfx::Message("%s (type sampling) recording setting options:\n", m_Name.c_str());
	advancedfx::Message(
		"%s settings [...] - Output settings.\n"
		"%s fps [...] - Output fps.\n"
		"%s method [...] - Sampling method (default: trapezoid).\n"
		"%s exposure [...] - Frame exposure (0.0 (0\xc2\xb0 shutter angle) - 1.0 (360\xc2\xb0 shutter angle), default: 1.0).\n"
		"%s strength [...] - Frame strength (0.0 (max cross-frame blur) - 1.0 (no cross-frame blur), default: 1.0).\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}



} // namespace advancedfx
