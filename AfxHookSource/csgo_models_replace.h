#pragma once

#include <list>
#include <string>

#define SOURCESDK_CSGO_VMODELINFOCLIENT_INTERFACE_VERSION "VModelInfoClient004"

class CCsgoModelsReplace {
public:
	bool GetDebug() {
		return m_Debug;
	}

	void SetDebug(bool value) {
		m_Debug = value;
	}

	bool HasHooks() {
		return nullptr != m_True_GetModel;
	}

	void InstallHooks(void* pVModelInfoClient);

	void Add(const char* szWildCardStringSourceName, const char* szTargetName);
	bool Remove(int index);
	bool MoveIndex(int index, int targetIndex);
	void Print();
	void Clear();

	const char * GetReplacement(const char * pModelName);

private:
	typedef void*	(__fastcall * GetModel_t)(void* This, void* Edx, const char* name, int iUnk);

	struct Replacement_s {
		std::string WildcardSourceName;
		std::string TargetName;

		Replacement_s(const char* szWcSource, const char* szTarget) : WildcardSourceName(szWcSource), TargetName(szTarget) {

		}
	};

	std::list<Replacement_s> m_Replacements;
	bool m_Debug = false;
	
	static GetModel_t m_True_GetModel;
	static void* __fastcall My_GetModel(void* This, void* Edx, const char* name, int iUnk);
};

extern CCsgoModelsReplace g_CCsgoModelsReplace;
