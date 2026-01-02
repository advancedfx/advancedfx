#pragma once

#include "AfxConsole.h"
#include "OutVideoStreamCreators.h"

#include <string>
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
	virtual bool GetStreamFolder(std::wstring& outFolder) const = 0;
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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) = 0;

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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override
	{
		if (m_DefaultSettings)
			return m_DefaultSettings->CreateOutVideoStreamCreator(streams, stream, fps, pathSuffix);

		return nullptr;
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
	{
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override
	{
		std::list<advancedfx::COutVideoStreamCreator*> outVideoStreams;

		for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
		{
			if (CRecordingSettings * setting = *it)
			{
				std::string mySuffix(pathSuffix);
				mySuffix.append("\\");
				mySuffix.append(setting->GetName());

				advancedfx::COutVideoStreamCreator* item = setting->CreateOutVideoStreamCreator(streams, stream, fps, mySuffix.c_str());
				outVideoStreams.push_back(item);
			}
		}

		auto result = new CMyOutVideoStreamCreator(std::move(outVideoStreams));
		result->AddRef();
		return result;
	}

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
	{
	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override;
};

class CFfmpegRecordingSettings : public CRecordingSettings
{
public:
	CFfmpegRecordingSettings(const char * name, bool bProtected, const char * szFfmpegOptions)
		: CRecordingSettings(name, bProtected)
		, m_FfmpegOptions(szFfmpegOptions)
	{

	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override;

private:
	std::string m_FfmpegOptions;
};

class CFfmpegExRecordingSettings : public CRecordingSettings
{
public:
	CFfmpegExRecordingSettings(const char * name, bool bProtected, const char * szFfmpegOptions)
		: CRecordingSettings(name, bProtected)
		, m_FfmpegOptions(szFfmpegOptions)
	{

	}

	virtual void Console_Edit(ICommandArgs * args) override;

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override;

private:
	std::string m_FfmpegOptions;
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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override;

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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override;

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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) override
	{
		return InputCreateOutVideoStreamCreator(streams, stream, fps, pathSuffix, 0);
	}

	virtual bool InheritsFrom(CRecordingSettings * setting) const override
	{
		if (CRecordingSettings::InheritsFrom(setting)) return true;

		if (m_OutputSettings) if (m_OutputSettings->InheritsFrom(setting)) return true;

		return false;
	}	

	class advancedfx::COutVideoStreamCreator* InputCreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix, size_t index)
	{
		advancedfx::COutVideoStreamCreator* result = nullptr;
		if(nullptr == m_OutVideoStreamCreator) {
			m_OutVideoStreamCreator = new CMyOutVideoStreamCreatorShared(m_InputCount);
			m_OutVideoStreamCreator->AddRef();
		}
		if(0 == index) {
			auto outputSettingsCreator = m_OutputSettings ? m_OutputSettings->CreateOutVideoStreamCreator(streams, stream, fps, pathSuffix) : nullptr;
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
