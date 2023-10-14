#pragma once

#include "../shared/AfxConsole.h"


class CMirvCommandArgs : public advancedfx::ICommandArgs
{
public:
	CMirvCommandArgs(struct cl_enginefuncs_s* pEngfuncs);

	virtual int ArgC();

	virtual char const* ArgV(int i);

private:
	struct cl_enginefuncs_s* pEngfuncs;
};
