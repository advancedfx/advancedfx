#pragma once

#include "AfxConsole.h"
#include "OutVideoStreamCreators.h"
#include "OutputPathTemplate.h"

#include <string>
#include <list>
#include <map>
#include <algorithm>

namespace advancedfx {

enum class StreamCaptureType
{
    Invalid = 0,
    Normal,
    Depth24,
    Depth24ZIP,
    DepthF,
    DepthFZIP
};

class IRecordStreamSettings {
public:
	/// Folder of the stream, only used by the default implementation of GetOutputPathValues.
	virtual bool GetStreamFolder(std::wstring& outFolder) const {
		return false;
	}

	/// Sets outValues to the values for output path templates of the stream.
	/// The default implementation only provides {STREAM_PATH} from GetStreamFolder.
	virtual void GetOutputPathValues(COutputPathValues& outValues) const {
		outValues = COutputPathValues();
		std::wstring folder;
		if (GetStreamFolder(folder)) outValues.SetString(OutputPathVariable_StreamPath, folder);
	}

	virtual StreamCaptureType GetCaptureType() const = 0;
    virtual CGrowingBufferPoolThreadSafe * GetImageBufferPool() const = 0;
    virtual bool GetFormatBmpNotTga() const = 0;
};

class CRecordingSettings
{
public:

	void AddRef() {
		m_RefCount++;
	}
	
	void Release() {
		if(0 == --m_RefCount)
			delete this;
	}

	int GetRefCount() {
		return m_RefCount;
	}

	static CRecordingSettings * GetDefault()
	{
		return m_Shared.m_DefaultSettings;
	}

	static CRecordingSettings * GetByName(const char * name)
	{
		auto it = m_Shared.m_NamedSettings.find(CNamedSettingKey(name));

		if (m_Shared.m_NamedSettings.end() != it)
			return it->second.Settings;
		else
			return nullptr;
	}

	static void Console(ICommandArgs * args);

	CRecordingSettings(const char * name, bool bProtected)
		: m_Name(name)
		, m_Protected(bProtected)
	{
	}

	const char * GetName() const
	{
		return m_Name.c_str();
	}

	bool GetProtected() const
	{
		return m_Protected;
	}

	virtual void Console_Edit(ICommandArgs * args) = 0;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) = 0;

	/// Appends the output path templates this setting writes to for the stream, expanded as far as possible.
	/// Used to determine {TAKE_NUMBER} before any output is created.
	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const {
	}

	virtual bool InheritsFrom(CRecordingSettings * setting) const
	{
		if (setting == this) return true;

		return false;
	}

protected:
	virtual ~CRecordingSettings() {		
	}

	int m_RefCount = 0;
	std::string m_Name;
	bool m_Protected;

	struct CNamedSettingValue {
		CRecordingSettings * Settings;

		CNamedSettingValue()
			: Settings(nullptr)
		{
		}

		CNamedSettingValue(CRecordingSettings * settings)
			: Settings(settings)
		{
			Settings->AddRef();
		}

		CNamedSettingValue(const CNamedSettingValue & copyFrom)
			: Settings(copyFrom.Settings)
		{
			if (Settings) Settings->AddRef();
		}

		~CNamedSettingValue()
		{
			if(Settings) Settings->Release();
		}
	};

	class CNamedSettingKey
	{
	public:
		CNamedSettingKey(const char * value)
			: m_Value(value)
		{
			std::transform(m_Value.begin(), m_Value.end(), m_Value.begin(), [](unsigned char c) { return std::tolower(c); });
		}

		bool operator < (const CNamedSettingKey & y) const
		{
			return m_Value.compare(y.m_Value) < 0;
		}

	private:
		std::string m_Value;
	};

	static struct CShared {
		std::map<CNamedSettingKey, CNamedSettingValue> m_NamedSettings;
		CRecordingSettings * m_DefaultSettings;

		CShared();
		~CShared();

		bool DeleteIfUnrefrenced(std::map<CNamedSettingKey, CNamedSettingValue>::iterator it)
		{
			if (it->second.Settings)
			{
				if (1 == it->second.Settings->GetRefCount())
				{
					it->second.Settings->Release();
					it->second.Settings = nullptr;
					m_NamedSettings.erase(it);
					return true;
				}
				else
				{
					return false;
				}
			}

			m_NamedSettings.erase(it);
			return true;
		}
	} m_Shared;
};

