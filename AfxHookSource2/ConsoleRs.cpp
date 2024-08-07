#include "stdafx.h"

#include "../shared/AfxConsole.h"

#include "../deps/release/prop/cs2/sdk_src/public/tier1/convar.h"
#include "../deps/release/prop/cs2/sdk_src/public/icvar.h"

class CCommandCallbackArgsRs :
    public advancedfx::ICommandArgs
{
public:
    CCommandCallbackArgsRs(const SOURCESDK::CS2::CCommand & command)
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
        return m_Command.Arg(i);
    }

private:
    const SOURCESDK::CS2::CCommand & m_Command;
};

typedef void (*CommandCallbackRs)(void * p_user_data, void * p_command_args);

class CConCommandRs
: public SOURCESDK::CS2::ICommandCallback {
public:
    CConCommandRs(const char * psz_name, const char * psz_help_string, int64_t flags, int64_t additional_flags, CommandCallbackRs p_callback, void * p_user_data)
        : m_Callback(p_callback)
        , m_p_user_data(p_user_data)
    {
        if(SOURCESDK::CS2::g_pCVar) {
            m_Cmd = new SOURCESDK::CS2::CCmd(psz_name, psz_help_string, flags, this);
            m_Handle = SOURCESDK::CS2::g_pCVar->RegisterConCommand(m_Cmd, additional_flags).GetIndex();
        }
    }

    ~CConCommandRs() {
        if(SOURCESDK::CS2::g_pCVar) {
            SOURCESDK::CS2::g_pCVar->UnregisterConCommand(m_Handle);
            delete m_Cmd;
        }
    }

	virtual void CommandCallback(void * _unknown1_rdx_ptr, SOURCESDK::CS2::CCommand * pArgs) override {
		CCommandCallbackArgsRs args(*pArgs);
		m_Callback(m_p_user_data, &args);
	}

private:
    SOURCESDK::CS2::CCmd * m_Cmd = nullptr;
    CommandCallbackRs m_Callback;
    void * m_p_user_data;
    size_t m_Handle = (unsigned short)-1;
};

extern "C" CConCommandRs * afx_hook_source2_new_command(const char * psz_name, const char * psz_help_string, int64_t flags, int64_t additional_flags, CommandCallbackRs p_callback, void * p_user_data) {
    return new CConCommandRs(psz_name, psz_help_string, flags, additional_flags, p_callback, p_user_data);
}

extern "C" void afx_hook_source2_delete_command(CConCommandRs * p_con_command) {
    delete p_con_command;
}

extern "C" int afx_hook_source2_command_args_argc(CCommandCallbackArgsRs * p_command_args) {
    return p_command_args->ArgC();
}

extern "C" const char * afx_hook_source2_command_args_argv(CCommandCallbackArgsRs * p_command_args, int index) {
    return p_command_args->ArgV(index);
}
