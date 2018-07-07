#include "stdafx.h"

#include "StringTools.h"

#include <windows.h>

#include <list>


bool UTF8StringToWideString(char const * ansiChars, std::wstring & outWideString)
{
	LPWSTR wideChars;
	int length;
	bool bOk;

	if (0 == (length = MultiByteToWideChar(
		CP_UTF8,
		0,
		ansiChars,
		-1,
		NULL,
		0
	)))
		return false;

	if (!(wideChars = (LPWSTR)malloc(sizeof(WCHAR) * length)))
		return false;

	bOk = length == MultiByteToWideChar(
		CP_UTF8,
		0,
		ansiChars,
		-1,
		wideChars,
		length
	);

	if (bOk)
		outWideString.assign(wideChars);

	free(wideChars);

	return bOk;
}

bool WideStringToUTF8String(wchar_t const * wideChars, std::string & outAnsiString)
{
	LPSTR ansiChars;
	int length;
	bool bOk;

	if (0 == (length = WideCharToMultiByte(
		CP_UTF8,
		0,
		wideChars,
		-1,
		NULL,
		0,
		NULL,
		NULL
	)))
		return false;

	if (!(ansiChars = (LPSTR)malloc(sizeof(CHAR) * length)))
		return false;

	bOk = length == WideCharToMultiByte(
		CP_UTF8,
		0,
		wideChars,
		-1,
		ansiChars,
		sizeof(CHAR) * length,
		NULL,
		NULL
	);

	if (bOk)
		outAnsiString.assign(ansiChars);

	free(ansiChars);

	return bOk;
}


bool AnsiStringToWideString(char const * ansiChars, std::wstring & outWideString)
{
	LPWSTR wideChars;
	int length;
	bool bOk;

	if(0 == (length = MultiByteToWideChar(
		CP_ACP,
		0,
		ansiChars,
		-1,
		NULL,
		0
	)))
		return false;
	
	if(!(wideChars = (LPWSTR)malloc(sizeof(WCHAR) * length)))
		return false;

	bOk = length == MultiByteToWideChar(
			CP_ACP,
			0,
			ansiChars,
			-1,
			wideChars,
			length
	);

	if(bOk)
		outWideString.assign(wideChars);

	free(wideChars);

	return bOk;
}

bool WideStringToAnsiString(wchar_t const * wideChars, std::string & outAnsiString)
{
	LPSTR ansiChars;
	int length;
	bool bOk;

	if(0 == (length = WideCharToMultiByte(
		CP_ACP,
		0,
		wideChars,
		-1,
		NULL,
		0,
		NULL,
		NULL
	)))
		return false;

	if(!(ansiChars = (LPSTR)malloc(sizeof(CHAR) * length)))
		return false;

	bOk = length == WideCharToMultiByte(
		CP_ACP,
		0,
		wideChars,
		-1,
		ansiChars,
		sizeof(CHAR) * length,
		NULL,
		NULL
	);

	if(bOk)
		outAnsiString.assign(ansiChars);

	free(ansiChars);

	return bOk;
}


bool StringEndsWith(char const * target, char const * ending) {
	size_t lenTarget = strlen(target);
	size_t lenEnding = strlen(ending);

	if(lenTarget < lenEnding) return false;

	return !strcmp(target +(lenTarget-lenEnding), ending);
}

bool StringEndsWithW(wchar_t const * target, wchar_t const * ending)
{
	size_t lenTarget = wcslen(target);
	size_t lenEnding = wcslen(ending);

	if (lenTarget < lenEnding) return false;

	return !wcscmp(target + (lenTarget - lenEnding), ending);
}


bool StringBeginsWith(char const * target, char const * beginning) {
	while(*target && *beginning) {
		if(*beginning != *target)
			return false;
		target++;
		beginning++;
	}

	if(*beginning && !*target)
		return false;

	return true;
}

bool StringIBeginsWith(char const * target, char const * beginning)
{
	while (*target && *beginning) {
		if (tolower(*beginning) != tolower(*target))
			return false;
		target++;
		beginning++;
	}

	if (*beginning && !*target)
		return false;

	return true;
}

bool StringIsAlphas(char const * value)
{
	if(StringIsEmpty(value)) return true;

	while(*value)
	{
		if(!isalpha(*value))
			return false;

		++value;
	}

	return true;
}

bool StringIsDigits(char const * value)
{
	if(StringIsEmpty(value)) return true;

	while(*value)
	{
		if(!isdigit(*value))
			return false;

		++value;
	}

	return true;
}

bool StringIsAlNum(char const * value)
{
	if(StringIsEmpty(value)) return true;

	while(*value)
	{
		if(!isalnum(*value))
			return false;

		++value;
	}

	return true;
}

bool StringIsEmpty(char const * value)
{
	return !StringIsNull(value) && *value == '\0';
}

bool StringIsNull(char const * value)
{
	return 0 == value;
}

bool StringWildCard1Matched( const char * sz_Mask, const char * sz_Target )
{
	std::list<std::string> maskWords;
	bool leadingWildCard = false;
	bool trailingWildCard = false;

	{
		bool firstMatch = true;
		bool lastWasWildCard = false;
		std::string curWord;
		bool hasWord = false;

		for(const char * curMask = sz_Mask; *curMask; curMask++)
		{
			if('\\' == *curMask)
			{
				curMask++;

				if('*' == *curMask)
				{
					if(firstMatch)
					{
						firstMatch = false;
						leadingWildCard = true;
					}

					if(hasWord)
					{
						maskWords.push_back(curWord);
						hasWord = false;
					}

					lastWasWildCard = true;

					continue;
				}
			}

			firstMatch = false;
			lastWasWildCard = false;
			if(!hasWord) curWord.assign("");
			hasWord = true;
			curWord.push_back(*curMask);
		}

		if(hasWord) maskWords.push_back(curWord);
		trailingWildCard = lastWasWildCard;
	}

	if(0 == maskWords.size())
		return 0 == strlen(sz_Target) || leadingWildCard && trailingWildCard;

	int idx = 0;

	for(std::list<std::string>::iterator it = maskWords.begin(); it != maskWords.end(); it++)
	{
		const char * matchPos = strstr(sz_Target, it->c_str());
		
		if(!matchPos)
			return false;

		if(0 == idx && !leadingWildCard && 0 < matchPos - sz_Target)
			return false;

		if(idx + 1 == maskWords.size() && !trailingWildCard)
		{
			return StringEndsWith(sz_Target, it->c_str());
		}

		idx++;
		sz_Target = matchPos +strlen(it->c_str()); // words don't overlap
	}

	return true;
}
