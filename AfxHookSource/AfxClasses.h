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

	/// <remarks>
	/// This is internal material object pointer based, meaning different references on the same material can have different keys
	/// </remarks>
	bool operator < (const CAfxMaterialKey & y) const;

	SOURCESDK::IMaterial_csgo * GetMaterial() const;

protected:
	SOURCESDK::IMaterial_csgo * m_Material;
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
	typedef void (_stdcall * MaterialHook_Free_t)(DWORD *this_ptr);

	struct CMaterialDetours
	{
		//:034
		MaterialHook_Free_t InterlockedDecrement;
	};

	/// <remarks>Hint: Give afxLastEngineRelease as 0 if you want just fast comparision on a map or s.th.</remarks>
	CAfxTrackedMaterial(SOURCESDK::IMaterial_csgo * material, IAfxMaterialFree * notifyee);

	virtual ~CAfxTrackedMaterial();

	void AfxMaterialFree(void);

private:
	static std::map<SOURCESDK::IMaterial_csgo *, std::set<CAfxTrackedMaterial *>> m_Notifyees;
	static std::mutex m_NotifyeesMutex;

	static std::map<int *, CMaterialDetours> m_VtableMap;
	static std::shared_timed_mutex m_VtableMapMutex;

	static void AddNotifyee(SOURCESDK::IMaterial_csgo * material, CAfxTrackedMaterial * notifyee);
	static void RemoveNotifyee(SOURCESDK::IMaterial_csgo * material, CAfxTrackedMaterial * notifyee);

	static void HooKVtable(SOURCESDK::IMaterial_csgo * orgMaterial);

	static void __stdcall CAfxTrackedMaterial::Material_InterlockedDecrement(DWORD *this_ptr);

	static void OnMaterialInterlockedDecrement(SOURCESDK::IMaterial_csgo * material);

	IAfxMaterialFree * m_Notifyee;
};


// CAfxTrackedMaterial /////////////////////////////////////////////////////////

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