class CDefaultRecordingSettings : public CRecordingSettings
{
public:
	CDefaultRecordingSettings(CRecordingSettings * useSettings)
		: CRecordingSettings("afxDefault", true)
		, m_DefaultSettings(useSettings)
	{
		if (m_DefaultSettings) m_DefaultSettings->AddRef();
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override
	{
		if (m_DefaultSettings)
			return m_DefaultSettings->CreateOutVideoStreamCreator(streams, stream, fps);

		return nullptr;
	}

	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override
	{
		if (m_DefaultSettings)
			m_DefaultSettings->GetOutputPathTemplates(stream, outTemplates);
	}

	virtual bool InheritsFrom(CRecordingSettings * setting) const override
	{
		if (CRecordingSettings::InheritsFrom(setting)) return true;

		if (m_DefaultSettings) if (m_DefaultSettings->InheritsFrom(setting)) return true;

		return false;
	}

protected:
	virtual ~CDefaultRecordingSettings()
	{
		if (m_DefaultSettings)
		{
			m_DefaultSettings->Release();
			m_DefaultSettings = nullptr;
		}
	}
private:
	CRecordingSettings * m_DefaultSettings;
};

class CMultiRecordingSettings : public CRecordingSettings
{
public:
	CMultiRecordingSettings(const char * name, bool bProtected)
		: CRecordingSettings(name, bProtected)
		, m_Path("{STREAM_PATH}\\{SETTING_NAME}", OutputPathVariables_Stream | OutputPathVariable_SettingName)
	{
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override;

	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override;

	virtual bool InheritsFrom(CRecordingSettings * setting) const override
	{
		if (CRecordingSettings::InheritsFrom(setting)) return true;

		for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
		{
			if (CRecordingSettings * itSetting = *it) if (itSetting->InheritsFrom(setting)) return true;
		}

		return false;
	}

protected:
	virtual ~CMultiRecordingSettings() override
	{
		for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
		{
			if (CRecordingSettings * setting = *it)
				setting->Release();
		}
	}
private:
	std::list<CRecordingSettings *> m_Settings;

	/// Template for {STREAM_PATH} of the child settings.
	COutputPathSetting m_Path;

	/// Passes on the stream, but with {STREAM_PATH} for a child setting.
	class CChildStreamSettings : public IRecordStreamSettings {
	public:
		CChildStreamSettings(const IRecordStreamSettings& stream, const std::wstring& streamPathTemplate)
			: m_Stream(stream)
			, m_StreamPathTemplate(streamPathTemplate)
		{
		}

		virtual void GetOutputPathValues(COutputPathValues& outValues) const override {
			m_Stream.GetOutputPathValues(outValues);
			outValues.SetTemplate(OutputPathVariable_StreamPath, m_StreamPathTemplate);
		}

		virtual StreamCaptureType GetCaptureType() const override {
			return m_Stream.GetCaptureType();
		}

		virtual CGrowingBufferPoolThreadSafe * GetImageBufferPool() const override {
			return m_Stream.GetImageBufferPool();
		}

		virtual bool GetFormatBmpNotTga() const override {
			return m_Stream.GetFormatBmpNotTga();
		}

	private:
		const IRecordStreamSettings& m_Stream;
		std::wstring m_StreamPathTemplate;
	};

	std::wstring GetChildStreamPathTemplate(const IRecordStreamSettings& stream, const CRecordingSettings * child) const;

	class CMyOutVideoStreamCreator
		: public advancedfx::COutVideoStreamCreator {
	public:
		CMyOutVideoStreamCreator(std::list<advancedfx::COutVideoStreamCreator*>&& list)
			: m_List(list)
		{

		}

		virtual advancedfx::TIOutVideoStream<true>* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) override {
			std::list<advancedfx::TIOutVideoStream<true>*> outVideoStreams;
			for (auto it = m_List.begin(); it != m_List.end(); it++) {
				if (nullptr == *it) continue; // creating failed.
				outVideoStreams.push_back((*it)->CreateOutVideoStream(imageFormat));
			}
			auto result = new advancedfx::COutMultiVideoStream(imageFormat, std::move(outVideoStreams));
			result->AddRef();
			return result;
		}

	protected:
		~CMyOutVideoStreamCreator() {
			for (auto it = m_List.begin(); it != m_List.end(); it++) {
				if(auto stream = *it) stream->Release();
			}
			m_List.clear();
		}

	private:
		std::list<advancedfx::COutVideoStreamCreator*> m_List;
	};
};

class CClassicRecordingSettings : public CRecordingSettings
{
public:
	CClassicRecordingSettings()
		: CRecordingSettings("afxClassic", true)
		, m_Path("{STREAM_PATH}\\{SEQUENCE_NR}.{EXT}", OutputPathVariables_Stream | OutputPathVariable_SequenceNr | OutputPathVariable_Ext, OutputPathVariable_SequenceNr)
	{
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override;

	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override;

private:
	COutputPathSetting m_Path;
};

#define ADVANCEDFX_FFMPEG_DEFAULT_PATH "{STREAM_PATH}\\video.mp4"

class CFfmpegRecordingSettings : public CRecordingSettings
{
public:
	CFfmpegRecordingSettings(const char * name, bool bProtected, const char * szFfmpegOptions, const char * szDefaultPath = ADVANCEDFX_FFMPEG_DEFAULT_PATH)
		: CRecordingSettings(name, bProtected)
		, m_FfmpegOptions(szFfmpegOptions)
		, m_Path(szDefaultPath, OutputPathVariables_Stream)
	{

	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override;

	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override;

private:
	std::string m_FfmpegOptions;
	COutputPathSetting m_Path;
};

class CFfmpegExRecordingSettings : public CRecordingSettings
{
public:
	CFfmpegExRecordingSettings(const char * name, bool bProtected, const char * szFfmpegOptions, const char * szDefaultPath = ADVANCEDFX_FFMPEG_DEFAULT_PATH)
		: CRecordingSettings(name, bProtected)
		, m_FfmpegOptions(szFfmpegOptions)
		, m_Path(szDefaultPath, OutputPathVariables_Stream)
	{

	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override;

	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override;

private:
	std::string m_FfmpegOptions;
	COutputPathSetting m_Path;
};

class CSamplingRecordingSettings : public CRecordingSettings
{
public:
	CSamplingRecordingSettings(const char * name, bool bProtected, CRecordingSettings * outputSettings, EasySamplerSettings::Method method, float outFps, double exposure, float frameStrength)
		: CRecordingSettings(name, bProtected)
		, m_OutputSettings(outputSettings)
		, m_Method(method)
		, m_OutFps(outFps)
		, m_Exposure(exposure)
		, m_FrameStrength(frameStrength)
	{
		if (m_OutputSettings) m_OutputSettings->AddRef();
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override;

	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override
	{
		if (m_OutputSettings) m_OutputSettings->GetOutputPathTemplates(stream, outTemplates);
	}

	virtual bool InheritsFrom(CRecordingSettings * setting) const override
	{
		if (CRecordingSettings::InheritsFrom(setting)) return true;

		if (m_OutputSettings) if (m_OutputSettings->InheritsFrom(setting)) return true;

		return false;
	}

protected:
	virtual ~CSamplingRecordingSettings()
	{
		if (m_OutputSettings)
		{
			m_OutputSettings->Release();
			m_OutputSettings = nullptr;
		}
	}
private:
	CRecordingSettings * m_OutputSettings;
	EasySamplerSettings::Method m_Method;
	float m_OutFps;
	double m_Exposure;
	float m_FrameStrength;
};

class CInterleaveRecordingSettings;

class CInterleaveInputRecordingSettings: public CRecordingSettings
{
public:
	CInterleaveInputRecordingSettings(const char * name, bool bProtected, CInterleaveRecordingSettings * pMainInterleaveInput, int index);

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override;

