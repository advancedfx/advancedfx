#include "stdafx.h"

#include "AfxGameRecord.h"

#include <string>

namespace advancedfx {

CAfxGameRecord::CAfxGameRecord()
: m_Recording(false) {

}

CAfxGameRecord::~CAfxGameRecord() {
    EndRecording();
}

bool CAfxGameRecord::GetRecording(void)
{
	return m_Recording;
}

bool CAfxGameRecord::StartRecording(wchar_t const * fileName, int version)
{
	EndRecording();

	m_Recording = true;

	Dictionary_Clear();
	m_File = 0;

	m_HiddenFileOffset = 0;
	m_Hidden.clear();

	_wfopen_s(&m_File, fileName, L"wb");

	if (m_File)
	{
		fputs("afxGameRecord", m_File);
		fputc('\0', m_File);

		fwrite(&version, sizeof(version), 1, m_File);
        return true;
	}
	else      
    {
        m_Recording = false;
		return false;
    }
}

void CAfxGameRecord::EndRecording()
{
	if (!m_Recording)
		return;

	if (m_File)
	{
		fclose(m_File);
	}

	Dictionary_Clear();

	m_Recording = false;
}

void CAfxGameRecord::BeginFrame(float frameTime)
{
    if (!m_Recording) return;

    WriteDictionary("afxFrame");
	Write((float)frameTime);
	m_HiddenFileOffset = ftell(m_File);
	Write((int)0);
}

void CAfxGameRecord::EndFrame()
{
    if (!m_Recording) return;

	if (m_File && m_HiddenFileOffset && 0 < m_Hidden.size())
	{
		WriteDictionary("afxHidden");

		size_t curOffset = ftell(m_File);

		int offset = (int)(curOffset - m_HiddenFileOffset);

		fseek(m_File, m_HiddenFileOffset, SEEK_SET);
		Write((int)offset);
		fseek(m_File, curOffset, SEEK_SET);

		Write((int)m_Hidden.size());

		for (std::set<int>::iterator it = m_Hidden.begin(); it != m_Hidden.end(); ++it)
		{
			Write((int)(*it));
		}

		m_Hidden.clear();
		m_HiddenFileOffset = 0;
	}

	WriteDictionary("afxFrameEnd");
}

void CAfxGameRecord::WriteDictionary(char const * value)
{
	int idx = Dictionary_Get(value);

	Write(idx);

	if (-1 == idx)
	{
		Write(value);
	}
}

void CAfxGameRecord::Write(bool value)
{
	if (!m_File) return;

	unsigned char ucValue = value ? 1 : 0;

	fwrite(&ucValue, sizeof(ucValue), 1, m_File);
}

void CAfxGameRecord::Write(int value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CAfxGameRecord::Write(float value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CAfxGameRecord::Write(double value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CAfxGameRecord::Write(char const * value)
{
	if (!m_File) return;

	fputs(value, m_File);
	fputc('\0', m_File);
}

void CAfxGameRecord::MarkHidden(int value)
{
	m_Hidden.insert(value);
}

FILE * CAfxGameRecord::GetFile() {
    return m_File;
}

void CAfxGameRecord::Dictionary_Clear()
{
    m_Dictionary.clear();
}

int CAfxGameRecord::Dictionary_Get(char const * value)
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

} // namespace advancedfx {