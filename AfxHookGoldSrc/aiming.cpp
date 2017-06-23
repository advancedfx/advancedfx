#include "stdafx.h"

#include "aiming.h"
#include "filming.h"
#include "cmdregister.h"

extern cl_enginefuncs_s *pEngfuncs;
extern engine_studio_api_s *pEngStudio;
extern playermove_s *ppmove;

REGISTER_CVAR(aim_oneway, "0", 0);
REGISTER_CVAR(aim_snapto, "0", 0);
REGISTER_CVAR(aim_lingertime, "50", 0);
REGISTER_CVAR(aim_onlyvisible, "0", 0);

REGISTER_DEBUGCVAR(aim_accel, "0.01", 0);
REGISTER_DEBUGCVAR(aim_deaccel, "0.01", 0);
REGISTER_DEBUGCVAR(aim_rest_x, "4.0", 0);
REGISTER_DEBUGCVAR(aim_rest_y, "4.0", 0);
REGISTER_DEBUGCVAR(aim_wake_x, "10.0", 0);
REGISTER_DEBUGCVAR(aim_wake_y, "20.0", 0);

// Our aiming singleton
Aiming g_Aiming;

void AnglesFromTo(Vector &from, Vector &to, Vector &angles);
float TraceLine(cl_entity_t *target);
float clamp(float i, float min, float max);
void MakeLocalCoords(Vector vAngles,Vector &vUp,Vector &vRight,Vector &vForward);

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

void Aiming::addAimLayer(int iSlot, int iEnt)
{
	std::list<int>::iterator iter = m_AimLayers.begin();
	int i = 0;

	// Shift the iterator up to the right position
	while (i < iSlot && iter != m_AimLayers.end())
	{
		i++;
		iter++;
	}

	m_AimLayers.insert(iter, iEnt);
	pEngfuncs->Con_Printf("Added entity %d at slot %d\n", iEnt, i);
}

void Aiming::removeAimLayer(int iSlot)
{
	std::list<int>::iterator iter = m_AimLayers.begin();
	int i = 0;

	// Shift the iterator up to the right position
	while (i < iSlot && iter != m_AimLayers.end())
	{
		i++;
		iter++;
	}

	if (iter == m_AimLayers.end())
	{
		pEngfuncs->Con_Printf("No such slot\n");
		return;
	}
	
	m_AimLayers.erase(iter);
	pEngfuncs->Con_Printf("Removed entity at slot %d\n", i);
}

void Aiming::showAimLayers()
{
	int i = 0;
	for (std::list<int>::iterator iter = m_AimLayers.begin(); iter != m_AimLayers.end(); iter++)
		pEngfuncs->Con_Printf(" %02d. Ent #%d\n", i++, *iter);
}

void Aiming::Start()
{
	m_Awake[0] = false;
	m_Awake[1] = false;
	m_D0[0] = 0;
	m_D0[1] = 0;
	m_D1[0] = 0;
	m_D1[1] = 0;
	m_LastAimTime = g_Filming.GetDebugClientTime();

	m_iHighestSlot = 9999;
	m_bActive = true;

	memset(m_ActiveTimes, 0, sizeof(m_ActiveTimes));
	memset(m_VisibleTimes, 0, sizeof(m_VisibleTimes));
	memset(m_LastMsgNums, 0, sizeof(m_LastMsgNums));
	memset(m_EntityStates, 0, sizeof(m_EntityStates));

	// Loop through all entities in the list and record their initial state
	// This fixes the fact that we aim at inactive things briefly at first
	for (std::list<int>::iterator iter = m_AimLayers.begin(); iter != m_AimLayers.end(); iter++)
	{
		int i = (*iter);

		// Point to the right entity
		cl_entity_t *them = pEngfuncs->GetEntityByIndex(i);

		// If they exist them take the actual values
		if (them)
			m_LastMsgNums[i] = them->curstate.messagenum;
	}
}

void Aiming::Stop()
{
	pEngfuncs->Con_DPrintf("Stopped aiming!\n");
	m_bActive = false;
}

void Aiming::LookAtCurrentEntity()
{
	cl_entity_t *them = pEngfuncs->GetEntityByIndex(m_iCurrentEntity);

	if (them)
	{
		Vector angles;
		Vector target;

		_RetriveTargetFromEnt(them,target);
		AnglesFromTo(ppmove->origin, target, angles);

		float cangles[3];

		pEngfuncs->GetViewAngles(cangles);
		cangles[0] = angles.y;
		cangles[1] = angles.x;
		pEngfuncs->SetViewAngles(cangles);
	}
}

