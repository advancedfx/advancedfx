#include "stdafx.h"

#include "WrpConsole.h"

#include <malloc.h>
#include <string.h>
#include <sstream>
#include <iomanip>

#include <windows.h>

// WrpConCommand ///////////////////////////////////////////////////////////////

WrpConCommand::WrpConCommand(char const * name, WrpCommandCallback callback, char const * helpString) {
	m_Callback = callback;

	if(!helpString) helpString = "";
	m_HelpString =(char *) malloc((1+strlen(helpString))*sizeof(char));
	strcpy(m_HelpString, helpString);
	
	m_Name = (char *)malloc((1+strlen(name))*sizeof(char));
	strcpy(m_Name, name);

	WrpConCommands::WrpConCommand_Register(this);
}

WrpConCommand::~WrpConCommand() {
	WrpConCommands::WrpConCommand_Unregister(this);

	delete m_Name;

	delete m_HelpString;
}

WrpCommandCallback WrpConCommand::GetCallback() {
	return m_Callback;
}
char const * WrpConCommand::GetHelpString() {
	return m_HelpString;
}

char const * WrpConCommand::GetName() {
	return m_Name;
}


// WrpConCommands //////////////////////////////////////////////////////////////

SOURCESDK::ICvar_003 * WrpConCommands::m_CvarIface_003 = 0;
SOURCESDK::ICvar_004 * WrpConCommands::m_CvarIface_004 = 0;
SOURCESDK::CSGO::ICvar * WrpConCommands::m_CvarIface_CSGO = 0;
SOURCESDK::SWARM::ICvar * WrpConCommands::m_CvarIface_SWARM = 0;
SOURCESDK::L4D2::ICvar * WrpConCommands::m_CvarIface_L4D2 = 0;
SOURCESDK::BM::ICvar * WrpConCommands::m_CvarIface_BM = 0;
WrpConCommandsListEntry * WrpConCommands::m_CommandListRoot = 0;
SOURCESDK::IVEngineClient_012 * WrpConCommands::m_VEngineClient_012 = 0;

SOURCESDK::IVEngineClient_012 * WrpConCommands::GetVEngineClient_012() {
	return m_VEngineClient_012;
}

SOURCESDK::CSGO::ICvar * WrpConCommands::GetVEngineCvar_CSGO()
{
	return m_CvarIface_CSGO;
}

void WrpConCommands::RegisterCommands(SOURCESDK::ICvar_003 * cvarIface, SOURCESDK::IVEngineClient_012 * vEngineClientInterface) {
	if(m_CvarIface_003)
		// already registered the current list
		return;

	m_CvarIface_003 = cvarIface;
	m_VEngineClient_012 = vEngineClientInterface;
	SOURCESDK::ConCommandBase_003::s_pAccessor = new WrpConCommandsRegistrar_003();

	for(WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		WrpConCommand * cmd = entry->Command;

		// will init themself since s_pAccessor is set:
		new SOURCESDK::ConCommand_003(cmd->GetName(), cmd->GetCallback(), cmd->GetHelpString(), FCVAR_CLIENTDLL);
	}
}

void WrpConCommands::RegisterCommands(SOURCESDK::ICvar_004 * cvarIface) {
	if(m_CvarIface_004)
		// already registered the current list
		return;

	m_CvarIface_004 = cvarIface;
	SOURCESDK::ConCommandBase_004::s_pAccessor = new WrpConCommandsRegistrar_004();

	for(WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		WrpConCommand * cmd = entry->Command;

		// will init themself since s_pAccessor is set:
		new SOURCESDK::ConCommand_004(cmd->GetName(), cmd->GetCallback(), cmd->GetHelpString(), FCVAR_CLIENTDLL);
	}
}

void WrpConCommands::RegisterCommands(SOURCESDK::CSGO::ICvar * cvarIface) {
	if(m_CvarIface_CSGO)
		// already registered the current list
		return;

	m_CvarIface_CSGO = cvarIface;
	SOURCESDK::CSGO::ConVar_Register(0, new WrpConCommandsRegistrar_CSGO());

	for(WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		WrpConCommand * cmd = entry->Command;

		// will init themself since s_pAccessor is set:
		new SOURCESDK::CSGO::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_CSGO_FCVAR_CLIENTDLL);
	}
}

void WrpConCommands::RegisterCommands(SOURCESDK::SWARM::ICvar * cvarIface) {
	if (m_CvarIface_SWARM)
		// already registered the current list
		return;

	m_CvarIface_SWARM = cvarIface;
	SOURCESDK::SWARM::ConVar_Register(0, new WrpConCommandsRegistrar_SWARM());

	for (WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		WrpConCommand * cmd = entry->Command;

		// will init themself since s_pAccessor is set:
		new SOURCESDK::SWARM::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_SWARM_FCVAR_CLIENTDLL);
	}
}

