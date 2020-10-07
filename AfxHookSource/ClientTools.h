#pragma once

#include "SourceInterfaces.h"
#include "WrpConsole.h"

#include <string>
#include <set>
#include <map>

class CClientTools abstract
{
public:
	static inline CClientTools * Instance(void)
	{
		return m_Instance;
	}

	CClientTools();
	virtual ~CClientTools();

	virtual void OnPostToolMessage(void * hEntity, void * msg);

	virtual void OnBeforeFrameRenderStart(void);

	virtual void OnAfterSetupEngineView(void) {

	}

	virtual void OnAfterFrameRenderEnd(void);

	virtual bool SuppotsAutoEnableRecordingMode(void) {
		return false;
	}

	virtual void EnableRecordingMode_set(bool value) {

	}

	virtual bool EnableRecordingMode_get() {
		return false;
	}

	bool GetRecording(void);

	virtual void StartRecording(wchar_t const * fileName);

	virtual void EndRecording();

	int Debug_get(void)
	{
		return m_Debug;
	}

	void Debug_set(int value)
	{
		m_Debug = value;
	}

	bool RecordWeapons_get(void)
	{
		return m_RecordWeapons;
	}

	void RecordWeapons_set(bool value)
	{
		m_RecordWeapons = value;
	}

	bool RecordProjectiles_get(void)
	{
		return m_RecordProjectiles;
	}

	void RecordProjectiles_set(bool value)
	{
		m_RecordProjectiles = value;
	}

	bool RecordPlayers_get(void)
	{
		return m_RecordPlayers;
	}

	void RecordPlayers_set(bool value)
	{
		m_RecordPlayers = value;
	}

	bool RecordCamera_get(void)
	{
		return m_RecordCamera;
	}

	void RecordCamera_set(bool value)
	{
		m_RecordCamera = value;
	}

	int RecordViewModels_get(void)
	{
		return m_RecordViewModels;
	}

	void RecordViewModels_set(int value)
	{
		m_RecordViewModels = value;
	}

	bool RecordInvisible_get(void)
	{
		return m_RecordInvisible;
	}

	void RecordInvisible_set(bool value)
	{
		m_RecordInvisible = value;
	}

	int RecordPlayerCameras_get(void)
	{
		return m_RecordPlayerCameras;
	}

	void RecordPlayerCameras_set(int value)
	{
		m_RecordPlayerCameras = value;
	}

protected:
	virtual float ScaleFov(int width, int height, float fov) { return fov; }

	void WriteDictionary(char const * value);

	void Write(bool value);
	void Write(int value);
	void Write(float value);
	void Write(double value);
	void Write(char const * value); // Consider using WriteDictionary instead (if string is long enough and likely to repeat often).
	void Write(SOURCESDK::Vector const & value);
	void Write(SOURCESDK::QAngle const & value);
	void Write(SOURCESDK::Quaternion const & value);

	void MarkHidden(int value);

private:
	static CClientTools * m_Instance;

	std::map<std::string, int> m_Dictionary;

	size_t m_HiddenFileOffset;
	std::set<int> m_Hidden;

	bool m_EnableRecording = false;
	bool m_Recording;
	FILE * m_File;

	int m_Debug = 0;
	bool m_RecordCamera = true;
	bool m_RecordPlayers = true;
	int m_RecordPlayerCameras = -1;
	bool m_RecordWeapons = true;
	bool m_RecordProjectiles = true;
	int m_RecordViewModels = 0;
	bool m_RecordInvisible = false;

	void Dictionary_Clear()
	{
		m_Dictionary.clear();
	}

	int Dictionary_Get(char const * value)
	{
		std::string sValue(value);

		std::map<std::string, int>::iterator it = m_Dictionary.find(sValue);

		if (it != m_Dictionary.end())
		{
			const std::pair<const std::string, int> & pair = *it;
			return pair.second;
		}

		size_t oldDictSize = m_Dictionary.size();

		m_Dictionary[sValue] = oldDictSize;
		return -1;
	}
};

bool ClientTools_Console_Cfg(IWrpCommandArgs * args);
