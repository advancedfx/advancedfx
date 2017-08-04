#pragma once

#include "SourceInterfaces.h"

#include <string>
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

	virtual void OnAfterFrameRenderEnd(void);

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

	bool RecordViewModel_get(void)
	{
		return m_RecordViewModel;
	}

	void RecordViewModel_set(bool value)
	{
		m_RecordViewModel = value;
	}

protected:
	void WriteDictionary(char const * value);

	void Write(bool value);
	void Write(int value);
	void Write(float value);
	void Write(double value);
	void Write(char const * value); // Consider using WriteDictionary instead (if string is long enough and likely to repeat often).
	void Write(SOURCESDK::Vector const & value);
	void Write(SOURCESDK::QAngle const & value);
	void Write(SOURCESDK::Quaternion const & value);

private:
	static CClientTools * m_Instance;

	std::map<std::string, int> m_Dictionary;

	bool m_Recording;
	FILE * m_File;

	int m_Debug = 0;
	bool m_RecordCamera = true;
	bool m_RecordPlayers = true;
	bool m_RecordWeapons = true;
	bool m_RecordProjectiles = true;
	bool m_RecordViewModel = true;

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

		m_Dictionary[sValue] = m_Dictionary.size();
		return -1;
	}
};

