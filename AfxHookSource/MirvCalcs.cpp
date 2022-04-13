#include "stdafx.h"

#include "MirvCalcs.h"

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvTime.h"
#include "addresses.h"
#include "CamIO.h"
#include "csgo/ClientToolsCsgo.h"

#include <shared/StringTools.h>
#include <ctype.h>

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

extern WrpVEngineClient * g_VEngineClient;

CMirvHandleCalcs g_MirvHandleCalcs;
CMirvVecAngCalcs g_MirvVecAngCalcs;
CMirvCamCalcs g_MirvCamCalcs;
CMirvFovCalcs g_MirvFovCalcs;
CMirvBoolCalcs g_MirvBoolCalcs;
CMirvIntCalcs g_MirvIntCalcs;
CMirvFloatCalcs g_MirvFloatCalcs;

int g_LevelInitCount = 0;

void CalcDeltaSmooth(double deltaT, double targetDeltaPos, double & resultDeltaPos, double & lastVel, double LimitVelocity, double LimitAcceleration)
{
	if (deltaT <= 0)
	{
		resultDeltaPos = 0;
		return;
	}

	// What we would like to have ideally is this:
	// https://www.physicsforums.com/threads/3rd-order-motion-profile-programming-sinusoidal.868148/

	// However that is way too complicated for now so we try s.th. easier for now,
	// a 2nd order motion profile that has sort of trapezoidal velocity profile:

	// Objective:
	//
	// For the _normal_ case
	// we build the 2nd order motion profile:
	//
	// phaseT_k is the (positive) phase time.
	//
	// accel(k) = c_k
	// velo(k) = vel(k-1) +Int(accel(k),0+eps..phaseT_k) = vel(k-1) +accel(k)*phaseT_k
	// pos(k) = pos(k-1) +Int(velo(k),0+eps..phaseT_k) = pos(k-1) +vel(k-1) * phaseT_k +accel(k)/2 * phaseT_k^2
	//
	// -LimitAccel <= accel(k) <= LimitAccel
	// -LimitVelocity <= vel(k) <= LimitVelocity
	//
	// Possible phases:
	// P1) velocity build-up
	// P2) limit velocity
	// P3) velocity ramp-down

	// Complete programming problem:
	//
	//
	// Input:
	//
	// LimitVelocity - absolute velocity limit, where 0 < LimitVelocity
	// LimitAcceleration - absolute acceleration limit, where 0 < LimitAcceleration
	// targetPos - target position
	// lastPos - last position
	// lastVel - last velocity, where -LimitVelocity <= lastVel <= LimitVelocity
	//
	//
	// Output:
	//
	// phase1T, phase2T, phase3T - Phase times
	// , where 0 <= phase1T, 0 <= phase2T, 0 <= phase3T
	//
	// dir - base accel direction, where dir \in {-1, 1}
	//
	//
	// Further Equations:
	//
	// accel(1) = +dir * LimitAcceleration
	// accel(2) = 0
	// accel(3) = -dir * LimitAcceleration
	//
	// vel(0) = lastVel
	// vel(1) = vel(0) +accel(1) * phase1T = lastVel + dir * LimitAcceleration * phase1T
	// vel(2) = vel(1) +accel(2) * phase2T = lastVel + dir * LimitAcceleration * phase1T
	// vel(3) = vel(2) +accel(3) * phase3T = lastVel + dir * LimitAcceleration * phase1T - dir * LimitAcceleration * phase3T
	// vel(3) = 0
	//
	// -LimitVelocity <= vel(1), vel(2), vel(3) <= LimitVelocity
	// 
	// resultDeltaPos = 0
	// +Int(vel(1),0+eps,phase1T)
	// +Int(vel(2),0+eps,phase2T)
	// +Int(vel(3),0+eps,phase3T)
	// = 0
	// +vel(0) * phase1T +accel(1)/2 * phase1T^2
	// +vel(1) * phase2T +accel(2)/2 * phase2T^2
	// +vel(2) * phase3T +accel(3)/2 * phase3T^2
	// = 0
	// + lastVel * phase1T +dir * LimitAcceleration / 2 * phase1T^2
	// + (lastVel + dir * LimitAcceleration * phase1T) * phase2T
	// + (lastVel + dir * LimitAcceleration * phase1T) * phase3T -dir * LimitAcceleration/2 * phase3T^2
	//
	// deltaPos = targetPos -lastPos
	//
	// Minimize:
	//
	// 1) Position error: abs(deltaPos -resultDeltaPos)
	// 2) Position time: phase1T +phase2T +phase3T +phase4T +phase5T +phase6T +phase7T

	while (0 < deltaT)
	{
		//Tier0_Msg("%f ", deltaT);

		if (lastVel > LimitVelocity)
		{
			// Error condition.

			// Solving Step:
			//
			// lower Velocity until we are within limits:

			// LimitVelocity = lastVel -LimitAcceleration * phaseT

			double phaseT = (LimitVelocity - lastVel) / -LimitAcceleration;

			// Limit by deltaT:

			phaseT = std::min(phaseT, deltaT);

			resultDeltaPos = lastVel * phaseT - LimitAcceleration / 2.0 * phaseT * phaseT;
			lastVel += -LimitAcceleration * phaseT;
			deltaT -= phaseT;
		}
		else
			if (lastVel < -LimitVelocity)
			{
				// Error condition.

				// Solving Step:
				//
				// increase Velocity until we are within limits:

				// -LimitVelocity = lastVel +LimitAcceleration * phaseT

				double phaseT = (-LimitVelocity - lastVel) / +LimitAcceleration;

				// Limit by deltaT:

				phaseT = std::min(phaseT, deltaT);

				resultDeltaPos = lastVel * phaseT + LimitAcceleration / 2.0 * phaseT * phaseT;
				lastVel += +LimitAcceleration * phaseT;
				deltaT -= phaseT;
			}
			else
			{
				double phase1T = 0;
				double phase2T = 0;
				double phase3T = 0;

				// Solving Step 1:
				//
				// Finding a feasible solution, that is an upperBound for the position error
				// and a lower bound for the position time:
				//
				// This equals a full stop, meaning phase1T_1 = 0, phase2T_1 = 0:
				//
				// resultDeltaPos_1 = lastVel * phase3T_1 -dir * LimitAcceleration/2 * phase3T_1^2
				// 0 = lastVel - dir * LimitAcceleration * phase3T_1
				//
				// It follows:
				// phase3T_1 = lastVel / (dir * limitAcceleration)

				double dir = 0 < lastVel ? 1 : (0 > lastVel ? -1 : (0 <= targetDeltaPos ? 1 : -1));
				phase3T = lastVel / (dir * LimitAcceleration);

				resultDeltaPos = lastVel * phase3T - dir * LimitAcceleration / 2.0 * phase3T * phase3T;

				if (
					0 < dir && 0 < targetDeltaPos - resultDeltaPos
					|| 0 > dir && 0 > targetDeltaPos - resultDeltaPos
					)
				{
					// Solving Step 2 (only if we didn't (overshoot or hit) in Step 1):
					//
					// phase1T and phase3T are equally increased until either LimitAcceleration is hit
					// or resultDetlaPos = deltaPos:
					//
					// phase2T_2 = 0, phase3T_2 = phase3T_2 +phase1T_2
					//
					// 2.1) Assume can hit deltaPos:
					// deltaPos = 0
					// + lastVel * phase1T_{2.1} +dir * LimitAcceleration / 2 * phase1T_{2.1}^2
					// + (lastVel + dir * LimitAcceleration *  phase1T_{2.1}) * (phase3T_1 +phase1T_{2.1})
					// -dir * LimitAcceleration/2 * (phase3T_1 +phase1T_{2.1})^2
					// = 0
					// + lastVel * phase1T_{2.1} +dir * LimitAcceleration / 2 * phase1T_{2.1}^2
					// + lastVel * phase3T_1 +dir * LimitAcceleration * phase3T_1 * phase1T_{2.1}
					// + lastVel * phase1T_{2.1} +dir * LimitAcceleration * phase1T_{2.1}^2
					// - dir * LimitAcceleration/2 * (phase3T_1^2 +2*phase3T_1*phase1T_{2.1} +phase1T_{2.1}^2)
					// = 0
					// + lastVel * phase3T_1 - dir * LimitAcceleration/2 * phase3T_1^2
					// + (lastVel +dir * LimitAcceleration * phase3T_1 +lastVel -dir * LimitAcceleration/2 *2*phase3T_1) * phase1T_{2.1}
					// + (dir * LimitAcceleration / 2 +dir * LimitAcceleration - dir * LimitAcceleration/2) * phase1T_{2.1}^2
					// It follows:
					// 0 =
					// [-deltaPos +lastVel * phase3T_1 - dir * LimitAcceleration/2 * phase3T_1^2]
					// +[2 * lastVel] * phase1T_{2.1}
					// +[dir * LimitAcceleration] * phase1T_{2.1}^2
					// =
					// [-deltaPos +lastVel * phase3T_1 - dir * LimitAcceleration/2 * phase3T_1^2]/[dir * LimitAcceleration]
					// +[2 * lastVel]/[dir * LimitAcceleration] * phase1T_{2.1}
					// +phase1T_{2.1}^2
					// It follows:
					// phase1T_{2.1.1}
					// = 1/2 * (-[2 * lastVel]/[dir * LimitAcceleration]) +/- sqrt( ([2 * lastVel]/[dir * LimitAcceleration])^2 -4*([-deltaPos +lastVel * phase3T_1 - dir * LimitAcceleration/2 * phase3T_1^2]/[dir * LimitAcceleration]) )
					// We only need the positive solution.
					//
					// 2.2) Assume we can hit LimitVelocity:
					// -LimitVelocity <= lastVel + dir * LimitAcceleration * phase1T_{2.2} <= LimitVelocity	
					// phase1T_{2.2} = (s_{2.2} * LimitVelocity -lastVel) / (dir * LimitAcceleration)
					// s_{2.2} = dir
					//
					// phase1T_2 = min{phase1T_{2.1},phase1T_{2.2}}

					double temp1 =
						(2.0 * lastVel) / (dir * LimitAcceleration);

					double phase1T_2d1 =
						0.5 * ((-temp1)
							+ sqrt(temp1*temp1 - 4 * (-targetDeltaPos + lastVel * phase3T - dir * LimitAcceleration / 2.0 * phase3T*phase3T) / (dir * LimitAcceleration)));

					double phase1T_2d2 = (dir * LimitVelocity - lastVel) / (dir * LimitAcceleration);

					phase1T = std::min(phase1T_2d1, phase1T_2d2);
					phase3T += phase1T;

					resultDeltaPos = lastVel * phase1T + dir * LimitAcceleration / 2.0 * phase1T * phase1T
						+ (lastVel + dir * LimitAcceleration * phase1T) * phase3T - dir * LimitAcceleration / 2.0 * phase3T*phase3T;

					if (
						0 < dir && 0 < targetDeltaPos - resultDeltaPos
						|| 0 > dir && 0 > targetDeltaPos - resultDeltaPos
						)
					{
						// Solving Step 3 (only if we didn't (overshoot or hit) in Step 2):
						// 
						// phase2T is increased until deltaPos is hit.
						//
						// deltaPos =
						// + lastVel * phase1T_2 +dir * LimitAcceleration / 2 * phase1T_2^2
						// + (lastVel + dir * LimitAcceleration * phase1T_2) * phase2T_3
						// + (lastVel + dir * LimitAcceleration * phase1T_2) * phase3T_2 -dir * LimitAcceleration/2 * phase3T_2^2
						//
						// phase2T_3 = [deltaPos -lastVel * phase1T_2 -dir * LimitAcceleration / 2 * phase1T_2^2
						// -(lastVel + dir * LimitAcceleration * phase1T_2) * phase3T_2 +dir * LimitAcceleration/2 * phase3T_2^2]
						// / (lastVel + dir * LimitAcceleration * phase1T_2)

						double temp2 = lastVel + dir * LimitAcceleration * phase1T;

						if (temp2)
							phase2T = (targetDeltaPos - lastVel * phase1T - dir * LimitAcceleration / 2.0 * phase1T*phase1T
								- (temp2)* phase3T + dir * LimitAcceleration / 2.0 * phase3T*phase3T)
							/ (temp2);
					}
				}

				// Limit by deltaT:

				phase3T = std::max(std::min(phase1T + phase2T + phase3T, deltaT) - phase2T - phase1T, 0.0);
				phase2T = std::max(std::min(phase1T + phase2T, deltaT) - phase1T, 0.0);
				phase1T = std::min(phase1T, deltaT);

				resultDeltaPos = lastVel * phase1T + dir * LimitAcceleration / 2.0 * phase1T * phase1T
					+ (lastVel + dir * LimitAcceleration * phase1T) * phase2T
					+ (lastVel + dir * LimitAcceleration * phase1T) * phase3T - dir * LimitAcceleration / 2.0 * phase3T*phase3T;
				lastVel += +dir * LimitAcceleration * phase1T - dir * LimitAcceleration * phase3T;
				deltaT -= phase1T + phase2T + phase3T;

				// If we can consider to be finished, then we use-up the time completely:
				if (abs(targetDeltaPos - resultDeltaPos) <= AFX_MATH_EPS && abs(0 - lastVel) <= AFX_MATH_EPS)
				{
					deltaT = 0;
					resultDeltaPos = targetDeltaPos;
				}
			}
	}
}

void CalcSmooth(double deltaT, double targetPos, double & lastPos, double & lastVel, double LimitVelocity, double LimitAcceleration)
{
	double deltaPos;

	CalcDeltaSmooth(deltaT, targetPos - lastPos, deltaPos, lastVel, LimitVelocity, LimitAcceleration);

	lastPos += deltaPos;
}

////////////////////////////////////////////////////////////////////////////////

class CMirvCalc : public IMirvCalc
{
public:
	CMirvCalc(char const * name)
		: m_Name(name ? name : "")
		, m_HasName(nullptr != name)
	{

	}

	virtual void AddRef(void)
	{
		++m_RefCount;
	}

	virtual void Release(void)
	{
		int cnt = --m_RefCount;

		if (0 == cnt)
			delete this;
	}

	virtual int GetRefCount(void)
	{
		return m_RefCount;
	}

	virtual  char const * GetName(void)
	{
		return m_Name.c_str();
	}

	virtual void Console_PrintBegin(void)
	{
		if(m_HasName) Tier0_Msg("{ name: \"%s\"", m_Name.c_str());
		else Tier0_Msg("{ name: null", m_Name.c_str());
	}
	
	virtual void Console_PrintEnd(void)
	{
		Tier0_Msg(" }");
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		Tier0_Msg("No editable options.\n");
	}

protected:
	virtual ~CMirvCalc()
	{

	}

private:
	bool m_HasName;
	std::string m_Name;
	int m_RefCount = 0;
};

class CMirvHandleCalc : public CMirvCalc, public IMirvHandleCalc
{
public:
	CMirvHandleCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();
		Tier0_Msg(", type: \"handle\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};


class CMirvHandleValueCalc : public CMirvHandleCalc
{
public:
	CMirvHandleValueCalc(char const * name, int handle)
		: CMirvHandleCalc(name)
		, m_Handle(handle)
	{

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		outHandle = m_Handle;
		return true;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();
		Tier0_Msg(", fn: \"value\", handle: %i", m_Handle);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);
		
		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("handle", arg1))
			{
				if (3 <= argc)
				{
					m_Handle = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s handle <iHandle> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Handle.ToInt()
				);
				return;
			}
		}

		Tier0_Msg(
			"%s handle [...]\n"
			, arg0
		);
	}

private:
	SOURCESDK::CSGO::CBaseHandle m_Handle;
};

