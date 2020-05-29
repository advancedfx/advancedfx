#include "stdafx.h"

#include "CommandSystem.h"

#include "WrpVEngineClient.h"
#include "RenderView.h"
#include "MirvTime.h"

#include <shared/rapidxml/rapidxml.hpp>
#include <shared/rapidxml/rapidxml_print.hpp>

#include <shared/StringTools.h>

//TODO: Reduce code duplication.

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
	double time = m_LastTime;

	AddAtTime(command, time);
}

void CommandSystem::AddAtTime(char const* command, double time)
{
	if (!IsSupportedByTime())
	{
		Tier0_Warning("Error: Missing hooks for supporting scheduling by time.\n");
		return;
	}

	CCommand* cmd = new CCommand();
	cmd->SetCommand(command);

	Interval<double> interval(time, time);
	m_TimeMap.insert({ interval, cmd });
	m_TimeTree = Insert<double>(m_TimeTree, interval, cmd);
}


void CommandSystem::AddTick(char const * command)
{
	int tick = m_LastTick;

	AddAtTick(command, tick);
}

void CommandSystem::AddAtTick(char const* command, int tick)
{
	if (!IsSupportedByTick())
	{
		Tier0_Warning("Error: Missing hooks for supporting scheduling by tick.\n");
		return;
	}

	CCommand* cmd = new CCommand();
	cmd->SetCommand(command);

	Interval<int> interval(tick, tick);
	m_TickMap.insert({ interval, cmd });
	m_TickTree = Insert<int>(m_TickTree, interval, cmd);
}

void CommandSystem::EditStart(double startTime)
{
	std::multimap<Interval<double>, CCommand*> tmpMap;

	tmpMap.swap(m_TimeMap);

	for (auto it = tmpMap.begin(); it != tmpMap.end(); ++it)
	{
		double time = it->first.Low;

		if (it == tmpMap.begin())
		{
			startTime = startTime - time;
		}

		time += startTime;

		m_TimeMap.insert({ Interval<double>(time, it->first.High + startTime), it->second});
	}

	DeleteTimeTree();
}

void CommandSystem::EditStartTick(int startTick)
{
	std::multimap<Interval<int>, CCommand*> tmpMap;

	tmpMap.swap(m_TickMap);

	for (auto it = tmpMap.begin(); it != tmpMap.end(); ++it)
	{
		int tick = it->first.Low;

		if (it == tmpMap.begin())
		{
			startTick = startTick - tick;
		}

		tick += startTick;

		m_TickMap.insert({ Interval<int>(tick, it->first.High + startTick) , it->second });
	}

	DeleteTickTree();
}

bool CommandSystem::Remove(int index)
{
	if (index < (int)m_TickMap.size())
	{
		int idx = 0;

		for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
		{
			if (idx == index)
			{
				m_TickMap.erase(it);
				DeleteTickTree();
				return true;
			}

			++idx;
		}
	}
	else
	{
		index -= m_TickMap.size();

		int idx = 0;

		for (auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it)
		{
			if (idx == index)
			{
				m_TimeMap.erase(it);
				DeleteTimeTree();
				return true;
			}

			++idx;
		}
	}

	return false;
}

