#include "stdafx.h"

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-02-09 dominik.matrixstorm.com
//
// First changes:
// 2017-02-09 dominik.matrixstorm.com

#include "csgo_GlowOverlay.h"

#include "addresses.h"
#include "SourceInterfaces.h"

#include "csgo_MemAlloc.h"

#include <shared/detours.h>

#include <map>


struct CCsgoCGlowOverlayMemLayout
{
	void * vtable_ptr;
	char _weDontCare_000[
		3 * 4 //m_vPos
			+ 4 //m_bDirectional
			+ 3 * 4 //m_vDirection
			+ 4 //m_bInSky
			+ 4 //m_skyObstructionScale
			+ 4 * (3 * 4 + 4 + 4 + 4) // CGlowSprite m_Sprites[MAX_SUN_LAYERS];
			+ 4 //m_nSprites
			+ 4 //m_flProxyRadius
			+ 4 //m_flHDRColorScale
	];

	float			m_flGlowObstructionScale;

	// ...
	// More we don't care about


};

bool csgo_CGlowOverlay_Hooks_Init(void);

class CCsgoCGlowOverlayFix
	: public ICsgoCGlowOverlayFix
	, public ITier0MemAllocFreeNotifyee
{
public:
	virtual void OnMainViewRenderBegin()
	{
		OutputDebugStringA("OnMainViewRenderBegin\n");

		csgo_CGlowOverlay_Hooks_Init();

		m_ActiveStream = 0;
	}

	virtual void OnStreamRenderViewBegin(CAfxRenderViewStream * idPtr)
	{
		m_ActiveStream = idPtr;

		std::map<void *, CGlowOverlaysCache>::iterator it = m_StreamsCache.find(idPtr);

		if (it == m_StreamsCache.end())
		{
			m_StreamsCache[idPtr].Copy(m_MainCache);
		}
	}

	virtual void OnStreamFinished(CAfxRenderViewStream * idPtr)
	{
		m_StreamsCache.erase(idPtr);
	}

	virtual void OnTier0MemAllocFree(void * pMem)
	{
		CCsgoCGlowOverlayMemLayout * instance = (CCsgoCGlowOverlayMemLayout *)pMem;

		OutputDebugStringA("OnCGlowOverlayDestruct\n");

		m_MainCache.OnGlowOverlayDestructed(instance);

		for (std::map<void *, CGlowOverlaysCache>::iterator it = m_StreamsCache.begin(); it != m_StreamsCache.end(); ++it)
			it->second.OnGlowOverlayDestructed(instance);
	}

	void OnCGlowOverlayDrawBefore(CCsgoCGlowOverlayMemLayout * instance)
	{
		OutputDebugStringA("OnCGlowOverlayDrawBefore\n");

		activeStreamValue = 0;

		if (m_MainCache.OnGlowOverlay(instance, &activeMainValue))
		{
			OutputDebugStringA("OnCGlowOverlayDrawBefore New\n");

			for (std::map<void *, CGlowOverlaysCache>::iterator it = m_StreamsCache.begin(); it != m_StreamsCache.end(); ++it)
			{
				float * streamValue;

				it->second.OnGlowOverlay(instance, &streamValue);

				if (it->first == m_ActiveStream)
				{
					activeStreamValue = streamValue;
				}
			}

			NotifiyOnTier0MemAllocFree(this, instance);
		}

		if (m_ActiveStream && !activeStreamValue)
		{
			m_StreamsCache[m_ActiveStream].OnGlowOverlay(instance, &activeStreamValue);
		}

		if (activeStreamValue)
		{
			instance->m_flGlowObstructionScale = *activeStreamValue; // set value from stream cache
		}

		instance->m_flGlowObstructionScale = 0.0f;
	}

	void OnCGlowOverlayDrawAfter(CCsgoCGlowOverlayMemLayout * instance)
	{
		if (activeStreamValue)
		{
			*activeStreamValue = instance->m_flGlowObstructionScale; // update stream cache value

			instance->m_flGlowObstructionScale = *activeMainValue; // restore main value

			activeStreamValue = 0;
		}
		else
		{
			*activeMainValue = instance->m_flGlowObstructionScale; // update main cache value
		}
	}

private:
	class CGlowOverlaysCache
	{
	public:
		void Copy(CGlowOverlaysCache & from)
		{
			for (std::map<CCsgoCGlowOverlayMemLayout *, float>::iterator it = from.m_Cache.begin(); it != from.m_Cache.end(); ++it)
			{
				m_Cache[it->first] = it->second;
			}
		}

		void OnGlowOverlayDestructed(CCsgoCGlowOverlayMemLayout * value)
		{
			if (m_Cache.find(value) == m_Cache.end()) OutputDebugStringA("OnGlowOverlayDestructed: Unknown CGlowOverlay!\n");

			m_Cache.erase(value);
		}

		/// <summary>Checks if the overlay is known (if not creats a new cache value for it from the current value)  and returns pointer on cache value.</summary>
		/// <returns>If new.</returns>
		bool OnGlowOverlay(CCsgoCGlowOverlayMemLayout * value, float ** outCacheValue)
		{
			std::map<CCsgoCGlowOverlayMemLayout *, float>::iterator it = m_Cache.find(value);

			if (it != m_Cache.end())
			{
				// Overlay known (not new).
				if (outCacheValue)
					*outCacheValue = &(it->second);
				return false;
			}

			// Overlay new:			 
			float * valuePtr = &(m_Cache[value] = value->m_flGlowObstructionScale); // also cache off the current value.
			if (outCacheValue)
				*outCacheValue = valuePtr;
			return true;
		}

	private:
		std::map<CCsgoCGlowOverlayMemLayout *, float> m_Cache;
	};

	void * m_ActiveStream = 0;
	CGlowOverlaysCache m_MainCache;
	std::map<void *, CGlowOverlaysCache> m_StreamsCache;
	float * activeMainValue;
	float * activeStreamValue;

} g_CsgoCGlowOverlayFix;

ICsgoCGlowOverlayFix * GetCsgoCGlowOverlayFix(void)
{
	return &g_CsgoCGlowOverlayFix;
}

typedef void(__stdcall *csgo_CGlowOverlay_Draw_t)(DWORD *this_ptr, bool bCacheFullSceneState);

csgo_CGlowOverlay_Draw_t g_csgo_CGlowOverlay_Draw;

void __stdcall touring_csgo_CGlowOverlay_Draw(DWORD * this_ptr, bool bCacheFullSceneState)
{
	g_CsgoCGlowOverlayFix.OnCGlowOverlayDrawBefore((CCsgoCGlowOverlayMemLayout *)this_ptr);

	g_csgo_CGlowOverlay_Draw(this_ptr, bCacheFullSceneState);

	g_CsgoCGlowOverlayFix.OnCGlowOverlayDrawAfter((CCsgoCGlowOverlayMemLayout *)this_ptr);
}

bool csgo_CGlowOverlay_Hooks_Init(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CGlowOverlay_Draw))
	{
		g_csgo_CGlowOverlay_Draw = (csgo_CGlowOverlay_Draw_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CGlowOverlay_Draw), (BYTE *)touring_csgo_CGlowOverlay_Draw, (int)AFXADDR_GET(csgo_CGlowOverlay_Draw_DSZ));

		firstResult = true;
	}

	return firstResult;
}