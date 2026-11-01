#include "EntMatPicker.h"

#include "../shared/AfxConsole.h"
#include "../deps/release/prop/cs2/sdk_src/public/const.h"

#include <mutex>

bool CEntMatPicker::Pick(bool pickEntityNotMaterial, bool wasVisible, const DescribeHandle_t& describeHandle) {
	std::unique_lock<std::shared_mutex> lock(m_Mutex);

	if (!m_Started) {
		m_Started = true;
		m_Collecting = true;
	}
	else {
		m_Collecting = false;

		if (m_PickingEntities) {
			std::set<std::string, std::less<>> usedMats;
			size_t index = 0;

			for (auto it = m_Entities.begin(); it != m_Entities.end(); ) {
				if (IsOdd(it->second.Index) == wasVisible) {
					it = m_Entities.erase(it);
				}
				else {
					usedMats.insert(it->second.Materials.begin(), it->second.Materials.end());
					it->second.Index = index;
					++index;
					++it;
				}
			}

			for (auto it = m_Materials.begin(); it != m_Materials.end(); ) {
				if (usedMats.end() != usedMats.find(it->first)) ++it;
				else it = m_Materials.erase(it);
			}
		}

		if (m_PickingMaterials) {
			std::set<uint32_t> usedEnts;
			size_t index = 0;

			for (auto it = m_Materials.begin(); it != m_Materials.end(); ) {
				if (IsOdd(it->second.Index) == wasVisible) {
					it = m_Materials.erase(it);
				}
				else {
					usedEnts.insert(it->second.Entities.begin(), it->second.Entities.end());
					it->second.Index = index;
					++index;
					++it;
				}
			}

			for (auto it = m_Entities.begin(); it != m_Entities.end(); ) {
				if (usedEnts.end() != usedEnts.find(it->first)) ++it;
				else it = m_Entities.erase(it);
			}
		}

		bool determinedEntities = m_Entities.size() <= 1;
		bool determinedMaterials = m_Materials.size() <= 1;

		if (pickEntityNotMaterial ? determinedEntities : determinedMaterials) {
			m_PickingEntities = false;
			m_PickingMaterials = false;
			UpdateActive_Locked();

			Print_Locked(describeHandle);
			if (pickEntityNotMaterial)
				advancedfx::Warning("==== Entity%s determined! ====\n", determinedMaterials ? " and Material" : "");
			else
				advancedfx::Warning("==== %sMaterial determined! ====\n", determinedEntities ? "Entity and " : "");
			return true;
		}

		advancedfx::Message("Picker: %u entities and %u materials left.\n", (unsigned int)m_Entities.size(), (unsigned int)m_Materials.size());
	}

	m_PickingEntities = pickEntityNotMaterial;
	m_PickingMaterials = !pickEntityNotMaterial;
	UpdateActive_Locked();
	return false;
}

void CEntMatPicker::Print(const DescribeHandle_t& describeHandle) const {
	std::shared_lock<std::shared_mutex> lock(m_Mutex);
	Print_Locked(describeHandle);
}

bool CEntMatPicker::Stop() {
	std::unique_lock<std::shared_mutex> lock(m_Mutex);

	bool wasStarted = m_Started;

	m_Materials.clear();
	m_Entities.clear();
	m_Started = false;
	m_Collecting = false;
	m_PickingEntities = false;
	m_PickingMaterials = false;
	UpdateActive_Locked();

	return wasStarted;
}

bool CEntMatPicker::GetHidden(const char* materialName, uint32_t handle) {
	if (!IsActive()) return false;

	if (nullptr == materialName) materialName = "";

	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);

		if (!m_Collecting) return FindHidden_Locked(materialName, handle);

		// Fast path: combination is already known, nothing to insert.
		auto itMat = m_Materials.find(materialName);
		auto itEnt = m_Entities.find(handle);
		if (itMat != m_Materials.end() && itEnt != m_Entities.end()
			&& itMat->second.Entities.end() != itMat->second.Entities.find(handle)
			&& itEnt->second.Materials.end() != itEnt->second.Materials.find(materialName)
		) {
			return IsHidden_Locked(itMat->second, itEnt->second);
		}
	}

	std::unique_lock<std::shared_mutex> lock(m_Mutex);

	// State might have changed while we didn't hold the lock:
	if (!m_Collecting) return FindHidden_Locked(materialName, handle);

	auto itMat = m_Materials.find(materialName);
	if (itMat == m_Materials.end()) {
		itMat = m_Materials.emplace(std::piecewise_construct, std::forward_as_tuple(materialName), std::forward_as_tuple(m_Materials.size())).first;
	}
	itMat->second.Entities.insert(handle);

	auto itEnt = m_Entities.find(handle);
	if (itEnt == m_Entities.end()) {
		itEnt = m_Entities.emplace(std::piecewise_construct, std::forward_as_tuple(handle), std::forward_as_tuple(m_Entities.size())).first;
	}
	if (itEnt->second.Materials.end() == itEnt->second.Materials.find(materialName)) {
		itEnt->second.Materials.emplace(materialName);
	}

	return IsHidden_Locked(itMat->second, itEnt->second);
}

bool CEntMatPicker::FindHidden_Locked(const char* materialName, uint32_t handle) const {
	if (m_PickingMaterials) {
		auto itMat = m_Materials.find(materialName);
		if (itMat != m_Materials.end() && IsOdd(itMat->second.Index)) return true;
	}
	if (m_PickingEntities) {
		auto itEnt = m_Entities.find(handle);
		if (itEnt != m_Entities.end() && IsOdd(itEnt->second.Index)) return true;
	}
	return false;
}

bool CEntMatPicker::IsHidden_Locked(const MatValue& matValue, const EntValue& entValue) const {
	return (m_PickingEntities && IsOdd(entValue.Index))
		|| (m_PickingMaterials && IsOdd(matValue.Index));
}

void CEntMatPicker::Print_Locked(const DescribeHandle_t& describeHandle) const {
	// Output is in the format of "mirv_streams edit <streamName> actionFilter addEx", so it can be copied from.
	advancedfx::Message("---- Materials: ----\n");
	for (auto it = m_Materials.begin(); it != m_Materials.end(); ++it) {
		const char* state = m_PickingMaterials && IsOdd(it->second.Index) ? "hidden" : "visible";
		if (it->first.empty())
			advancedfx::Message("(no material) (%s)\n", state);
		else
			advancedfx::Message("\"name=%s\" (%s)\n", it->first.c_str(), state);
	}
	advancedfx::Message("---- Entities: ----\n");
	for (auto it = m_Entities.begin(); it != m_Entities.end(); ++it) {
		const char* state = m_PickingEntities && IsOdd(it->second.Index) ? "hidden" : "visible";
		if (it->first == SOURCESDK_CS2_INVALID_EHANDLE_INDEX) {
			advancedfx::Message("(no entity) (%s)\n", state);
		}
		else {
			std::string description;
			if (describeHandle) description = describeHandle(it->first);
			advancedfx::Message("\"handle=%u\" %s%s(%s)\n", it->first, description.c_str(), description.empty() ? "" : " ", state);
		}
	}
	advancedfx::Message("---- END ----\n");
}

void CEntMatPicker::UpdateActive_Locked() {
	m_Active.store(m_Started && (m_Collecting || m_PickingEntities || m_PickingMaterials), std::memory_order_release);
}