bool Aiming::getValidTarget(Vector &outTarget)
{
	cl_entity_t *them = NULL;

	bool bLowerPriority, bFirstEntity = true;
	float vis;
	int msg;

	// we track net status in order to make sure we do not kill ents to fast:
	static net_status_s nssNetStatus;
	static double dOldLatency=0;

	pEngfuncs->pNetAPI->Status(&nssNetStatus);

	// Some booleans
	bool bOneWay = !(aim_oneway->value == 0);
	bool bViewOnly = !(aim_onlyvisible->value == 0);
	bool bNetActive = (dOldLatency != nssNetStatus.latency); // not perfect yet

	// The time we linger before we move on!
	int iLingerTime = (int) aim_lingertime->value;

	// Now this is code to get the right entity to aim at!
	std::list<int>::iterator iter = m_AimLayers.begin();

	int slot_position = 0;

	// Loop through all entities in the list
	for (std::list<int>::iterator iter = m_AimLayers.begin(); iter != m_AimLayers.end(); iter++)
	{
		// What we need
		//	If the entity meets our targetting criteria (vis/active) we store the
        //  coords for it, which is what gets returned.
		//  When the entity starts failing, we keep returning the last updated
		//  coords until the linger time runs out, after which we just return false
		//  and the cameraman can do whatever

		int i = (*iter);

		// Point to the right entity
		them = pEngfuncs->GetEntityByIndex(i);

		// If they exist them take the actual values
		if (them)
		{
			vis = TraceLine(them);
			msg = them->curstate.messagenum;
		}
		// Otherwise use their last known message and assume they're not visible
		else
		{
			vis = 0;
			msg = m_LastMsgNums[i];
		}

		// And is this up or down the list?
		bLowerPriority = (slot_position > m_iHighestSlot);

		// This entity was active within the last few frames
		// m_EntityStates[i] == ES_DEAD means that the entity has died since it has been tracked
		if (m_EntityStates[i] != ES_DEAD && (m_LastMsgNums[i] != msg || m_ActiveTimes[i] > 0))
		{
			if (m_LastMsgNums[i] != msg)
				m_ActiveTimes[i] = 5; //WARNING note: actuall this might have to be s.th. like c*(FPS/updaterate)

			// We don't care about whether it is visible, so assume it is
			if (!bViewOnly)
			{
				_RetriveTargetFromEnt(them,m_LastPositions[i]);
				m_VisibleTimes[i] = iLingerTime;
			}
			// Actually we only look at visible entities
			else if (bViewOnly && (vis == 1.0f || m_VisibleTimes[i] > 0))
			{
				if (vis == 1.0f)
					m_VisibleTimes[i] = iLingerTime;

				_RetriveTargetFromEnt(them,m_LastPositions[i]);
			}
		}
		// This entity is not active
		else
		{
			// If a previously targetted entity has died then don't retarget it
			if (m_EntityStates[i] == ES_TARGET)
				m_EntityStates[i] = ES_DEAD;
		}

		// By now if m_VisibleTimes[i] > 0 then we will have a location which we want to
		// aim at, calculated from either this frame or a previous one
		if (m_VisibleTimes[i] > 0)
		{
			// It's the first entity we've searched for and we're allowed to pick it
			if (bFirstEntity && (!bOneWay || !bLowerPriority))
			{
				outTarget = m_LastPositions[i];
				bFirstEntity = false;

				m_EntityStates[i] = ES_TARGET;
			}

			m_VisibleTimes[i]--;
		}

		// We just need to decrement this however
		// BUT DO NOT KILL WHEN NET IS NOT ACTIVE (i.e. Demo Paused)s
		if ((m_ActiveTimes[i] > 0) && bNetActive)
			m_ActiveTimes[i]--;

		// Now we just update their values
		m_LastMsgNums[i] = msg;

		slot_position++;
	}

	// save new connection time
	dOldLatency = nssNetStatus.latency;

	// Return whether we found an entity
	return !bFirstEntity;
}

