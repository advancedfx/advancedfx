#pragma once

#include "../shared/AfxConsole.h"

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>

// Simplified port of the "mirv_streams ... actionFilter addEx" logic from AfxHookSource (AfxStreams.cpp).
//
// This module is intentionally independent of the rest of AfxHookSource2 (no scene system,
// no entity system, no stream settings).
//
// - CSceneActionFilterList is a plain copyable value type, one lives in each CStreamSettings.
// - CActiveSceneActionFilter is the thread safe slot the scene system queries; the stream that
//   is currently rendered copies its list into it (Set) and clears it when done (Clear).
//
// Filters are evaluated in list order, the first match wins. If nothing matches, Match
// returns false and the caller is expected to fall through to its regular logic.

enum class SceneActionFilterAction {
	Draw,
	Hide,
	DepthPassesOnly
};

/// <summary>Data of the current draw call. Everything is optional, absent data can never satisfy a criterion.</summary>
struct SceneActionFilterQuery {
	/// <summary>Material name or nullptr if not available.</summary>
	const std::string* MaterialName = nullptr;
	/// <summary>Raw entity handle number if available.</summary>
	std::optional<uint32_t> EntityHandle;
	/// <summary>Scene layer context view name or nullptr if not available.</summary>
	const std::string* ViewName = nullptr;
	/// <summary>Scene layer context view pass or nullptr if not available.</summary>
	const std::string* ViewPass = nullptr;
};

class CSceneActionFilterList {
public:
	bool Empty() const { return m_Entries.empty(); }

	/// <summary>Finds the first matching filter. Returns false if none matched (fall through).</summary>
	bool Match(const SceneActionFilterQuery& query, SceneActionFilterAction& outAction) const;

	/// <summary>Total order, for CStreamSettings::CompareRenderPass.</summary>
	int Compare(const CSceneActionFilterList& other) const;

	/// <summary>Handles the arguments following "actionFilter". Same convention as the other
	/// sub commands: args->ArgV(0) is the command prefix (e.g. "mirv_streams edit name actionFilter"),
	/// args->ArgV(1) is the subcommand (addEx, print, remove, move, clear).
	/// Returns true if the list was modified.</summary>
	bool Console(advancedfx::ICommandArgs* args);

	void Clear() { m_Entries.clear(); }

private:
	struct Entry {
		bool UseHandle = false;
		uint32_t Handle = 0;
		// Wildcard masks (\* = wildcard, \\ = \), empty = don't care.
		std::string MaterialName;
		std::string ViewName;
		std::string ViewPass;
		SceneActionFilterAction Action = SceneActionFilterAction::Draw;

		bool Matches(const SceneActionFilterQuery& query) const;
		int Compare(const Entry& other) const;
	};

	static bool ParseEx(advancedfx::ICommandArgs* args, Entry& outEntry);
	static const char* ActionToString(SceneActionFilterAction action);
	static bool TryParseAction(const char* value, SceneActionFilterAction& outAction);

	void Print() const;
	bool Remove(int id);
	bool Move(int id, int beforeId);

	std::list<Entry> m_Entries;
};

/// <summary>Thread safe holder of the filter list that is currently in effect (queried from render threads).</summary>
class CActiveSceneActionFilter {
public:
	/// <summary>Cheap lock free check, so callers can skip building the query if there is nothing to match.</summary>
	bool IsActive() const { return m_Active.load(std::memory_order_relaxed); }

	/// <summary>Makes the list the active one. No copy, the list is shared and must not be modified
	/// afterwards (hence const; modify a copy and Set that instead).</summary>
	void Set(std::shared_ptr<const CSceneActionFilterList> list);
	void Clear();

	bool Match(const SceneActionFilterQuery& query, SceneActionFilterAction& outAction) const;

private:
	mutable std::shared_mutex m_Mutex;
	std::shared_ptr<const CSceneActionFilterList> m_List;
	std::atomic<bool> m_Active{false};
};
