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

	/// <remarks>On Drawing thread only.</remarks>
	static CAfxTrackedMaterial * TrackMaterial(SOURCESDK::IMaterial_csgo * material);

	/// <remarks>Threadsafe.</remarks>
	void AddRef();

	/// <remarks>Threadsafe.</remarks>
	void Release();

	/// <remarks>On Drawing thread only.</remarks>
	void AddNotifyee(IAfxMaterialFree * notifyee);

	/// <remarks>Threadsafe.</remarks>
	void RemoveNotifyee(IAfxMaterialFree * notifyee);

	// TODO: move this elsewhere
	/// <remarks>On Drawing thread only.</remarks>
	SOURCESDK::IMaterial_csgo * GetReplacement(void)
	{
		return m_Replacement;
	}

	// TODO: move this elsewhere
	/// <remarks>On Drawing thread only.</remarks>
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

	std::atomic_int m_RefCount = 1;

	std::mutex m_ThisNotifyeesMutex;
	std::set<IAfxMaterialFree *> m_ThisNotifyees;
	bool m_Deleting = false;

	SOURCESDK::IMaterial_csgo * m_Replacement = 0;

	void Delete();
};

class CAfxTrackedMaterialRef
{
public:
	CAfxTrackedMaterialRef(CAfxTrackedMaterial* trackedMaterial)
		: m_TrackedMaterial(trackedMaterial)
	{
		m_TrackedMaterial->AddRef();
	}

	CAfxTrackedMaterialRef(const CAfxTrackedMaterialRef& trackedMaterialRef)
		: m_TrackedMaterial(trackedMaterialRef.m_TrackedMaterial)
	{
		m_TrackedMaterial->AddRef();
	}

	~CAfxTrackedMaterialRef()
	{
		m_TrackedMaterial->Release();
	}

	bool operator < (const CAfxTrackedMaterialRef& y) const
	{
		return m_TrackedMaterial < y.m_TrackedMaterial;
	}

	CAfxTrackedMaterial* Get() const {
		return m_TrackedMaterial;
	}

private:
	CAfxTrackedMaterial* m_TrackedMaterial;
};
