#include "stdafx.h"

#include "CommandSystem.h"

#include "WrpVEngineClient.h"

#include <shared/rapidxml/rapidxml.hpp>
#include <shared/rapidxml/rapidxml_print.hpp>


extern WrpVEngineClient * g_VEngineClient;

CommandSystem g_CommandSystem;

CommandSystem::CommandSystem()
: Enabled(true)
, m_LastTime(0)
{

}

void CommandSystem::Add(double time, char const * command)
{
	std::map<double, std::string>::iterator it = m_Map.find(time);

	std::string cmds("");

	if(it != m_Map.end())
	{
		cmds.append(" ");
	}

	cmds.append(command);

	m_Map[time] = cmds;
}

bool CommandSystem::Remove(int index)
{
	int idx = 0;
	for(std::map<double, std::string>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(idx == index)
		{
			m_Map.erase(it);
			return true;
		}

		++idx;
	}

	return false;
}

void CommandSystem::Clear(void)
{
	m_Map.clear();
}

namespace CommandSystemXML {

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
					rapidxml::xml_attribute<> * timeAttr = cur_node->first_attribute("t");
					if(!timeAttr) continue;

					m_Map[atof(timeAttr->value())] = std::string(cur_node->value());
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

	return bOk;
}

void CommandSystem::Console_List(void)
{
	Tier0_Msg("index: time -> command\n");

	int idx = 0;
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

void CommandSystem::Do_Queue_Commands(double time)
{
	if(g_VEngineClient && Enabled)
	{
		for(
			std::map<double, std::string>::iterator it = m_Map.upper_bound(m_LastTime);
			it != m_Map.end() && it->first <= time;
			++it)
		{
			g_VEngineClient->ClientCmd_Unrestricted(it->second.c_str());
		}
	}

	m_LastTime = time;
}
