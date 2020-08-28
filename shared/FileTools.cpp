#include "stdafx.h"

#include "FileTools.h"

#include <windows.h>

// TODO: this needs to be checked for errors:
// There are probably errors when takeDigits is 0 or when all digits are taken.
bool SuggestTakePath(wchar_t const * takePath, int takeDigits, std::wstring & outPath)
{
	bool bOk;
	LPWSTR buf = NULL;
	size_t bufLen;

	if(takeDigits < 0)
		takeDigits = 0;

	bufLen = wcslen(takePath) +takeDigits +1;

	bOk =
		0 != (buf = (LPWSTR)malloc(sizeof(WCHAR)*bufLen))
	;
		
	if(bOk)
	{
		int ofs = wcslen(takePath);
		int iMax = -1;
		WIN32_FIND_DATAW findData;
		HANDLE hFindFile;

		wcscpy_s(buf, bufLen, takePath);

		for(int i=0; i<takeDigits; i++)
		{
			buf[ofs +i] = L'?';
		}

		buf[ofs +takeDigits] = L'\0';

		hFindFile = FindFirstFileW(buf, &findData);

		if(INVALID_HANDLE_VALUE != hFindFile)
		{
			do
			{
				int flen = wcslen(findData.cFileName);
				if(takeDigits <= flen)
				{
					wchar_t * curDigits = findData.cFileName +flen -takeDigits;
					int iCur = _wtoi(curDigits);
					if(iMax < iCur) iMax = iCur;
				}

			} while(FindNextFileW(hFindFile, &findData));

			FindClose(hFindFile);
		}

		iMax++;

		wchar_t tmpDigits[33];

		_itow_s(iMax, tmpDigits, 10);

		for(int i=0; i<takeDigits; i++)
		{
			buf[ofs +i] = L'0';
		}

		int digitsLen = wcslen(tmpDigits);

		for(int i=0; i<takeDigits && i<digitsLen; i++)
		{
			buf[ofs +takeDigits -1 -i] = tmpDigits[digitsLen -1 -i];
		}

		outPath.assign(buf);
	}

	free(buf);

	return bOk;
}


bool CreatePath(wchar_t const * path, std::wstring & outPath, bool noErrorIfExits)
{
	bool bOk;
	LPWSTR buf = NULL;
	int length;

	bOk =
		0 != (length = GetFullPathNameW(path, 0, NULL, NULL))
		&& 0 != (buf = (LPWSTR)malloc(sizeof(WCHAR)*length))
		&& length == GetFullPathNameW(path, length, buf, NULL) +1
	;

	if(bOk)
	{
		bool bCreated = false;
		LPWSTR findPos = 0;
		int numstacked = 0;

		// if the diectory does not exist
		// then walk the up until one is created or all attempts failed:
		do {
			if(findPos)
			{
				*findPos = L'\0';
				numstacked++;
			}

			bCreated = CreateDirectoryW(buf, NULL) || noErrorIfExits && GetLastError() == ERROR_ALREADY_EXISTS;
		} while(!bCreated && 0 != (findPos = wcsrchr(buf, '\\')));

		// try to walk down again until the last directory is created:
		if(bCreated)
		{
			while(bCreated && numstacked) {
				buf[wcslen(buf)] = '\\';
				numstacked--;

				bCreated = 0 != CreateDirectoryW(buf, NULL);
			}
		}

		bOk = bCreated;
	}

	if(bOk)
	{
		outPath.assign(buf);
	}

	free(buf);

	return bOk;
}
