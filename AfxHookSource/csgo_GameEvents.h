#pragma once

#include "SourceInterfaces.h"

#include <string>
#include <set>
#include <map>

class IAfxGameEventListener
{
public:
	virtual void FireEvent(SOURCESDK::CSGO::CGameEvent * gameEvent) = 0;
};

class CAfxGameEventListener : public IAfxGameEventListener
{
public:
	virtual void FireEvent(SOURCESDK::CSGO::CGameEvent * gameEvent)
	{
		if (nullptr == gameEvent) return;

		const char * eventName = gameEvent->GetName();

		if (!m_WhiteList.empty() && m_WhiteList.end() == m_WhiteList.find(eventName))
			return;

		if (!m_BlackList.empty() && m_BlackList.end() != m_BlackList.find(eventName))
			return;

		FireHandledEvent(gameEvent);
	}

	virtual void Restart()
	{
		ClearWhiteList();
		ClearBlackList();
	}

	void ClearWhiteList()
	{
		m_WhiteList.clear();
	}

	void WhiteList(const char * eventName)
	{
		if (m_WhiteList.end() == m_WhiteList.find(eventName)) m_WhiteList.emplace(eventName);
	}

	void UnWhiteList(const char * eventName)
	{
		auto it = m_WhiteList.find(eventName);
		if (it != m_WhiteList.end()) m_WhiteList.erase(it);
	}

	void ClearBlackList()
	{
		m_BlackList.clear();
	}

	void BlackList(const char * eventName)
	{
		if (m_BlackList.end() == m_BlackList.find(eventName)) m_BlackList.emplace(eventName);
	}

	void UnBlackList(const char * eventName)
	{
		auto it = m_BlackList.find(eventName);
		if (it != m_BlackList.end()) m_BlackList.erase(it);
	}


protected:
	virtual void FireHandledEvent(SOURCESDK::CSGO::CGameEvent * gameEvent) = 0;

private:
	std::set<std::string> m_BlackList;
	std::set<std::string> m_WhiteList;
};

class CAfxGameEventListenerSerialzer : public CAfxGameEventListener
{
public:
	enum EnrichmentType_e {
		EnrichmentType_None = 0,
		EnrichmentType_UseridWithSteamId = 1 << 0,
		EnrichmentType_EntnumWithOrigin = 1 << 1,
		EnrichmentType_EntnumWithAngles = 1 << 2,
		EnrichmentType_UseridWithEyePosition = 1 << 3,
		EnrichmentType_UseridWithEyeAngels = 1 << 4
	};

	bool TransmitClientTime = false;
	bool TransmitTick = false;
	bool TransmitSystemTime = false;

	virtual void Restart() override
	{
		TransmitClientTime = false;
		TransmitTick = false;
		TransmitSystemTime = false;

		ClearEnrichments();

		m_KnownEventIds.clear();

		CAfxGameEventListener::Restart();
	}

	void ClearEnrichments()
	{
		m_Enrichments.clear();
	}

	void Enrich(const char* eventName, const char* eventProperty, bool enable, EnrichmentType_e enrichmentType)
	{
		if (enable)
			m_Enrichments[eventName][eventProperty] |= enrichmentType;
		else {
			auto& curType = m_Enrichments[eventName][eventProperty];
			curType &= ~(unsigned int)enrichmentType;
			if (EnrichmentType_None == curType)
			{
				auto& enrichment = m_Enrichments[eventName];
				enrichment.erase(eventProperty);
				if (enrichment.empty())
					m_Enrichments.erase(eventName);
			}
		}
	}

	void EnrichSet(const char* eventName, const char* eventProperty, unsigned int enrichmentType)
	{
		if (EnrichmentType_None != enrichmentType)
			m_Enrichments[eventName][eventProperty] = enrichmentType;
		else {
			auto& enrichment = m_Enrichments[eventName];
			enrichment.erase(eventProperty);
			if (enrichment.empty())
				m_Enrichments.erase(eventName);
		}
	}

	void EnrichUseridWithSteamId(const char * eventName, const char * eventProperty, bool enable = true)
	{
		Enrich(eventName, eventProperty, enable, EnrichmentType_UseridWithSteamId);
	}

	void Enrich_EntnumWithOrigin(const char * eventName, const char * eventProperty, bool enable = true)
	{
		Enrich(eventName, eventProperty, enable, EnrichmentType_EntnumWithOrigin);
	}

	void Enrich_EntnumWithAngles(const char * eventName, const char * eventProperty, bool enable = true)
	{
		Enrich(eventName, eventProperty, enable, EnrichmentType_EntnumWithAngles);
	}

	void Enrich_UseridWithEyePosition(const char * eventName, const char * eventProperty, bool enable = true)
	{
		Enrich(eventName, eventProperty, enable, EnrichmentType_UseridWithEyePosition);
	}

	void Enrich_UseridWithEyeAngels(const char * eventName, const char * eventProperty, bool enable = true)
	{
		Enrich(eventName, eventProperty, enable, EnrichmentType_UseridWithEyeAngels);
	}

protected:

	std::map<std::string, std::map<std::string, unsigned int>> m_Enrichments;

	virtual void FireHandledEvent(SOURCESDK::CSGO::CGameEvent * gameEvent) override;

	/// <returns>Returns false if to abort serialization, otherwise true.</returns>
	virtual bool BeginSerialize() = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void EndSerialize() = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteCString(const char * value) = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteFloat(float value) = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteLong(long value) = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteShort(short value) = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteByte(char value) = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteBoolean(bool value) = 0;

	/// <remarks>Only called if BeginSerialize returned true.</remarks>
	virtual void WriteUInt64(unsigned __int64 value) = 0;

private:
	std::set<int> m_KnownEventIds;
};

class CAfxGameEvents
{
public:
	void AddListener(IAfxGameEventListener * listener);

	void RemoveListener(IAfxGameEventListener * listener)
	{
		auto it = m_Listeners.find(listener);

		if(it != m_Listeners.end())
			m_Listeners.erase(listener);
	}

	void FireEvent(SOURCESDK::CSGO::CGameEvent * gameEvent);

private:
	std::set<IAfxGameEventListener *> m_Listeners;
};

extern CAfxGameEvents g_AfxGameEvents;