class CMirvHandleIndexCalc : public CMirvHandleCalc
{
public:
	CMirvHandleIndexCalc(char const * name, int index)
		: CMirvHandleCalc(name)
		, m_Index(index)
	{

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(m_Index);

		if (ce)
		{
			outHandle = ce->GetRefEHandle();
			return true;
		}

		return false;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"index\", index: %i", m_Index);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("index", arg1))
			{
				if (3 <= argc)
				{
					m_Index = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s index <iIndex> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Index
				);
				return;
			}
		}

		Tier0_Msg(
			"%s index [...]\n"
			, arg0
		);
	}

private:
	int m_Index;
};

typedef bool(*csgon_fnPlayerSidesWappedOnScreen_t)(void);

class CMirvHandleKeyCalc : public CMirvHandleCalc
{
public:
	CMirvHandleKeyCalc(char const * name, int key)
		: CMirvHandleCalc(name)
		, m_Key(key)
		, m_ClSpecSwapPlayerSides("cl_spec_swapplayersides")
	{

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		// Left screen side keys: 1, 2, 3, 4, 5
		// Right screen side keys: 6, 7, 8, 9, 0

		csgon_fnPlayerSidesWappedOnScreen_t fnPlayerSidesWappedOnScreen = (csgon_fnPlayerSidesWappedOnScreen_t)AFXADDR_GET(csgo_Unknown_GetTeamsSwappedOnScreen);

		bool swapPlayerSide = fnPlayerSidesWappedOnScreen && fnPlayerSidesWappedOnScreen();

		int nr = ((m_Key +9) % 10);

		bool isOtherScreenSide = 0 != ((nr / 5) % 2);

		int slot = (nr % 5);

		int slotCT = 0;
		int slotT = 0;

		WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();
		int imax = gl ? gl->maxclients_get() : 0;

		for (int i = 1; i <= imax; ++i)
		{
			SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
			SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;
			if (be && be->IsPlayer())
			{
				int be_team = be->GetTeamNumber();

				if (3 == be_team) // CT
				{
					if (isOtherScreenSide == swapPlayerSide && slotCT == slot)
					{
						outHandle = ce->GetRefEHandle();
						return true;
					}

					++slotCT;
				}
				else if (2 == be_team) // T
				{
					if (isOtherScreenSide != swapPlayerSide && slotT == slot)
					{
						outHandle = ce->GetRefEHandle();
						return true;
					}

					++slotT;
				}
			}
		}

		return false;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"key\", key: %i", m_Key);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("key", arg1))
			{
				if (3 <= argc)
				{
					m_Key = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s key <iKey> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Key
				);
				return;
			}
		}

		Tier0_Msg(
			"%s key [...]\n"
			, arg0
		);
	}

private:
	int m_Key;
	WrpConVarRef m_ClSpecSwapPlayerSides;
};

class CMirvHandleActiveWeaponCalc : public CMirvHandleCalc
{
public:
	CMirvHandleActiveWeaponCalc(char const * name, IMirvHandleCalc * parentCalc, bool world)
		: CMirvHandleCalc(name)
		, m_ParentCalc(parentCalc)
		, m_World(world)
	{
		m_ParentCalc->AddRef();

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::CSGO::CBaseHandle parentHandle;

		if (m_ParentCalc->CalcHandle(parentHandle) && parentHandle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(parentHandle);
			SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : 0;
			SOURCESDK::C_BaseCombatCharacter_csgo * combatCharacter = baseEntity ? baseEntity->MyCombatCharacterPointer() : 0;
			SOURCESDK::C_BaseCombatWeapon_csgo * activeWeapon = combatCharacter ? combatCharacter->GetActiveWeapon() : 0;

			if (activeWeapon)
			{
				SOURCESDK::IClientEntity_csgo * ce = 0;
				
				if(!m_World)
					ce = activeWeapon->GetIClientEntity();
				else if(-1 != AFXADDR_GET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel))
				{
					SOURCESDK::CSGO::CBaseHandle hWeaponWorldModel = *(SOURCESDK::CSGO::CBaseHandle *)((char const *)activeWeapon + AFXADDR_GET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel));
					ce = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(hWeaponWorldModel);
				}

				if (ce)
				{
					outHandle = ce->GetRefEHandle();
					return true;
				}
			}
		}

		return false;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"activeWeapon\"");
		Tier0_Msg(", parent: "); m_ParentCalc->Console_PrintBegin(); m_ParentCalc->Console_PrintEnd();
		Tier0_Msg(", getWorld: %i", m_World ? 1 : 0);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("getWorld", arg1))
			{
				if (3 <= argc)
				{
					m_World = 0 != atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s getWorld <bGetWorld> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_World ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s getWorld [...]\n"
			, arg0
		);
	}

protected:
	virtual ~CMirvHandleActiveWeaponCalc()
	{
		m_ParentCalc->Release();
	}

private:
	IMirvHandleCalc * m_ParentCalc;
	bool m_World;
};

class CMirvHandleLocalPlayerCalc : public CMirvHandleCalc
{
public:
	CMirvHandleLocalPlayerCalc(char const * name)
		: CMirvHandleCalc(name)
	{
	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_VEngineClient->GetLocalPlayer());

		if (ce)
		{
			outHandle = ce->GetRefEHandle();
			return true;
		}

		return false;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"localPlayer\"");
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		return CMirvHandleCalc::Console_Edit(args);
	}

protected:
	virtual ~CMirvHandleLocalPlayerCalc()
	{
	}

private:

};

class CMirvHandleObserverTargetCalc : public CMirvHandleCalc
{
public:
	CMirvHandleObserverTargetCalc(char const * name, IMirvHandleCalc * parentCalc)
		: CMirvHandleCalc(name)
		, m_ParentCalc(parentCalc)
	{
		m_ParentCalc->AddRef();

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::CSGO::CBaseHandle parentHandle;

		if (m_ParentCalc->CalcHandle(parentHandle) && parentHandle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(parentHandle);
			SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : 0;

			if (baseEntity && baseEntity->IsPlayer())
			{
				SOURCESDK::C_BasePlayer_csgo * player = (SOURCESDK::C_BasePlayer_csgo *)baseEntity;

				if (SOURCESDK::C_BaseEntity_csgo * be = player->GetObserverTarget())
				{
					if (SOURCESDK::IClientEntity_csgo * ce = be->GetIClientEntity())
					{
						outHandle = ce->GetRefEHandle();
						return true;
					}
				}
			}
		}

		return false;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"observerTarget\"");
		Tier0_Msg(", parent: "); m_ParentCalc->Console_PrintBegin(); m_ParentCalc->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		return CMirvHandleCalc::Console_Edit(args);
	}

protected:
	virtual ~CMirvHandleObserverTargetCalc()
	{
		m_ParentCalc->Release();
	}

private:
	IMirvHandleCalc * m_ParentCalc;
};


class CMirvHandleOwnerEntityCalc : public CMirvHandleCalc
{
public:
	CMirvHandleOwnerEntityCalc(char const * name, IMirvHandleCalc * calc, unsigned int maxDepth = 1)
		: CMirvHandleCalc(name)
		, m_ParentCalc(calc)
		, m_MaxDepth(maxDepth)
	{
		m_ParentCalc->AddRef();

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::CSGO::CBaseHandle parentHandle;

		if (m_ParentCalc->CalcHandle(parentHandle) && parentHandle.IsValid())
		{
			if (CClientToolsCsgo::Instance())
			{
				if (SOURCESDK::CSGO::IClientTools * clientTools = CClientToolsCsgo::Instance()->GetClientToolsInterface())
				{
					if (SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(parentHandle))
					{
						SOURCESDK::C_BaseEntity_csgo * ent = clientEntity->GetBaseEntity();

						unsigned int maxDepth = m_MaxDepth;

						while (ent && 0 < maxDepth)
						{
							--maxDepth;

							if (SOURCESDK::C_BaseEntity_csgo * owningEnt = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(clientTools->GetOwnerEntity(reinterpret_cast<SOURCESDK::CSGO::EntitySearchResult>(ent))))
								ent = owningEnt;
							else break;
						}

						if (ent)
						{
							outHandle = ent->GetRefEHandle();
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvHandleCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"observerTarget\"");
		Tier0_Msg(", calc: "); m_ParentCalc->Console_PrintBegin(); m_ParentCalc->Console_PrintEnd();
		Tier0_Msg(", maxDepth: %u", m_MaxDepth);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		return CMirvHandleCalc::Console_Edit(args);
	}

protected:
	virtual ~CMirvHandleOwnerEntityCalc()
	{
		m_ParentCalc->Release();
	}

private:
	IMirvHandleCalc * m_ParentCalc;
	unsigned int m_MaxDepth;
};

class CMirvVecAngCalc : public CMirvCalc, public IMirvVecAngCalc
{
public:
	CMirvVecAngCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();
		Tier0_Msg(", type: \"vecAng\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};


class CMirvCamCalc : public CMirvCalc, public IMirvCamCalc
{
public:
	CMirvCamCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();
		Tier0_Msg(", type: \"cam\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};

double CalcExpSmooth(double deltaT, double oldVal, double newVal)
{
	const double limitTime = 19.931568569324174087221916576936;

	if (deltaT < 0)
		return oldVal;
	else if (limitTime < deltaT)
		return newVal;

	const double halfTime = 0.69314718055994530941723212145818;

	double x = 1 / exp(deltaT * halfTime);

	return x * oldVal +  (1 - x) * newVal;
}

double CalcDeltaExpSmooth(double deltaT, double deltaVal)
{
	const double limitTime = 19.931568569324174087221916576936;

	if (deltaT < 0)
		return 0;
	else if (limitTime < deltaT)
		return deltaVal;

	const double halfTime = 0.69314718055994530941723212145818;

	double x = 1 / exp(deltaT * halfTime);

	return (1 - x) * deltaVal;
}

class CMirvCamSmoothCalc : public CMirvCamCalc
{
public:
	static void LevelInitPreEntity()
	{

	}

	CMirvCamSmoothCalc(char const * name, IMirvCamCalc * cam, IMirvHandleCalc * trackHandle)
		: CMirvCamCalc(name)
		, m_Parent(cam)
		, m_TrackHandle(trackHandle)
	{
		m_Parent->AddRef();
		m_TrackHandle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCamCalc::Console_PrintBegin();
		Tier0_Msg(", fn: \"smooth\"");
		Tier0_Msg(", parent: "); m_Parent->Console_PrintBegin(); m_Parent->Console_PrintEnd();
		Tier0_Msg(", trackHandle: "); m_TrackHandle->Console_PrintBegin(); m_TrackHandle->Console_PrintEnd();
		Tier0_Msg(", halfTimeVec: %f, halfTimeAng : %f, halfTimeFov : %f", (float)m_HalfTimeVec, (float)m_HalfTimeAng, (float)m_HalfTimeFov);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("halfTime", arg1))
			{
				if (3 <= argc)
				{
					m_HalfTimeVec = m_HalfTimeAng = m_HalfTimeFov = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s halfTime <fHalfTimeSecs> - Set new value for vec,ang,fov.\n"
					, arg0
				);
				return;
			}
			else if (0 == _stricmp("halfTimeVec", arg1))
			{
				if (3 <= argc)
				{
					m_HalfTimeVec = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s halfTimeVec <fHalfTimeSecs> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_HalfTimeVec
				);
				return;
			}
			else if (0 == _stricmp("halfTimeAng", arg1))
			{
				if (3 <= argc)
				{
					m_HalfTimeAng = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s halfTimeAng <fHalfTimeSecs> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_HalfTimeAng
				);
				return;
			}
			else if (0 == _stricmp("halfTimeFov", arg1))
			{
				if (3 <= argc)
				{
					m_HalfTimeFov = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s halfTimeFov <fHalfTimeSecs> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_HalfTimeFov
				);
				return;
			}
			else if (0 == _stricmp("rotShortestPath", arg1))
			{
				if (3 <= argc)
				{
					m_RotShortestPath = 0 != atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rotShortestPath 0|1 - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_RotShortestPath ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s halfTime [...]\n"
			"%s halfTimeVec [...]\n"
			"%s halfTimeAng [...]\n"
			"%s halfTimeFov [...]\n"
			"%s rotShortestPath [...] - If to rotate shortest path.\n"
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
		);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov) override
	{
		SOURCESDK::Vector parentVector;
		SOURCESDK::QAngle parentAngles;
		float parentFov;

		SOURCESDK::CSGO::CBaseHandle handle;

		bool calcedParent = m_Parent->CalcCam(parentVector, parentAngles, parentFov);
		bool calcedHandle = m_TrackHandle->CalcHandle(handle);

		if (m_LevelInitCount != g_LevelInitCount)
		{
			m_LevelInitCount = g_LevelInitCount;
			m_Reset = true;
		}

		m_Reset = m_Reset || !(calcedParent && calcedHandle && m_LastHandle == handle);

		if(calcedHandle) m_LastHandle = handle;

		if (calcedParent && calcedHandle)
		{
			if (m_Reset)
			{
				m_Reset = false;

				m_LastClientTime = g_MirvTime.GetTime();

				m_LastX = outVector.x = parentVector.x;
				m_LastY = outVector.y = parentVector.y;
				m_LastZ = outVector.z = parentVector.z;

				outAngles.x = parentAngles.x;
				outAngles.y = parentAngles.y;
				outAngles.z = parentAngles.z;
				m_LastOutQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(outAngles.x, outAngles.y, outAngles.z)));

				m_LastFov = outFov = parentFov;
			}
			else
			{
				float clientTime = g_MirvTime.GetTime();
				double deltaT = clientTime - m_LastClientTime;
				m_LastClientTime = clientTime;

				if (m_HalfTimeAng)
				{
					double t = deltaT / m_HalfTimeAng;

					Quaternion targetQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(parentAngles.x, parentAngles.y, parentAngles.z))).Normalized();

					if (m_RotShortestPath)
					{
						// Take shortest path:
						double dotProduct = DotProduct(targetQuat, m_LastOutQuat);
						if (dotProduct < 0)
						{
							targetQuat = -1.0 * targetQuat;
						}
					}

					double targetAngle = m_LastOutQuat.GetAng(targetQuat, Vector3()) * 180.0 / M_PI;
					double angle = CalcDeltaExpSmooth(t, targetAngle);

					if (abs(angle) > AFX_MATH_EPS)
					{
						m_LastOutQuat = m_LastOutQuat.Slerp(targetQuat, angle / targetAngle).Normalized();
					}

					QEulerAngles newAngles = m_LastOutQuat.ToQREulerAngles().ToQEulerAngles();

					outAngles.x = (float)newAngles.Pitch;
					outAngles.y = (float)newAngles.Yaw;
					outAngles.z = (float)newAngles.Roll;
				}
				else
				{
					outAngles.x = parentAngles.x;
					outAngles.y = parentAngles.y;
					outAngles.z = parentAngles.z;
					m_LastOutQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(outAngles.x, outAngles.y, outAngles.z)));
				}

				if (m_HalfTimeVec)
				{
					double t = deltaT / m_HalfTimeVec;

					m_LastX = outVector.x = (float)CalcExpSmooth(t, m_LastX, parentVector.x);
					m_LastY = outVector.y = (float)CalcExpSmooth(t, m_LastY, parentVector.y);
					m_LastZ = outVector.z = (float)CalcExpSmooth(t, m_LastZ, parentVector.z);
				}
				else
				{
					m_LastX = outVector.x = parentVector.x;
					m_LastY = outVector.y = parentVector.y;
					m_LastZ = outVector.z = parentVector.z;
				}

				if (m_HalfTimeFov)
				{
					double t = deltaT / m_HalfTimeFov;

					double oldOpposite = 2 * tan(0.5 * m_LastFov * M_PI / 180.0);
					double targetOpposite = 2 * tan(0.5 * parentFov * M_PI / 180.0);

					double newOppposite = CalcExpSmooth(t, oldOpposite, targetOpposite);
					m_LastFov = outFov = (float)(2 * atan(0.5 * newOppposite) * 180.0 / M_PI);
				}
				else
				{
					m_LastFov = outFov = parentFov;
				}
			}

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvCamSmoothCalc()
	{
		m_TrackHandle->Release();
		m_Parent->Release();
	}

private:
	IMirvCamCalc * m_Parent;
	IMirvHandleCalc * m_TrackHandle;

	bool m_Reset = true;
	int m_LevelInitCount = 0;
	SOURCESDK::CSGO::CBaseHandle m_LastHandle;
	float m_LastClientTime = 0;
	bool m_RotShortestPath = true;

	double m_LastX = 0;
	double m_LastY = 0;
	double m_LastZ = 0;

	Quaternion m_LastOutQuat;

	double m_LastFov = 90.0;

	double m_HalfTimeVec = 0.5;
	double m_HalfTimeAng = 0.5;
	double m_HalfTimeFov = 0.5;
};


class CMirvCamVecAngFovCalc : public CMirvCamCalc
{
public:
	CMirvCamVecAngFovCalc(char const * name, IMirvVecAngCalc * vecAng, IMirvFovCalc * fov)
		: CMirvCamCalc(name)
		, m_VecAng(vecAng)
		, m_Fov(fov)
	{
		m_VecAng->AddRef();
		m_Fov->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCamCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"vecAngFov\"");
		Tier0_Msg(", vecAng: "); m_VecAng->Console_PrintBegin(); m_VecAng->Console_PrintEnd();
		Tier0_Msg(", fov: "); m_Fov->Console_PrintBegin(); m_Fov->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCamCalc::Console_Edit(args);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov) override
	{
		SOURCESDK::Vector parentVector;
		SOURCESDK::QAngle parentAngles;
		float parentFov;

		SOURCESDK::CSGO::CBaseHandle handle;

		if (
			m_VecAng->CalcVecAng(parentVector, parentAngles)
			&& m_Fov->CalcFov(parentFov))
		{
			outVector.x = parentVector.x;
			outVector.y = parentVector.y;
			outVector.z = parentVector.z;
			outAngles.x = parentAngles.x;
			outAngles.y = parentAngles.y;
			outAngles.z = parentAngles.z;
			outFov = parentFov;

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvCamVecAngFovCalc()
	{
		m_Fov->Release();
		m_VecAng->Release();
	}

private:
	IMirvVecAngCalc * m_VecAng;
	IMirvFovCalc * m_Fov;
};

class CMirvFovCalc : public CMirvCalc, public IMirvFovCalc
{
public:
	CMirvFovCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();
		Tier0_Msg(", type: \"fov\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcFov(float & outFov)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};

class CMirvVecAngValueCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngValueCalc(char const * name, float x, float y, float z, float rX, float rY, float rZ)
		: CMirvVecAngCalc(name)
	{
		m_Vec.x = x;
		m_Vec.y = y;
		m_Vec.z = z;
		m_Ang.z = rX;
		m_Ang.x = rY;
		m_Ang.y = rZ;
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"value\", x: %f, y: %f, z: %f, rX: %f, rY: %f, rZ: %f", m_Vec.x, m_Vec.y, m_Vec.z, m_Ang.z, m_Ang.x, m_Ang.y);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		outVector = m_Vec;
		outAngles = m_Ang;

		return true;
	}


	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("x", arg1))
			{
				if (3 <= argc)
				{
					m_Vec.x = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s x <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Vec.x
				);
				return;
			}
			else if (0 == _stricmp("y", arg1))
			{
				if (3 <= argc)
				{
					m_Vec.y = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s y <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Vec.y
				);
				return;
			}
			else if (0 == _stricmp("z", arg1))
			{
				if (3 <= argc)
				{
					m_Vec.z = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s z <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Vec.z
				);
				return;
			}
			else if (0 == _stricmp("rX", arg1))
			{
				if (3 <= argc)
				{
					m_Ang.z = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rX <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Ang.z
				);
				return;
			}
			else if (0 == _stricmp("rY", arg1))
			{
				if (3 <= argc)
				{
					m_Ang.x = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rY <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Ang.x
				);
				return;
			}
			else if (0 == _stricmp("rZ", arg1))
			{
				if (3 <= argc)
				{
					m_Ang.y = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rZ <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Ang.y
				);
				return;
			}
		}

		Tier0_Msg(
			"%s x [...]\n"
			"%s y [...]\n"
			"%s z [...]\n"
			"%s rX [...]\n"
			"%s rY [...]\n"
			"%s rZ [...]\n"
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
		);
	}

private:
	SOURCESDK::Vector m_Vec;
	SOURCESDK::QAngle m_Ang;
};


class CMirvVecAngOffsetCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngOffsetCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset, bool order)
		: CMirvVecAngCalc(name)
		, m_Parent(parent)
		, m_Offset(offset)
		, m_Order(order)
	{
		m_Parent->AddRef();
		m_Offset->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"offset\"");
		Tier0_Msg(", parent: "); m_Parent->Console_PrintBegin(); m_Parent->Console_PrintEnd();
		Tier0_Msg(", offset: "); m_Offset->Console_PrintBegin(); m_Offset->Console_PrintEnd();
		Tier0_Msg(", order: %i", m_Order ? 1 : 0);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("order", arg1))
			{
				if (3 <= argc)
				{
					m_Order = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s order <bValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Order ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s order [...]\n"
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::Vector parentVector;
		SOURCESDK::QAngle parentAngles;
		SOURCESDK::Vector offsetVector;
		SOURCESDK::QAngle offsetAngles;
		bool calcedParent = m_Parent->CalcVecAng(parentVector, parentAngles);
		bool calcedOffset = calcedParent && m_Offset->CalcVecAng(offsetVector, offsetAngles);

		if (calcedParent && calcedOffset)
		{
			if (m_Order)
			{
				if (offsetVector.x || offsetVector.y || offsetVector.z)
				{
					double forward[3], right[3], up[3];

					Afx::Math::MakeVectors(parentAngles.z, parentAngles.x, parentAngles.y, forward, right, up);

					outVector.x = (float)(parentVector.x + offsetVector.x*forward[0] - offsetVector.y*right[0] + offsetVector.z*up[0]);
					outVector.y = (float)(parentVector.y + offsetVector.x*forward[1] - offsetVector.y*right[1] + offsetVector.z*up[1]);
					outVector.z = (float)(parentVector.z + offsetVector.x*forward[2] - offsetVector.y*right[2] + offsetVector.z*up[2]);
				}
				else
				{
					outVector = parentVector;
				}

				if (offsetAngles.x || offsetAngles.y || offsetAngles.z)
				{
					Afx::Math::Quaternion q1 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(parentAngles.x, parentAngles.y, parentAngles.z)));
					Afx::Math::Quaternion q2 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(offsetAngles.x, offsetAngles.y, offsetAngles.z)));

					Afx::Math::QEulerAngles angs = (q1 * q2).ToQREulerAngles().ToQEulerAngles();

					outAngles.z = (float)angs.Roll;
					outAngles.x = (float)angs.Pitch;
					outAngles.y = (float)angs.Yaw;
				}
				else
				{
					outAngles = parentAngles;
				}
			}
			else
			{
				double forward[3], right[3], up[3];

				Afx::Math::Quaternion q1 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(parentAngles.x, parentAngles.y, parentAngles.z)));
				Afx::Math::Quaternion q2 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(offsetAngles.x, offsetAngles.y, offsetAngles.z)));

				Afx::Math::QEulerAngles angs = (q1 * q2).ToQREulerAngles().ToQEulerAngles();

				outAngles.z = (float)angs.Roll;
				outAngles.x = (float)angs.Pitch;
				outAngles.y = (float)angs.Yaw;

				Afx::Math::MakeVectors(outAngles.z, outAngles.x, outAngles.y, forward, right, up);

				outVector.x = (float)(parentVector.x + offsetVector.x*forward[0] - offsetVector.y*right[0] + offsetVector.z*up[0]);
				outVector.y = (float)(parentVector.y + offsetVector.x*forward[1] - offsetVector.y*right[1] + offsetVector.z*up[1]);
				outVector.z = (float)(parentVector.z + offsetVector.x*forward[2] - offsetVector.y*right[2] + offsetVector.z*up[2]);
			}
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngOffsetCalc()
	{
		m_Offset->Release();
		m_Parent->Release();
	}

