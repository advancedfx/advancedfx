#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int _tmain(int argc, TCHAR * argv[])
{
	if(1 >= argc)
	{
		_tprintf(_T("DummLoader.exe <DllName>*\n"));
		return 0;
	}

	for(int i=1; i<argc; i++)
	{
		_tprintf(_T("Loading: \"%s\" "), argv[i]);
		HMODULE hm = LoadLibrary(argv[i]);
		DWORD lastError = GetLastError();
		_tprintf(_T("[ %s ] GetLastError = %u\n"), hm ? _T("OK") : _T("FAIL"), lastError);
	}

	return 0;
}