	// GetOutputPathTemplates: Output is created by the main input.

	virtual bool InheritsFrom(CRecordingSettings * setting) const override;

protected:
	virtual ~CInterleaveInputRecordingSettings() override;

private:
	CInterleaveRecordingSettings * m_pMainInterleaveInput;
	int m_Index;
};

class CInterleaveRecordingSettings: public CRecordingSettings
{
public:
	CInterleaveRecordingSettings(const char * name, bool bProtected, CRecordingSettings * outputSettings, size_t inputCount)
		: CRecordingSettings(name, bProtected)
		, m_OutputSettings(outputSettings)
		, m_InputCount(inputCount)
	{
		if(m_OutputSettings) m_OutputSettings->AddRef();
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps) override
	{
		return InputCreateOutVideoStreamCreator(streams, stream, fps, 0);
	}

	/// The output uses the stream of the main input (index 0), e.g. for {STREAM_NAME}.
	virtual void GetOutputPathTemplates(const IRecordStreamSettings& stream, std::list<std::wstring>& outTemplates) const override
	{
		if (m_OutputSettings) m_OutputSettings->GetOutputPathTemplates(stream, outTemplates);
	}

	virtual bool InheritsFrom(CRecordingSettings * setting) const override
	{
		if (CRecordingSettings::InheritsFrom(setting)) return true;

		if (m_OutputSettings) if (m_OutputSettings->InheritsFrom(setting)) return true;

		return false;
	}	