void CommandSystem::Clear(void)
{
	for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it) delete it->second;
	m_TickMap.clear();
	DeleteTickTree();

	for (auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it) delete it->second;
	m_TimeMap.clear();
	DeleteTimeTree();
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

	for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
	{
		rapidxml::xml_node<> * cmd = doc.allocate_node(rapidxml::node_element, "c");
		cmd->append_attribute(doc.allocate_attribute("tick", int2xml(doc, it->first.Low)));

		if (it->first.Low != it->first.High)
		{
			cmd->append_attribute(doc.allocate_attribute("end", int2xml(doc, it->first.High)));
		}

		if (it->second->GetFormated())
		{
			cmd->append_attribute(doc.allocate_attribute("formated", "1"));
		}

		size_t numCurves = it->second->GetSize();
		for (size_t idxCurve = 0; idxCurve < numCurves; ++idxCurve)
		{
			rapidxml::xml_node<>* curve = doc.allocate_node(rapidxml::node_element, "curve");

			if (it->second->GetInterp(idxCurve) == CDoubleInterp::Method_Cubic)
				cmd->append_attribute(doc.allocate_attribute("interp", "cubic"));

			for (auto itPt = it->second->GetMap(idxCurve).begin(); itPt != it->second->GetMap(idxCurve).end(); ++itPt)
			{
				rapidxml::xml_node<>* pt = doc.allocate_node(rapidxml::node_element, "p");

				pt->append_attribute(doc.allocate_attribute("t", double2xml(doc, itPt->first)));
				pt->append_attribute(doc.allocate_attribute("v", double2xml(doc, itPt->second)));

				curve->append_node(pt);
			}

			cmd->append_node(curve);
		}

		rapidxml::xml_node<>* body = doc.allocate_node(rapidxml::node_element, "body", it->second->GetCommand());
		cmd->append_node(body);

		cmds->append_node(cmd);
	}

	for(auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it)
	{
		rapidxml::xml_node<> * cmd = doc.allocate_node(rapidxml::node_element, "c");
		cmd->append_attribute(doc.allocate_attribute("t", double2xml(doc,it->first.Low)));

		if (it->first.Low != it->first.High)
		{
			cmd->append_attribute(doc.allocate_attribute("end", double2xml(doc, it->first.High)));
		}

		if (it->second->GetFormated())
		{
			cmd->append_attribute(doc.allocate_attribute("formated", "1"));
		}

		size_t numCurves = it->second->GetSize();
		for (size_t idxCurve = 0; idxCurve < numCurves; ++idxCurve)
		{
			rapidxml::xml_node<>* curve = doc.allocate_node(rapidxml::node_element, "curve");
			
			if(it->second->GetInterp(idxCurve) == CDoubleInterp::Method_Cubic)
				cmd->append_attribute(doc.allocate_attribute("interp", "cubic"));

			for (auto itPt = it->second->GetMap(idxCurve).begin(); itPt != it->second->GetMap(idxCurve).end(); ++itPt)
			{
				rapidxml::xml_node<>* pt = doc.allocate_node(rapidxml::node_element, "p");

				pt->append_attribute(doc.allocate_attribute("t", double2xml(doc, itPt->first)));
				pt->append_attribute(doc.allocate_attribute("v", double2xml(doc, itPt->second)));

				curve->append_node(pt);
			}

			cmd->append_node(curve);
		}

		rapidxml::xml_node<>* body = doc.allocate_node(rapidxml::node_element, "body", it->second->GetCommand());
		cmd->append_node(body);

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
						int tick = atoi(tickAttr->value());

						CCommand* cmd = new CCommand();

						Interval<int> range = Interval<int>(tick,tick);

						if (NULL == cur_node->first_node())
						{
							cmd->SetCommand(cur_node->value());
						}

						for (rapidxml::xml_node<>* cur_body = cur_node->first_node("body"); cur_body; cur_body = cur_body->next_sibling("body"))
						{
							cmd->SetCommand(cur_body->value());
						}

						if (rapidxml::xml_attribute<>* endTickAttr = cur_node->first_attribute("end"))
						{
							range = Interval<int>(tick, atoi(endTickAttr->value()));
						}

						if (rapidxml::xml_attribute<>* formatedAttr = cur_node->first_attribute("formated"))
						{
							if (0 != atoi(formatedAttr->value())) cmd->SetFormated(true);
						}

						for (rapidxml::xml_node<>* cur_curve = cur_node->first_node("curve"); cur_curve; cur_curve = cur_curve->next_sibling("curve"))
						{
							cmd->SetSize(cmd->GetSize() + 1);

							if (rapidxml::xml_attribute<>* methodAttr = cur_node->first_attribute("interp"))
							{
								if (0 == strcmp("cubic", methodAttr->value()))
									cmd->SetInterp(cmd->GetSize() - 1, CDoubleInterp::Method_Cubic);
							}

							for (rapidxml::xml_node<>* cur_point = cur_curve->first_node("p"); cur_point; cur_point = cur_point->next_sibling("p"))
							{
								rapidxml::xml_attribute<>* pointTimeAttr = cur_point->first_attribute("t");
								rapidxml::xml_attribute<>* pointValueAttr = cur_point->first_attribute("v");

								if (pointTimeAttr && pointValueAttr)
								{
									cmd->GetMap(cmd->GetSize() - 1).insert({atof(pointTimeAttr->value()), atof(pointValueAttr->value())});
								}
							}
						}

						bUsedByTick = true;
						m_TickMap.insert({ range, cmd });
						m_TickTree = Insert<int>(m_TickTree, range, cmd);
					}

					if (rapidxml::xml_attribute<> * timeAttr = cur_node->first_attribute("t"))
					{
						double time = atof(timeAttr->value());

						CCommand* cmd = new CCommand();
						
						Interval<double> range = Interval<double>(time,time);

						if (NULL == cur_node->first_node())
						{
							cmd->SetCommand(cur_node->value());
						}

						for (rapidxml::xml_node<>* cur_body = cur_node->first_node("body"); cur_body; cur_body = cur_body->next_sibling("body"))
						{
							cmd->SetCommand(cur_body->value());
						}

						if (rapidxml::xml_attribute<>* endTimeAttr = cur_node->first_attribute("end"))
						{
							range = Interval<double>(time, atof(endTimeAttr->value()));
						}

						if (rapidxml::xml_attribute<>* formatedAttr = cur_node->first_attribute("formated"))
						{
							if (0 != atoi(formatedAttr->value())) cmd->SetFormated(true);
						}

						for (rapidxml::xml_node<>* cur_curve = cur_node->first_node("curve"); cur_curve; cur_curve = cur_curve->next_sibling("curve"))
						{
							cmd->SetSize(cmd->GetSize() + 1);

							if (rapidxml::xml_attribute<>* methodAttr = cur_node->first_attribute("interp"))
							{
								if (0 == strcmp("cubic", methodAttr->value()))
									cmd->SetInterp(cmd->GetSize() - 1, CDoubleInterp::Method_Cubic);
							}

							for (rapidxml::xml_node<>* cur_point = cur_curve->first_node("p"); cur_point; cur_point = cur_point->next_sibling("p"))
							{
								rapidxml::xml_attribute<>* pointTimeAttr = cur_point->first_attribute("t");
								rapidxml::xml_attribute<>* pointValueAttr = cur_point->first_attribute("v");

								if (pointTimeAttr && pointValueAttr)
								{
									cmd->GetMap(cmd->GetSize() - 1).insert({ atof(pointTimeAttr->value()), atof(pointValueAttr->value()) });
								}
							}
						}

						bUsedByTime = true;
						m_TimeMap.insert({ range, cmd });
						m_TimeTree = Insert<double>(m_TimeTree, range, cmd);
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

	delete[] pData;

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

	for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
	{
		if (it->first.Low == it->first.High)
		{
			Tier0_Msg("%i: %i -> %s\n",
				idx,
				it->first.Low,
				it->second->GetCommand());
		}
		else
		{
			Tier0_Msg("%i: %i - %i -> %s\n",
				idx,
				it->first.Low,
				it->first.High,
				it->second->GetCommand());
		}

		++idx;
	}

	Tier0_Msg("----\n");

	Tier0_Msg("index: time -> command\n");

	for (auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it)
	{
		if (it->first.Low == it->first.High)
		{
			Tier0_Msg("%i: %f -> %s\n",
				idx,
				it->first.Low,
				it->second->GetCommand());
		}
		else
		{
			Tier0_Msg("%i: %f - %f -> %s\n",
				idx,
				it->first.Low,
				it->first.High,
				it->second->GetCommand());
		}

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
			if (Enabled && 0 < m_TickMap.size() && m_LastTick != tick)
			{
				EnsureTickTree();

				OverlapExecute(m_TickTree, Interval<int>(m_LastTick, tick), g_Hook_VClient_RenderView.GetGlobals()->interpolation_amount_get());
			}

			m_LastTick = tick;
		}
	}

	if (IsSupportedByTime())
	{
		double time = g_MirvTime.GetTime();

		//Tier0_Msg("%f\n", time);

		if (Enabled && 0 < m_TimeMap.size() && m_LastTime != time)
		{
			EnsureTimeTree();

			OverlapExecute(m_TimeTree, Interval<double>(m_LastTime, time));
		}

		m_LastTime = time;
	}
}

