#include "stdafx.h"

#include <windows.h>

#include <hlsdk.h>

#include "hl_addresses.h"

#include "cmd_tools.h"

void *getCommandTreeBasePtr(void* pfnAddCommand)
{
	return (void*)AFXADDR_GET(p_cmd_functions);
}

CHlaeCmdTools::CHlaeCmdTools()
{
	_pMyEngfuncs = NULL;
	_pCommandTree = NULL;
	_pHookTree = NULL;
}

CHlaeCmdTools::~CHlaeCmdTools()
{
	// clean the Hooked cmd Tracker tree:
	cmd_function_t *tcmd;
	for (cmd_function_t *hcmd=_pHookTree;hcmd;hcmd=tcmd)
	{
		tcmd=hcmd->next;
		delete hcmd;
	}
}

cmd_function_t *CHlaeCmdTools::Init(void *pMyEngfuncs)
{
	_pMyEngfuncs = pMyEngfuncs;
	_pCommandTree = (cmd_function_t *)*(cmd_function_t **)(getCommandTreeBasePtr(((cl_enginefuncs_s *)_pMyEngfuncs)->pfnAddCommand));
	return _pCommandTree;
}

cmd_function_t *CHlaeCmdTools::GiveCommandTreePtr()
{
	return _pCommandTree;
}

xcommand_t CHlaeCmdTools::GiveCommandFn(char *pszCmdName)
{
	cmd_function_t *cmd;
	if (cmd=_FindCmdTreeEntry(pszCmdName,_pCommandTree))
		return cmd->function;
	return NULL;
}

xcommand_t CHlaeCmdTools::GiveOriginalCommandFn(char *pszCmdName)
{
	cmd_function_t *cmd;
	if (cmd=_FindCmdTreeEntry(pszCmdName,_pHookTree))
		return cmd->function;
	return GiveCommandFn(pszCmdName);
}

xcommand_t CHlaeCmdTools::HookCommand(char *pszCmdName,xcommand_t pfnMyHook)
{
	cmd_function_t *hcmd;
	cmd_function_t *cmd;

	// make sure the command exists:
	if (!(cmd=_FindCmdTreeEntry(pszCmdName,_pCommandTree)))
		return NULL;

	// make sure it is not already hooked:
	if ((hcmd=_FindCmdTreeEntry(pszCmdName,_pHookTree)))
		return NULL;

	// add to top:
	cmd_function_t *newcmd = new cmd_function_t;
	newcmd->name = pszCmdName; // store cmd name
	newcmd->function = cmd->function; // store old cmd function
	newcmd->next = _pHookTree;
	_pHookTree = newcmd;

	// fix old cmd to point on the hook:
	cmd->function = pfnMyHook;

	return newcmd->function;
}

bool CHlaeCmdTools::UnHookCommand(char *pszCmdName)
{
	cmd_function_t *hcmdprev;
	cmd_function_t *hcmd;
	cmd_function_t *cmd;

	hcmdprev = _pHookTree;
	hcmd = NULL;

	// make sure we have hooked the command and find it's preceder (if existent):
	for (hcmd=hcmdprev ; hcmd ; hcmd = hcmd->next)
	{
		if (!strcmp(pszCmdName, hcmd->name))
			break;

		hcmdprev = hcmd;
	}

	if(!hcmd) return false; // command not found

	// make the cmd tree point on the original function again:
	if ((cmd=_FindCmdTreeEntry(pszCmdName,_pCommandTree)))
	{
		cmd->function = hcmd->function;
	}

	// remove from hook tree:
	
	if (hcmdprev)
		// update preceder:
		hcmdprev->next = hcmd->next;
	else
		// update the root pointer (tree is empty now):
		_pHookTree = NULL;

	delete hcmd;

	return true;
}

cmd_function_t * CHlaeCmdTools::_FindCmdTreeEntry(char *pszCmdName,cmd_function_t *pTree)
{
	for (cmd_function_t *aentry=pTree ; aentry ; aentry=aentry->next)
	{
		if (!strcmp(pszCmdName, aentry->name))
			return aentry;
	}
	return NULL; // not found
}

//
// global singelton:
//

CHlaeCmdTools g_CmdTools;