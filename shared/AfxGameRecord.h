#pragma once

#include <stdio.h>

#include <string>
#include <set>
#include <map>

namespace advancedfx {

class CAfxGameRecord
{
public:
	CAfxGameRecord();
	~CAfxGameRecord();

	bool GetRecording(void);

	bool StartRecording(wchar_t const * fileName, int version);

	void EndRecording();

	void BeginFrame(float frameTime);

	void EndFrame();

	void WriteDictionary(char const * value);

	void Write(bool value);
	void Write(int value);
	void Write(float value);
	void Write(double value);
	void Write(char const * value); // Consider using WriteDictionary instead (if string is long enough and likely to repeat often).

	void MarkHidden(int value);

	FILE * GetFile();

private:
	std::map<std::string, int> m_Dictionary;

	size_t m_HiddenFileOffset;
	std::set<int> m_Hidden;

	bool m_Recording;
	FILE * m_File;

	void Dictionary_Clear();

	int Dictionary_Get(char const * value);
};

} // namespace advancedfx
