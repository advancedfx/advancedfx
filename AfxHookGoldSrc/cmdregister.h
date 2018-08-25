#pragma once

#define PREFIX "mirv_"
#define DEBUG_PREFIX "__mirv_"

class RegisterCvar
{
public:
	RegisterCvar(char * name, char * value, int flags, struct cvar_s * * outCvar);
};


class RegisterCmd
{
public:
	RegisterCmd(char * name, void (*function)(void));
};


#define _REGISTER_CVAR(var, cvar, def, flags) \
	struct cvar_s * var; \
	RegisterCvar register_cvar_ ## var ## _(cvar, def, flags, &var); 

#define REGISTER_CVAR(var, def, flags) _REGISTER_CVAR(var, PREFIX # var, def, flags)
#define REGISTER_DEBUGCVAR(var, def, flags) _REGISTER_CVAR(var, DEBUG_PREFIX # var, def, flags)


#define _REGISTER_CMD(cmd, func) \
	void func(); \
	RegisterCmd register_cmd_ ## func ## _(cmd, func);

#define REGISTER_CMD(cmd) _REGISTER_CMD(PREFIX # cmd, cmd ## _cmd)
#define REGISTER_CMD_BEGIN(cmd) _REGISTER_CMD("+" ## PREFIX # cmd, cmd ## _begincmd)
#define REGISTER_CMD_END(cmd) _REGISTER_CMD("-" ## PREFIX # cmd, cmd ## _endcmd)

#define REGISTER_CMD_FUNC(cmd) \
	_REGISTER_CMD(PREFIX # cmd, cmd ## _cmd) \
	void cmd ## _cmd()

#define REGISTER_CMD_FUNC_BEGIN(cmd) \
	_REGISTER_CMD("+" ## PREFIX # cmd, cmd ## _begincmd) \
	void cmd ## _begincmd()

#define REGISTER_CMD_FUNC_END(cmd) \
	_REGISTER_CMD("-" ## PREFIX # cmd, cmd ## _endcmd) \
	void cmd ## _endcmd()

#define CALL_CMD(cmd) cmd ## _cmd();
#define CALL_CMD_BEGIN(cmd) cmd ## _begincmd();
#define CALL_CMD_END(cmd) cmd ## _endcmd();

#define REGISTER_DEBUGCMD_FUNC(cmd) \
	_REGISTER_CMD(DEBUG_PREFIX # cmd, cmd ## _cmd) \
	void cmd ## _cmd()