void Aiming::aim()
{
	Vector target(0, 0, 0);

	// Nothing to aim at here
	if (!getValidTarget(target))
		return;

	double curAimTime = g_Filming.GetDebugClientTime();
	Vector idealaim;

	AnglesFromTo(ppmove->origin, target, idealaim);		

	// Are we using snapto or nice aiming
	if (aim_snapto->value != 0)
	{
		// Simple aim
		float cangles[3];
		pEngfuncs->GetViewAngles(cangles);
		cangles[0] = idealaim.y;
		cangles[1] = idealaim.x;
		pEngfuncs->SetViewAngles(cangles);
	}
	else
	{
		float angles[3];
		float reaim[2];
		float deltaT = (float)(curAimTime - m_LastAimTime);

		if(deltaT < 0) deltaT = 0;

		pEngfuncs->GetViewAngles(angles);

		reaim[0] = idealaim.x - angles[1];
		reaim[1] = idealaim.y - angles[0];

		// For when angles are on the 359..0 crossover
		if (reaim[0] > 180.0f) reaim[0] -= 360.0f;
		else if (reaim[0] < -180.0f) reaim[0] += 360.0f;
		if (reaim[1] > 180.0f) reaim[1] -= 360.0f;
		else if (reaim[1] < -180.0f) reaim[1] += 360.0f;

		// avoid over-controlling:

		m_Awake[0] = m_Awake[0] && 0.5f * aim_rest_x->value < abs(reaim[0])
			|| 0.5f * aim_wake_x->value < abs(reaim[0]);

		m_Awake[1] = m_Awake[1] && 0.5f * aim_rest_y->value < abs(reaim[1])
			|| 0.5f * aim_wake_y->value < abs(reaim[1]);

		if(m_Awake[0]) m_D1[0] += deltaT * aim_accel->value;
		else m_D1[0] -= deltaT * aim_deaccel->value;

		if(m_Awake[1]) m_D1[1] += deltaT * aim_accel->value;
		else m_D1[1] -= deltaT * aim_deaccel->value;

		m_D1[0] = clamp(m_D1[0], 0, 1);
		m_D1[1] = clamp(m_D1[1], 0, 1);

		m_D0[0] = m_D1[0] * (reaim[0] -m_D0[0]);
		m_D0[1] = m_D1[1] * (reaim[1] -m_D0[1]);

		// apply re-aiming:

		angles[1] += m_D0[0];
		angles[0] += m_D0[1];

		pEngfuncs->SetViewAngles(angles);
	}

	m_LastAimTime = curAimTime;
}

void Aiming::SetAimOfs(float fOfsRight,float fOfsForward,float fOfsUp)
{
	_fAimOfsRight=fOfsRight;
	_fAimOfsForward=fOfsForward;
	_fAimOfsUp=fOfsUp;
	_bUseAimOfs =  fOfsRight!=0 || fOfsForward!=0 || fOfsUp!=0;
}

////

void Aiming::_RetriveTargetFromEnt(cl_entity_t *pmyEnt,Vector &outTarget)
{
	outTarget=pmyEnt->origin;

	if (_bUseAimOfs)
	{
		Vector vUp,vRight,vForward;

		MakeLocalCoords(pmyEnt->angles,vUp,vRight,vForward);

		outTarget = outTarget + (_fAimOfsRight*vRight) + (_fAimOfsForward*vForward) + (_fAimOfsUp*vUp);
	}
}

////

void AnglesFromTo(Vector &from, Vector &to, Vector &angles)
{
	Vector dir = to - from;

	// Store then zero height
	float dz = dir.z;
	
	dir.z = 0;

	// Need this for later
	float length = dir.Length();

	dir = dir.Normalize();

	// This is our forward angle
	Vector vForward(1.0f, 0.0f, 0.0f);

	float dot_product = (dir.x * vForward.x) + (dir.y * vForward.y) + (dir.z * vForward.z);

	float angle = acos(dot_product) * 180.0f / (float)M_PI;

	if (dir.y < 0)
		angle = 360.0f - angle;

	// This is our pitchup/down
	if (length == 0)
		length = 0.01f;

	float pitch = atan(dz / length) * 180.0f / (float)M_PI;

	angles.x = angle;
	angles.y = -pitch;
}

float TraceLine(cl_entity_t *target)
{
	// Get coords of entity and us
	float ppmove_origin[3], target_origin[3];

	ppmove->origin.CopyToArray(ppmove_origin);
	target->origin.CopyToArray(target_origin);

	pmtrace_t *tr;
	tr = pEngfuncs->PM_TraceLine(ppmove_origin, target_origin, PM_TRACELINE_ANYVISIBLE, 2, -1); //target->index /*shouldnt matter with studio ignore*/);

	return tr->fraction;
}

float clamp(float i, float min, float max)
{
	if (i < min)
		return min;
	if (i > max)
		return max;
	else
		return i;
}