private:
	IMirvVecAngCalc * m_Parent;
	IMirvVecAngCalc * m_Offset;
	bool m_Order;
};

class CMirvVecAngHandleCalcEx : public CMirvVecAngCalc
{
public:
	CMirvVecAngHandleCalcEx(char const * name, IMirvHandleCalc * handle, bool eyeVec, bool eyeAng)
		: CMirvVecAngCalc(name)
		, m_Handle(handle)
		, m_EyeVec(eyeVec)
		, m_EyeAng(eyeAng)
	{
		m_Handle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();
		Tier0_Msg(", fn: \"handleEx\"");
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
		Tier0_Msg(", eyeVec: %i, eyeAng: %i", m_EyeVec ? 1 : 0, m_EyeAng ? 1 : 0);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("eyeVec", arg1))
			{
				if (3 <= argc)
				{
					m_EyeVec = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s eyeVec <bValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_EyeVec ? 1 : 0
				);
				return;
			}
			else if (0 == _stricmp("eyeAng", arg1))
			{
				if (3 <= argc)
				{
					m_EyeAng = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s eyeAng <bValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_EyeAng ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s eyeVec [...]\n"
			"%s eyeAng [...]\n"
			, arg0
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		bool calcedHandle = m_Handle->CalcHandle(handle);
		SOURCESDK::IClientEntity_csgo * ce = calcedHandle ? SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle) : 0;
		SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

		if (be || ce && !m_EyeVec && !m_EyeAng)
		{
			outVector = m_EyeVec ? be->EyePosition() : ce->GetAbsOrigin();
			outAngles = m_EyeAng ? be->EyeAngles() : ce->GetAbsAngles();
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngHandleCalcEx()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
	bool m_EyeVec;
	bool m_EyeAng;
};

class CMirvVecAngHandleAttachmentCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngHandleAttachmentCalc(char const * name, IMirvHandleCalc * handle, char const * attachmentName)
		: CMirvVecAngCalc(name)
		, m_Handle(handle)
		, m_AttachmentName(attachmentName)
	{
		m_Handle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"handleAttachment\"");
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
		Tier0_Msg(", attachmentName: \"%s\"", m_AttachmentName.c_str());
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("attachmentName", arg1))
			{
				if (3 <= argc)
				{
					m_AttachmentName = args->ArgV(2);
					return;
				}

				Tier0_Msg(
					"%s attachmentName <sValue> - Set new value.\n"
					"Current value: %s\n"
					, arg0
					, m_AttachmentName.c_str()
				);
				return;
			}
		}

		Tier0_Msg(
			"%s attachmentName [...]\n"
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		bool calcedHandle = m_Handle->CalcHandle(handle);
		SOURCESDK::IClientEntity_csgo * ce = calcedHandle ? SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle) : 0;

		if (ce)
		{
			int idx = ce->LookupAttachment(m_AttachmentName.c_str());
			if (-1 != idx)
			{
				return ce->GetAttachment(idx, outVector, outAngles);
			}
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngHandleAttachmentCalc()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
	std::string m_AttachmentName;
};

class CMirvVecAngIfCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngIfCalc(char const * name, IMirvBoolCalc * condition, IMirvVecAngCalc * condTrue, IMirvVecAngCalc * condFalse)
		: CMirvVecAngCalc(name)
		, m_Condition(condition)
		, m_CondTrue(condTrue)
		, m_CondFalse(condFalse)
	{
		m_Condition->AddRef();
		m_CondTrue->AddRef();
		m_CondFalse->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"if\"");
		Tier0_Msg(", condition: "); m_Condition->Console_PrintBegin(); m_Condition->Console_PrintEnd();
		Tier0_Msg(", true: "); m_CondTrue->Console_PrintBegin(); m_CondTrue->Console_PrintEnd();
		Tier0_Msg(", false: "); m_CondFalse->Console_PrintBegin(); m_CondFalse->Console_PrintEnd();
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		bool condition;

		bool result = m_Condition->CalcBool(condition);
		
		condition = result && condition;

		return condition ? m_CondTrue->CalcVecAng(outVector, outAngles) : m_CondFalse->CalcVecAng(outVector, outAngles);
	}

protected:
	virtual ~CMirvVecAngIfCalc()
	{
		m_CondFalse->Release();
		m_CondTrue->Release();
		m_Condition->Release();
	}

private:
	IMirvBoolCalc * m_Condition;
	IMirvVecAngCalc * m_CondTrue;
	IMirvVecAngCalc * m_CondFalse;
};


class CMirvVecAngOrCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngOrCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b)
		: CMirvVecAngCalc(name)
		, m_A(a)
		, m_B(b)
	{
		m_A->AddRef();
		m_B->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"or\"");
		Tier0_Msg(", a: "); m_A->Console_PrintBegin(); m_A->Console_PrintEnd();
		Tier0_Msg(", b: "); m_B->Console_PrintBegin(); m_B->Console_PrintEnd();
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		return m_A->CalcVecAng(outVector, outAngles) || m_B->CalcVecAng(outVector, outAngles);
	}

protected:
	virtual ~CMirvVecAngOrCalc()
	{
		m_B->Release();
		m_A->Release();
	}

private:
	IMirvVecAngCalc * m_A;
	IMirvVecAngCalc * m_B;
};


