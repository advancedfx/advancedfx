#include "stdafx.h"

#include "WrpGlobals.h"

#include "addresses.h"

// WrpGlobalsCsGo //////////////////////////////////////////////////////////////

WrpGlobalsCsGo::WrpGlobalsCsGo(void * pGlobals)
{
	m_pGlobals = pGlobals;
}

float WrpGlobalsCsGo::frametime_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +0x14);
}

void WrpGlobalsCsGo::frametime_set(float value)
{
	*(float *)((unsigned char *)m_pGlobals + 0x14) = value;
}

int WrpGlobalsCsGo::maxclients_get(void)
{
	return *(int *)((unsigned char *)m_pGlobals + 0x18);
}

int WrpGlobalsCsGo::framecount_get(void)
{
	return *(int *)((unsigned char *)m_pGlobals + 0x4);
}

float WrpGlobalsCsGo::absoluteframetime_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(cstrike_gpGlobals_OFS_absoluteframetime));
}

void WrpGlobalsCsGo::curtime_set(float value)
{
	*(float *)((unsigned char *)m_pGlobals + AFXADDR_GET(csgo_gpGlobals_OFS_curtime)) = value;
}

float WrpGlobalsCsGo::curtime_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(csgo_gpGlobals_OFS_curtime));
}

float WrpGlobalsCsGo::interval_per_tick_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(csgo_gpGlobals_OFS_interval_per_tick));
}

float WrpGlobalsCsGo::interpolation_amount_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(csgo_gpGlobals_OFS_interpolation_amount));
}



// WrpGlobalsOther /////////////////////////////////////////////////////////////

WrpGlobalsOther::WrpGlobalsOther(void * pGlobals)
{
	m_pGlobals = pGlobals;
}

int WrpGlobalsOther::framecount_get(void)
{
	return *(int *)((unsigned char *)m_pGlobals + 0x4);
}

float WrpGlobalsOther::absoluteframetime_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(cstrike_gpGlobals_OFS_absoluteframetime));
}

float WrpGlobalsOther::curtime_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(cstrike_gpGlobals_OFS_curtime));
}

float WrpGlobalsOther::interval_per_tick_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(cstrike_gpGlobals_OFS_interval_per_tick));
}

float WrpGlobalsOther::interpolation_amount_get(void)
{
	return *(float *)((unsigned char *)m_pGlobals +AFXADDR_GET(cstrike_gpGlobals_OFS_interpolation_amount));
}

// WrpGlobalsCss ///////////////////////////////////////////////////////////////

WrpGlobalsCss::WrpGlobalsCss(void * pGlobals):WrpGlobalsOther(pGlobals) {}

int WrpGlobalsCss::maxclients_get(void)
{
	return *(int *)((unsigned char *)m_pGlobals + 5*4);
}

