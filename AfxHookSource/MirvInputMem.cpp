#include "stdafx.h"

#include "MirvInputMem.h"

#include "WrpVEngineClient.h"
#include "RenderView.h"

#include <shared/StringTools.h>

#include <shared/rapidxml/rapidxml.hpp>
#include <shared/rapidxml/rapidxml_print.hpp>


MirvInputMem g_MirvInputMem;


void MirvInputMem::Console(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * cmd0 = 0 < argc ? args->ArgV(0) : "n/a";

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("store", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			m_Map[std::string(name)] = CData(
				g_Hook_VClient_RenderView.LastCameraOrigin[0]
				, g_Hook_VClient_RenderView.LastCameraOrigin[1]
				, g_Hook_VClient_RenderView.LastCameraOrigin[2]
				, g_Hook_VClient_RenderView.LastCameraAngles[0]
				, g_Hook_VClient_RenderView.LastCameraAngles[1]
				, g_Hook_VClient_RenderView.LastCameraAngles[2]
				, g_Hook_VClient_RenderView.LastCameraFov
			);

			return;
		}
		else if (!_stricmp("use", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			bool useOrigin = false;
			bool useAngles = false;
			bool useFov = false;

			for (int i = 3; i < argc; ++i)
			{
				char const * opt = args->ArgV(i);

				if (!_stricmp("origin", opt))
					useOrigin = true;
				if (!_stricmp("angles", opt))
					useAngles = true;
				if (!_stricmp("fov", opt))
					useFov = true;
			}

			if (!(useOrigin || useAngles || useFov))
				useOrigin = useAngles = useFov = true;

			std::map<std::string, CData>::iterator it = m_Map.find(std::string(name));

			if (it != m_Map.end())
			{
				CData & data = it->second;

				if (useOrigin)
				{
					g_Hook_VClient_RenderView.LastCameraOrigin[0] = data.Origin[0];
					g_Hook_VClient_RenderView.LastCameraOrigin[1] = data.Origin[1];
					g_Hook_VClient_RenderView.LastCameraOrigin[2] = data.Origin[2];
				}
				if (useAngles)
				{
					g_Hook_VClient_RenderView.LastCameraAngles[0] = data.Angles[0];
					g_Hook_VClient_RenderView.LastCameraAngles[1] = data.Angles[1];
					g_Hook_VClient_RenderView.LastCameraAngles[2] = data.Angles[2];
				}
				if (useFov)
				{
					g_Hook_VClient_RenderView.LastCameraFov = data.Fov;
				}

				return;
			}
			Tier0_Msg("Error: There is no state with name %s.\n", name);
			return;
		}
		else if (!_stricmp("remove", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			std::map<std::string, CData>::iterator it = m_Map.find(std::string(name));

			if (it != m_Map.end())
			{
				m_Map.erase(it);

				return;
			}
			Tier0_Msg("Error: There is no state with name %s.\n", name);
			return;
		}
		else if (!_stricmp("print", cmd1))
		{
			Tier0_Msg("name: x y z | xRoll yPitch zYaw | fov\n");
			for (std::map<std::string, CData>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
			{
				char const * name = it->first.c_str();
				CData & data = it->second;

				Tier0_Msg(
					"%s: %f %f %f | %f %f %f | %f\n"
					, name
					, data.Origin[0], data.Origin[1], data.Origin[2]
					, data.Angles[1], data.Angles[0], data.Angles[2]
					, data.Fov
				);
			}

			return;
		}
		else if (!_stricmp("clear", cmd1))
		{
			m_Map.clear();
			return;
		}
		else if (!_stricmp("save", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			std::wstring wideString;
			bool bOk = UTF8StringToWideString(name, wideString)
				&& Save(wideString.c_str());

			Tier0_Msg("Saving: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if (!_stricmp("load", cmd1) && 3 <= argc)
		{
			char const * name = args->ArgV(2);

			std::wstring wideString;
			bool bOk = UTF8StringToWideString(name, wideString)
				&& Load(wideString.c_str());

			Tier0_Msg("Loading: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}

	}

	Tier0_Msg(
		"%s store <name> - Store current view state.\n"
		"%s use <name> [origin] [angles] [fov] - Restore view state, if only name is given, then all properties are restored, otherwise only the given properties.\n"
		"%s remove <name> - Remove stored state.\n"
		"%s print - Print currently stored states.\n"
		"%s clear - Clear all stored states.\n"
		"%s save <fileName> - Save all states to file in XML format.\n"
		"%s load <fileName> - Load states from file (adding to existing states / overwriting states with same name).\n"
		, cmd0
		, cmd0
		, cmd0
		, cmd0
		, cmd0
		, cmd0
		, cmd0
	);
}

char * double2xml(rapidxml::xml_document<> & doc, double value);

bool MirvInputMem::Save(wchar_t const * fileName)
{
	rapidxml::xml_document<> doc;

	rapidxml::xml_node<> * decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	rapidxml::xml_node<> * mirvInput = doc.allocate_node(rapidxml::node_element, "mirvInput");
	doc.append_node(mirvInput);

	rapidxml::xml_node<> * states = doc.allocate_node(rapidxml::node_element, "states");
	mirvInput->append_node(states);

	for (std::map<std::string, CData>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		char const * name = it->first.c_str();
		CData & data = it->second;

		rapidxml::xml_node<> * state = doc.allocate_node(rapidxml::node_element, "s");
		state->append_attribute(doc.allocate_attribute("name", name));
		state->append_attribute(doc.allocate_attribute("x", double2xml(doc, data.Origin[0])));
		state->append_attribute(doc.allocate_attribute("y", double2xml(doc, data.Origin[1])));
		state->append_attribute(doc.allocate_attribute("z", double2xml(doc, data.Origin[2])));
		state->append_attribute(doc.allocate_attribute("rx", double2xml(doc, data.Angles[1])));
		state->append_attribute(doc.allocate_attribute("ry", double2xml(doc, data.Angles[0])));
		state->append_attribute(doc.allocate_attribute("rz", double2xml(doc, data.Angles[2])));
		state->append_attribute(doc.allocate_attribute("fov", double2xml(doc, data.Fov)));

		states->append_node(state);
	}

	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	FILE * pFile = 0;
	_wfopen_s(&pFile, fileName, L"wb");

	if (0 != pFile)
	{
		fputs(xmlString.c_str(), pFile);
		fclose(pFile);
		return true;
	}

	return false;
}

bool MirvInputMem::Load(wchar_t const * fileName)
{
	bool bOk = false;

	FILE * pFile = 0;

	_wfopen_s(&pFile, fileName, L"rb");

	if (!pFile)
		return false;

	fseek(pFile, 0, SEEK_END);
	size_t fileSize = ftell(pFile);
	rewind(pFile);

	char * pData = new char[fileSize + 1];
	pData[fileSize] = 0;

	size_t readSize = fread(pData, sizeof(char), fileSize, pFile);
	bOk = readSize == fileSize;
	if (bOk)
	{
		try
		{
			do
			{
				rapidxml::xml_document<> doc;
				doc.parse<0>(pData);

				rapidxml::xml_node<> * cur_node = doc.first_node("mirvInput");
				if (!cur_node) break;

				cur_node = cur_node->first_node("states");
				if (!cur_node) break;

				for (cur_node = cur_node->first_node("s"); cur_node; cur_node = cur_node->next_sibling("s"))
				{
					rapidxml::xml_attribute<> * nameAttr = cur_node->first_attribute("name");
					if (!nameAttr) continue;

					rapidxml::xml_attribute<> * xA = cur_node->first_attribute("x");
					rapidxml::xml_attribute<> * yA = cur_node->first_attribute("y");
					rapidxml::xml_attribute<> * zA = cur_node->first_attribute("z");
					rapidxml::xml_attribute<> * rxA = cur_node->first_attribute("rx");
					rapidxml::xml_attribute<> * ryA = cur_node->first_attribute("ry");
					rapidxml::xml_attribute<> * rzA = cur_node->first_attribute("rz");
					rapidxml::xml_attribute<> * fovA = cur_node->first_attribute("fov");

					double dX = xA ? atof(xA->value()) : 0.0;
					double dY = yA ? atof(yA->value()) : 0.0;
					double dZ = zA ? atof(zA->value()) : 0.0;
					double dRX = rxA ? atof(rxA->value()) : 0.0;
					double dRY = ryA ? atof(ryA->value()) : 0.0;
					double dRZ = rzA ? atof(rzA->value()) : 0.0;
					double dFov = fovA ? atof(fovA->value()) : 90.0;

					m_Map[std::string(nameAttr->value())] = CData(
						dX, dY, dZ
						, dRY, dRX, dRZ
						, dFov
					);
				}
			} while (false);
		}
		catch (rapidxml::parse_error &)
		{
			bOk = false;
		}
	}

	delete pData;

	fclose(pFile);

	return bOk;
}