void CommandSystem::OnLevelInitPreEntityAllTools(void)
{
	m_LastTime = -1;
	m_LastTick = -1;

	for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it) it->second->LastTick = -1;
	for (auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it) it->second->LastTime = -1;
}

bool CommandSystem::IsSupportedByTime(void)
{
	return 0 != g_VEngineClient && 0 != g_Hook_VClient_RenderView.GetGlobals();
}

bool CommandSystem::IsSupportedByTick(void)
{
	return 0 != g_VEngineClient && 0 != g_Hook_VClient_RenderView.GetGlobals() && 0 != g_VEngineClient->GetDemoInfoEx();
}


bool CommandSystem::CCommand::DoCommand(double t01)
{
	if (!m_Formated)
	{
		g_VEngineClient->ClientCmd_Unrestricted(m_Command.c_str()); // We don't use ExecuteCliendCmd here, because people might clear the commands while they are executed, which we would need to track otherwise in order to prevent crashes, should be accurate enough hopefully.
		return true;
	}
	
	std::string fCommand;

	bool inBegin = false;
	bool inEnd = false;
	bool inCmd = false;
	std::string::const_iterator itBegin;
	std::string::const_iterator itEnd;
	std::string strIdx;
	int idx = 0;
	double val = 0;
	char pszValue[100];
	pszValue[0] = 0;

	for (auto it = m_Command.cbegin(); it != m_Command.cend(); ++it)
	{
		if (inBegin)
		{
			switch (*it)
			{
			case '{':
				inBegin = false;
				break;
			case '}':
				if (inCmd) return false;
				inCmd = false;
				strIdx.assign(itBegin, itEnd);
				idx = atoi(strIdx.c_str());
				if (idx < 0 || idx >= (int)m_Interp.size()) return false;
				if (!m_Interp[idx].CanEval()) return false;
				val = m_Interp[idx].Eval(t01);
				_snprintf_s(pszValue, _TRUNCATE, "%f", val);
				fCommand += pszValue;
				break;
			default:
				++itEnd;
			}
		}
		else if(inEnd)
		{
			switch (*it)
			{
			case '}':
				inEnd = false;
				fCommand += '}';
				break;
			default:
				return false;
			}
		}
		else
		{
			switch (*it)
			{
			case '{':
				inBegin = true;
				itBegin = it + 1;
				itEnd = itBegin;
				break;
			case '}':
				inEnd = true;
				break;
			default:
				fCommand += *it;
			}
		}
	}

	g_VEngineClient->ClientCmd_Unrestricted(fCommand.c_str());

	return true;
}