	class advancedfx::COutVideoStreamCreator* InputCreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, size_t index)
	{
		advancedfx::COutVideoStreamCreator* result = nullptr;
		if(nullptr == m_OutVideoStreamCreator) {
			m_OutVideoStreamCreator = new CMyOutVideoStreamCreatorShared(m_InputCount);
			m_OutVideoStreamCreator->AddRef();
		}
		if(0 == index) {
			auto outputSettingsCreator = m_OutputSettings ? m_OutputSettings->CreateOutVideoStreamCreator(streams, stream, fps) : nullptr;
			m_OutVideoStreamCreator->SetOutVideoStreamCreator(outputSettingsCreator);
			if(outputSettingsCreator) outputSettingsCreator->Release();
			result = m_OutVideoStreamCreator;
		} else {
			result = new CMyOutVideoStreamCreator(m_OutVideoStreamCreator, index);
		}
		result->AddRef();

		m_CreateCount++;
		if(m_CreateCount % m_InputCount == 0) {
			m_CreateCount = 0;
			m_OutVideoStreamCreator->Release();
			m_OutVideoStreamCreator = nullptr;
		}
		return result;
	}

protected:
	virtual ~CInterleaveRecordingSettings()
	{
		if(m_OutVideoStreamCreator) {
			m_OutVideoStreamCreator->Release();
			m_OutVideoStreamCreator = nullptr;
		}

		if (m_OutputSettings)
		{
			m_OutputSettings->Release();
			m_OutputSettings = nullptr;
		}
	}

private:
	class CMyOutVideoStreamCreatorShared;

	size_t m_InputCount;
	size_t m_CreateCount = 0;
	CRecordingSettings * m_OutputSettings;
	CMyOutVideoStreamCreatorShared * m_OutVideoStreamCreator = nullptr;

	class CMyOutVideoStreamCreatorShared
	: public advancedfx::COutVideoStreamCreator
	{
	public:
		CMyOutVideoStreamCreatorShared(size_t inputCount)
		: m_InputCount(inputCount)
		{

		}

		void SetOutVideoStreamCreator(advancedfx::COutVideoStreamCreator* value) {
			if(m_OutVideoStreamCreator) m_OutVideoStreamCreator->Release();
			m_OutVideoStreamCreator = value;
			if(m_OutVideoStreamCreator) m_OutVideoStreamCreator->AddRef();
		}

		virtual advancedfx::TIOutVideoStream<true>* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) override {
			return InputCreateOutVideoStream(imageFormat,0);
		}		

		advancedfx::TIOutVideoStream<true>* InputCreateOutVideoStream(const advancedfx::CImageFormat& imageFormat,size_t index) {

			std::unique_lock<std::mutex> lock(m_Mutex);
			
			if(!m_OutStreamCreated) {
				m_OutStreamCreated = true;
				auto outOutStream = m_OutVideoStreamCreator ? m_OutVideoStreamCreator->CreateOutVideoStream(imageFormat) : nullptr;
				m_OutStream = new COutInterleaveVideoStream<true>(imageFormat, outOutStream, m_InputCount);
				m_OutStream->AddRef();
				if(outOutStream) outOutStream->Release();
			}

			advancedfx::TIOutVideoStream<true>* result;

			if(index == 0) {
				m_OutStream->AddRef();
				result = m_OutStream;
			} else {
				result = m_OutStream->AddInput(index);
			}

			m_CreatedCount++;
			if(m_CreatedCount == m_InputCount) {
				m_CreatedCount = 0;
				if(m_OutStream) m_OutStream->Release();
				m_OutStream = nullptr;
			}

			return result;
		}		

	protected:
		virtual ~CMyOutVideoStreamCreatorShared() override {
			if(m_OutStream) {
				m_OutStream->Release();
				m_OutStream = nullptr;
			}
			SetOutVideoStreamCreator(nullptr);
		}

	private:
		advancedfx::COutVideoStreamCreator* m_OutVideoStreamCreator = nullptr;
		COutInterleaveVideoStream<true>* m_OutStream = nullptr;
		bool m_OutStreamCreated = false;
		size_t m_InputCount;
		size_t m_CreatedCount=0;
		std::mutex m_Mutex;
	};

	class CMyOutVideoStreamCreator
		: public advancedfx::COutVideoStreamCreator {
	public:
		CMyOutVideoStreamCreator(CMyOutVideoStreamCreatorShared * main, size_t index)
		: m_Main(main)
		, m_Index(index)
		{
			m_Main->AddRef();
		}

		virtual advancedfx::TIOutVideoStream<true>* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) override {
			return m_Main->InputCreateOutVideoStream(imageFormat, m_Index);
		}

	protected:
		~CMyOutVideoStreamCreator() {
			m_Main->Release();
		}

	private:
		CMyOutVideoStreamCreatorShared * m_Main;
		size_t m_Index;
	};	
};

} // namespace advancedfx
