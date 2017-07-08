#include "stdafx.h"

#include "aiming.h"

#include "SourceInterfaces.h"

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

Aiming g_Aiming;

void LookAnglesFromTo(Vector3 const &from, Vector3 const &to, double fallBackPitch, double fallBackYaw, double & outPitch, double & outYaw)
{
	Vector3 dir = to - from;

	// Store then zero height
	double dz = dir.Z;
	
	dir.Z = 0;

	// Need this for later
	double length = dir.Length();

	if(length <= AFX_MATH_EPS)
	{
		outPitch = fallBackPitch;
		outYaw = fallBackYaw;
		return;
	}

	dir = dir.Normalize();

	// This is our forward angle
	Vector3 vForward(1.0, 0.0, 0.0);

	double dot_product = (dir.X * vForward.X) + (dir.Y * vForward.Y) + (dir.Z * vForward.Z);

	double angle = acos(dot_product) * 180.0f / (float)M_PI;

	if (dir.Y < 0)
		angle = 360.0 - angle;

	double pitch = atan(dz / length) * 180.0f / (float)M_PI;

	outYaw = angle;
	outPitch = -pitch;
}

Aiming::Aiming()
: Active(false)
, SoftDeactivate(false)
, OffSet(0.0,0.0,0.0)
, SnapTo(true)
, LimitVelocity(360)
, LimitAcceleration(90)
//, LimitJerk(0.01)
, Origin(O_View)
, Angles(A_Net)
, Up(U_World)
, m_YPitchVelocity(0)
, m_ZYawVelocity(0)
, m_XRollVelocity(0)
, LastTargetOrigin(0, 0, 0)
, LastYPitch(0)
, LastZYaw(0)
, LastXRoll(0)
, m_Deactivating(false)
{

}

void Aiming::RebuildCalc(void)
{
	if (-1 != EntityIndex)
	{
		IMirvHandleCalc * handleCalc = g_MirvHandleCalcs.NewIndexCalc(0, EntityIndex);
		if (handleCalc)
		{
			handleCalc->AddRef();

			Source_set(
				g_MirvVecAngCalcs.NewHandleCalcEx(0, handleCalc, O_View == Origin, A_View == Angles)
			);

			handleCalc->Release();
		}
	}
	else
		Source_set(0);
}

bool Aiming::Aim(double deltaT, Vector3 const camOrigin, double & yPitch, double & zYaw, double & xRoll)
{
	if(deltaT <= 0)
	{
		return false;
	}

	bool camRotationChanged = false;

	m_Deactivating = Active || m_Deactivating && SoftDeactivate;

	double targetYPitch = yPitch;
	double targetZYaw = zYaw;
	double targetXRoll = xRoll;

	if(Active)
	{
		if (m_Source)
		{

			SOURCESDK::Vector o;
			SOURCESDK::QAngle a;

			if (m_Source->CalcVecAng(o, a))
			{
				double forward[3], right[3], up[3];

				MakeVectors(a.z, a.x, a.y, forward, right, up);

				LastTargetOrigin = Vector3(
					o.x + OffSet.X * forward[0] + OffSet.Y * right[0] + OffSet.Z * up[0],
					o.y + OffSet.X * forward[1] + OffSet.Y * right[1] + OffSet.Z * up[1],
					o.z + OffSet.X * forward[2] + OffSet.Y * right[2] + OffSet.Z * up[2]
				);
			}
		}
	}
	else
	{
		LastTargetOrigin = camOrigin;
	}

	if(Active || m_Deactivating)
	{
		if(Active)
		{
			LookAnglesFromTo(camOrigin, LastTargetOrigin, LastYPitch, LastZYaw, targetYPitch, targetZYaw);
			targetXRoll = U_World == Up ? 0 : targetXRoll;
		}

		if(SnapTo)
		{
			yPitch = targetYPitch;
			zYaw = targetZYaw;
			xRoll = targetXRoll;
			camRotationChanged = true;
			m_Deactivating = false;
		}
		else
		{
			double reaimYPitch = targetYPitch -LastYPitch;
			double reaimZYaw = targetZYaw -LastZYaw;
			double reaimXRoll = targetXRoll -LastXRoll;

			// Force reaim angles to be in [-180°, 180°)

			reaimYPitch = fmod(reaimYPitch + 180.0, 360.0) -180.0;
			reaimZYaw = fmod(reaimZYaw + 180.0, 360.0) -180.0;
			reaimXRoll = fmod(reaimXRoll + 180.0, 360.0) -180.0;

			m_Deactivating = Active || abs(reaimYPitch) > AFX_MATH_EPS || abs(reaimZYaw) > AFX_MATH_EPS || abs(reaimXRoll) > AFX_MATH_EPS;

			// apply re-aiming:

			//Tier0_Msg("+++pitch+++\n");
			CalcSmooth(deltaT, LastYPitch +reaimYPitch, LastYPitch, m_YPitchVelocity);
			yPitch = LastYPitch;

			//Tier0_Msg("+++yaw+++\n");
			CalcSmooth(deltaT, LastZYaw +reaimZYaw, LastZYaw, m_ZYawVelocity);
			zYaw = LastZYaw;

			//Tier0_Msg("+++roll+++\n");
			CalcSmooth(deltaT, LastXRoll +reaimXRoll, LastXRoll, m_XRollVelocity);
			xRoll = LastXRoll;

			camRotationChanged = true;
		}
	}
	else
	{
		m_YPitchVelocity = 0;
		m_ZYawVelocity = 0;
		m_XRollVelocity = 0;
	}

	// Force remembered angels to be in [-180°, 180°)

	LastYPitch = fmod(yPitch +180.0, 360.0) -180.0;
	LastZYaw = fmod(zYaw +180.0, 360.0) -180.0;
	LastXRoll = fmod(xRoll +180.0, 360.0) -180.0;

	return camRotationChanged;
}

