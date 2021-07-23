#include "stdafx.h"

#include "csgo_models_replace.h"

#include "addresses.h"

#include "WrpConsole.h"

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


bool CCsgoModelsReplace::m_HooksInstalled = false;
CCsgoModelsReplace::GetModel_t CCsgoModelsReplace::m_True_GetModel = nullptr;


void* __fastcall CCsgoModelsReplace::My_GetModel(void* This, void* Edx, const char* name, int iUnk) {
	auto replacement = g_CCsgoModelsReplace.GetLoaderReplacement(name);

	void* result = m_True_GetModel(This, Edx, (nullptr != replacement ? replacement->TargetName.c_str() : name), iUnk);

	if (result && replacement && replacement->Transparent) {
		// Fix up model name.

		size_t len = strlen(name) + 1;

		memcpy((unsigned char*)result + 4, name, len * sizeof(char));
	}

	return result;
}

bool CCsgoModelsReplace::InstallHooks() {
	if (m_HooksInstalled) return true;

	if (0 == AFXADDR_GET(csgo_engine_CModelLoader_vtable)) return false;

	void** vtable_CModelLoader = (void**)AFXADDR_GET(csgo_engine_CModelLoader_vtable);

	m_True_GetModel = (GetModel_t)(vtable_CModelLoader[7]);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)m_True_GetModel, My_GetModel);

	m_HooksInstalled = NO_ERROR == DetourTransactionCommit();

	return m_HooksInstalled;
}

void CCsgoModelsReplace::Add(const char* szWildCardStringSourceName, const char* szTargetName, bool transparent) {
	auto& replacments = m_LoaderReplacements;

	replacments.emplace_back(szWildCardStringSourceName, szTargetName, transparent);
}

bool CCsgoModelsReplace::Remove(int index) {
	auto& replacments = m_LoaderReplacements;

	if (index < 0 || index >= (int)replacments.size())
	{
		Tier0_Warning("Error: %i is not in valid range for <iIndex>\n", index);
		return false;
	}
	auto sourceIt = replacments.begin();

	std::advance(sourceIt, index);

	replacments.erase(sourceIt);

	return true;
}

bool CCsgoModelsReplace::MoveIndex(int index, int targetIndex) {
	auto& replacments = m_LoaderReplacements;

	if (index < 0 || index >= (int)replacments.size())
	{
		Tier0_Warning("Error: %i is not in valid range for <iSourceIndex>\n", index);
		return false;
	}

	if (targetIndex < 0 || targetIndex >(int)replacments.size())
	{
		Tier0_Warning("Error: %i is not in valid range for <iTargetIndex>\n", targetIndex);
		return false;
	}

	auto sourceIt = replacments.begin();

	auto targetIt = replacments.begin();

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

	replacments.splice(targetIt, replacments, sourceIt);

	return true;
}
void CCsgoModelsReplace::Print() {
	auto& replacments = m_LoaderReplacements;

	int i = 0;
	for (auto it = replacments.begin(); it != replacments.end(); ++it) {
		Tier0_Msg("%i: \"%s\" -> \"%s\" (transparent=%i)\n", i, it->WildcardSourceName.c_str(), it->TargetName.c_str(), it->Transparent ? 1 : 0);
	}
}

void CCsgoModelsReplace::Clear() {
	auto& replacments = m_LoaderReplacements;

	replacments.clear();
}

struct CCsgoModelsReplace::Replacement_s* CCsgoModelsReplace::GetSomeReplacement(const char* pModelName, std::list<Replacement_s>& replacements, const char* szDebugName) {
	if (nullptr != pModelName) {
		for (auto it = replacements.begin(); it != replacements.end(); ++it) {
			if (StringWildCard1Matched(it->WildcardSourceName.c_str(), pModelName)) {
				if (m_Debug) Tier0_Msg("CCsgoModelsReplace::%s(\"%s\") -> \"%s\"\n", szDebugName, pModelName, it->TargetName.c_str());
				return &(*it);
			}
		}

		if (m_Debug) Tier0_Msg("CCsgoModelsReplace::%s(\"%s\")\n", szDebugName, pModelName);
	}
	else {
		if (m_Debug) Tier0_Msg("CCsgoModelsReplace::%s(null)\n", szDebugName);
	}
	return nullptr;
}


CCsgoModelsReplace g_CCsgoModelsReplace;

