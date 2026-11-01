#include "SceneActionFilter.h"

#include "../shared/StringTools.h"

#include <cstdlib>
#include <cstring>
#include <mutex>

// CSceneActionFilterList::Entry ///////////////////////////////////////////////

bool CSceneActionFilterList::Entry::Matches(const SceneActionFilterQuery& query) const {
	if (UseHandle && !(query.EntityHandle.has_value() && *query.EntityHandle == Handle)) return false;
	if (!MaterialName.empty() && !(query.MaterialName && StringWildCard1Matched(MaterialName.c_str(), query.MaterialName->c_str()))) return false;
	if (!ViewName.empty() && !(query.ViewName && StringWildCard1Matched(ViewName.c_str(), query.ViewName->c_str()))) return false;
	if (!ViewPass.empty() && !(query.ViewPass && StringWildCard1Matched(ViewPass.c_str(), query.ViewPass->c_str()))) return false;
	return true;
}

int CSceneActionFilterList::Entry::Compare(const Entry& o) const {
	if (UseHandle != o.UseHandle) return UseHandle ? 1 : -1;
	if (UseHandle && Handle != o.Handle) return Handle > o.Handle ? 1 : -1;
	if (int cmp = MaterialName.compare(o.MaterialName)) return cmp;
	if (int cmp = ViewName.compare(o.ViewName)) return cmp;
	if (int cmp = ViewPass.compare(o.ViewPass)) return cmp;
	if (Action != o.Action) return (int)Action > (int)o.Action ? 1 : -1;
	return 0;
}

// CSceneActionFilterList //////////////////////////////////////////////////////

bool CSceneActionFilterList::Match(const SceneActionFilterQuery& query, SceneActionFilterAction& outAction) const {
	for (const Entry& entry : m_Entries) {
		if (entry.Matches(query)) {
			outAction = entry.Action;
			return true;
		}
	}
	return false;
}

int CSceneActionFilterList::Compare(const CSceneActionFilterList& other) const {
	auto itL = m_Entries.begin();
	auto itR = other.m_Entries.begin();
	while (itL != m_Entries.end() && itR != other.m_Entries.end()) {
		if (int cmp = itL->Compare(*itR)) return cmp;
		++itL;
		++itR;
	}
	if (itL != m_Entries.end()) return 1;
	if (itR != other.m_Entries.end()) return -1;
	return 0;
}

const char* CSceneActionFilterList::ActionToString(SceneActionFilterAction action) {
	switch (action) {
	case SceneActionFilterAction::Draw: return "draw";
	case SceneActionFilterAction::Hide: return "hide";
	case SceneActionFilterAction::DepthPassesOnly: return "zonly";
	default: return "unknown";
	}
}

bool CSceneActionFilterList::TryParseAction(const char* value, SceneActionFilterAction& outAction) {
	if (0 == _stricmp(value, "draw") || 0 == _stricmp(value, "1")) { outAction = SceneActionFilterAction::Draw; return true; }
	if (0 == _stricmp(value, "hide") || 0 == _stricmp(value, "0")) { outAction = SceneActionFilterAction::Hide; return true; }
	if (0 == _stricmp(value, "zonly")) { outAction = SceneActionFilterAction::DepthPassesOnly; return true; }
	return false;
}

bool CSceneActionFilterList::ParseEx(advancedfx::ICommandArgs* args, Entry& outEntry) {
	int argC = args->ArgC();

	if (argC <= 1) {
		advancedfx::Message(
			"Usage:\n"
			"%s <option> <option> ...\n"
			"\n"
			"<option> can be:\n"
			"\"handle=<handleNumber>\" (of entity)\n"
			"\"name=<wildCardString>\" (of material)\n"
			"\"viewName=<wildCardString>\" (of scene layer context)\n"
			"\"viewPass=<wildCardString>\" (of scene layer context)\n"
			"\"action=draw|hide|zonly\"\n"
			"\n"
			"- The action option must be given!\n"
			"- <wildCardString> is a string without quotes, where \\* is the wildcard and \\\\ is \\\n"
			"- Any option except action that is not given will be treated as if it doesn't matter for a match.\n"
			, args->ArgV(0)
		);
		return false;
	}

	Entry entry;
	bool hasAction = false;

	for (int i = 1; i < argC; ++i) {
		std::string sArg(args->ArgV(i));

		size_t posDelimiter = sArg.find_first_of('=');
		if (posDelimiter == std::string::npos) {
			advancedfx::Warning("Error: \"%s\" is not a valid option (expected key=value)!\n", sArg.c_str());
			return false;
		}

		std::string sKey = sArg.substr(0, posDelimiter);
		std::string sValue = sArg.substr(posDelimiter + 1);
		const char* key = sKey.c_str();

		if (!_stricmp("handle", key)) {
			entry.UseHandle = true;
			entry.Handle = (uint32_t)strtoul(sValue.c_str(), nullptr, 0);
		}
		else if (!_stricmp("name", key)) {
			entry.MaterialName = sValue;
		}
		else if (!_stricmp("viewName", key)) {
			entry.ViewName = sValue;
		}
		else if (!_stricmp("viewPass", key)) {
			entry.ViewPass = sValue;
		}
		else if (!_stricmp("action", key)) {
			if (!TryParseAction(sValue.c_str(), entry.Action)) {
				advancedfx::Warning("Error: \"%s\" is not a valid action (draw|hide|zonly)!\n", sValue.c_str());
				return false;
			}
			hasAction = true;
		}
		else {
			advancedfx::Warning("Error: %s is not a valid option!\n", sKey.c_str());
			return false;
		}
	}

	if (!hasAction) {
		advancedfx::Warning("Error: Action must be given!\n");
		return false;
	}

	outEntry = std::move(entry);
	return true;
}