void Aiming::TargetPoint(Vector3 origin)
{
	Source_set(0);
	LastTargetOrigin = origin;
}

void Aiming::TargetPointFromLast(Vector3 offset)
{
	TargetPoint(LastTargetOrigin +offset);
}

void Aiming::CalcSmooth(double deltaT, double targetPos, double & lastPos, double & lastVel)
{
	if(deltaT <= 0)
		return;

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

	while(0 < deltaT)
	{
		//Tier0_Msg("%f ", deltaT);

		if(lastVel > LimitVelocity)
		{
			// Error condition.
			
			// Solving Step:
			//
			// lower Velocity until we are within limits:

			// LimitVelocity = lastVel -LimitAcceleration * phaseT

			double phaseT = (LimitVelocity -lastVel) / -LimitAcceleration;

			// Limit by deltaT:

			phaseT = std::min(phaseT,deltaT);

			lastPos += lastVel * phaseT - LimitAcceleration / 2.0 * phaseT * phaseT;
			lastVel += -LimitAcceleration * phaseT;
			deltaT -= phaseT;
		}
		else
		if(lastVel < -LimitVelocity)
		{
			// Error condition.
			
			// Solving Step:
			//
			// increase Velocity until we are within limits:

			// -LimitVelocity = lastVel +LimitAcceleration * phaseT

			double phaseT = (-LimitVelocity -lastVel) / +LimitAcceleration;

			// Limit by deltaT:

			phaseT = std::min(phaseT,deltaT);

			lastPos += lastVel * phaseT + LimitAcceleration / 2.0 * phaseT * phaseT;
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

			double deltaPos = targetPos -lastPos;

			double dir = 0 < lastVel ? 1 : (0 > lastVel ? -1 : (0 <= deltaPos ? 1 : -1));
			phase3T = lastVel / (dir * LimitAcceleration);

			double resultDeltaPos = lastVel * phase3T -dir * LimitAcceleration/2.0 * phase3T * phase3T;

			if(
				0 < dir && 0 < deltaPos -resultDeltaPos
				|| 0 > dir && 0 > deltaPos -resultDeltaPos
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
					(2.0 * lastVel)/(dir * LimitAcceleration);

				double phase1T_2d1 =
					0.5 * ((-temp1)
					+ sqrt(temp1*temp1 -4*(-deltaPos +lastVel * phase3T - dir * LimitAcceleration/2.0 * phase3T*phase3T)/(dir * LimitAcceleration)));

				double phase1T_2d2 = (dir * LimitVelocity -lastVel) / (dir * LimitAcceleration);
				
				phase1T = std::min(phase1T_2d1, phase1T_2d2);
				phase3T += phase1T;

				resultDeltaPos = lastVel * phase1T +dir * LimitAcceleration/2.0 * phase1T * phase1T
					+ (lastVel + dir * LimitAcceleration * phase1T) * phase3T -dir * LimitAcceleration/2.0 * phase3T*phase3T;

				if(
					0 < dir && 0 < deltaPos -resultDeltaPos
					|| 0 > dir && 0 > deltaPos -resultDeltaPos
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

					if(temp2)
						phase2T = (deltaPos -lastVel * phase1T -dir * LimitAcceleration/2.0 * phase1T*phase1T
						-(temp2) * phase3T +dir * LimitAcceleration/2.0 * phase3T*phase3T)
						/ (temp2);
				}
			}

			// Limit by deltaT:

			phase3T = std::max(std::min(phase1T +phase2T +phase3T, deltaT) -phase2T -phase1T, 0.0);
			phase2T = std::max(std::min(phase1T +phase2T, deltaT) -phase1T, 0.0);
			phase1T = std::min(phase1T, deltaT);


			lastPos += lastVel * phase1T +dir * LimitAcceleration/2.0 * phase1T * phase1T
				+ (lastVel + dir * LimitAcceleration * phase1T) * phase2T
				+ (lastVel + dir * LimitAcceleration * phase1T) * phase3T -dir * LimitAcceleration/2.0 * phase3T*phase3T;
			lastVel += + dir * LimitAcceleration * phase1T - dir * LimitAcceleration * phase3T;
			deltaT -= phase1T +phase2T +phase3T;

			// If we can consider to be finished, then we use-up the time completely:
			if(abs(targetPos-lastPos) <= AFX_MATH_EPS && abs(0 -lastVel) <= AFX_MATH_EPS)
				deltaT = 0;
		}
	}
}
