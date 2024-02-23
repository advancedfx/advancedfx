#pragma once

#include "../shared/AfxConsole.h"

#include "../deps/release/prop/cs2/sdk_src/public/tier1/convar.h"
#include "../deps/release/prop/cs2/sdk_src/public/icvar.h"

class CWrpConCommandCallback
	: public SOURCESDK::CS2::ICommandCallback {
public:
    CWrpConCommandCallback(advancedfx::CommandCallback_t callback)
        : m_Callback(callback)
    {

    }

	virtual void CommandCallback(void * _unknown1_rdx_ptr, SOURCESDK::CS2::CCommand * pArgs) override {			
		ArgsFromCCommand args(*pArgs);
		m_Callback(&args);
	}

private:
	class ArgsFromCCommand :
		public advancedfx::ICommandArgs
	{
	public:
		ArgsFromCCommand(const SOURCESDK::CS2::CCommand &command)
			: m_Command(command)
		{

		}

		/// <comments> implements advancedfx::ICommandArgs </comments>
		virtual int ArgC()
		{
			return m_Command.ArgC();
		}

		/// <comments> implements advancedfx::ICommandArgs </comments>
		virtual char const * ArgV(int i)
		{
			return m_Command.ArgV((size_t)i);
		}
	private:
		const SOURCESDK::CS2::CCommand & m_Command;
	};

    advancedfx::CommandCallback_t m_Callback;
};

class CWrpAddCommand {
public:
	CWrpAddCommand(class SOURCESDK::CS2::CCmd * command);
};

void WrpRegisterCommands();

#define CON_COMMAND( name, description ) \
   static void name(advancedfx::ICommandArgs * args); \
   static CWrpConCommandCallback name##_callback( name ); \
   static SOURCESDK::CS2::CCmd name##_command(#name, description, 0, &name##_callback); \
   static CWrpAddCommand name##_command_add(&name##_command); \
   static void name(advancedfx::ICommandArgs * args)