class CMirvVecAngMotionProfile2Calc : public CMirvVecAngCalc
{
public:
	CMirvVecAngMotionProfile2Calc(char const * name, IMirvVecAngCalc * parent, IMirvHandleCalc * trackHandle)
		: CMirvVecAngCalc(name)
		, m_Parent(parent)
		, m_TrackHandle(trackHandle)
	{
		m_Parent->AddRef();
		m_TrackHandle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"motionProfile\"");
		Tier0_Msg(", parent: "); m_Parent->Console_PrintBegin(); m_Parent->Console_PrintEnd();
		Tier0_Msg(", trackHandle: "); m_TrackHandle->Console_PrintBegin(); m_TrackHandle->Console_PrintEnd();
		Tier0_Msg(
			", limXVelo: %f, limXAcel: %f, limYVelo: %f, limYAcel: %f, limZVelo: %f, limZAcel: %f, limAngVelo: %f, limAngAcel: %f\n"
			, m_LimitVelocityX, m_LimitAccelerationX		
			, m_LimitVelocityY, m_LimitAccelerationY
			, m_LimitVelocityZ, m_LimitAccelerationZ
			, m_LimitVelocityAng, m_LimitAccelerationAng
		);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("limXVelo", arg1))
			{
				if (3 <= argc)
				{
					m_LimitVelocityX = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limXVelo <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitVelocityX
				);
				return;
			}
			else if (0 == _stricmp("limXAcel", arg1))
			{
				if (3 <= argc)
				{
					m_LimitAccelerationX = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limXAcel <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitAccelerationX
				);
				return;
			}
			else if (0 == _stricmp("limYVelo", arg1))
			{
				if (3 <= argc)
				{
					m_LimitVelocityY = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limYVelo <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitVelocityY
				);
				return;
			}
			else if (0 == _stricmp("limYAcel", arg1))
			{
				if (3 <= argc)
				{
					m_LimitAccelerationY = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limYAcel <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitAccelerationY
				);
				return;
			}
			else if (0 == _stricmp("limZVelo", arg1))
			{
				if (3 <= argc)
				{
					m_LimitVelocityZ = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limZVelo <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitVelocityZ
				);
				return;
			}
			else if (0 == _stricmp("limZAcel", arg1))
			{
				if (3 <= argc)
				{
					m_LimitAccelerationZ = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limZAcel <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitAccelerationZ
				);
				return;
			}
			else if (0 == _stricmp("limAngVelo", arg1))
			{
				if (3 <= argc)
				{
					m_LimitVelocityAng = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limAngVelo <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitVelocityAng
				);
				return;
			}
			else if (0 == _stricmp("limAngAcel", arg1))
			{
				if (3 <= argc)
				{
					m_LimitAccelerationAng = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s limAngAcel <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_LimitAccelerationAng
				);
				return;
			}
			else if (0 == _stricmp("rotShortestPath", arg1))
			{
				if (3 <= argc)
				{
					m_RotShortestPath = 0 != atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rotShortestPath 0|1 - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_RotShortestPath ? 1: 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s limXVelo [...] - X-location velocity limit in inch.\n"
			"%s limXAcel [...] - X-location acceleration limit in inch.\n"
			"%s limYVelo [...] - y-location velocity limit in inch.\n"
			"%s limYAcel [...] - Y-location acceleration limit in inch.\n"
			"%s limZVelo [...] - Z-location velocity limit in inch.\n"
			"%s limZAcel [...] - Z-location acceleration limit in inch.\n"
			"%s limAngVelo [...] - Angular velocity limit in degrees.\n"
			"%s limAngAcel [...] - Angular acceleration limit in degrees.\n"
			"%s rotShortestPath [...] - If to rotate shortest path.\n"
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::Vector parentVector;
		SOURCESDK::QAngle parentAngles;

		SOURCESDK::CSGO::CBaseHandle handle;

		bool calcedParent = m_Parent->CalcVecAng(parentVector, parentAngles);
		bool calcedHandle = m_TrackHandle->CalcHandle(handle);

		if (m_LevelInitCount != g_LevelInitCount)
		{
			m_LevelInitCount = g_LevelInitCount;
			m_Reset = true;
		}

		m_Reset = m_Reset || !(calcedParent && calcedHandle && m_LastHandle == handle);

		if (calcedHandle) m_LastHandle = handle;

		if (calcedParent && calcedHandle)
		{
			if (m_Reset)
			{
				m_Reset = false;

				m_LastClientTime = g_MirvTime.GetTime();

				m_LastX = parentVector.x;
				m_LastY = parentVector.y;
				m_LastZ = parentVector.z;

				m_XVelocity = 0;
				m_YVelocity = 0;
				m_ZVelocity = 0;

				outVector.x = (float)m_LastX;
				outVector.y = (float)m_LastY;
				outVector.z = (float)m_LastZ;

				outAngles.x = parentAngles.x;
				outAngles.y = parentAngles.y;
				outAngles.z = parentAngles.z;
				m_LastAngVelocity = 0;
				m_LastOutQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(outAngles.x, outAngles.y, outAngles.z)));
			}
			else
			{
				float clientTime = g_MirvTime.GetTime();
				double deltaT = clientTime - m_LastClientTime;
				m_LastClientTime = clientTime;

				Quaternion targetQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(parentAngles.x, parentAngles.y, parentAngles.z))).Normalized();

				if (m_RotShortestPath)
				{
					// Take shortest path:
					double dotProduct = DotProduct(targetQuat, m_LastOutQuat);
					if (dotProduct < 0)
					{
						targetQuat = -1.0 * targetQuat;
					}
				}

				double targetAngle = m_LastOutQuat.GetAng(targetQuat, Vector3()) * 180.0 / M_PI;
				double angle;

				CalcDeltaSmooth(deltaT, targetAngle, angle, m_LastAngVelocity, m_LimitVelocityAng, m_LimitAccelerationAng);

				if (abs(angle) > AFX_MATH_EPS)
				{
					m_LastOutQuat = m_LastOutQuat.Slerp(targetQuat, angle / targetAngle).Normalized();
				}

				QEulerAngles newAngles = m_LastOutQuat.ToQREulerAngles().ToQEulerAngles();

				outAngles.x = (float)newAngles.Pitch;
				outAngles.y = (float)newAngles.Yaw;
				outAngles.z = (float)newAngles.Roll;

				//

				CalcSmooth(deltaT, parentVector.x, m_LastX, m_XVelocity, m_LimitVelocityX, m_LimitAccelerationX);
				outVector.x = (float)m_LastX;

				CalcSmooth(deltaT, parentVector.y, m_LastY, m_YVelocity, m_LimitVelocityY, m_LimitAccelerationY);
				outVector.y = (float)m_LastY;

				CalcSmooth(deltaT, parentVector.z, m_LastZ, m_ZVelocity, m_LimitVelocityZ, m_LimitAccelerationZ);
				outVector.z = (float)m_LastZ;
			}

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngMotionProfile2Calc()
	{
		m_TrackHandle->Release();
		m_Parent->Release();
	}

private:
	IMirvVecAngCalc * m_Parent;
	IMirvHandleCalc * m_TrackHandle;

	bool m_Reset = true;
	int m_LevelInitCount = 0;
	SOURCESDK::CSGO::CBaseHandle m_LastHandle;
	float m_LastClientTime = 0;
	bool m_RotShortestPath = true;

	double m_LastX = 0;
	double m_LastY = 0;
	double m_LastZ = 0;
	double m_XVelocity = 0;
	double m_ZVelocity = 0;
	double m_YVelocity = 0;

	double m_LimitVelocityX = 600;
	double m_LimitAccelerationX = 60;
	double m_LimitVelocityY = 600;
	double m_LimitAccelerationY = 60;
	double m_LimitVelocityZ = 600;
	double m_LimitAccelerationZ = 60;

	Quaternion m_LastOutQuat;
	double m_LastAngVelocity = 0;

	double m_LimitVelocityAng = 360;
	double m_LimitAccelerationAng = 90;
};

class CMirvVecAngAddCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngAddCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b)
		: CMirvVecAngCalc(name)
		, m_A(a)
		, m_B(b)
	{
		m_A->AddRef();
		m_B->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"add\"");
		Tier0_Msg(", a: "); m_A->Console_PrintBegin(); m_A->Console_PrintEnd();
		Tier0_Msg(", b: "); m_B->Console_PrintBegin(); m_B->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvVecAngCalc::Console_Edit(args);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::Vector aVector;
		SOURCESDK::QAngle aAngles;
		SOURCESDK::Vector bVector;
		SOURCESDK::QAngle bAngles;
		bool cacledA = m_A->CalcVecAng(aVector, aAngles);
		bool calcedB = cacledA && m_B->CalcVecAng(bVector, bAngles);

		if (cacledA && calcedB)
		{
			// Add location:

			outVector.x = aVector.x + bVector.x;
			outVector.y = aVector.y + bVector.y;
			outVector.z = aVector.z + bVector.z;

			// Add rotation:

			Quaternion quatA = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(aAngles.x, aAngles.y, aAngles.z)));
			Quaternion quatB = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(bAngles.x, bAngles.y, bAngles.z)));

			Quaternion quatR = quatB * quatA;

			QEulerAngles angles = quatR.ToQREulerAngles().ToQEulerAngles();

			outAngles.x = (float)angles.Pitch;
			outAngles.y = (float)angles.Yaw;
			outAngles.z = (float)angles.Roll;

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngAddCalc()
	{
		m_B->Release();
		m_A->Release();
	}

private:
	IMirvVecAngCalc * m_A;
	IMirvVecAngCalc * m_B;
};

class CMirvVecAngSubtractCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngSubtractCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b)
		: CMirvVecAngCalc(name)
		, m_A(a)
		, m_B(b)
	{
		m_A->AddRef();
		m_B->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"subtract\"");
		Tier0_Msg(", a: "); m_A->Console_PrintBegin(); m_A->Console_PrintEnd();
		Tier0_Msg(", b: "); m_B->Console_PrintBegin(); m_B->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvVecAngCalc::Console_Edit(args);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::Vector sourceVector;
		SOURCESDK::QAngle sourceAngles;
		SOURCESDK::Vector targetVector;
		SOURCESDK::QAngle targetAngles;
		bool cacledSource = m_B->CalcVecAng(sourceVector, sourceAngles);
		bool calcedTarget = cacledSource && m_A->CalcVecAng(targetVector, targetAngles);

		if (cacledSource && calcedTarget)
		{
			// Delta location:

			outVector.x = targetVector.x - sourceVector.x;
			outVector.y = targetVector.y - sourceVector.y;
			outVector.z = targetVector.z - sourceVector.z;

			// Delta rotation:

			Quaternion quatSource = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(sourceAngles.x, sourceAngles.y, sourceAngles.z)));
			Quaternion quatDest = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(targetAngles.x, targetAngles.y, targetAngles.z)));

			// Make sure we take the shortest path:
			double dotProduct = DotProduct(quatDest, quatSource);
			if (dotProduct < 0.0)
			{
				quatSource = -1.0 * quatSource;
			}

			Quaternion quatR = quatDest * (/*(1.0 / quatSource.Norm()) * */quatSource.Conjugate());

			QEulerAngles angles = quatR.ToQREulerAngles().ToQEulerAngles();

			outAngles.x = (float)angles.Pitch;
			outAngles.y = (float)angles.Yaw;
			outAngles.z = (float)angles.Roll;


			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngSubtractCalc()
	{
		m_B->Release();
		m_A->Release();
	}

private:
	IMirvVecAngCalc * m_A;
	IMirvVecAngCalc * m_B;
};

class CMirvVecAngSwitchInterpCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngSwitchInterpCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * switchHandle, IMirvHandleCalc * resetHandle, float holdTime, float interpTime)
		: CMirvVecAngCalc(name)
		, m_Source(source)
		, m_SwitchHandle(switchHandle)
		, m_ResetHandle(resetHandle)
		, m_HoldTime(holdTime)
		, m_InterpTime(interpTime)
		, m_LastPos(0.0 , 0.0, 0.0)
		, m_LastQuat(1.0, 0.0, 0.0, 0.0)
	{
		m_Source->AddRef();
		m_SwitchHandle->AddRef();
		m_ResetHandle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"switchInterp\"");
		Tier0_Msg(", source: "); m_Source->Console_PrintBegin(); m_Source->Console_PrintEnd();
		Tier0_Msg(", switchHandle: "); m_SwitchHandle->Console_PrintBegin(); m_SwitchHandle->Console_PrintEnd();
		Tier0_Msg(", resetHandle: "); m_ResetHandle->Console_PrintBegin(); m_ResetHandle->Console_PrintEnd();
		Tier0_Msg(", holdTime: %f, interpTime: %f", (float)m_HoldTime, (float)m_InterpTime);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("holdTime", arg1))
			{
				if (3 <= argc)
				{
					m_HoldTime = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s holdTime <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_HoldTime
				);
				return;
			}
			else if (0 == _stricmp("interpTime", arg1))
			{
				if (3 <= argc)
				{
					m_InterpTime = atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s interpTime <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_InterpTime
				);
				return;
			}
		}

		Tier0_Msg(
			"%s holdTime [...].\n"
			"%s interpTime [...]\n"
			, arg0
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		SOURCESDK::CSGO::CBaseHandle resetHandle;

		if (m_SwitchHandle->CalcHandle(handle) && m_ResetHandle->CalcHandle(resetHandle))
		{
			float curTime = g_MirvTime.GetTime();

			if (m_LastResetHandle != resetHandle)
			{
				m_LastResetHandle = resetHandle;
				m_Mode = Mode_Init;
			}

			if (Mode_Init == m_Mode)
			{
				m_LastHandle = handle;
				m_Mode = Mode_Pasthrough;
			}

			if (m_LastHandle != handle)
			{
				m_LastHandle = handle;
				m_Mode = Mode_Hold;
				m_SwitchedTime = curTime;
			}

			if (Mode_Hold == m_Mode)
			{
				if (m_HoldTime <= std::fabs(curTime - m_SwitchedTime))
					m_Mode = Mode_Interp;
			}

			if (Mode_Interp == m_Mode)
			{
				if ((m_InterpTime + m_HoldTime) <= std::fabs(curTime - m_SwitchedTime))
					m_Mode = Mode_Pasthrough;
			}

			SOURCESDK::Vector sourceVector;
			SOURCESDK::QAngle sourceAngles;

			if (m_Source->CalcVecAng(sourceVector, sourceAngles))
			{
				switch (m_Mode)
				{
				case Mode_Hold:
				{
					outVector.x = (float)m_LastPos.X;
					outVector.y = (float)m_LastPos.Y;
					outVector.z = (float)m_LastPos.Z;
					QEulerAngles angles = m_LastQuat.ToQREulerAngles().ToQEulerAngles();
					outAngles.x = (float)angles.Pitch;
					outAngles.y = (float)angles.Yaw;
					outAngles.z = (float)angles.Roll;
					return true;
				}
				case Mode_Interp:
				{
					double t = 0 != m_InterpTime ? (curTime - m_HoldTime - m_SwitchedTime) / m_InterpTime : 0;
	
					Vector3 pos = (1 - t) * m_LastPos + t * Vector3((double)sourceVector.x, (double)sourceVector.y, (double)sourceVector.z);
					
					outVector.x = (float)pos.X;
					outVector.y = (float)pos.Y;
					outVector.z = (float)pos.Z;

					Quaternion sourceQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(sourceAngles.x, sourceAngles.y, sourceAngles.z))).Normalized();

					double dotProduct = DotProduct(sourceQuat, m_LastQuat);

					if (dotProduct < 0)
					{
						sourceQuat = -1.0 * sourceQuat;
					}
					
					QEulerAngles newAngles = m_LastQuat.Slerp(sourceQuat, t).Normalized().ToQREulerAngles().ToQEulerAngles();

					outAngles.x = (float)newAngles.Pitch;
					outAngles.y = (float)newAngles.Yaw;
					outAngles.z = (float)newAngles.Roll;

					return true;
				}
				default:
					outVector = sourceVector;
					outAngles = sourceAngles;
					m_LastPos.X = sourceVector.x;
					m_LastPos.Y = sourceVector.y;
					m_LastPos.Z = sourceVector.z;
					m_LastQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(sourceAngles.x, sourceAngles.y, sourceAngles.z)));
					return true;
				}
			}
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngSwitchInterpCalc()
	{
		m_ResetHandle->Release();
		m_SwitchHandle->Release();
		m_Source->Release();
	}

private:
	IMirvVecAngCalc * m_Source;
	IMirvHandleCalc * m_SwitchHandle;
	IMirvHandleCalc * m_ResetHandle;
	double m_HoldTime;
	double m_InterpTime;
	enum Mode_e {
		Mode_Init,
		Mode_Pasthrough,
		Mode_Hold,
		Mode_Interp
	} m_Mode = Mode_Init;
	float m_SwitchedTime = 0;
	SOURCESDK::CSGO::CBaseHandle m_LastHandle;
	SOURCESDK::CSGO::CBaseHandle m_LastResetHandle;
	Vector3 m_LastPos;
	Quaternion m_LastQuat;
};


class CMirvVecAngLocalToGlobalCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngLocalToGlobalCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * handle)
		: CMirvVecAngCalc(name)
		, m_Source(source)
		, m_Handle(handle)
	{
		m_Source->AddRef();
		m_Handle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"localToGlobal\"");
		Tier0_Msg(", source: "); m_Source->Console_PrintBegin(); m_Source->Console_PrintEnd();
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvVecAngCalc::Console_Edit(args);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		SOURCESDK::Vector sourceVector;
		SOURCESDK::QAngle sourceAngles;

		if (m_Handle->CalcHandle(handle) && m_Source->CalcVecAng(sourceVector, sourceAngles))
		{
			SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle);
			SOURCESDK::IClientRenderable_csgo * re = ce ? ce->GetClientRenderable() : 0;

			if (re)
			{
				const SOURCESDK::matrix3x4_t & matrix = re->RenderableToWorldTransform();
				
				SOURCESDK::VectorTransform(sourceVector, matrix, outVector);

				SOURCESDK::QAngle matrixAngles;

				SOURCESDK::MatrixAngles(matrix, matrixAngles);

				Quaternion sourceQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(sourceAngles.x, sourceAngles.y, sourceAngles.z)));
				Quaternion matrixQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(matrixAngles.x, matrixAngles.y, matrixAngles.z)));

				QEulerAngles angles = (matrixQuat * sourceQuat).ToQREulerAngles().ToQEulerAngles();
				outAngles.x = (float)angles.Pitch;
				outAngles.y = (float)angles.Yaw;
				outAngles.z = (float)angles.Roll;

				return true;
			}

			return false;

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngLocalToGlobalCalc()
	{
		m_Handle->Release();
		m_Source->Release();
	}

