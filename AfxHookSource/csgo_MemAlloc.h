#pragma once

#include "SourceInterfaces.h"

class ITier0MemAllocFreeNotifyee abstract
{
public:
	/// <summary>
	/// When implementing this
	/// YOU MUST NOT call cany of the functions in this header (i.e. NotifiyOnTier0MemAllocFree)
	/// AND NOT call anything of the game, since it might trigger additional Free Notifications
	/// WHICH WOULD DEADLOCK!<br />
	/// The notifyee will be automatically removed!
	/// </summary>
	virtual void OnTier0MemAllocFree(void * pMem) = 0;
};

// currently not hooked // void NotifiyOnTier0MemAllocFree(ITier0MemAllocFreeNotifyee * notifyee, void * pMem);
void DenotifiyOnTier0MemAllocFree(ITier0MemAllocFreeNotifyee * notifyee, void * pMem);

bool Hook_csgo_MemAlloc(void);
