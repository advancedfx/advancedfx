#pragma once

// from Quake 1 cmd.h:
typedef void (*xcommand_t) (void);

// cmd_function_t from Quake 1 cmd.c:
typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
} cmd_function_t;

class CHlaeCmdTools
// singelton class, however no checks performed atm.
{
public:
	CHlaeCmdTools();
	~CHlaeCmdTools();

	cmd_function_t *Init(void *pMyEngfuncs); // this shall be called first, before other functions are used, same return like GiveCommandTreePtr
	cmd_function_t *GiveCommandTreePtr(); // returns the pointer to the Command Tree or NULL on fail

	xcommand_t GiveCommandFn(char *pszCmdName); // returns pointer to the command with name equal to (strcmp) pszCmdName if found, if not found or on error NULL
	xcommand_t GiveOriginalCommandFn(char *pszCmdName); // similar to GiveCommandFn, except that it returns the pointer to the original func
	
	xcommand_t HookCommand(char *pszCmdName,xcommand_t pfnMyHook); // returns NULL on error or if the command has been already hooked, otherwise returns original cmd func
	bool UnHookCommand(char *pszCmdName); // returns false on error (or if the command was not hooked)

private:
	void *_pMyEngfuncs;
	cmd_function_t *_pCommandTree;
	cmd_function_t *_pHookTree;

	cmd_function_t * _FindCmdTreeEntry(char *pszCmdName,cmd_function_t *pTree);
};

// global singelton:
extern CHlaeCmdTools g_CmdTools;