private:
	IMirvVecAngCalc * m_Source;
	IMirvHandleCalc * m_Handle;
};

class CMirvVecAngGlobalToLocalCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngGlobalToLocalCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * handle)
		: CMirvVecAngCalc(name)
		, m_Source(source)
		, m_Handle(handle)
	{
		m_Source->AddRef();
		m_Handle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"globalToLocal\"");
		Tier0_Msg(", source: "); m_Source->Console_PrintBegin(); m_Source->Console_PrintEnd();
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvVecAngCalc::Console_Edit(args);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		SOURCESDK::Vector sourceVector;
		SOURCESDK::QAngle sourceAngles;

		if (m_Handle->CalcHandle(handle) && m_Source->CalcVecAng(sourceVector, sourceAngles))
		{
			SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle);
			SOURCESDK::IClientRenderable_csgo * re = ce ? ce->GetClientRenderable() : 0;

			if (re)
			{
				const SOURCESDK::matrix3x4_t & matrix = re->RenderableToWorldTransform();

				SOURCESDK::matrix3x4_t transposed;
				
				transposed[0][0] = matrix[0][0];
				transposed[0][1] = matrix[1][0];
				transposed[0][2] = matrix[2][0];

				transposed[1][0] = matrix[0][1];
				transposed[1][1] = matrix[1][1];
				transposed[1][2] = matrix[2][1];

				transposed[2][0] = matrix[0][2];
				transposed[2][1] = matrix[1][2];
				transposed[2][2] = matrix[2][2];

				float tmp[3];
				tmp[0] = matrix[0][3];
				tmp[1] = matrix[1][3];
				tmp[2] = matrix[2][3];

				transposed[0][3] = -SOURCESDK::DotProduct(tmp, transposed[0]);
				transposed[1][3] = -SOURCESDK::DotProduct(tmp, transposed[1]);
				transposed[2][3] = -SOURCESDK::DotProduct(tmp, transposed[2]);

				SOURCESDK::VectorTransform(sourceVector, transposed, outVector);

				SOURCESDK::QAngle matrixAngles;

				SOURCESDK::MatrixAngles(transposed, matrixAngles);

				Quaternion sourceQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(sourceAngles.x, sourceAngles.y, sourceAngles.z)));
				Quaternion matrixQuat = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(matrixAngles.x, matrixAngles.y, matrixAngles.z)));

				QEulerAngles angles = (matrixQuat * sourceQuat).ToQREulerAngles().ToQEulerAngles();
				outAngles.x = (float)angles.Pitch;
				outAngles.y = (float)angles.Yaw;
				outAngles.z = (float)angles.Roll;

				return true;
			}

			return false;

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngGlobalToLocalCalc()
	{
		m_Handle->Release();
		m_Source->Release();
	}

private:
	IMirvVecAngCalc * m_Source;
	IMirvHandleCalc * m_Handle;
};


extern SOURCESDK::CSGO::IEngineTrace * g_pClientEngineTrace;

class CMirvVecAngCollisionClipCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngCollisionClipCalc(char const * name, IMirvVecAngCalc * source, IMirvVecAngCalc * clipTo, IMirvHandleCalc * ignoreHandle, float safeDist)
		: CMirvVecAngCalc(name)
		, m_Source(source)
		, m_ClipTo(clipTo)
		, m_Ignore(ignoreHandle)
		, m_SafeZoneDist(safeDist)
	{
		m_Source->AddRef();
		m_ClipTo->AddRef();
		m_Ignore->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"globalToLocal\"");
		Tier0_Msg(", source: "); m_Source->Console_PrintBegin(); m_Source->Console_PrintEnd();
		Tier0_Msg(", clipTo: "); m_ClipTo->Console_PrintBegin(); m_ClipTo->Console_PrintEnd();
		Tier0_Msg(", ignore: "); m_Ignore->Console_PrintBegin(); m_Ignore->Console_PrintEnd();
		Tier0_Msg(", safeZoneDist: %f", m_SafeZoneDist);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("safeZoneDist", arg1))
			{
				if (3 <= argc)
				{
					m_SafeZoneDist = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s safeZoneDist <fSaveZoneDist> - Set safe zone distance.\n"
					"Current value: %f\n"
					, arg0
					, m_SafeZoneDist
				);
				return;
			}
		}

		Tier0_Msg(
			"%s safeZoneDist [...]\n"
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle ignore;
		SOURCESDK::Vector sourceVector;
		SOURCESDK::QAngle sourceAngles;
		SOURCESDK::Vector clipToVector;
		SOURCESDK::QAngle clipToAngles;

		if (m_Ignore->CalcHandle(ignore) && m_Source->CalcVecAng(sourceVector, sourceAngles) && m_ClipTo->CalcVecAng(clipToVector, clipToAngles) )
		{
			SOURCESDK::IClientEntity_csgo* ignoreEnt = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(ignore);
			CTraceFilter traceFilter;
			if(ignoreEnt) traceFilter.Add(ignoreEnt);

			SOURCESDK::C_BaseEntity_csgo * localPlayer = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(CClientToolsCsgo::Instance()->GetClientToolsInterface()->GetLocalPlayer());
			if(localPlayer && localPlayer->GetIClientEntity() != ignoreEnt) traceFilter.Add(localPlayer);

			SOURCESDK::Vector traceEnd = sourceVector;
			SOURCESDK::Vector vDirOrg = (sourceVector - clipToVector);
			SOURCESDK::Vector vTrace = vDirOrg;
			float sourceLenSqr = vTrace.LengthSqr();
			float bestLenSqr = sourceLenSqr;
			if(sourceLenSqr >= AFX_MATH_EPS) {
				float sourceLen = std::sqrtf(sourceLenSqr);
				vDirOrg *= 1 / sourceLen;
				vTrace = vDirOrg;
				vTrace *= SOURCESDK_CSGO_MAX_TRACE_LENGTH;
				traceEnd = clipToVector + vTrace;
			}

			// c^2 = d^2 + d^2
			// c = sqrt(2) * d
			const float ofsLen = 1.4142135623730950488016887242097f * m_SafeZoneDist;

			float ofsSourceVecs[5][3] = {
				{0,0,0},
				{0,ofsLen,ofsLen},
				{0,ofsLen,-ofsLen},
				{0,-ofsLen,-ofsLen},
				{0,-ofsLen,ofsLen},
			};

			SOURCESDK::QAngle traceAngs;
			SOURCESDK::VectorAngles(vDirOrg,traceAngs);
			SOURCESDK::matrix3x4_t traceRotMatrix;
			SOURCESDK::AngleMatrix(traceAngs,traceRotMatrix);

			for(int i =0; i < 5; ++i) {
				SOURCESDK::Vector vOffs;
				SOURCESDK::VectorTransform(SOURCESDK::Vector(ofsSourceVecs[i][0],ofsSourceVecs[i][1],ofsSourceVecs[i][2]), traceRotMatrix, vOffs);

				SOURCESDK::Vector srcVec = clipToVector + vOffs;
				SOURCESDK::Vector dstVec = traceEnd + vOffs;

				SOURCESDK::CSGO::Ray_t ray;
				ray.Init(srcVec, dstVec);

				SOURCESDK::CSGO::trace_t tr;
				g_pClientEngineTrace->TraceRay(ray, SOURCESDK_CSGO_MASK_ALL, &traceFilter, &tr);

				if (tr.DidHit())
				{
					SOURCESDK::Vector vDir = tr.endpos - tr.startpos;
					float curLenSqr = vDir.LengthSqr();

					//Tier0_Msg("[%d]: %f\n", i, std::sqrtf(vDir.LengthSqr()));

					if(curLenSqr > bestLenSqr + m_SafeZoneDist) {
						continue;	
					}

					if(tr.allsolid || m_SafeZoneDist <= AFX_MATH_EPS) {
						// plane invalid
						// OR safezone is 0
						bestLenSqr = curLenSqr;
						continue;
					}

					// https://en.wikipedia.org/wiki/Intercept_theorem
					// https://math.stackexchange.com/a/100783 (Projecting tr.startpos onto the plane using the plane normal)

					float s = tr.plane.normal.LengthSqr();
					float c = AFX_MATH_EPS <= s ? -(vDir.x * tr.plane.normal.x + vDir.y * tr.plane.normal.y + vDir.z * tr.plane.normal.z) / tr.plane.normal.LengthSqr() : 0;
					SOURCESDK::Vector cV = tr.plane.normal;
					cV *= c;
					c = std::sqrtf(cV.LengthSqr());
					float t = 0 != c ? 1 - m_SafeZoneDist / c : 1;

					if(t < 0) t = 0;
					else if(t > 1) t = 1;

					vDir *= t;
					bestLenSqr = std::min(bestLenSqr, vDir.LengthSqr());
					continue;
				}
			}

			if(AFX_MATH_EPS >= std::abs(sourceLenSqr - bestLenSqr)) {
				// Not much of a change ...
				outVector = sourceVector;
				outAngles = sourceAngles;
			} else {
				vDirOrg *= std::sqrtf(bestLenSqr);
				outVector = clipToVector + vDirOrg;
				outAngles = sourceAngles;
			}

			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngCollisionClipCalc()
	{
		m_Ignore->Release();
		m_ClipTo->Release();
		m_Source->Release();
	}

private:
	IMirvVecAngCalc * m_Source;
	IMirvVecAngCalc * m_ClipTo;
	IMirvHandleCalc * m_Ignore;
	float m_SafeZoneDist;

	class CTraceFilter : public SOURCESDK::CSGO::CTraceFilter
	{
	public:
		CTraceFilter() {

		}

		void Add(SOURCESDK::CSGO::IHandleEntity *pHandleEntity) {
			m_PassEnts.emplace(pHandleEntity);
		}
		
		virtual bool ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask) {
			return pHandleEntity && m_PassEnts.end() == m_PassEnts.find(pHandleEntity);
		}

	private:
		std::set<const SOURCESDK::CSGO::IHandleEntity *> m_PassEnts;
	};

};

class CMirvCamCamCalc : public CMirvCamCalc
{
public:
	CMirvCamCamCalc(char const * name, const char * camFileName, const char * startClientTime)
		: CMirvCamCalc(name)
		, m_CamFileName(camFileName)
		, m_StartClientTime(StartClientTimeFromString(startClientTime))
	{
		m_CamImport = new CamImport(m_CamFileName.c_str(), m_StartClientTime);
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCamCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"cam\", camFileName: \"%s\", startClientTime: %f", m_CamFileName.c_str(), m_StartClientTime);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("filePath", arg1))
			{
				if (3 <= argc)
				{
					m_CamFileName = args->ArgV(2);
					delete m_CamImport;
					m_CamImport = new CamImport(m_CamFileName.c_str(), m_StartClientTime);
					if(m_CamImport->IsBad()) Tier0_Warning("Error importing CAM file \"%s\"\n", m_CamFileName.c_str());
					return;
				}

				Tier0_Msg(
					"%s filePath <sFilePath> - Set mirv_camio input file name / path.\n"
					"Current value: %s\n"
					, arg0
					, m_CamFileName.c_str()
				);
				return;
			}
			else if (0 == _stricmp("startTime", arg1))
			{
				if (3 <= argc)
				{
					m_StartClientTime = StartClientTimeFromString(args->ArgV(2));
					m_CamImport->SetStart(m_StartClientTime);
					return;
				}

				Tier0_Msg(
					"%s startTime <fStartTime>|current - Set mirv_camio input file name / path.\n"
					"Current value: %f\n"
					, arg0
					, m_StartClientTime
				);
				return;
			}
		}

		Tier0_Msg(
			"%s filePath [...]\n"
			"%s startTime [...]\n"
			, arg0
			, arg0
		);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov)
	{
		CamIO::CamData outCamData;

		if (m_CamImport->GetCamData(g_MirvTime.GetTime(), g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, outCamData))
		{
			outVector.x = (float)outCamData.XPosition;
			outVector.y = (float)outCamData.YPosition;
			outVector.z = (float)outCamData.ZPosition;
			outAngles.x = (float)outCamData.YRotation;
			outAngles.y = (float)outCamData.ZRotation;
			outAngles.z = (float)outCamData.XRotation;
			outFov = (float)outCamData.Fov;

			return true;
		}

		return false;
	}

	virtual bool IsBad() {
		return m_CamImport->IsBad();
	}

protected:
	virtual ~CMirvCamCamCalc()
	{
		delete m_CamImport;
	}

private:
	CamImport * m_CamImport;
	std::string m_CamFileName;
	double m_StartClientTime;

	double StartClientTimeFromString(const char * startClientTime)
	{
		return 0 == _stricmp(startClientTime, "current") ? g_MirvTime.GetTime() : atof(startClientTime);
	}
};

class CMirvCamGameCalc : public CMirvCamCalc
{
public:
	CMirvCamGameCalc(char const * name)
		: CMirvCamCalc(name)
	{
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCamCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"game\"");
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCamCalc::Console_Edit(args);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov)
	{
		outVector.x = g_Hook_VClient_RenderView.GameCameraOrigin[0];
		outVector.y = g_Hook_VClient_RenderView.GameCameraOrigin[1];
		outVector.z = g_Hook_VClient_RenderView.GameCameraOrigin[2];
		outAngles.x = g_Hook_VClient_RenderView.GameCameraAngles[0];
		outAngles.y = g_Hook_VClient_RenderView.GameCameraAngles[1];
		outAngles.z = g_Hook_VClient_RenderView.GameCameraAngles[2];
		outFov = g_Hook_VClient_RenderView.GameCameraFov;

		return true;
	}

protected:
	virtual ~CMirvCamGameCalc()
	{
	}

private:

};


class CMirvCamCurrentCalc : public CMirvCamCalc
{
public:
	CMirvCamCurrentCalc(char const * name)
		: CMirvCamCalc(name)
	{
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCamCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"current\"");
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCamCalc::Console_Edit(args);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov)
	{
		outVector.x = (float)g_Hook_VClient_RenderView.CurrentCameraOrigin[0];
		outVector.y = (float)g_Hook_VClient_RenderView.CurrentCameraOrigin[1];
		outVector.z = (float)g_Hook_VClient_RenderView.CurrentCameraOrigin[2];
		outAngles.x = (float)g_Hook_VClient_RenderView.CurrentCameraAngles[0];
		outAngles.y = (float)g_Hook_VClient_RenderView.CurrentCameraAngles[1];
		outAngles.z = (float)g_Hook_VClient_RenderView.CurrentCameraAngles[2];
		outFov = (float)g_Hook_VClient_RenderView.CurrentCameraFov;

		return true;
	}

protected:
	virtual ~CMirvCamCurrentCalc()
	{
	}

private:

};


class CMirvCamPlayerCalc : public CMirvCamCalc
{
public:
	CMirvCamPlayerCalc(char const* name, IMirvHandleCalc * handleCalc)
		: CMirvCamCalc(name)
		, m_HandleCalc(handleCalc)
	{
		m_HandleCalc->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCamCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"player\"");
	}

	virtual void Console_Edit(IWrpCommandArgs* args)
	{
		CMirvCamCalc::Console_Edit(args);
	}

	virtual bool CalcCam(SOURCESDK::Vector& outVector, SOURCESDK::QAngle& outAngles, float& outFov)
	{
		SOURCESDK::CSGO::CBaseHandle handle;

		if (m_HandleCalc->CalcHandle(handle) && handle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo* clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle);
			SOURCESDK::C_BaseEntity_csgo* baseEntity = clientEntity ? clientEntity->GetBaseEntity() : nullptr;
			if (baseEntity && baseEntity->IsPlayer())
			{
				float dummy1 = 0;
				float dummy2 = 1;
				SOURCESDK::C_BasePlayer_csgo* player = (SOURCESDK::C_BasePlayer_csgo*)baseEntity;
				player->CalcView(outVector, outAngles, dummy1, dummy2, outFov);
				return true;
			}
		}

		return false;
	}

