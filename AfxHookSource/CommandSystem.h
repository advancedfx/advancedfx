#pragma once

#include <string>
#include <map>

class CommandSystem
{
public:
	bool Enabled;

	CommandSystem();

	void Add(double time, char const * command);
	bool Remove(int index);

	void Clear(void);

	bool Save(wchar_t const * fileName);
	bool Load(wchar_t const * fileName);

	void Console_List(void);

	void Do_Queue_Commands(double time);

private:
	std::map<double,std::string> m_Map;

	double m_LastTime;
};

extern CommandSystem g_CommandSystem;
