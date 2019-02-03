#include "stdafx.h"

#include "CommandSystem.h"

#include "WrpVEngineClient.h"
#include "RenderView.h"
#include "MirvTime.h"

#include <shared/rapidxml/rapidxml.hpp>
#include <shared/rapidxml/rapidxml_print.hpp>


extern WrpVEngineClient * g_VEngineClient;

CommandSystem g_CommandSystem;

CommandSystem::CommandSystem()
: Enabled(true)
, m_LastTime(-1)
, m_LastTick(-1)
{

}

void CommandSystem::Add(char const * command)
{
	if (!IsSupportedByTime())
	{
		Tier0_Warning("Error: Missing hooks for supporting scheduling by time.\n");
		return;
	}

	double time = m_LastTime;

	std::map<double, std::string>::iterator it = m_Map.find(time);

	std::string cmds("");

	if(it != m_Map.end())
	{
		cmds.append(it->second);
		cmds.append("; ");
	}

	cmds.append(command);

	m_Map[time] = cmds;
}

void CommandSystem::AddTick(char const * command)
{
	if (!IsSupportedByTick())
	{
		Tier0_Warning("Error: Missing hooks for supporting scheduling by tick.\n");
		return;
	}

	int tick = m_LastTick;

	std::map<int, std::string>::iterator it = m_TickMap.find(tick);

	std::string cmds("");

	if (it != m_TickMap.end())
	{
		cmds.append(it->second);
		cmds.append("; ");
	}

	cmds.append(command);

	m_TickMap[tick] = cmds;
}

void CommandSystem::EditStart(double startTime)
{
	std::map<double, std::string> tmpMap;

	tmpMap.swap(m_Map);

	for (std::map<double, std::string>::iterator it = tmpMap.begin(); it != tmpMap.end(); ++it)
	{
		double time = it->first;

		if (it == tmpMap.begin())
		{
			startTime = startTime - time;
		}

		time += startTime;

		m_Map[time] = it->second;
	}
}

void CommandSystem::EditStartTick(int startTick)
{
	std::map<int, std::string> tmpMap;

	tmpMap.swap(m_TickMap);

	for (std::map<int, std::string>::iterator it = tmpMap.begin(); it != tmpMap.end(); ++it)
	{
		int tick = it->first;

		if (it == tmpMap.begin())
		{
			startTick = startTick - tick;
		}

		tick += startTick;

		m_TickMap[tick] = it->second;
	}
}