void CSceneActionFilterList::Print() const {
	int id = 0;
	for (const Entry& entry : m_Entries) {
		std::string handleStr = entry.UseHandle ? std::to_string(entry.Handle) : "\\*";
		advancedfx::Message(
			"id=%i, \"handle=%s\", \"name=%s\", \"viewName=%s\", \"viewPass=%s\", \"action=%s\"\n",
			id,
			handleStr.c_str(),
			entry.MaterialName.empty() ? "\\*" : entry.MaterialName.c_str(),
			entry.ViewName.empty() ? "\\*" : entry.ViewName.c_str(),
			entry.ViewPass.empty() ? "\\*" : entry.ViewPass.c_str(),
			ActionToString(entry.Action)
		);
		++id;
	}
}

bool CSceneActionFilterList::Remove(int id) {
	int curId = 0;
	for (auto it = m_Entries.begin(); it != m_Entries.end(); ++it, ++curId) {
		if (curId == id) {
			m_Entries.erase(it);
			return true;
		}
	}

	advancedfx::Warning("Error: %i is not a valid actionFilter id!\n", id);
	return false;
}

bool CSceneActionFilterList::Move(int id, int beforeId) {
	if (beforeId < 0 || beforeId > (int)m_Entries.size()) {
		advancedfx::Warning("Error: %i is not in valid range for <beforeId>!\n", beforeId);
		return false;
	}

	auto itFrom = m_Entries.begin();
	for (int curId = 0; curId < id && itFrom != m_Entries.end(); ++curId) ++itFrom;
	if (id < 0 || itFrom == m_Entries.end()) {
		advancedfx::Warning("Error: %i is not a valid actionFilter id!\n", id);
		return false;
	}

	auto itTo = m_Entries.begin();
	for (int curId = 0; curId < beforeId; ++curId) ++itTo;

	// splice handles itTo == itFrom (no-op) and does not invalidate iterators.
	m_Entries.splice(itTo, m_Entries, itFrom);
	return true;
}

bool CSceneActionFilterList::Console(advancedfx::ICommandArgs* args) {
	int argC = args->ArgC();
	const char* prefix = args->ArgV(0);

	if (2 <= argC) {
		const char* cmd1 = args->ArgV(1);

		if (!_stricmp(cmd1, "addEx")) {
			advancedfx::CSubCommandArgs subArgs(args, 2); // ArgV(0) = "<prefix> addEx", ArgV(1..) = options
			Entry entry;
			if (ParseEx(&subArgs, entry)) {
				m_Entries.push_back(std::move(entry));
				return true;
			}
			return false;
		}
		else if (!_stricmp(cmd1, "print")) {
			Print();
			return false;
		}
		else if (!_stricmp(cmd1, "remove")) {
			if (3 <= argC) return Remove(atoi(args->ArgV(2)));
			advancedfx::Message("%s remove <id> - Removes filter with id number <id>.\n", prefix);
			return false;
		}
		else if (!_stricmp(cmd1, "move")) {
			if (4 <= argC) return Move(atoi(args->ArgV(2)), atoi(args->ArgV(3)));
			advancedfx::Message(
				"%s move <id> <beforeId> - Moves filter with id number <id> before filter with id <beforeId> "
				"(<beforeId> can be 1 greater than the last id to move to the end).\n", prefix);
			return false;
		}
		else if (!_stricmp(cmd1, "clear")) {
			bool modified = !m_Entries.empty();
			Clear();
			return modified;
		}
	}

	advancedfx::Message(
		"%s addEx [...] - Add a new filter.\n"
		"%s print - Print current filters.\n"
		"%s remove [...] - Remove a filter.\n"
		"%s move [...] - Move filter (change priority).\n"
		"%s clear - Remove all filters.\n"
		, prefix, prefix, prefix, prefix, prefix
	);
	return false;
}

// CActiveSceneActionFilter ////////////////////////////////////////////////////

void CActiveSceneActionFilter::Set(std::shared_ptr<const CSceneActionFilterList> list) {
	std::unique_lock<std::shared_mutex> lock(m_Mutex);
	m_List = std::move(list);
	m_Active = m_List && !m_List->Empty();
}

void CActiveSceneActionFilter::Clear() {
	std::unique_lock<std::shared_mutex> lock(m_Mutex);
	m_List.reset();
	m_Active = false;
}

bool CActiveSceneActionFilter::Match(const SceneActionFilterQuery& query, SceneActionFilterAction& outAction) const {
	if (!IsActive()) return false;

	std::shared_lock<std::shared_mutex> lock(m_Mutex);
	return m_List && m_List->Match(query, outAction);
}
