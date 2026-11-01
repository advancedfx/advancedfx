#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>

// Port of the "mirv_streams edit <streamName> picker" logic from AfxHookSource (AfxStreams.cpp).
//
// This module is intentionally independent of the rest of AfxHookSource2 (no scene system,
// no entity system), entities are identified by their raw handle number and materials by name.
//
// Thread safety:
// - GetHidden may be called concurrently from any number of render threads.
// - Pick / Print / Stop are meant to be called from the main thread (console commands),
//   but are safe to call from any thread.
// - While collecting, a draw that was already seen only takes a shared lock, only new
//   entity / material combinations take the exclusive lock.

class CEntMatPicker {
public:
	/// <summary>Optionally returns additional info about an entity (e.g. "(className=...)") for printing, can return an empty string.</summary>
	typedef std::function<std::string(uint32_t handle)> DescribeHandle_t;

	/// <summary>Cheap lock free check, false if GetHidden would return false for everything.</summary>
	bool IsActive() const { return m_Active.load(std::memory_order_acquire); }

	/// <summary>Starts picking (if not started) or tells the picker whether the entity / material
	/// it was asking about was visible (wasVisible) or not.</summary>
	/// <returns>True if the pick was determined (result has been printed and nothing is hidden anymore).</returns>
	bool Pick(bool pickEntityNotMaterial, bool wasVisible, const DescribeHandle_t& describeHandle);

	void Print(const DescribeHandle_t& describeHandle) const;

	/// <returns>True if the picker was running.</returns>
	bool Stop();

	/// <param name="materialName">Can be nullptr.</param>
	/// <param name="handle">Raw entity handle number (can be the invalid handle).</param>
	bool GetHidden(const char* materialName, uint32_t handle);

private:
	struct MatValue {
		size_t Index;
		std::set<uint32_t> Entities;

		MatValue(size_t index) : Index(index) {}
	};

	struct EntValue {
		size_t Index;
		std::set<std::string, std::less<>> Materials;

		EntValue(size_t index) : Index(index) {}
	};

	static bool IsOdd(size_t index) { return 1 == (index & 0x1); }

	// These require m_Mutex to be locked (shared or exclusive):
	bool FindHidden_Locked(const char* materialName, uint32_t handle) const;
	bool IsHidden_Locked(const MatValue& matValue, const EntValue& entValue) const;
	void Print_Locked(const DescribeHandle_t& describeHandle) const;

	// Requires m_Mutex to be locked exclusively:
	void UpdateActive_Locked();

	mutable std::shared_mutex m_Mutex;

	std::map<std::string, MatValue, std::less<>> m_Materials;
	std::map<uint32_t, EntValue> m_Entities;

	bool m_Started = false;
	bool m_Collecting = false;
	bool m_PickingEntities = false;
	bool m_PickingMaterials = false;

	std::atomic<bool> m_Active{false};
};
