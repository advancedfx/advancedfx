#include "stdafx.h"

// Mirv Demo Tool

#include "mirv_commands.h"

#include "cmdregister.h"
#include "hooks/HookHw.h"
#include <hlsdk.h>
#include <list>

struct AfxCmdCvarData
{
	char * Value;
	int Flags;
	struct cvar_s * * OutCvar;
};

struct AfxCmdCmdData
{
	void (*Function)(void);
};

enum AfxCmdEntryType
{
	ACET_NULL,
	ACET_Cmd,
	ACET_Cvar
};

struct AfxCmdEntry
{
	char * Name;
	AfxCmdEntryType Type;
	union {
		AfxCmdCmdData Cmd;
		AfxCmdCvarData Cvar;
	} Data;

	AfxCmdEntry(char * name, void (*function)(void))
	{
		this->Name = name;
		this->Type = ACET_Cmd;
		this->Data.Cmd.Function = function;
	}

	AfxCmdEntry(char * name, char * value, int flags, struct cvar_s * * outCvar)
	{
		this->Name = name;
		this->Type = ACET_Cvar;
		this->Data.Cvar.Value = value;
		this->Data.Cvar.Flags = flags;
		this->Data.Cvar.OutCvar = outCvar;
	}
};

std::list<AfxCmdEntry> & GetAfxCmdEntries() {
	static std::list<AfxCmdEntry> afxCmdEntries;
	return afxCmdEntries;
}

bool CompareAfxCmdEntry(AfxCmdEntry first, AfxCmdEntry second)
{
	return 0 <= _stricmp(first.Name, second.Name); // sort backwards
}

void AfxRegisterCommands()
{
	// Register the commands:
	{
		// sort the list so that Valve's console autocompletion won't be confused:
		// Acutally this will only work partially, Commands will still take
		// precedence over Cvars, despite our sorting effort.
		GetAfxCmdEntries().sort(CompareAfxCmdEntry);

		for(
			std::list<AfxCmdEntry>::iterator i = GetAfxCmdEntries().begin();
			i != GetAfxCmdEntries().end();
			i++
		)
		{
			switch(i->Type)
			{
			case ACET_Cmd:
				pEngfuncs->pfnAddCommand(i->Name, i->Data.Cmd.Function);
				break;
			case ACET_Cvar:
				if(0 != i->Data.Cvar.OutCvar)
					*(i->Data.Cvar.OutCvar) = pEngfuncs->pfnRegisterVariable(i->Name, i->Data.Cvar.Value, i->Data.Cvar.Flags);
				else
					pEngfuncs->pfnRegisterVariable(i->Name, i->Data.Cvar.Value, i->Data.Cvar.Flags);
				break;
			default:
				throw "Unsopported AfxCmdEntryType.";
			}
		}
	}
}


RegisterCvar::RegisterCvar(char * name, char * value, int flags, struct cvar_s * * outCvar)
{
	AfxCmdEntry entry(name, value, flags, outCvar);
	GetAfxCmdEntries().push_back(entry);
}

RegisterCmd::RegisterCmd(char * name, void (*function)(void))
{
	AfxCmdEntry entry(name, function);
	GetAfxCmdEntries().push_back(entry);
}


REGISTER_DEBUGCMD_FUNC(listcmds)
{
	int cntCmds = 0;
	int cntVars = 0;

	pEngfuncs->Con_Printf("---- cmds/cvars ----\n");

	// commands:
	{
		for(
			std::list<AfxCmdEntry>::iterator i = GetAfxCmdEntries().begin();
			i != GetAfxCmdEntries().end();
			i++
		) {
			switch(i->Type)
			{
			case ACET_Cmd:
				pEngfuncs->Con_Printf("%s\n", i->Name);
				cntCmds++;
				break;
			case ACET_Cvar:
				pEngfuncs->Con_Printf("%s, %s, 0x%08x\n", i->Name, i->Data.Cvar.Value, i->Data.Cvar.Flags);
				cntVars++;
				break;
			default:
				throw "Unsopported AfxCmdEntryType.";
			}
		}
	}

	pEngfuncs->Con_Printf("----------------\nCvars:%i, Cmds: %i, Total: %i\n", cntVars, cntCmds, cntVars +cntCmds);
}