protected:
	virtual ~CMirvCamPlayerCalc()
	{
		m_HandleCalc->Release();
	}

private:
	IMirvHandleCalc * m_HandleCalc;
};

class CMirvVecAngCamCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngCamCalc(char const * name, IMirvCamCalc * cam)
		: CMirvVecAngCalc(name)
		, m_Cam(cam)
	{
		m_Cam->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvVecAngCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"cam\"");
		Tier0_Msg(", cam: "); m_Cam->Console_PrintBegin(); m_Cam->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvVecAngCalc::Console_Edit(args);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		float dummyFov;

		return m_Cam->CalcCam(outVector, outAngles, dummyFov);
	}

protected:
	virtual ~CMirvVecAngCamCalc()
	{
		m_Cam->Release();
	}

private:
	IMirvCamCalc * m_Cam;
};

class CMirvFovCamCalc : public CMirvFovCalc
{
public:
	CMirvFovCamCalc(char const * name, IMirvCamCalc * cam)
		: CMirvFovCalc(name)
		, m_Cam(cam)
	{
		m_Cam->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvFovCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"cam\"");
		Tier0_Msg(", cam: "); m_Cam->Console_PrintBegin(); m_Cam->Console_PrintEnd();
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvFovCalc::Console_Edit(args);
	}

	virtual bool CalcFov(float & outFov)
	{
		SOURCESDK::Vector dummyVector;
		SOURCESDK::QAngle dummyAngles;

		return m_Cam->CalcCam(dummyVector, dummyAngles, outFov);
	}

protected:
	virtual ~CMirvFovCamCalc()
	{
		m_Cam->Release();
	}

private:
	IMirvCamCalc * m_Cam;
};

class CMirvBoolCalc : public CMirvCalc, public IMirvBoolCalc
{
public:
	CMirvBoolCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();

		Tier0_Msg(", type: \"bool\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcBool(void)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};

class CMirvBoolAndCalc : public CMirvBoolCalc
{
public:
	CMirvBoolAndCalc(char const * name, IMirvBoolCalc * calcA, IMirvBoolCalc * calcB)
		: CMirvBoolCalc(name)
		, m_CalcA(calcA)
		, m_CalcB(calcB)
	{
		m_CalcA->AddRef();
		m_CalcB->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"and\"");
		Tier0_Msg(", calcA: "); m_CalcA->Console_PrintBegin(); m_CalcA->Console_PrintEnd();
		Tier0_Msg(", calcB: "); m_CalcB->Console_PrintBegin(); m_CalcB->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		bool calcValueA, calcValueB;

		if (m_CalcA->CalcBool(calcValueA) && m_CalcB->CalcBool(calcValueB))
		{
			outResult = calcValueA || calcValueB;
			return true;
		}
		
		return false;
	}

protected:
	virtual ~CMirvBoolAndCalc()
	{
		m_CalcB->Release();
		m_CalcA->Release();
	}

private:
	IMirvBoolCalc * m_CalcA;
	IMirvBoolCalc * m_CalcB;
};

class CMirvBoolOrCalc : public CMirvBoolCalc
{
public:
	CMirvBoolOrCalc(char const * name, IMirvBoolCalc * calcA, IMirvBoolCalc * calcB)
		: CMirvBoolCalc(name)
		, m_CalcA(calcA)
		, m_CalcB(calcB)
	{
		m_CalcA->AddRef();
		m_CalcB->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"or\"");
		Tier0_Msg(", calcA: "); m_CalcA->Console_PrintBegin(); m_CalcA->Console_PrintEnd();
		Tier0_Msg(", calcB: "); m_CalcB->Console_PrintBegin(); m_CalcB->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		bool calcValueA, calcValueB;

		if (m_CalcA->CalcBool(calcValueA))
		{
			outResult = calcValueA;
			if(calcValueA) return true;
		}

		if (m_CalcB->CalcBool(calcValueB))
		{
			outResult = calcValueB;
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvBoolOrCalc()
	{
		m_CalcB->Release();
		m_CalcA->Release();
	}

private:
	IMirvBoolCalc * m_CalcA;
	IMirvBoolCalc * m_CalcB;
};

class CMirvBoolNotCalc : public CMirvBoolCalc
{
public:
	CMirvBoolNotCalc(char const * name, IMirvBoolCalc * calc)
		: CMirvBoolCalc(name)
		, m_Calc(calc)
	{
		m_Calc->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"not\"");
		Tier0_Msg(", calc: "); m_Calc->Console_PrintBegin(); m_Calc->Console_PrintEnd();

	}

	virtual bool CalcBool(bool & outResult)
	{
		bool calcValue;

		if (m_Calc->CalcBool(calcValue))
		{
			outResult = !calcValue;
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvBoolNotCalc()
	{
		m_Calc->Release();
	}

private:
	IMirvBoolCalc * m_Calc;
};

class CMirvBoolHandleCalc : public CMirvBoolCalc
{
public:
	CMirvBoolHandleCalc(char const * name, IMirvHandleCalc * handle)
		: CMirvBoolCalc(name)
		, m_Handle(handle)
	{
		m_Handle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"handle\"");
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		SOURCESDK::CSGO::CBaseHandle dummy;

		outResult = m_Handle->CalcHandle(dummy);

		return true;
	}

protected:
	virtual ~CMirvBoolHandleCalc()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
};

class CMirvBoolVecAngCalc : public CMirvBoolCalc
{
public:
	CMirvBoolVecAngCalc(char const * name, IMirvVecAngCalc * vecAng)
		: CMirvBoolCalc(name)
		, m_VecAng(vecAng)
	{
		m_VecAng->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"vecAng\"");
		Tier0_Msg(", vecAng: "); m_VecAng->Console_PrintBegin(); m_VecAng->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		SOURCESDK::Vector dummyVec;
		SOURCESDK::QAngle dummyAng;

		outResult = m_VecAng->CalcVecAng(dummyVec, dummyAng);

		return true;
	}

protected:
	virtual ~CMirvBoolVecAngCalc()
	{
		m_VecAng->Release();
	}

private:
	IMirvVecAngCalc * m_VecAng;
};

class CMirvBoolAliveCalc : public CMirvBoolCalc
{
public:
	CMirvBoolAliveCalc(char const * name, IMirvHandleCalc * handle)
		: CMirvBoolCalc(name)
		, m_Handle(handle)
	{
		m_Handle->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"alive\"");
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		SOURCESDK::CSGO::CBaseHandle parentHandle;

		if (m_Handle->CalcHandle(parentHandle) && parentHandle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(parentHandle);
			SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : 0;

			if (baseEntity)
			{
				outResult = baseEntity->IsAlive();
				return true;
			}
		}

		return false;
	}

protected:
	virtual ~CMirvBoolAliveCalc()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
};

class CMirvBoolHandlesEqualCalc : public CMirvBoolCalc
{
public:
	CMirvBoolHandlesEqualCalc(char const * name, IMirvHandleCalc * calcA, IMirvHandleCalc * calcB)
		: CMirvBoolCalc(name)
		, m_CalcA(calcA)
		, m_CalcB(calcB)
	{
		m_CalcA->AddRef();
		m_CalcB->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"handlesEqual\"");
		Tier0_Msg(", calcA: "); m_CalcA->Console_PrintBegin(); m_CalcA->Console_PrintEnd();
		Tier0_Msg(", calcB: "); m_CalcB->Console_PrintBegin(); m_CalcB->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		SOURCESDK::CSGO::CBaseHandle calcValueA, calcValueB;

		if (m_CalcA->CalcHandle(calcValueA) && m_CalcB->CalcHandle(calcValueB))
		{
			outResult = calcValueA == calcValueB;
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvBoolHandlesEqualCalc()
	{
		m_CalcB->Release();
		m_CalcA->Release();
	}

private:
	IMirvHandleCalc * m_CalcA;
	IMirvHandleCalc * m_CalcB;
};

class CMirvBoolIntsEqualCalc : public CMirvBoolCalc
{
public:
	CMirvBoolIntsEqualCalc(char const * name, IMirvIntCalc * calcA, IMirvIntCalc * calcB)
		: CMirvBoolCalc(name)
		, m_CalcA(calcA)
		, m_CalcB(calcB)
	{
		m_CalcA->AddRef();
		m_CalcB->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"intsEqual\"");
		Tier0_Msg(", calcA: "); m_CalcA->Console_PrintBegin(); m_CalcA->Console_PrintEnd();
		Tier0_Msg(", calcB: "); m_CalcB->Console_PrintBegin(); m_CalcB->Console_PrintEnd();
	}

	virtual bool CalcBool(bool & outResult)
	{
		int calcValueA, calcValueB;

		if (m_CalcA->CalcInt(calcValueA) && m_CalcB->CalcInt(calcValueB))
		{
			outResult = calcValueA == calcValueB;
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvBoolIntsEqualCalc()
	{
		m_CalcB->Release();
		m_CalcA->Release();
	}

private:
	IMirvIntCalc * m_CalcA;
	IMirvIntCalc * m_CalcB;
};


class CMirvBoolClassNameMatchesCalc : public CMirvBoolCalc
{
public:
	CMirvBoolClassNameMatchesCalc(char const * name, IMirvHandleCalc * calc, const char * wildCardString)
		: CMirvBoolCalc(name)
		, m_Calc(calc)
		, m_WildCardString(wildCardString)
	{
		m_Calc->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvBoolCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"classnameMatches\"");
		Tier0_Msg(", calc: "); m_Calc->Console_PrintBegin(); m_Calc->Console_PrintEnd();
		Tier0_Msg(", wildCardString: \"%s\"", m_WildCardString.c_str());
	}

	virtual bool CalcBool(bool & outResult)
	{
		SOURCESDK::CSGO::CBaseHandle handle;

		if (m_Calc->CalcHandle(handle) && handle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle);
			SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : 0;

			if (baseEntity)
			{
				outResult = StringWildCard1Matched(m_WildCardString.c_str(), baseEntity->GetClassname());
				return true;
			}
		}

		return false;
	}

protected:
	virtual ~CMirvBoolClassNameMatchesCalc()
	{
		m_Calc->Release();
	}

private:
	IMirvHandleCalc * m_Calc;
	std::string m_WildCardString;
};


class CMirvIntCalc : public CMirvCalc, public IMirvIntCalc
{
public:
	CMirvIntCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();

		Tier0_Msg(", type: \"int\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcInt(void)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};


class CMirvIntValueCalc : public CMirvIntCalc
{
public:
	CMirvIntValueCalc(char const * name, int value)
		: CMirvIntCalc(name)
		, m_Value(value)
	{
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvIntCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"value\", value: %i", m_Value);
	}

	virtual bool CalcInt(int & outResult)
	{
		outResult = m_Value;

		return true;
	}

protected:
	virtual ~CMirvIntValueCalc()
	{
	}

private:
	int m_Value;
};

class CMirvIntTeamNumberCalc : public CMirvIntCalc
{
public:
	CMirvIntTeamNumberCalc(char const * name, IMirvHandleCalc * handle)
		: CMirvIntCalc(name)
		, m_Handle(handle)
	{
		m_Handle->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvIntCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"handle\"");
		Tier0_Msg(", handle: "); m_Handle->Console_PrintBegin(); m_Handle->Console_PrintEnd();
	}

	virtual bool CalcInt(int & outResult)
	{
		SOURCESDK::CSGO::CBaseHandle parentHandle;

		if (m_Handle->CalcHandle(parentHandle) && parentHandle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(parentHandle);
			SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : 0;

			if (baseEntity)
			{
				outResult = baseEntity->GetTeamNumber();
				return true;
			}
		}

		return false;
	}

protected:
	virtual ~CMirvIntTeamNumberCalc()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
};


class CMirvFloatCalc : public CMirvCalc, public IMirvFloatCalc
{
public:
	CMirvFloatCalc(char const* name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const* GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvCalc::Console_PrintBegin();

		Tier0_Msg(", type: \"float\"");
	}

	virtual void Console_PrintEnd(void)
	{
		CMirvCalc::Console_PrintEnd();
	}

	virtual bool CalcFloat(void)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs* args)
	{
		CMirvCalc::Console_Edit(args);
	}
};


class CMirvFloatValueCalc : public CMirvFloatCalc
{
public:
	CMirvFloatValueCalc(char const* name, float value)
		: CMirvFloatCalc(name)
		, m_Value(value)
	{
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvFloatCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"value\", value: %f", m_Value);
	}

	virtual bool CalcFloat(float& outResult)
	{
		outResult = m_Value;

		return true;
	}

protected:
	virtual ~CMirvFloatValueCalc()
	{
	}

private:
	float m_Value;
};


class CMirvFloatIfCalc : public CMirvFloatCalc
{
public:
	CMirvFloatIfCalc(char const* name, IMirvBoolCalc* condition, IMirvFloatCalc* condTrue, IMirvFloatCalc* condFalse)
		: CMirvFloatCalc(name)
		, m_Condition(condition)
		, m_CondTrue(condTrue)
		, m_CondFalse(condFalse)
	{
		m_Condition->AddRef();
		m_CondTrue->AddRef();
		m_CondFalse->AddRef();
	}

	virtual void Console_PrintBegin(void)
	{
		CMirvFloatCalc::Console_PrintBegin();

		Tier0_Msg(", fn: \"if\"");
		Tier0_Msg(", condition: "); m_Condition->Console_PrintBegin(); m_Condition->Console_PrintEnd();
		Tier0_Msg(", true: "); m_CondTrue->Console_PrintBegin(); m_CondTrue->Console_PrintEnd();
		Tier0_Msg(", false: "); m_CondFalse->Console_PrintBegin(); m_CondFalse->Console_PrintEnd();
	}

	virtual bool CalcFloat(float& outResult)
	{
		bool condition;

		bool result = m_Condition->CalcBool(condition);

		condition = result && condition;

		return condition ? m_CondTrue->CalcFloat(outResult) : m_CondFalse->CalcFloat(outResult);
	}

protected:
	virtual ~CMirvFloatIfCalc()
	{
		m_CondFalse->Release();
		m_CondTrue->Release();
		m_Condition->Release();
	}

private:
	IMirvBoolCalc* m_Condition;
	IMirvFloatCalc* m_CondTrue;
	IMirvFloatCalc* m_CondFalse;
};

