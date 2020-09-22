#pragma once

// Description:
// Wrapper(s) for Source engine ConCommands and ConVars.

#include <SourceInterfaces.h>
#include <csgo/sdk_src/public/tier1/convar.h>
#include <swarm/sdk_src/public/tier1/convar.h>
#include <l4d2/sdk_src/public/tier1/convar.h>
#include <bm/sdk_src/public/tier1/convar.h>

typedef advancedfx::CSubCommandArgs CSubWrpCommandArgs;
typedef advancedfx::CFakeCommandArgs CFakeWrpCommandArgs;

// WrpConCommand ///////////////////////////////////////////////////////////////

class WrpConCommand
	: public SOURCESDK::CSGO::ICommandCallback
	, public SOURCESDK::SWARM::ICommandCallback
	, public SOURCESDK::L4D2::ICommandCallback
	, public SOURCESDK::BM::ICommandCallback
{
public:
	WrpConCommand(char const * name, WrpCommandCallback callback, char const * helpString = 0);
	virtual ~WrpConCommand();

	WrpCommandCallback GetCallback();
	char const * GetHelpString();
	char const * GetName();

	virtual void SOURCESDK::CSGO::ICommandCallback::CommandCallback(const SOURCESDK::CSGO::CCommand &command)
	{
		ArgsFromCCommand_CSGO args(command);

		m_Callback(&args);
	}

	virtual void SOURCESDK::SWARM::ICommandCallback::CommandCallback(const SOURCESDK::SWARM::CCommand &command)
	{
		ArgsFromCCommand_SWARM args(command);

		m_Callback(&args);
	}

	virtual void SOURCESDK::L4D2::ICommandCallback::CommandCallback(const SOURCESDK::L4D2::CCommand &command)
	{
		ArgsFromCCommand_L4D2 args(command);

		m_Callback(&args);
	}

	virtual void SOURCESDK::BM::ICommandCallback::CommandCallback(const SOURCESDK::BM::CCommand &command)
	{
		ArgsFromCCommand_BM args(command);

		m_Callback(&args);
	}

private:
	class ArgsFromCCommand_CSGO :
		public IWrpCommandArgs
	{
	public:
		ArgsFromCCommand_CSGO(const SOURCESDK::CSGO::CCommand &command)
			: m_Command(command)
		{

		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual int ArgC()
		{
			return m_Command.ArgC();
		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual char const * ArgV(int i)
		{
			return (m_Command.ArgV())[i];
		}
	private:
		const SOURCESDK::CSGO::CCommand & m_Command;
	};

	class ArgsFromCCommand_SWARM :
		public IWrpCommandArgs
	{
	public:
		ArgsFromCCommand_SWARM(const SOURCESDK::SWARM::CCommand &command)
			: m_Command(command)
		{

		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual int ArgC()
		{
			return m_Command.ArgC();
		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual char const * ArgV(int i)
		{
			return (m_Command.ArgV())[i];
		}
	private:
		const SOURCESDK::SWARM::CCommand & m_Command;
	};

	class ArgsFromCCommand_L4D2 :
		public IWrpCommandArgs
	{
	public:
		ArgsFromCCommand_L4D2(const SOURCESDK::L4D2::CCommand &command)
			: m_Command(command)
		{

		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual int ArgC()
		{
			return m_Command.ArgC();
		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual char const * ArgV(int i)
		{
			return (m_Command.ArgV())[i];
		}
	private:
		const SOURCESDK::L4D2::CCommand & m_Command;
	};

	class ArgsFromCCommand_BM :
		public IWrpCommandArgs
	{
	public:
		ArgsFromCCommand_BM(const SOURCESDK::BM::CCommand &command)
			: m_Command(command)
		{

		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual int ArgC()
		{
			return m_Command.ArgC();
		}

		/// <comments> implements IWrpCommandArgs </comments>
		virtual char const * ArgV(int i)
		{
			return (m_Command.ArgV())[i];
		}
	private:
		const SOURCESDK::BM::CCommand & m_Command;
	};

	WrpCommandCallback m_Callback;
	char * m_HelpString;
	char * m_Name;
};


// WrpConCommandsRegistrar_003 /////////////////////////////////////////////////

class WrpConCommandsRegistrar_003 :
	public SOURCESDK::IConCommandBaseAccessor_003
{
public:
	virtual bool RegisterConCommandBase(SOURCESDK::ConCommandBase_003 *pVar);
};

// WrpConCommandsRegistrar_004 /////////////////////////////////////////////////

class WrpConCommandsRegistrar_004 :
	public SOURCESDK::IConCommandBaseAccessor_004
{
public:
	virtual bool RegisterConCommandBase(SOURCESDK::ConCommandBase_004 *pVar);
};

// WrpConCommandsRegistrar_CSGO /////////////////////////////////////////////////

class WrpConCommandsRegistrar_CSGO :
	public SOURCESDK::CSGO::IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(SOURCESDK::CSGO::ConCommandBase *pVar);
};

// WrpConCommandsRegistrar_SWARM /////////////////////////////////////////////////

class WrpConCommandsRegistrar_SWARM :
	public SOURCESDK::SWARM::IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(SOURCESDK::SWARM::ConCommandBase *pVar);
};

// WrpConCommandsRegistrar_L4D2 /////////////////////////////////////////////////

class WrpConCommandsRegistrar_L4D2 :
	public SOURCESDK::L4D2::IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(SOURCESDK::L4D2::ConCommandBase *pVar);
};

// WrpConCommandsRegistrar_BM /////////////////////////////////////////////////

class WrpConCommandsRegistrar_BM :
	public SOURCESDK::BM::IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(SOURCESDK::BM::ConCommandBase *pVar);
};

// WrpConCommands //////////////////////////////////////////////////////////////

struct WrpConCommandsListEntry {
	WrpConCommand * Command;
	WrpConCommandsListEntry * Next;
};

class WrpConCommands
{
public:
	/// <remarks> only valid when Registered with ICvar_003 </remarks>
	static SOURCESDK::IVEngineClient_012 * GetVEngineClient_012();
	
	/// <remarks> only valid when Registered with SOURCESDK::CSGO::ICvar </remarks>
	static SOURCESDK::CSGO::ICvar * GetVEngineCvar_CSGO();

	static void RegisterCommands(SOURCESDK::ICvar_003 * cvarIface, SOURCESDK::IVEngineClient_012 * vEngineClientInterface);
	static void RegisterCommands(SOURCESDK::ICvar_004 * cvarIface);
	static void RegisterCommands(SOURCESDK::CSGO::ICvar * cvarIface);
	static void RegisterCommands(SOURCESDK::SWARM::ICvar * cvarIface);
	static void RegisterCommands(SOURCESDK::L4D2::ICvar * cvarIface);
	static void RegisterCommands(SOURCESDK::BM::ICvar * cvarIface);

	static void WrpConCommand_Register(WrpConCommand * cmd);
	static void WrpConCommand_Unregister(WrpConCommand * cmd);

	static bool WrpConCommandsRegistrar_003_Register(SOURCESDK::ConCommandBase_003 *pVar);
	static bool WrpConCommandsRegistrar_004_Register(SOURCESDK::ConCommandBase_004 *pVar);
	static bool WrpConCommandsRegistrar_CSGO_Register(SOURCESDK::CSGO::ConCommandBase *pVar);
	static bool WrpConCommandsRegistrar_SWARM_Register(SOURCESDK::SWARM::ConCommandBase *pVar);
	static bool WrpConCommandsRegistrar_L4D2_Register(SOURCESDK::L4D2::ConCommandBase *pVar);
	static bool WrpConCommandsRegistrar_BM_Register(SOURCESDK::BM::ConCommandBase *pVar);

private:
	static SOURCESDK::ICvar_003 * m_CvarIface_003;
	static SOURCESDK::ICvar_004 * m_CvarIface_004;
	static SOURCESDK::CSGO::ICvar * m_CvarIface_CSGO;
	static SOURCESDK::SWARM::ICvar * m_CvarIface_SWARM;
	static SOURCESDK::L4D2::ICvar * m_CvarIface_L4D2;
	static SOURCESDK::BM::ICvar * m_CvarIface_BM;
	static WrpConCommandsListEntry * m_CommandListRoot;
	static SOURCESDK::IVEngineClient_012 * m_VEngineClient_012;
};


#define CON_COMMAND( name, description ) \
   static void name(IWrpCommandArgs * args); \
   static WrpConCommand name##_command( #name, name, description ); \
   static void name(IWrpCommandArgs * args)

#define CON_COMMAND_F( name, description, flags ) \
   static void name(IWrpCommandArgs * args); \
   static WrpConCommand name##_command( #name, name, description, flags ); \
   static void name(IWrpCommandArgs * args)

////////////////////////////////////////////////////////////////////////////////

/// <remaks>Currently only supports CS:GO!</remarks>
class WrpConVarRef
{
public:
	WrpConVarRef();

	WrpConVarRef(char const * pName);

	float GetFloat(void) const;
	int GetInt(void) const;

	void SetValue(float value);

	/// <remarks>This guarantees the int and float m_Value members to be set immediatelly. However Changecallback and string versions might still be triggered later.</remarks>
	void SetValueFastHack(float value);

	/// <summary>This sets the values directly, without calling callbacks, this might have unwanted side-effects!</summary>
	void SetDirectHack(float value);

	void RetryIfNull(char const* pName);

private:
	SOURCESDK::CSGO::ConVar * m_pConVar007;

};
