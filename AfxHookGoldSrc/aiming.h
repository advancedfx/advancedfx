#pragma once

#ifdef min
#undef min
#undef max
#endif

#include <list>
#include <algorithm>

#include <hlsdk.h>

class Aiming
{
private:
	enum ENT_STATE { ES_NORMAL, ES_TARGET, ES_DEAD };

private:
	std::list<int> m_AimLayers;
	bool m_Awake[2];
	float m_D0[2];
	float m_D1[2];
	double m_LastAimTime;

	int m_iHighestSlot;
	int m_iCurrentEntity;
	bool m_bActive;

	const static int MAX_ENTITIES = 600;

	int m_ActiveTimes[MAX_ENTITIES];
	int m_VisibleTimes[MAX_ENTITIES];
	int m_LastMsgNums[MAX_ENTITIES];
	Vector m_LastPositions[MAX_ENTITIES];
	int m_EntityStates[MAX_ENTITIES];

	float m_flRealAimSpeed;

	float _fAimOfsRight;
	float _fAimOfsForward;
	float _fAimOfsUp;
	bool _bUseAimOfs;

	void _RetriveTargetFromEnt(cl_entity_t *pmyEnt,Vector &outTarget);

public:

	Aiming()
	{
		m_iCurrentEntity = 0;
		m_iHighestSlot = -1;
		m_flRealAimSpeed = 0;

		_fAimOfsRight = 0;
		_fAimOfsForward = 0;
		_fAimOfsUp = 0;
		_bUseAimOfs = false;
	}

	void addAimLayer(int iSlot, int iEnt);
	void removeAimLayer(int iSlot);
	void showAimLayers();

	void Start();
	void Stop();

	void LookAtCurrentEntity();

	void nextEntity() { m_iCurrentEntity = std::min(m_iCurrentEntity + 1, MAX_ENTITIES); }
	void prevEntity() { m_iCurrentEntity = std::max(m_iCurrentEntity - 1, 0); }
	void setEntity(int i) { m_iCurrentEntity = i; }
	
	bool getValidTarget(Vector &outTarget);
	bool isAiming() { return m_bActive; }
	void aim();

	void SetAimOfs(float fOfsRight,float fOfsForward,float fOfsUp);
};

extern Aiming g_Aiming;
