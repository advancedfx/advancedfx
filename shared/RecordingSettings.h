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
    virtual IImageBufferPool * GetImageBufferPool() const = 0;
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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const = 0;

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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const override
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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const override
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
				item->AddRef();
				outVideoStreams.push_back(item);
			}
		}

		return new CMyOutVideoStreamCreator(std::move(outVideoStreams));
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

		virtual advancedfx::COutVideoStream* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) const override {
			std::list<advancedfx::COutVideoStream*> outVideoStreams;
			for (auto it = m_List.begin(); it != m_List.end(); it++) {
				outVideoStreams.push_back((*it)->CreateOutVideoStream(imageFormat));
			}
			return new advancedfx::COutMultiVideoStream(imageFormat, std::move(outVideoStreams));
		}

	protected:
		~CMyOutVideoStreamCreator() {
			for (auto it = m_List.begin(); it != m_List.end(); it++) {
				(*it)->Release();
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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const override;
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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const override;

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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const override;

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

	virtual class advancedfx::COutVideoStreamCreator* CreateOutVideoStreamCreator(const IRecordStreamSettings & streams, const IRecordStreamSettings& stream, float fps, const char * pathSuffix) const override;

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

} // namespace advancedfx
