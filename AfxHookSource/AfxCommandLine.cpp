#include "stdafx.h"

#include "AfxCommandLine.h"

#include <Windows.h>

CAfxCommandLine  * g_CommandLine = nullptr;

CAfxCommandLine::CAfxCommandLine()
{
	m_Args = CommandLineToArgvW(GetCommandLineW(), &m_nArgs);
	if (nullptr == m_Args) m_nArgs = 0;
}

CAfxCommandLine::~CAfxCommandLine()
{
	if(m_Args) LocalFree(m_Args);
}

int CAfxCommandLine::GetArgC() const
{
	return m_nArgs;
}

const wchar_t * CAfxCommandLine::GetArgV(int index) const
{
	if (index < 0 || m_nArgs <= index) return L"";

	return m_Args[index];
}

int CAfxCommandLine::FindParam(const wchar_t * param) const
{
	for (int i = 1; i < m_nArgs; ++i)
	{
		if (0 == _wcsicmp(param, m_Args[i])) return i;
	}

	return 0;
}