void WrpConCommands::RegisterCommands(SOURCESDK::L4D2::ICvar * cvarIface) {
	if (m_CvarIface_L4D2)
		// already registered the current list
		return;

	m_CvarIface_L4D2 = cvarIface;
	SOURCESDK::L4D2::ConVar_Register(0, new WrpConCommandsRegistrar_L4D2());

	for (WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		WrpConCommand * cmd = entry->Command;

		// will init themself since s_pAccessor is set:
		new SOURCESDK::L4D2::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_L4D2_FCVAR_CLIENTDLL);
	}
}

void WrpConCommands::RegisterCommands(SOURCESDK::BM::ICvar * cvarIface) {
	if (m_CvarIface_BM)
		// already registered the current list
		return;

	m_CvarIface_BM = cvarIface;
	SOURCESDK::BM::ConVar_Register(0, new WrpConCommandsRegistrar_BM());

	for (WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		WrpConCommand * cmd = entry->Command;

		// will init themself since s_pAccessor is set:
		new SOURCESDK::BM::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_BM_FCVAR_CLIENTDLL);
	}
}

void WrpConCommands::WrpConCommand_Register(WrpConCommand * cmd) {
	WrpConCommandsListEntry * entry = new WrpConCommandsListEntry();
	entry->Command = cmd;
	entry->Next = m_CommandListRoot;
	m_CommandListRoot = entry;

	// if the list is already live, create (and thus register) the command instantly
	// in the engine:

	if(m_CvarIface_CSGO)
		new SOURCESDK::CSGO::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_CSGO_FCVAR_CLIENTDLL);
	else if (m_CvarIface_SWARM)
		new SOURCESDK::SWARM::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_SWARM_FCVAR_CLIENTDLL);
	else if (m_CvarIface_L4D2)
		new SOURCESDK::L4D2::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_L4D2_FCVAR_CLIENTDLL);
	else if (m_CvarIface_BM)
		new SOURCESDK::BM::ConCommand(cmd->GetName(), cmd, cmd->GetHelpString(), SOURCESDK_BM_FCVAR_CLIENTDLL);
	else if(m_CvarIface_004)
		new SOURCESDK::ConCommand_004(cmd->GetName(), cmd->GetCallback(), cmd->GetHelpString());
	else if(m_CvarIface_003)
		new SOURCESDK::ConCommand_003(cmd->GetName(), cmd->GetCallback(), cmd->GetHelpString());
}

void WrpConCommands::WrpConCommand_Unregister(WrpConCommand * cmd) {
	WrpConCommandsListEntry ** pLastNext = &m_CommandListRoot;
	for(WrpConCommandsListEntry * entry = m_CommandListRoot; entry; entry = entry->Next) {
		if(cmd == entry->Command) {
			*pLastNext = entry->Next;
			delete entry;
			break;
		}
		pLastNext = &(entry->Next);
	}
}

bool WrpConCommands::WrpConCommandsRegistrar_003_Register(SOURCESDK::ConCommandBase_003 *pVar ) {
	if(!m_CvarIface_003)
		return false;

//	MessageBox(0, "WrpConCommands::WrpConCommandsRegistrar_003_Register", "AFX_DEBUG", MB_OK);

	m_CvarIface_003->RegisterConCommandBase(pVar);
	return true;
}


bool WrpConCommands::WrpConCommandsRegistrar_004_Register(SOURCESDK::ConCommandBase_004 *pVar ) {
	if(!m_CvarIface_004)
		return false;

//	MessageBox(0, "WrpConCommands::WrpConCommandsRegistrar_004_Register", "AFX_DEBUG", MB_OK);

	m_CvarIface_004->RegisterConCommand(pVar);
	return true;
}

bool WrpConCommands::WrpConCommandsRegistrar_CSGO_Register(SOURCESDK::CSGO::ConCommandBase *pVar ) {
	if(!m_CvarIface_CSGO)
		return false;

//	MessageBox(0, "WrpConCommands::WrpConCommandsRegistrar_007_Register", "AFX_DEBUG", MB_OK);

	m_CvarIface_CSGO->RegisterConCommand(pVar);
	return true;
}

bool WrpConCommands::WrpConCommandsRegistrar_SWARM_Register(SOURCESDK::SWARM::ConCommandBase *pVar) {
	if (!m_CvarIface_SWARM)
		return false;

	//	MessageBox(0, "WrpConCommands::WrpConCommandsRegistrar_007_Register", "AFX_DEBUG", MB_OK);

	m_CvarIface_SWARM->RegisterConCommand(pVar);
	return true;
}

bool WrpConCommands::WrpConCommandsRegistrar_L4D2_Register(SOURCESDK::L4D2::ConCommandBase *pVar) {
	if (!m_CvarIface_L4D2)
		return false;

	//	MessageBox(0, "WrpConCommands::WrpConCommandsRegistrar_007_Register", "AFX_DEBUG", MB_OK);

	m_CvarIface_L4D2->RegisterConCommand(pVar);
	return true;
}