void MakeLocalCoords(Vector vAngles,Vector &vUp,Vector &vRight,Vector &vForward)
{
	// >> begin calculate transform vectors
	// we have to calculate our own transformation vectors from the angles and can not use pparams->forward etc., because in spectator mode they might be not present:
	// (adapted from HL1SDK/multiplayer/pm_shared.c/AngleVectors) and modified for quake order of angles:

	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = vAngles.y * ((float)M_PI*2 / 360); // yaw
	sy = sin((float)angle);
	cy = cos((float)angle);
	angle = vAngles.x * ((float)M_PI*2 / 360); // pitch
	sp = sin((float)angle);
	cp = cos((float)angle);
	angle = vAngles.z * ((float)M_PI*2 / 360); // roll
	sr = sin((float)angle);
	cr = cos((float)angle);

	vForward.x = cp*cy;
	vForward.y = cp*sy;
	vForward.z = -sp;

	vRight.x = (-1*sr*sp*cy+-1*cr*-sy);
	vRight.y = (-1*sr*sp*sy+-1*cr*cy);
	vRight.z = -1*sr*cp;

	vUp.x = (cr*sp*cy+-sr*-sy);
	vUp.y = (cr*sp*sy+-sr*cy);
	vUp.z = cr*cp;
}

REGISTER_CMD_FUNC(entity_lookat)
{
	g_Aiming.LookAtCurrentEntity();
}

REGISTER_CMD_FUNC(entity_next)
{
	g_Aiming.nextEntity();
	g_Aiming.LookAtCurrentEntity();
}

REGISTER_CMD_FUNC(entity_prev)
{
	g_Aiming.prevEntity();
	g_Aiming.LookAtCurrentEntity();
}

REGISTER_CMD_FUNC(entity_jump)
{
	if (pEngfuncs->Cmd_Argc() != 2)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "entity_jump <entnum>\n");
		return;
	}

	g_Aiming.setEntity(atoi(pEngfuncs->Cmd_Argv(1)));
	g_Aiming.LookAtCurrentEntity();
}

REGISTER_CMD_FUNC_BEGIN(aim)
{
	g_Aiming.Start();
}

REGISTER_CMD_FUNC_END(aim)
{
	g_Aiming.Stop();
}

REGISTER_CMD_FUNC(aim_start)
{
	CALL_CMD_BEGIN(aim);
}

REGISTER_CMD_FUNC(aim_stop)
{
	CALL_CMD_END(aim);
}


REGISTER_CMD_FUNC(aim_layers)
{
	bool showHelp = true;

	int argc = pEngfuncs->Cmd_Argc();

	if(2 <= argc)
	{
		char const * cmd1 = pEngfuncs->Cmd_Argv(1);

		if(2 == argc && !_stricmp("list", cmd1))
		{
			g_Aiming.showAimLayers();
			showHelp = false;
		}
		else if(3 <= argc)
		{
			int slot = atoi(pEngfuncs->Cmd_Argv(2));
			
			if(3 == argc && !_stricmp("del", cmd1))
			{
				g_Aiming.removeAimLayer(slot);
				showHelp = false;
			}
			else if(4 == argc && !_stricmp("add", cmd1))
			{
				int ent = atoi(pEngfuncs->Cmd_Argv(3));
				g_Aiming.addAimLayer(slot, ent);
				showHelp = false;
			}
		}
	}

	if(showHelp)
	{
		pEngfuncs->Con_Printf(
			"Usage:\n"
			"\t" PREFIX "aim_layers add <slot> <entid>\n"
			"\t" PREFIX "aim_layers del <slot>\n"
			"\t" PREFIX "aim_layers list\n"
		);
	}
}


REGISTER_CMD_FUNC(showentities)
{
	cl_entity_t *pEnt = NULL;

	pEngfuncs->Con_Printf("Showing known entities with models:\n");

	for (int i = 0; i < 1024; i++)
	{
		pEnt = pEngfuncs->GetEntityByIndex(i);

		if(pEnt && pEnt->model)
		{
			int iDistance = (int) (pEnt->origin - ppmove->origin).Length();
			pEngfuncs->Con_Printf("  %03d. %s (dist: %d)\n", i, pEnt->model->name, iDistance);
		}
	}
}


REGISTER_CMD_FUNC(aim_offset)
{
	if (pEngfuncs->Cmd_Argc() == 4)
		g_Aiming.SetAimOfs((float)atof(pEngfuncs->Cmd_Argv(1)),(float)atof(pEngfuncs->Cmd_Argv(3)),(float)atof(pEngfuncs->Cmd_Argv(2)));
	else
		pEngfuncs->Con_Printf("Usage: " PREFIX "aim_offset <right> <up> <forward>\n");
}
