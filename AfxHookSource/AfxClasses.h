#pragma once

#include "AfxInterfaces.h"
#include "SourceInterfaces.h"

#include <set>
#include <map>
#include <atomic>
#include <mutex>
#include <shared_mutex>

#include <Windows.h>

// CAfxMaterialKey /////////////////////////////////////////////////////////////

class CAfxMaterialKey
{
public:
	CAfxMaterialKey(SOURCESDK::IMaterial_csgo * material);

	CAfxMaterialKey::CAfxMaterialKey(const CAfxMaterialKey & x);

	virtual ~CAfxMaterialKey();

private:
	/// <remarks>
	/// This is internal material object pointer based, meaning different references on the same material can have different keys
	/// </remarks>
	bool operator < (const CAfxMaterialKey & y) const;

public:
	SOURCESDK::IMaterial_csgo * GetMaterial() const;

protected:
	SOURCESDK::IMaterial_csgo * m_Material;
};


// CAfxOwnedMaterial ////////////////////////////////////////////////////////////

class CAfxOwnedMaterial : public CAfxMaterialKey
{
public:
	CAfxOwnedMaterial(SOURCESDK::IMaterial_csgo * material);
	virtual ~CAfxOwnedMaterial();

private:
	static std::map<SOURCESDK::IMaterial_csgo *, std::atomic_int> m_OwnedMaterials;
	static std::shared_timed_mutex m_OwnedMaterialsMutex;

	static void AddRef(SOURCESDK::IMaterial_csgo * material);
	static void Release(SOURCESDK::IMaterial_csgo * material);
};


// CAfxTrackedMaterial /////////////////////////////////////////////////////////

class CAfxTrackedMaterial;

class IAfxMaterialFree abstract
{
public:
	virtual void AfxMaterialFree(CAfxTrackedMaterial * material) abstract = 0;
};

class CAfxTrackedMaterial : public CAfxMaterialKey
{
public:
	//:192
	typedef void (__fastcall * MaterialHook_Free_t)(void* This, void* Edx);

	struct CMaterialDetours
	{
		//:034
		MaterialHook_Free_t InterlockedDecrement;
	};

	static CAfxTrackedMaterial * TrackMaterial(SOURCESDK::IMaterial_csgo * material);

	void AddNotifyee(IAfxMaterialFree * notifyee);
	void RemoveNotifyee(IAfxMaterialFree * notifyee);

	SOURCESDK::IMaterial_csgo * GetReplacement(void)
	{
		return m_Replacement;
	}

	void SetReplacement(SOURCESDK::IMaterial_csgo * value)
	{
		m_Replacement = value;
	}

protected:
	CAfxTrackedMaterial(SOURCESDK::IMaterial_csgo * material);

	virtual ~CAfxTrackedMaterial();

private:
	static std::map<SOURCESDK::IMaterial_csgo *, CAfxTrackedMaterial *> m_Trackeds;
	static std::shared_timed_mutex m_TrackedsMutex;

	static std::map<int *, CMaterialDetours> m_VtableMap;
	static std::shared_timed_mutex m_VtableMapMutex;

	static void HooKVtable(SOURCESDK::IMaterial_csgo * orgMaterial);

	static void __fastcall CAfxTrackedMaterial::Material_InterlockedDecrement(void* This, void* Edx);

	static void OnMaterialInterlockedDecrement(SOURCESDK::IMaterial_csgo * material);

	std::set<IAfxMaterialFree *> m_ThisNotifyees;
	SOURCESDK::IMaterial_csgo * m_Replacement = 0;
};