bool CommandSystem::Remove(int index)
{
	if (index < (int)m_TickMap.size())
	{
		int idx = 0;

		for (std::map<int, std::string>::iterator it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
		{
			if (idx == index)
			{
				m_TickMap.erase(it);
				return true;
			}

			++idx;
		}
	}
	else
	{
		index -= m_TickMap.size();

		int idx = 0;

		for (std::map<double, std::string>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
		{
			if (idx == index)
			{
				m_Map.erase(it);
				return true;
			}

			++idx;
		}
	}

	return false;
}

void CommandSystem::Clear(void)
{
	m_TickMap.clear();
	m_Map.clear();
}

namespace CommandSystemXML {

char * int2xml(rapidxml::xml_document<> & doc, int value)
{
	char szTmp[12];
	_snprintf_s(szTmp, _TRUNCATE, "%i", value);
	return doc.allocate_string(szTmp);
}

char * double2xml(rapidxml::xml_document<> & doc, double value)
{
	char szTmp[196];
	_snprintf_s(szTmp, _TRUNCATE,"%f", value);
	return doc.allocate_string(szTmp);
}

};

using namespace CommandSystemXML;

bool CommandSystem::Save(wchar_t const * fileName)
{
	rapidxml::xml_document<> doc;

	rapidxml::xml_node<> * decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	rapidxml::xml_node<> * commandSystem = doc.allocate_node(rapidxml::node_element, "commandSystem");
	doc.append_node(commandSystem);

	rapidxml::xml_node<> * cmds = doc.allocate_node(rapidxml::node_element, "commands");
	commandSystem->append_node(cmds);

	for (std::map<int, std::string>::iterator it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
	{
		int tick = it->first;

		rapidxml::xml_node<> * cmd = doc.allocate_node(rapidxml::node_element, "c", it->second.c_str());
		cmd->append_attribute(doc.allocate_attribute("tick", int2xml(doc, it->first)));

		cmds->append_node(cmd);
	}

	for(std::map<double, std::string>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double time = it->first;

		rapidxml::xml_node<> * cmd = doc.allocate_node(rapidxml::node_element, "c", it->second.c_str());
		cmd->append_attribute(doc.allocate_attribute("t", double2xml(doc,it->first)));

		cmds->append_node(cmd);
	}

	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	FILE * pFile = 0;
	_wfopen_s(&pFile, fileName, L"wb");

	if(0 != pFile)
	{
		fputs(xmlString.c_str(), pFile);
		fclose(pFile);
		return true;
	}
	
	return false;
}

bool CommandSystem::Load(wchar_t const * fileName)
{
	bool bOk = false;
	bool bUsedByTick = false;
	bool bUsedByTime = false;

	FILE * pFile = 0;

	_wfopen_s(&pFile, fileName, L"rb");

	if(!pFile)
		return false;
	
	fseek(pFile, 0, SEEK_END);
	size_t fileSize = ftell(pFile);
	rewind(pFile);

	char * pData = new char[fileSize+1];
	pData[fileSize] = 0;

	size_t readSize = fread(pData, sizeof(char), fileSize, pFile);
	bOk = readSize == fileSize;
	if(bOk)
	{
		try
		{
			do
			{
				rapidxml::xml_document<> doc;
				doc.parse<0>(pData);

				rapidxml::xml_node<> * cur_node = doc.first_node("commandSystem");
				if(!cur_node) break;

				// Clear current Map:
				Clear();

				cur_node = cur_node->first_node("commands");
				if(!cur_node) break;

				for(cur_node = cur_node->first_node("c"); cur_node; cur_node = cur_node->next_sibling("c"))
				{
					if (rapidxml::xml_attribute<> * tickAttr = cur_node->first_attribute("tick"))
					{
						bUsedByTick = true;
						m_TickMap[atoi(tickAttr->value())] = std::string(cur_node->value());
					}

					if (rapidxml::xml_attribute<> * timeAttr = cur_node->first_attribute("t"))
					{
						bUsedByTime = true;
						m_Map[atof(timeAttr->value())] = std::string(cur_node->value());
					}
				}
			}
			while (false);
		}
		catch(rapidxml::parse_error &)
		{
			bOk=false;
		}
	}

	delete pData;

	fclose(pFile);

	if (bUsedByTick && !IsSupportedByTick())
	{
		Tier0_Warning("Error: Missing hooks for supporting scheduling by tick.\n");
	}
	if (bUsedByTick && !IsSupportedByTime())
	{
		Tier0_Warning("Error: Missing hooks for supporting scheduling by time.\n");
	}

	return bOk;
}

void CommandSystem::Console_List(void)
{
	int idx = 0;

	Tier0_Msg("index: tick -> command\n");

	for (std::map<int, std::string>::iterator it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
	{
		Tier0_Msg("%i: %i -> %s\n",
			idx,
			it->first,
			it->second.c_str());

		++idx;
	}

	Tier0_Msg("----\n");

	Tier0_Msg("index: time -> command\n");

	for(std::map<double, std::string>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		Tier0_Msg("%i: %f -> %s\n",
			idx,
			it->first,
			it->second.c_str());

		++idx;
	}

	Tier0_Msg("----\n");
}

void CommandSystem::Do_Commands(void)
{
	if (IsSupportedByTick())
	{
		int tick = g_VEngineClient->GetDemoInfoEx()->GetDemoPlaybackTick();

		//Tier0_Msg("%i\n", tick);

		if (0 != tick) // this can happen when using the prev-tick button on demoui
		{

			if (Enabled && 0 < m_TickMap.size())
			{
				for (
					std::map<int, std::string>::iterator it = m_TickMap.upper_bound(m_LastTick);
					it != m_TickMap.end() && it->first <= tick;
					++it)
				{
					//Tier0_Msg("%i < %i <= %i\n", m_LastTick, it->first, tick);
					g_VEngineClient->ExecuteClientCmd(it->second.c_str());
				}
			}

			m_LastTick = tick;
		}
	}

	if (IsSupportedByTime())
	{
		double time = g_MirvTime.GetTime();

		//Tier0_Msg("%f\n", time);

		if (Enabled && 0 < m_Map.size())
		{
			for (
				std::map<double, std::string>::iterator it = m_Map.upper_bound(m_LastTime);
				it != m_Map.end() && it->first <= time;
				++it)
			{
				g_VEngineClient->ExecuteClientCmd(it->second.c_str());
			}
		}

		m_LastTime = time;
	}
}

void CommandSystem::OnLevelInitPreEntityAllTools(void)
{
	m_LastTime = -1;
	m_LastTick = -1;
}

bool CommandSystem::IsSupportedByTime(void)
{
	return 0 != g_VEngineClient && 0 != g_Hook_VClient_RenderView.GetGlobals();
}

bool CommandSystem::IsSupportedByTick(void)
{
	return 0 != g_VEngineClient && 0 != g_VEngineClient->GetDemoInfoEx();
}
