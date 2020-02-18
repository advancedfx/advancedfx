#include "stdafx.h"

#include "AfxConsole.h"

#include <sstream>

namespace advancedfx {

// CSubCommandArgs /////////////////////////////////////////////////////////////

CSubCommandArgs::CSubCommandArgs(ICommandArgs* commandArgs, int offset)
	: m_Offset(offset)
	, m_CommandArgs(commandArgs)
{
	std::ostringstream oss;

	for (int i = 0; i < m_Offset; ++i)
	{
		if (0 < i)
			oss << " ";
		oss << m_CommandArgs->ArgV(i);
	}

	m_Prefix = oss.str();
}

int CSubCommandArgs::ArgC()
{
	return m_CommandArgs->ArgC() - m_Offset + 1;
}

char const* CSubCommandArgs::ArgV(int i)
{
	if (0 == i)
	{
		return m_Prefix.c_str();
	}

	return m_CommandArgs->ArgV(i + m_Offset - 1);
}


} // namespace advancedfx {