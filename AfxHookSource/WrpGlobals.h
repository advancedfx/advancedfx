#pragma once

class WrpGlobals abstract
{
public:
	virtual void frametime_set(float value) { } // csgo only
	virtual float frametime_get(void) { return 0; } // csgo only
	virtual void curtime_set(float value) { } // csgo only
	virtual int maxclients_get(void) { return 0; } // csgo and css only

	virtual int framecount_get(void) abstract = 0;
	virtual float absoluteframetime_get(void) abstract = 0;
	virtual float curtime_get(void) abstract = 0;
	virtual float interval_per_tick_get(void) abstract = 0;
	virtual float interpolation_amount_get(void) abstract = 0;

	virtual void * GetRaw() {
		return nullptr;
	}
};

class WrpGlobalsCsGo : public WrpGlobals
{
public:
	WrpGlobalsCsGo(void * pGlobals);

	virtual void frametime_set(float value);
	virtual float frametime_get(void);
	virtual void curtime_set(float value);
	virtual int maxclients_get(void);

	virtual int framecount_get(void);
	virtual float absoluteframetime_get(void);
	virtual float curtime_get(void);
	virtual float interval_per_tick_get(void);
	virtual float interpolation_amount_get(void);

	virtual void * GetRaw() {
		return m_pGlobals;
	}

private:
	void * m_pGlobals;
};

class WrpGlobalsOther : public WrpGlobals
{
public:
	WrpGlobalsOther(void * pGlobals);

	virtual int framecount_get(void);
	virtual float absoluteframetime_get(void);
	virtual float curtime_get(void);
	virtual float interval_per_tick_get(void);
	virtual float interpolation_amount_get(void);

	virtual void * GetRaw() {
		return m_pGlobals;
	}

protected:
	void * m_pGlobals;
};

class WrpGlobalsCss : public WrpGlobalsOther
{
public:
	WrpGlobalsCss(void * pGlobals);

	virtual int maxclients_get(void);
};

class WrpGlobalsGarrysmod : public WrpGlobals
{
public:
	WrpGlobalsGarrysmod(void * pGlobals);

	virtual int framecount_get(void);
	virtual float absoluteframetime_get(void);
	virtual float curtime_get(void);
	virtual float interval_per_tick_get(void);
	virtual float interpolation_amount_get(void);

	virtual void * GetRaw() {
		return m_pGlobals;
	}

protected:
	void * m_pGlobals;
};