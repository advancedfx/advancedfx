#include "stdafx.h"

#include "csgo_MemAlloc.h"

#include "addresses.h"
#include "SourceInterfaces.h"

#include <shared/detours.h>

#include <Windows.h>

#include <set>
#include <list>
#include <map>
#include <mutex>

class CTier0MemAllocFreeNotifyees
{
public:
	void Insert(ITier0MemAllocFreeNotifyee * notifyee)
	{
		m_Set.insert(notifyee);
	}

	void Remove(ITier0MemAllocFreeNotifyee * notifyee)
	{
		m_Set.erase(notifyee);
	}

	bool IsEmpty(void)
	{
		return m_Set.empty();
	}

	void Notify(void *pMem)
	{
		for (std::set<ITier0MemAllocFreeNotifyee *>::iterator it = m_Set.begin(); it != m_Set.end(); ++it)
		{
			(*it)->OnTier0MemAllocFree(pMem);
		}
	}

private:
	std::set<ITier0MemAllocFreeNotifyee *> m_Set;

};

std::map<void *, CTier0MemAllocFreeNotifyees> g_Tier0MemAllocFreeNotifyees;
std::mutex g_Tier0MemAllocFreeNotifyeesMutex;
bool g_Tier0MemAllocFreeNotifyeesChanged = false;

void NotifiyOnTier0MemAllocFree(ITier0MemAllocFreeNotifyee * notifyee, void * pMem)
{
	std::unique_lock<std::mutex> lock(g_Tier0MemAllocFreeNotifyeesMutex);

	g_Tier0MemAllocFreeNotifyees[pMem].Insert(notifyee);

	g_Tier0MemAllocFreeNotifyeesChanged = true;
}

void DenotifiyOnTier0MemAllocFree(ITier0MemAllocFreeNotifyee * notifyee, void * pMem)
{
	std::unique_lock<std::mutex> lock(g_Tier0MemAllocFreeNotifyeesMutex);

	std::map<void *, CTier0MemAllocFreeNotifyees>::iterator it = g_Tier0MemAllocFreeNotifyees.find(pMem);

	if (it != g_Tier0MemAllocFreeNotifyees.end())
	{
		it->second.Remove(notifyee);
		if (it->second.IsEmpty())
			g_Tier0MemAllocFreeNotifyees.erase(it);
	}
}

typedef void (__stdcall * Tier0_CMemAlloc_Free_t)(DWORD * this_ptr, void *pMem);

Tier0_CMemAlloc_Free_t g_Tier0_CMemAlloc_Free;

void __stdcall touring_Tier0_CMemAlloc_Free(DWORD * this_ptr, void *pMem)
{
	if(pMem)
	{
		size_t size = SOURCESDK::Get_g_pMemAlloc()->GetSize(pMem); // Make sure GetSize is not called with 0 pointer!

		void * upperBound = (char *)pMem + size;

		std::unique_lock<std::mutex> lock(g_Tier0MemAllocFreeNotifyeesMutex);
		for (std::map<void *, CTier0MemAllocFreeNotifyees>::iterator it = g_Tier0MemAllocFreeNotifyees.lower_bound(pMem); it != g_Tier0MemAllocFreeNotifyees.end(); )
		{
			if (it->first < upperBound)
			{
				it->second.Notify(it->first);

				std::map<void *, CTier0MemAllocFreeNotifyees>::iterator itErase = it;
				++it;

				g_Tier0MemAllocFreeNotifyees.erase(itErase);
			}
			else
				break;
		}
	}

	g_Tier0_CMemAlloc_Free(this_ptr, pMem);
}

bool Hook_csgo_MemAlloc(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (SourceSdkVer_CSGO == g_SourceSdkVer || SourceSdkVer_CSCO == g_SourceSdkVer)
	{
		if (SOURCESDK::IMemAlloc_csgo * iface = SOURCESDK::Get_g_pMemAlloc())
		{
			int * vtable = *(int**)iface;

			 DetourIfacePtr((DWORD *)&(vtable[5]), touring_Tier0_CMemAlloc_Free, (DetourIfacePtr_fn &)g_Tier0_CMemAlloc_Free);

			firstResult = true;
		}
	}

	return firstResult;
}
