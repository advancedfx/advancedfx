#include "stdafx.h"

#include "MirvCommandArgs.h"

#include "hlsdk.h"

CMirvCommandArgs::CMirvCommandArgs(struct cl_enginefuncs_s* pEngfuncs)
	: pEngfuncs(pEngfuncs)
{

}

int CMirvCommandArgs::ArgC() {
	return pEngfuncs->Cmd_Argc();
}

char const* CMirvCommandArgs::ArgV(int i) {
	return pEngfuncs->Cmd_Argv(i);
}
