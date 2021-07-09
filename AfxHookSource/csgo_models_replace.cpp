#include "stdafx.h"

#include "csgo_models_replace.h"

#include "addresses.h"

#include "WrpConsole.h"

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


CCsgoModelsReplace::GetModel_t CCsgoModelsReplace::m_True_GetModel = nullptr;

void* __fastcall CCsgoModelsReplace::My_GetModel(void* This, void* Edx, const char* name, int iUnk) {
	const char* replacement = g_CCsgoModelsReplace.GetReplacement(name);

	void* result = m_True_GetModel(This, Edx, (nullptr != replacement ? replacement : name), iUnk);

	if (result && replacement) {
		// Fix up model name.

		size_t len = strlen(name) + 1;

		memcpy((unsigned char*)result + 4, name, len * sizeof(char));
	}

	return result;
}

void CCsgoModelsReplace::InstallHooks(void* pVModelInfoClient) {
	if (m_True_GetModel) return;

	if (0 == AFXADDR_GET(csgo_engine_CModelLoader_vtable)) return;

	void** vtable = (void**)AFXADDR_GET(csgo_engine_CModelLoader_vtable);

	m_True_GetModel = (GetModel_t)(vtable[7]);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)m_True_GetModel, My_GetModel);
	if (NO_ERROR != DetourTransactionCommit()) m_True_GetModel = nullptr;
}

void CCsgoModelsReplace::Add(const char* szWildCardStringSourceName, const char* szTargetName) {
	m_Replacements.emplace_back(szWildCardStringSourceName, szTargetName);
}

bool CCsgoModelsReplace::Remove(int index) {
	if (index < 0 || index >= (int)m_Replacements.size())
	{
		Tier0_Warning("Error: %i is not in valid range for <iIndex>\n", index);
		return false;
	}
	auto sourceIt = m_Replacements.begin();

	std::advance(sourceIt, index);

	m_Replacements.erase(sourceIt);

	return true;
}

bool CCsgoModelsReplace::MoveIndex(int index, int targetIndex) {
	if (index < 0 || index >= (int)m_Replacements.size())
	{
		Tier0_Warning("Error: %i is not in valid range for <iSourceIndex>\n", index);
		return false;
	}

	if (targetIndex < 0 || targetIndex >(int)m_Replacements.size())
	{
		Tier0_Warning("Error: %i is not in valid range for <iTargetIndex>\n", targetIndex);
		return false;
	}

	auto sourceIt = m_Replacements.begin();

	auto targetIt = m_Replacements.begin();

	if (index <= targetIndex)
	{
		std::advance(sourceIt, index);

		targetIt = sourceIt;

		std::advance(targetIt, targetIndex - index);
	}
	else
	{
		std::advance(targetIt, targetIndex);

		sourceIt = targetIt;

		std::advance(sourceIt, index - targetIndex);
	}

	m_Replacements.splice(targetIt, m_Replacements, sourceIt);

	return true;
}
void CCsgoModelsReplace::Print() {
	int i = 0;
	for (auto it = m_Replacements.begin(); it != m_Replacements.end(); ++it) {
		Tier0_Msg("%i: \"%s\" -> \"%s\"\n", i, it->WildcardSourceName.c_str(), it->TargetName.c_str());
	}
}

void CCsgoModelsReplace::Clear() {
	m_Replacements.clear();
}

const char* CCsgoModelsReplace::GetReplacement(const char* pModelName) {
	if (nullptr != pModelName) {
		for (auto it = m_Replacements.begin(); it != m_Replacements.end(); ++it) {
			if (StringWildCard1Matched(it->WildcardSourceName.c_str(), pModelName)) {
				if (m_Debug) Tier0_Msg("CCsgoModelsReplace::GetReplacement(\"%s\") -> \"%s\"\n", pModelName, it->TargetName.c_str());
				return it->TargetName.c_str();
			}
		}

		if (m_Debug) Tier0_Msg("CCsgoModelsReplace::GetReplacement(\"%s\")\n", pModelName);
	}
	else {
		if (m_Debug) Tier0_Msg("CCsgoModelsReplace::GetReplacement(null)\n");
	}
	return nullptr;
}


CCsgoModelsReplace g_CCsgoModelsReplace;

