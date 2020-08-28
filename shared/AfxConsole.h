#pragma once

#include <string>
#include <vector>

namespace advancedfx {

typedef void (*Con_Printf_t)(const char* fmt, ...);
typedef void (*Con_DevPrintf_t)(int level, const char* fmt, ...);

extern Con_Printf_t Message;
extern Con_Printf_t Warning;
extern Con_DevPrintf_t DevMessage;
extern Con_DevPrintf_t DevWarning;

// ICommandArgs ////////////////////////////////////////////////////////////////

class ICommandArgs abstract
{
public:
	/// <summary> returns the count of passed arguments </summary>
	virtual int ArgC() abstract = 0;

	/// <summary> returns the i-th argument, where 0 is being the first one </summary>
	virtual char const * ArgV(int i) abstract = 0;
};

typedef void (*CommandCallback_t)(ICommandArgs* args);

// CSubCommandArgs /////////////////////////////////////////////////////////////

class CSubCommandArgs
: public ICommandArgs
{
public:
	CSubCommandArgs(ICommandArgs* commandArgs, int offset);

	/// <summary> returns the count of passed arguments </summary>
	virtual int ArgC();

	/// <summary> returns the i-th argument, where 0 is being the first one </summary>
	virtual char const * ArgV(int i);

private:
	int m_Offset;
	ICommandArgs*m_CommandArgs;
	std::string m_Prefix;
};

// CFakeCommandArgs ////////////////////////////////////////////////////////////

class CFakeCommandArgs
	: public ICommandArgs
{
public:
	CFakeCommandArgs(const char * command)
	{
		AddArg(command);
	}

	void AddArg(const char * arg)
	{
		m_Args.push_back(arg);
	}

	/// <summary> returns the count of passed arguments </summary>
	virtual int ArgC() {
		return m_Args.size();
	}

	/// <summary> returns the i-th argument, where 0 is being the first one </summary>
	virtual char const * ArgV(int i) {
		if (i < 0 || i >= (int)m_Args.size())
			return "";

		return m_Args[i].c_str();
	}

private:
	std::vector<std::string> m_Args;
};

} // namespace advancedfx {
