#pragma once

#include "SourceInterfaces.h"

#include <string>
#include <map>

bool Hook_C_BaseEntity_ToolRecordEnties(void);

class ClientTools
{
public:
	ClientTools();

	void DebugEntIndex(int index);

	void SetClientTools(SOURCESDK::CSGO::IClientTools * clientTools);

	void OnPostToolMessage(SOURCESDK::CSGO::HTOOLHANDLE hEntity, SOURCESDK::CSGO::KeyValues * msg);

	void OnC_BaseEntity_ToolRecordEntities(void);

	bool GetRecording(void);

	void StartRecording(wchar_t const * fileName);
	void EndRecording();


private:
	std::map<std::string, int> m_Dictionary;
	std::map<SOURCESDK::CSGO::HTOOLHANDLE, int> m_TrackedHandles;

	bool m_Recording;
	SOURCESDK::CSGO::IClientTools * m_ClientTools;
	FILE * m_File;

	void UpdateRecording();

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

	void WriteDictionary(char const * value);

	void Write(bool value);
	void Write(int value);
	void Write(double value);
	void Write(char const * value); // Consider using WriteDictionary instead (if string is long enough and likely to repeat often).
	void Write(SOURCESDK::Vector const & value);
	void Write(SOURCESDK::QAngle const & value);
	void Write(SOURCESDK::Quaternion const & value);
	void Write(SOURCESDK::CSGO::CBoneList const * value);
};

extern ClientTools g_ClientTools;