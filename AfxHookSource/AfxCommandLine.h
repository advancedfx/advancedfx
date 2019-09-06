#pragma once

class CAfxCommandLine
{
public:
	CAfxCommandLine();
	~CAfxCommandLine();

	int GetArgC() const;
	const wchar_t * GetArgV(int index) const;

	int FindParam(const wchar_t * param) const;

private:
	wchar_t ** m_Args;
	int m_nArgs;
};

extern CAfxCommandLine  * g_CommandLine;