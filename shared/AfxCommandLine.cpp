#include "stdafx.h"

#include "AfxCommandLine.h"

#include <Windows.h>

namespace advancedfx {

CCommandLine::CCommandLine()
{
	m_Args = CommandLineToArgvW(GetCommandLineW(), &m_nArgs);
	if (nullptr == m_Args) m_nArgs = 0;
}

CCommandLine::~CCommandLine()
{
	if(m_Args) LocalFree(m_Args);
}

int CCommandLine::GetArgC() const
{
	return m_nArgs;
}

const wchar_t * CCommandLine::GetArgV(int index) const
{
	if (index < 0 || m_nArgs <= index) return L"";

	return m_Args[index];
}

int CCommandLine::FindParam(const wchar_t * param) const
{
	for (int i = 1; i < m_nArgs; ++i)
	{
		if (0 == _wcsicmp(param, m_Args[i])) return i;
	}

	return 0;
}

} // namespace advancedfx {