bool WrpConCommands::WrpConCommandsRegistrar_BM_Register(SOURCESDK::BM::ConCommandBase *pVar) {
	if (!m_CvarIface_BM)
		return false;

	//	MessageBox(0, "WrpConCommands::WrpConCommandsRegistrar_007_Register", "AFX_DEBUG", MB_OK);

	m_CvarIface_BM->RegisterConCommand(pVar);
	return true;
}

// WrpConCommandsRegistrar_003 ////////////////////////////////////////////////////

bool WrpConCommandsRegistrar_003::RegisterConCommandBase(SOURCESDK::ConCommandBase_003 *pVar ) {
	return WrpConCommands::WrpConCommandsRegistrar_003_Register(pVar);
}

// WrpConCommandsRegistrar_004 ////////////////////////////////////////////////////

bool WrpConCommandsRegistrar_004::RegisterConCommandBase(SOURCESDK::ConCommandBase_004 *pVar ) {
	return WrpConCommands::WrpConCommandsRegistrar_004_Register(pVar);
}

// WrpConCommandsRegistrar_CSGO ////////////////////////////////////////////////////

bool WrpConCommandsRegistrar_CSGO::RegisterConCommandBase(SOURCESDK::CSGO::ConCommandBase *pVar ) {
	return WrpConCommands::WrpConCommandsRegistrar_CSGO_Register(pVar);
}

// WrpConCommandsRegistrar_SWARM ////////////////////////////////////////////////////

bool WrpConCommandsRegistrar_SWARM::RegisterConCommandBase(SOURCESDK::SWARM::ConCommandBase *pVar) {
	return WrpConCommands::WrpConCommandsRegistrar_SWARM_Register(pVar);
}

// WrpConCommandsRegistrar_L4D2 ////////////////////////////////////////////////////

bool WrpConCommandsRegistrar_L4D2::RegisterConCommandBase(SOURCESDK::L4D2::ConCommandBase *pVar) {
	return WrpConCommands::WrpConCommandsRegistrar_L4D2_Register(pVar);
}

// WrpConCommandsRegistrar_BM ////////////////////////////////////////////////////

bool WrpConCommandsRegistrar_BM::RegisterConCommandBase(SOURCESDK::BM::ConCommandBase *pVar) {
	return WrpConCommands::WrpConCommandsRegistrar_BM_Register(pVar);
}


// WrpConVar ///////////////////////////////////////////////////////////////////

WrpConVarRef::WrpConVarRef(char const * pName)
: m_pConVar007(0)
{
	if(SOURCESDK::CSGO::g_pCVar)
	{
		m_pConVar007 = SOURCESDK::CSGO::g_pCVar->FindVar(pName);
	}

	if(!m_pConVar007)
	{
		Tier0_Warning("AfxError: WrpConVarRef::WrpConVarRef(%s): Could not get ConVar.\n", pName);
	}
}

typedef union {
	DWORD d;
	float f;
	int i;
} xor_helper_t;

float WrpConVarRef::GetFloat(void) const
{
	if(m_pConVar007)
	{
		//return m_pConVar007->GetFloat();

		xor_helper_t h;
		h.f = m_pConVar007->m_Value.m_fValue;
		h.d ^= (DWORD)m_pConVar007;
		return h.f;
	}

	return 0;
}

int WrpConVarRef::GetInt(void) const
{
	if (m_pConVar007)
	{
		//return m_pConVar007->GetInt();

		xor_helper_t h;
		h.i = m_pConVar007->m_Value.m_nValue;
		h.d ^= (DWORD)m_pConVar007;
		return h.i;
	}

	return 0;
}

void WrpConVarRef::SetValue(float value)
{
	if(m_pConVar007)
		m_pConVar007->SetValue(value);
}

void WrpConVarRef::SetValueFastHack(float value)
{
	SetDirectHack(value);

	SetValue(value);
}

void WrpConVarRef::SetDirectHack(float value)
{
	if(m_pConVar007)
	{
		xor_helper_t h;

		h.f = value;
		h.d ^= (DWORD)m_pConVar007;
		m_pConVar007->m_Value.m_fValue = h.f;

		h.i = (int)value;
		h.d ^= (DWORD)m_pConVar007;
		m_pConVar007->m_Value.m_nValue = h.i;
	}
}

// CSubWrpCommandArgs //////////////////////////////////////////////////////////

CSubWrpCommandArgs::CSubWrpCommandArgs(IWrpCommandArgs * commandArgs, int offset)
: m_Offset(offset)
, m_CommandArgs(commandArgs)
{
	std::ostringstream oss;

	for (int i = 0; i<m_Offset; ++i)
	{
		if (0 < i)
			oss << " ";
		oss << m_CommandArgs->ArgV(i);
	}

	m_Prefix = oss.str();
}

int CSubWrpCommandArgs::ArgC()
{
	return m_CommandArgs->ArgC() -m_Offset +1;
}

char const * CSubWrpCommandArgs::ArgV(int i)
{
	if(0 == i)
	{
		return m_Prefix.c_str();
	}

	return m_CommandArgs->ArgV(i +m_Offset -1);
}
