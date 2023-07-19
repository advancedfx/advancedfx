#pragma once

namespace advancedfx {

class CCommandLine
{
public:
	CCommandLine();
	~CCommandLine();

	int GetArgC() const;
	const wchar_t * GetArgV(int index) const;

	int FindParam(const wchar_t * param) const;

private:
	wchar_t ** m_Args;
	int m_nArgs;
};

} // namespace advancedfx {