void CommandSystem::AddCurves(IWrpCommandArgs* args)
{
	const char* arg0 = args->ArgV(0);
	int argC = args->ArgC();

	if (5 <= argC)
	{
		CCommand* cmd = new CCommand();

		cmd->SetFormated(true);

		int idx = 4;
		int dim = 0;

		while (idx < argC && 0 == strcmp("-", args->ArgV(idx)))
		{
			cmd->SetSize(dim + 1);

			++idx;

			if (StringIBeginsWith(args->ArgV(idx), "interp="))
			{
				cmd->SetInterp(dim, 0 == _stricmp("cubic", args->ArgV(idx) + strlen("interp=")) ? CDoubleInterp::Method_Cubic : CDoubleInterp::Method_Linear);

				++idx;
			}

			int breakPoint = idx;

			while (breakPoint < argC && 0 != strcmp("-", args->ArgV(breakPoint)) && 0 != strcmp("--", args->ArgV(breakPoint)))
				++breakPoint;

			if ((breakPoint - idx) & 0x1)
			{
				Tier0_Warning("AFXERRORR: CurveTime-Value pairs are not balanced (are odd).\n");
				delete cmd;
				return;
			}

			int ptArgs = (breakPoint - idx) / 2;

			for (int i = 0; i < ptArgs; ++i)
			{
				cmd->GetMap(dim).insert({ atof(args->ArgV(idx + i * 2)), atof(args->ArgV(idx + i * 2 + 1)) });
			}

			++dim;
			idx = breakPoint;
		}

		if(argC <= idx || 0 != strcmp("--", args->ArgV(idx)))
		{
			Tier0_Warning("AFXERRORR: Commands separator \"--\" not found.\n");
			delete cmd;
			return;
		}

		++idx;

		std::string cmdStr = "";

		while (idx < argC)
		{
			if(!cmdStr.empty()) cmdStr += " ";
			cmdStr += args->ArgV(idx);

			++idx;
		}

		cmd->SetCommand(cmdStr.c_str());

		if (0 == _stricmp("tick", args->ArgV(1)))
		{
			Interval<int> interval(atoi(args->ArgV(2)), atoi(args->ArgV(3)));
			m_TickMap.insert({ interval, cmd });
			m_TickTree = Insert<int>(m_TickTree, interval, cmd);
			return;
		}
		else if (0 == _stricmp("time", args->ArgV(1)))
		{
			Interval<double> interval(atof(args->ArgV(2)), atof(args->ArgV(3)));
			m_TimeMap.insert({interval, cmd });
			m_TimeTree = Insert<double>(m_TimeTree, interval, cmd);
			return;
		}
		else {
			Tier0_Warning("AFXERRORR: Expected time or tick.\n");
			delete cmd;
			return;
		}
	}

	Tier0_Msg(
		"%s tick <iStartTick> <iEndTick> [\"-\" [interp=(linear|cubic)] <fCurveTime1> <fCurveValue1> ... <fCurveTimeN> <fCurveValueN>]* \"--\" <formatedCommand1> ... <formatedCommandM> - adds a curve based on ticks, curveTime is in [0,1]. \n"
		"%s time <fStartTime> <iEndTime> [\"-\" [interp=(linear|cubic)] <fCurveTime1> <fCurveValue1> ... <fCurveTimeN> <fCurveValueN>]* \"--\" <formatedCommand1> ... <formatedCommandM> - adds a curve based on time, curveTime is in [0,1]. \n"
		, arg0
		, arg0
	);
}
