#pragma once

#include <list>
#include <string>
#include <map>

class CCsgoModelsReplace {
public:
	struct Replacement_s {
		std::string WildcardSourceName;
		std::string TargetName;
		bool Transparent;

		Replacement_s(const char* szWcSource, const char* szTarget, bool transparent) : WildcardSourceName(szWcSource), TargetName(szTarget), Transparent(transparent) {

		}
	};

	bool GetDebug() {
		return m_Debug;
	}

	void SetDebug(bool value) {
		m_Debug = value;
	}

	bool InstallHooks();

	void Add(const char* szWildCardStringSourceName, const char* szTargetName, bool transparent);
	bool Remove(int index);
	bool MoveIndex(int index, int targetIndex);
	void Print();
	void Clear();

	struct Replacement_s* GetLoaderReplacement(const char* pModelName) {
		return GetSomeReplacement(pModelName, m_LoaderReplacements, "GetLoaderReplacement");
	}

private:
	typedef void* (__fastcall * FindModel_t)(void* This, void* Edx, const char* name);

	std::list<Replacement_s> m_LoaderReplacements;

	bool m_Debug = false;
	
	static bool m_HooksInstalled;
	static FindModel_t m_True_FindModel;

	static void* __fastcall My_FindModel(void* This, void* Edx, const char* name);

	struct Replacement_s* GetSomeReplacement(const char* pModelName, std::list<Replacement_s> & replacements, const char * szDebugName);
};

extern CCsgoModelsReplace g_CCsgoModelsReplace;