CMirvHandleCalcs::~CMirvHandleCalcs()
{
	for (std::list<IMirvHandleCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvHandleCalc * CMirvHandleCalcs::GetByName(char const * name)
{
	std::list<IMirvHandleCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvHandleCalc * CMirvHandleCalcs::NewValueCalc(char const * name, int handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleValueCalc(name, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc * CMirvHandleCalcs::NewIndexCalc(char const * name, int entityIndex)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleIndexCalc(name, entityIndex);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc * CMirvHandleCalcs::NewActiveWeaponCalc(char const * name, IMirvHandleCalc * parent, bool world)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleActiveWeaponCalc(name, parent, world);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc *  CMirvHandleCalcs::NewLocalPlayerCalc(char const * name)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleLocalPlayerCalc(name);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc *  CMirvHandleCalcs::NewObserverTargetCalc(char const * name, IMirvHandleCalc * parent)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleObserverTargetCalc(name, parent);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc *  CMirvHandleCalcs::NewOwnerEntityCalc(char const * name, IMirvHandleCalc * calc, unsigned int maxDepth)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleOwnerEntityCalc(name, calc, maxDepth);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}



IMirvHandleCalc * CMirvHandleCalcs::NewKeyCalc(char const * name, int key)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleKeyCalc(name, key);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvHandleCalcs::Console_Remove(char const * name)
{
	std::list<IMirvHandleCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvHandleCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvHandleCalcs::Console_Print(void)
{
	for (std::list<IMirvHandleCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvHandleCalcs::GetIteratorByName(char const * name, std::list<IMirvHandleCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvVecAngCalcs::~CMirvVecAngCalcs()
{
	for (std::list<IMirvVecAngCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvVecAngCalc * CMirvVecAngCalcs::GetByName(char const * name)
{
	std::list<IMirvVecAngCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewValueCalc(char const * name, float x, float y, float z, float rX, float rY, float rZ)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngValueCalc(name, x, y, z, rX, rY, rZ);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewAddCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngAddCalc(name, parent, offset);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewSubtractCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngSubtractCalc(name, parent, offset);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


IMirvVecAngCalc * CMirvVecAngCalcs::NewOffsetCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset, bool order)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngOffsetCalc(name, parent, offset, order);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleCalcEx(name, handle, false, false);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleEyeCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleCalcEx(name, handle, true, true);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleCalcEx(char const * name, IMirvHandleCalc * handle, bool eyeVec, bool eyeAng)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleCalcEx(name, handle, eyeVec, eyeAng);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleAttachmentCalc(char const * name, IMirvHandleCalc * handle, char const * attachmentName)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleAttachmentCalc(name, handle, attachmentName);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewIfCalc(char const * name, IMirvBoolCalc * condition, IMirvVecAngCalc * condTrue, IMirvVecAngCalc * condFalse)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngIfCalc(name, condition, condTrue, condFalse);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewOrCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngOrCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewCamCalc(char const * name, IMirvCamCalc * src)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngCamCalc(name, src);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewMotionProfile2Calc(char const * name, IMirvVecAngCalc * parent, IMirvHandleCalc * trackHandle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngMotionProfile2Calc(name, parent, trackHandle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewSwitchInterpCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * switchHandle, IMirvHandleCalc * resetHandle, float holdTime, float interpTime)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngSwitchInterpCalc(name, source, switchHandle, resetHandle, holdTime, interpTime);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewLocalToGlobalCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngLocalToGlobalCalc(name, source, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewGlobalToLocalCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngGlobalToLocalCalc(name, source, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewCollisionClipCalc(char const * name, IMirvVecAngCalc * source, IMirvVecAngCalc * clipTo, IMirvHandleCalc * ignoreHandle, float safeZoneDist)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngCollisionClipCalc(name, source, clipTo, ignoreHandle, safeZoneDist);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


void CMirvVecAngCalcs::Console_Remove(char const * name)
{
	std::list<IMirvVecAngCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvVecAngCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvVecAngCalcs::Console_Print(void)
{
	for (std::list<IMirvVecAngCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvVecAngCalcs::GetIteratorByName(char const * name, std::list<IMirvVecAngCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}



CMirvCamCalcs::~CMirvCamCalcs()
{
	for (std::list<IMirvCamCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvCamCalc * CMirvCamCalcs::GetByName(char const * name)
{
	std::list<IMirvCamCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvCamCalc * CMirvCamCalcs::NewCamCalc(char const * name, const char * camFileName, const char * startClientTime)
{
	if (name && !Console_CheckName(name))
		return 0;

	CMirvCamCamCalc * result = new CMirvCamCamCalc(name, camFileName, startClientTime);

	if(result->IsBad()) Tier0_Warning("Error importing CAM file \"%s\"\n", name);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvCamCalc * CMirvCamCalcs::NewGameCalc(char const * name)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvCamCalc * result = new CMirvCamGameCalc(name);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


IMirvCamCalc * CMirvCamCalcs::NewCurrentCalc(char const * name)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvCamCalc * result = new CMirvCamCurrentCalc(name);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvCamCalc* CMirvCamCalcs::NewPlayerCalc(char const* name, IMirvHandleCalc* handleCalc)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvCamCalc* result = new CMirvCamPlayerCalc(name, handleCalc);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvCamCalc * CMirvCamCalcs::NewSmoothCalc(char const * name, IMirvCamCalc * parent, IMirvHandleCalc * trackHandle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvCamCalc * result = new CMirvCamSmoothCalc(name, parent, trackHandle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvCamCalc * CMirvCamCalcs::NewVecAngFovCalc(char const * name, IMirvVecAngCalc * vecAng, IMirvFovCalc * fov)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvCamCalc * result = new CMirvCamVecAngFovCalc(name, vecAng, fov);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


void CMirvCamCalcs::Console_Remove(char const * name)
{
	std::list<IMirvCamCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvCamCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvCamCalcs::Console_Print(void)
{
	for (std::list<IMirvCamCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvCamCalcs::GetIteratorByName(char const * name, std::list<IMirvCamCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvFovCalcs::~CMirvFovCalcs()
{
	for (std::list<IMirvFovCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvFovCalc * CMirvFovCalcs::GetByName(char const * name)
{
	std::list<IMirvFovCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvFovCalc * CMirvFovCalcs::NewCamCalc(char const * name, IMirvCamCalc * src)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvFovCalc * result = new CMirvFovCamCalc(name, src);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvFovCalcs::Console_Remove(char const * name)
{
	std::list<IMirvFovCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvFovCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvFovCalcs::Console_Print(void)
{
	for (std::list<IMirvFovCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvFovCalcs::GetIteratorByName(char const * name, std::list<IMirvFovCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvBoolCalcs::~CMirvBoolCalcs()
{
	for (std::list<IMirvBoolCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvBoolCalc * CMirvBoolCalcs::GetByName(char const * name)
{
	std::list<IMirvBoolCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvBoolCalc * CMirvBoolCalcs::NewHandleCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolHandleCalc(name, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewVecAngCalc(char const * name, IMirvVecAngCalc * vecAng)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolVecAngCalc(name, vecAng);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewAliveCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolAliveCalc(name, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewAndCalc(char const * name, IMirvBoolCalc * a, IMirvBoolCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolAndCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewOrCalc(char const * name, IMirvBoolCalc * a, IMirvBoolCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolOrCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewNotCalc(char const * name, IMirvBoolCalc * a)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolNotCalc(name, a);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewHandlesEqualCalc(char const * name, IMirvHandleCalc * a, IMirvHandleCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolHandlesEqualCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewIntsEqualCalc(char const * name, IMirvIntCalc * a, IMirvIntCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolIntsEqualCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewClassnameMatchesCalc(char const * name, IMirvHandleCalc * handle, const char * wildCardString)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolClassNameMatchesCalc(name, handle, wildCardString);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvBoolCalcs::Console_Remove(char const * name)
{
	std::list<IMirvBoolCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvBoolCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvBoolCalcs::Console_Print(void)
{
	for (std::list<IMirvBoolCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvBoolCalcs::GetIteratorByName(char const * name, std::list<IMirvBoolCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvIntCalcs::~CMirvIntCalcs()
{
	for (std::list<IMirvIntCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvIntCalc * CMirvIntCalcs::GetByName(char const * name)
{
	std::list<IMirvIntCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}


IMirvIntCalc * CMirvIntCalcs::NewValueCalc(char const * name, int value)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvIntCalc * result = new CMirvIntValueCalc(name, value);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvIntCalc * CMirvIntCalcs::NewTeamNumberCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvIntCalc * result = new CMirvIntTeamNumberCalc(name, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvIntCalcs::Console_Remove(char const * name)
{
	std::list<IMirvIntCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvIntCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvIntCalcs::Console_Print(void)
{
	for (std::list<IMirvIntCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvIntCalcs::GetIteratorByName(char const * name, std::list<IMirvIntCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}



CMirvFloatCalcs::~CMirvFloatCalcs()
{
	for (std::list<IMirvFloatCalc*>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvFloatCalc* CMirvFloatCalcs::GetByName(char const* name)
{
	std::list<IMirvFloatCalc*>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}


IMirvFloatCalc* CMirvFloatCalcs::NewValueCalc(char const* name, float value)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvFloatCalc* result = new CMirvFloatValueCalc(name, value);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


IMirvFloatCalc* CMirvFloatCalcs::NewIfCalc(char const* name, IMirvBoolCalc* condition, IMirvFloatCalc* condTrue, IMirvFloatCalc* condFalse)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvFloatCalc* result = new CMirvFloatIfCalc(name, condition, condTrue, condFalse);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


void CMirvFloatCalcs::Console_Remove(char const* name)
{
	std::list<IMirvFloatCalc*>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvFloatCalcs::Console_CheckName(char const* name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvFloatCalcs::Console_Print(void)
{
	for (std::list<IMirvFloatCalc*>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_PrintBegin(); (*it)->Console_PrintEnd(); Tier0_Msg(";\n");
	}
}

void CMirvFloatCalcs::GetIteratorByName(char const* name, std::list<IMirvFloatCalc*>::iterator& outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


void mirv_calcs_handle(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("value", arg2) && 5 <= argc)
				{
					g_MirvHandleCalcs.NewValueCalc(args->ArgV(3), atoi(args->ArgV(4)));
					return;
				}
				else if (0 == _stricmp("index", arg2) && 5 <= argc)
				{
					g_MirvHandleCalcs.NewIndexCalc(args->ArgV(3), atoi(args->ArgV(4)));
					return;
				}
				else if (0 == _stricmp("key", arg2) && 5 <= argc)
				{
					g_MirvHandleCalcs.NewKeyCalc(args->ArgV(3), atoi(args->ArgV(4)));
					return;
				}
				else if (0 == _stricmp("activeWeapon", arg2) && 6 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvHandleCalcs.NewActiveWeaponCalc(args->ArgV(3), parentCalc, 0 != atoi(args->ArgV(5)));
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("localPlayer", arg2) && 4 <= argc)
				{
					g_MirvHandleCalcs.NewLocalPlayerCalc(args->ArgV(3));

					return;
				}
				else if (0 == _stricmp("observerTarget", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvHandleCalcs.NewObserverTargetCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("observerTarget", arg2) && 6 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);
					unsigned int maxDepth = strtoul(args->ArgV(5), nullptr, 10);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvHandleCalcs.NewOwnerEntityCalc(args->ArgV(3), parentCalc, maxDepth);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add value <sName> <iHandle> - Add a new calc with a constant value.\n"
				"%s add index <sName> <iIndex> - Add a new index calc.\n"
				"%s add key <sName> <iKeyNumber> - Add a new key calc (like spectator HUD).\n"
				"%s add activeWeapon <sName> <sParentCalcHandleName> <bGetWorld> - Add an active weapon calc, <bGetWorld> is 0 or 1.\n"
				"%s add localPlayer <sName> - Add localPlayer calc.\n"
				"%s add observerTarget <sName> <sParentCalcHandleName> - Add observer target calc (use e.g. localPlayer calc as parent name).\n"
				"%s add ownerEntityCalc <sName> <sCalcHandleName> <iMaxDepth>\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvHandleCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvHandleCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				SOURCESDK::CSGO::CBaseHandle handle;
				bool calced = parentCalc->CalcHandle(handle);

				if(calced)
					Tier0_Msg("\nResult: true, handle=%i\n", handle);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new handle calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}

void mirv_calcs_vecang(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("value", arg2) && 10 <= argc)
				{
					g_MirvVecAngCalcs.NewValueCalc(args->ArgV(3), (float)atof(args->ArgV(4)), (float)atof(args->ArgV(5)), (float)atof(args->ArgV(6)), (float)atof(args->ArgV(7)), (float)atof(args->ArgV(8)), (float)atof(args->ArgV(9)));
					return;
				}
				else if (0 == _stricmp("add", arg2) && 6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvVecAngCalc * calcB = g_MirvVecAngCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewAddCalc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No vecAng A with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng B with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("subtract", arg2) && 6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvVecAngCalc * calcB = g_MirvVecAngCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewSubtractCalc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No vecAng A with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng B with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("offset", arg2) && 7 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvVecAngCalc * calcB = g_MirvVecAngCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewOffsetCalc(args->ArgV(3), calcA, calcB, 0 != atoi(args->ArgV(6)));
						}
						else
							Tier0_Warning("Error: No vecAng parent with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng offset with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("handle", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewHandleCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("handleEye", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewHandleEyeCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("handleAttachment", arg2) && 6 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewHandleAttachmentCalc(args->ArgV(3), parentCalc, args->ArgV(5));
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("or", arg2) && 6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvVecAngCalc * calcB = g_MirvVecAngCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewOrCalc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No vecAng calcB with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng calcA with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("cam", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewCamCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No cam calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("motionProfile2", arg2) && 6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvHandleCalc * calcB = g_MirvHandleCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewMotionProfile2Calc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng parent with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("switchInterp", arg2) && 9 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					
					if (IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName))
					{
						char const * calcBName = args->ArgV(5);
						
						if (IMirvHandleCalc * calcB = g_MirvHandleCalcs.GetByName(calcBName))
						{
							char const * calcCName = args->ArgV(6);

							if (IMirvHandleCalc * calcC = g_MirvHandleCalcs.GetByName(calcCName))
							{
								g_MirvVecAngCalcs.NewSwitchInterpCalc(args->ArgV(3), calcA, calcB, calcC, (float)atof(args->ArgV(7)), (float)atof(args->ArgV(8)));
							}
							else
								Tier0_Warning("Error: No handle resetHandle calc with name \"%s\" found.\n", calcCName);
						}
						else
							Tier0_Warning("Error: No handle switchHandle calc with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng source calc with name \"%s\" found.\n", calcAName);
						

					return;
				}
				else if (0 == _stricmp("localToGlobal", arg2) &&6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvHandleCalc * calcB = g_MirvHandleCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewLocalToGlobalCalc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No handle handle calc with name \"%s\" found.\n", calcBName);							
					}
					else
						Tier0_Warning("Error: No vecAng source calc with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("globalToLocal", arg2) && 6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvHandleCalc * calcB = g_MirvHandleCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewGlobalToLocalCalc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No handle handle calc with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No vecAng source calc with name \"%s\" found.\n", calcAName);
					
					return;
				}
				else if (0 == _stricmp("collisionClip", arg2) && 8 <= argc)
				{
					char const * calcSourceName = args->ArgV(4);
					IMirvVecAngCalc * calcSource = g_MirvVecAngCalcs.GetByName(calcSourceName);

					if (calcSource)
					{
						char const * calcClipToName = args->ArgV(5);
						IMirvVecAngCalc * calcClipTo = g_MirvVecAngCalcs.GetByName(calcClipToName);

						if (calcClipTo)
						{
							char const * calcIngoreName = args->ArgV(6); 
							IMirvHandleCalc * calcIgnore = g_MirvHandleCalcs.GetByName(calcIngoreName);

							if(calcIgnore) {
								g_MirvVecAngCalcs.NewCollisionClipCalc(args->ArgV(3), calcSource, calcClipTo, calcIgnore, (float)atof(args->ArgV(7)));
							}
							else Tier0_Warning("Error: No handle ugnore with name \"%s\" found.\n", calcIngoreName);
						}
						else
							Tier0_Warning("Error: No vecAng clipTo with name \"%s\" found.\n", calcClipToName);
					}
					else
						Tier0_Warning("Error: No vecAng source with name \"%s\" found.\n", calcSourceName);

					return;
				}
			}

			Tier0_Msg(
				"%s add value <sName> <fX> <fY> <fZ> <rX> <rY> <rZ> - Add a new calc with a constant value.\n"
				"%s add offset <sName> <sParentName> <sOffSetName> <bOrder> - Add a new offset calc, <bOrder>: 0 rotate first then offset, 1: offset first then rotate.\n"
				"%s add add <sName> <sAName> <sBName> - A + B.\n"
				"%s add subtract <sName> <sAName> <sBName> - A - B.\n"
				"%s add handle <sName> <sHandleCalcName> - Add a calc that gets its values from an entity using a handle calc named <sHandleCalcName>.\n"
				"%s add handleEye <sName> <sHandleCalcName> - Add a calc that gets its values from an entity's eye point using a handle calc named <sHandleCalcName>.\n"
				"%s add handleAttachment <sName> <sHandleCalcName> <sAttachMentName> - Add a calc that gets its values from an entity's attachment.\n"
				"%s add or <sName> <sAName> <sBName> - Add an OR calc.\n"
				"%s add cam <sName> <sCamCalName> - Adds a calc that gets its values from a cam calc named <sCamCalName>.\n"
				"%s add motionProfile2 <sName> <sParentName> <sTrackHandleName> - Add a velocity motion profile calc, <sParentName> is the motion target, <sTrackHandleName> is used to detect target changes (reset).\n"
				"%s add switchInterp <sName> <sSourceVecAngName> <sSwitchHandleName> <sResetHandleName> <fHoldTime> <fInterpTime> - Add a calc that after <sSwitchHandleName> value changes, holds the last value <sSourceVecAngName> for <fHoldTime> seconds and then interpolates between last value and current value for <fInterpTime>, if <sResetHandleName> changes the change is instant (reset like initial).\n"
				"%s add localToGlobal <sName> <sSourceVecAngName> <sHandleName> - Add a calc that transforms from local <sSwitchHandleName> space <sSourceVecAngName> to global space.\n"
				"%s add globalToLocal <sName> <sSourceVecAngName> <sHandleName> - Add a calc that transforms to local <sSwitchHandleName> space <sSourceVecAngName> from global space.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			Tier0_Msg(
				"%s add collisionClip <sName> <sSourceVecAngName> <sClipToVecAngName> <sIgnoreHandleName> <fSafeZoneDist> - Checks for collisions between source and clipTo and if there are any moves source nearer to clip accordingly.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvVecAngCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvVecAngCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvVecAngCalc * parentCalc = g_MirvVecAngCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				SOURCESDK::Vector vec;
				SOURCESDK::QAngle ang;
				bool calced = parentCalc->CalcVecAng(vec, ang);

				if(calced)
					Tier0_Msg("\nResult: true, vec=(%f, %f, %f), ang=(%f, %f, %f)\n", vec.x, vec.y, vec.z, ang.z, ang.x, ang.y);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvVecAngCalc * parentCalc = g_MirvVecAngCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}


void mirv_calcs_cam(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("cam", arg2) && 6 <= argc)
				{
					char const * camFileName = args->ArgV(4);
					char const * szStartClientTime = args->ArgV(5);

					g_MirvCamCalcs.NewCamCalc(args->ArgV(3), camFileName, szStartClientTime);

					return;
				}
				else if (0 == _stricmp("game", arg2))
				{
					g_MirvCamCalcs.NewGameCalc(args->ArgV(3));

					return;
				}
				else if (0 == _stricmp("current", arg2))
				{
					g_MirvCamCalcs.NewCurrentCalc(args->ArgV(3));

					return;
				}
				else if (0 == _stricmp("player", arg2) && 5 <= argc)
				{
					char const * handleCalcName = args->ArgV(4);

					if (IMirvHandleCalc * handleCalc = g_MirvHandleCalcs.GetByName(handleCalcName))
					{
						g_MirvCamCalcs.NewPlayerCalc(args->ArgV(3), handleCalc);
					}
					else Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", handleCalcName);
					
					return;
				}
				else if (0 == _stricmp("smooth", arg2) && 6 <= argc)
				{
					char const* parentCalcName = args->ArgV(4);
					char const* trackHandleCalcName = args->ArgV(5);

					if (IMirvCamCalc* parentCalc = g_MirvCamCalcs.GetByName(parentCalcName))
					{
						if (IMirvHandleCalc* trackHandleCalc = g_MirvHandleCalcs.GetByName(trackHandleCalcName))
						{
							g_MirvCamCalcs.NewSmoothCalc(args->ArgV(3), parentCalc, trackHandleCalc);
						}
						else Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", trackHandleCalcName);
					}
					else Tier0_Warning("Error: No cam calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("vecAngFov", arg2) && 6 <= argc)
				{
					char const * vecAngCalcName = args->ArgV(4);
					char const * fovCalcName = args->ArgV(5);

					if (IMirvVecAngCalc * vecAngCalc = g_MirvVecAngCalcs.GetByName(vecAngCalcName))
					{
						if (IMirvFovCalc * fovCalc = g_MirvFovCalcs.GetByName(fovCalcName))
						{
							g_MirvCamCalcs.NewVecAngFovCalc(args->ArgV(3), vecAngCalc, fovCalc);
						}
						else Tier0_Warning("Error: No fov calc with name \"%s\" found.\n", fovCalcName);
					}
					else Tier0_Warning("Error: No vecAng calc with name \"%s\" found.\n", vecAngCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add cam <sName> <sfilePath> <fStartTime>|current - Adds a mirv_camio file as calc.\n"
				"%s add game <sName> - Current game camera.\n"
				"%s add current <sName> - Current camera (depends on mirv_cam order and overrides).\n"
				"%s add player <sName> <sPlayerHandleCalcName> - Camera of player.\n"
				"%s add smooth <sName> <sParentCamCalcName> <sTrackHandleCalcName>- Smooth calc.\n"
				"%s add vecAngFov <sName> <sVecAngCalcName> <sFovCalcName>- Combine a vecAng and a fov calc into a cam calc.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvCamCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvCamCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				SOURCESDK::Vector vec;
				SOURCESDK::QAngle ang;
				float fov;
				bool calced = parentCalc->CalcCam(vec, ang, fov);

				if (calced)
					Tier0_Msg("\nResult: true, vec=(%f, %f, %f), ang=(%f, %f, %f), fov=%f\n", vec.x, vec.y, vec.z, ang.z, ang.x, ang.y, fov);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}



void mirv_calcs_fov(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("cam", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvFovCalcs.NewCamCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No cam calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add cam <sName> <sCamCalName> - Adds a calc that gets its values from a cam calc named <sCamCalName>.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvFovCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvFovCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvFovCalc * parentCalc = g_MirvFovCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				float fov;
				bool calced = parentCalc->CalcFov(fov);

				if (calced)
					Tier0_Msg("\nResult: true, fov=%f\n", fov);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvFovCalc * parentCalc = g_MirvFovCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}


void mirv_calcs_bool(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("and", arg2) && 6 <= argc)
				{
					char const * parentCalcNameA = args->ArgV(4);
					char const * parentCalcNameB = args->ArgV(5);

					IMirvBoolCalc * parentCalcA = g_MirvBoolCalcs.GetByName(parentCalcNameA);
					IMirvBoolCalc * parentCalcB = g_MirvBoolCalcs.GetByName(parentCalcNameB);

					if (parentCalcA && parentCalcB)
						g_MirvBoolCalcs.NewAndCalc(args->ArgV(3), parentCalcA, parentCalcB);
					else
					{
						if(!parentCalcA) Tier0_Warning("Error: No bool calc with name \"%s\" found.\n", parentCalcA);
						if (!parentCalcB) Tier0_Warning("Error: No bool calc with name \"%s\" found.\n", parentCalcB);
					}

					return;
				}
				else if (0 == _stricmp("or", arg2) && 6 <= argc)
				{
					char const * parentCalcNameA = args->ArgV(4);
					char const * parentCalcNameB = args->ArgV(5);

					IMirvBoolCalc * parentCalcA = g_MirvBoolCalcs.GetByName(parentCalcNameA);
					IMirvBoolCalc * parentCalcB = g_MirvBoolCalcs.GetByName(parentCalcNameB);

					if (parentCalcA && parentCalcB)
						g_MirvBoolCalcs.NewOrCalc(args->ArgV(3), parentCalcA, parentCalcB);
					else
					{
						if(!parentCalcA) Tier0_Warning("Error: No bool calc with name \"%s\" found.\n", parentCalcA);
						if (!parentCalcB) Tier0_Warning("Error: No bool calc with name \"%s\" found.\n", parentCalcB);
					}

					return;
				}
				else if (0 == _stricmp("not", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvBoolCalc * parentCalc = g_MirvBoolCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvBoolCalcs.NewNotCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No bool calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("handle", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvBoolCalcs.NewHandleCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("vecAng", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvVecAngCalc * parentCalc = g_MirvVecAngCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvBoolCalcs.NewVecAngCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No vecAng calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("alive", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvBoolCalcs.NewAliveCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("handlesEqual", arg2) && 6 <= argc)
				{
					char const * parentCalcNameA = args->ArgV(4);
					char const * parentCalcNameB = args->ArgV(5);

					IMirvHandleCalc * parentCalcA = g_MirvHandleCalcs.GetByName(parentCalcNameA);
					IMirvHandleCalc * parentCalcB = g_MirvHandleCalcs.GetByName(parentCalcNameB);

					if (parentCalcA && parentCalcB)
						g_MirvBoolCalcs.NewHandlesEqualCalc(args->ArgV(3), parentCalcA, parentCalcB);
					else
					{
						if (!parentCalcA) Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcA);
						if (!parentCalcB) Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcB);
					}

					return;
				}
				else if (0 == _stricmp("intsEqual", arg2) && 6 <= argc)
				{
					char const * parentCalcNameA = args->ArgV(4);
					char const * parentCalcNameB = args->ArgV(5);

					IMirvIntCalc * parentCalcA = g_MirvIntCalcs.GetByName(parentCalcNameA);
					IMirvIntCalc * parentCalcB = g_MirvIntCalcs.GetByName(parentCalcNameB);

					if (parentCalcA && parentCalcB)
						g_MirvBoolCalcs.NewIntsEqualCalc(args->ArgV(3), parentCalcA, parentCalcB);
					else
					{
						if (!parentCalcA) Tier0_Warning("Error: No int calc with name \"%s\" found.\n", parentCalcA);
						if (!parentCalcB) Tier0_Warning("Error: No int calc with name \"%s\" found.\n", parentCalcB);
					}

					return;
				}
				else if (0 == _stricmp("classnameMatches", arg2) && 6 <= argc)
				{
				char const * parentCalcNameA = args->ArgV(4);
				char const * wildCardString = args->ArgV(5);

				IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcNameA);

				if (parentCalc)
					g_MirvBoolCalcs.NewClassnameMatchesCalc(args->ArgV(3), parentCalc, wildCardString);
				else
					Tier0_Warning("Error: No int calc with name \"%s\" found.\n", parentCalc);

				return;
				}
			}

			Tier0_Msg(
				"%s add and <sName> <boolCalcNameA> <boolCalcNameB> - Returns AND of the bool calcs if valid.\n"
				"%s add or <sName> <boolCalcNameA> <boolCalcNameB> - Returns OR of the bool calcs if valid.\n"
				"%s add not <sName> <boolCalcName> - Returns NOT of bool calc if valid.\n"
				"%s add handle <sName> <handleCalcName> - Returns true if handleCalc is valid.\n"
				"%s add vecAng <sName> <vecAngCalcNae> - Returns true if vecAngCalc is valid.\n"
				"%s add alive <sName> <handleCalcName> - Returns true if alive.\n"
				"%s add handlesEqual <sName> <handleCalcNameA> <handleCalcNameB>\n"
				"%s add intsEqual <sName> <intCalcNameA> <intCalcNameB>\n"
				"%s add classnameMatches <sName> <handleCalcname> <sWildCardString>\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvBoolCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvBoolCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvBoolCalc * parentCalc = g_MirvBoolCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				bool value;
				bool calced = parentCalc->CalcBool(value);

				if (calced)
					Tier0_Msg("\nResult: true, bool=%i\n", value ? 1 : 0);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new bool calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}



void mirv_calcs_int(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("value", arg2) && 5 <= argc)
				{
					int value = atoi(args->ArgV(4));

					g_MirvIntCalcs.NewValueCalc(args->ArgV(3), value);
					return;
				}
				else if (0 == _stricmp("teamNumber", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvIntCalcs.NewTeamNumberCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add value <sName> <iValue>.\n"
				"%s add teamNumber <sName> <handleCalcName> - Returns team number (1 = spec, 2 = T, 3 = CT).\n"
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvIntCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvIntCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvIntCalc * parentCalc = g_MirvIntCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				int value;
				bool calced = parentCalc->CalcInt(value);

				if (calced)
					Tier0_Msg("\nResult: true, int=%i\n", value);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new bool calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}


void mirv_calcs_float(IWrpCommandArgs* args)
{
	int argc = args->ArgC();

	char const* arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const* arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const* arg2 = args->ArgV(2);

				if (0 == _stricmp("value", arg2) && 5 <= argc)
				{
					float value = (float)(atof(args->ArgV(4)));

					g_MirvFloatCalcs.NewValueCalc(args->ArgV(3), value);
					return;
				}
				else if (0 == _stricmp("if", arg2) && 7 <= argc)
				{
					const char* boolCalcNameCondition = args->ArgV(4);
					const char* floatCalcNameTrue = args->ArgV(5);
					const char* floatCalcNameFalse = args->ArgV(6);

					if (IMirvBoolCalc* boolCalcCondition = g_MirvBoolCalcs.GetByName(boolCalcNameCondition))
					{
						if (IMirvFloatCalc* floatCalcTrue = g_MirvFloatCalcs.GetByName(floatCalcNameTrue))
						{
							if (IMirvFloatCalc* floatCalcFalse = g_MirvFloatCalcs.GetByName(floatCalcNameFalse))
							{
								g_MirvFloatCalcs.NewIfCalc(args->ArgV(3), boolCalcCondition, floatCalcTrue, floatCalcFalse);
							}
							else Tier0_Warning("Error: No float calc with name \"%s\" found.\n", floatCalcNameFalse);
						}
						else Tier0_Warning("Error: No float calc with name \"%s\" found.\n", floatCalcNameTrue);
					}
					else Tier0_Warning("Error: No bool calc with name \"%s\" found.\n", boolCalcNameCondition);

					return;
				}
			}

			Tier0_Msg(
				"%s add value <sName> <iValue>.\n"
				"%s add if <sName> <boolCalcNameCondition> <floatCalcNameTrue> <floatCalcNameFalse>.\n"
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvIntCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvIntCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const* parentCalcName = args->ArgV(2);

			IMirvIntCalc* parentCalc = g_MirvIntCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				int value;
				bool calced = parentCalc->CalcInt(value);

				if (calced)
					Tier0_Msg("\nResult: true, int=%i\n", value);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const* parentCalcName = args->ArgV(2);

			IMirvHandleCalc* parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new bool calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}



CON_COMMAND(mirv_calcs, "Expressions, currently mainly for usage mirv_calcs, mirv_cam, mirv_aim")
{
	int argc = args->ArgC();
	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("handle", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_handle(&sub);
			return;
		}
		else if (0 == _stricmp("vecAng", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_vecang(&sub);
			return;
		}
		else if (0 == _stricmp("fov", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_fov(&sub);
			return;
		}
		else if (0 == _stricmp("cam", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_cam(&sub);
			return;
		}
		else if (0 == _stricmp("bool", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_bool(&sub);
			return;
		}
		else if (0 == _stricmp("int", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_int(&sub);
			return;
		}
		else if (0 == _stricmp("float", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_float(&sub);
			return;
		}
	}

	Tier0_Msg(
		"%s handle [...] - Calcs that return an entity handle.\n"
		"%s vecAng [...] - Calcs that return VecAng (location and rotation).\n"
		"%s fov [...] - Calcs that return FOV (field of view).\n"
		"%s cam [...] - Calcs that return a view (location, rotation and FOV).\n"
		"%s bool [...] - Calc that returns true or false (if it could be evaluated that is).\n"
		"%s int [...] - Calc that returns an integer or nothing.\n"
		"%s float [...] - Calc that returns an integer or nothing.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}

void MirvCalcs_LevelInitPreEntity()
{
	++g_LevelInitCount;
}
