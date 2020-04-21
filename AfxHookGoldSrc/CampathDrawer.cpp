#include "stdafx.h"

#include "CampathDrawer.h"

#include "filming.h"

#include "hooks/DemoPlayer/DemoPlayer.h"
#include "hooks/HookHw.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <gl/GL.h>


const FLOAT c_CampathCrossRadius = 36.0f;
const FLOAT c_CampathCrossPixelWidth = 4.0f;

const FLOAT c_CameraRadius = c_CampathCrossRadius / 2.0f;
const FLOAT c_CameraPixelWidth = 4.0f;

const FLOAT c_CameraTrajectoryPixelWidth = 8.0f;

/// <summary>Epsilon for point reduction in world units (inch).</summary>
/// <remarks>Must be at least 0.0.</remarks>
const double c_CameraTrajectoryEpsilon = 1.0f;

/// <remarks>Must be at least 2.</remarks>
const size_t c_CameraTrajectoryMaxPointsPerInterval = 1024;


CCampathDrawer g_CampathDrawer;


// CCampathDrawer //////////////////////////////////////////////////////////////

CCampathDrawer::CCampathDrawer()
	: m_Draw(false)
	, m_RebuildDrawing(true)
{
}

CCampathDrawer::~CCampathDrawer()
{
}

void CCampathDrawer::Begin()
{
	g_Filming.GetCamPath()->OnChanged_set(this);
	m_RebuildDrawing = true;
}

void CCampathDrawer::End()
{
	g_Filming.GetCamPath()->OnChanged_set(nullptr);
}

void CCampathDrawer::CamPathChanged(CamPath* obj)
{
	m_RebuildDrawing = true;
}

#define ValCondInv(value,invert) ((invert) ? 1.0f - (value) : (value) )

void CCampathDrawer::Draw(float width, float height)
{
	if (!m_Draw)
		return;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);

	// Draw:
	{
		CamPath *camPath = g_Filming.GetCamPath();

		double curTime = g_DemoPlayer->GetDemoTime() - camPath->GetOffset();
		bool inCampath = 1 <= camPath->GetSize()
			&& camPath->GetLowerBound() <= curTime
			&& curTime <= camPath->GetUpperBound();
		bool campathCanEval = camPath->CanEval();
		bool campathEnabled = camPath->Enabled_get();
		bool cameraMightBeSelected = false;

		// Draw trajectory:
		if (2 <= camPath->GetSize() && campathCanEval)
		{
			if (m_RebuildDrawing)
			{
				// Rebuild trajectory points.
				// This operation can be quite expensive (up to O(N^2)),
				// so it should be done only when s.th.
				// changed (which is what we do here).

				m_TrajectoryPoints.clear();

				CamPathIterator last = camPath->GetBegin();
				CamPathIterator it = last;

				TempPoint* pts = new TempPoint[c_CameraTrajectoryMaxPointsPerInterval];

				for (++it; it != camPath->GetEnd(); ++it)
				{
					double delta = it.GetTime() - last.GetTime();

					for (size_t i = 0; i < c_CameraTrajectoryMaxPointsPerInterval; i++)
					{
						double t = last.GetTime() + delta * ((double)i / (c_CameraTrajectoryMaxPointsPerInterval - 1));

						CamPathValue cpv = camPath->Eval(t);

						pts[i].t = t;
						pts[i].y = Vector3(cpv.X, cpv.Y, cpv.Z);
						pts[i].nextPt = i + 1 < c_CameraTrajectoryMaxPointsPerInterval ? &(pts[i + 1]) : 0;
					}

					RamerDouglasPeucker(&(pts[0]), &(pts[c_CameraTrajectoryMaxPointsPerInterval - 1]), c_CameraTrajectoryEpsilon);

					// add all points except the last one (to avoid duplicates):
					for (TempPoint* pt = &(pts[0]); pt && pt->nextPt; pt = pt->nextPt)
					{
						m_TrajectoryPoints.push_back(pt->t);
					}

					last = it;
				}

				// add last point:
				m_TrajectoryPoints.push_back(pts[c_CameraTrajectoryMaxPointsPerInterval - 1].t);

				delete[] pts;

				m_RebuildDrawing = false;
			}

			glLineWidth(c_CameraTrajectoryPixelWidth);
			std::list<double>::iterator itPts = m_TrajectoryPoints.begin();

			CamPathIterator itKeysLast = camPath->GetBegin();
			CamPathIterator itKeysNext = itKeysLast;
			++itKeysNext;

			double curPtTime;
			CamPathValue curPtValue;

			glBegin(GL_LINE_STRIP);

			do
			{
				curPtTime = *itPts;
				curPtValue = camPath->Eval(curPtTime);
				++itPts;

				// emit current point:
				{
					double deltaTime = abs(curTime - curPtTime);

					float colour[4];

					// determine colour:
					if (deltaTime < 1.0)
					{
						double t = (deltaTime - 0.0) / 1.0;
						colour[0] = ValCondInv((float)t, curPtValue.Selected);
						colour[1] = ValCondInv(1.0f, curPtValue.Selected);
						colour[2] = ValCondInv(0.0f, curPtValue.Selected);
						colour[3] = (0.5f * (1.0f - (float)t)) + 0.5f;
					}
					else if (deltaTime < 2.0)
					{
						double t = (deltaTime - 1.0) / 1.0;
						colour[0] = ValCondInv(1.0f, curPtValue.Selected);
						colour[1] = ValCondInv(1.0f - (float)t, curPtValue.Selected);
						colour[2] = ValCondInv(0.0f, curPtValue.Selected);
						colour[3] = (0.25f * (1.0f - (float)t)) + 0.25f;
					}
					else
					{
						colour[0] = ValCondInv(1.0f, curPtValue.Selected);
						colour[1] = ValCondInv(0.0f, curPtValue.Selected);
						colour[2] = ValCondInv(0.0f, curPtValue.Selected);
						colour[3] = 0.25f;
					}

					glColor4f(colour[0], colour[1], colour[2], colour[3]);
					glVertex3f(
						(float)curPtValue.X, (float)curPtValue.Y, (float)curPtValue.Z
					);
				}
			} while (itPts != m_TrajectoryPoints.end());

			glEnd();
		}

		// Draw keyframes:
		{
			glLineWidth(c_CampathCrossPixelWidth);

			bool lpSelected = false;
			double lpTime;

			for (CamPathIterator it = camPath->GetBegin(); it != camPath->GetEnd(); ++it)
			{
				double cpT = it.GetTime();
				CamPathValue cpv = it.GetValue();

				cameraMightBeSelected = cameraMightBeSelected || lpSelected && cpv.Selected && lpTime <= curTime && curTime <= cpT;

				lpSelected = cpv.Selected;
				lpTime = cpT;

				double deltaTime = abs(curTime - cpT);

				bool selected = cpv.Selected;

				float colour[4];

				// determine colour:
				if (deltaTime < 1.0)
				{
					double t = (deltaTime - 0.0) / 1.0;
					colour[0] = ValCondInv((float)t, selected);
					colour[1] = ValCondInv(1.0f, selected);
					colour[2] = ValCondInv(0.0f, selected);
					colour[3] = (0.5f * (1.0f - (float)t)) + 0.5f;
				}
				else if (deltaTime < 2.0)
				{
					double t = (deltaTime - 1.0) / 1.0;
					colour[0] = ValCondInv(1.0f, selected);
					colour[1] = ValCondInv(1.0f  - (float)t, selected);
					colour[2] = ValCondInv(0.0f, selected);
					colour[3] = (0.25f * (1.0f - (float)t)) + 0.25f;
				}
				else
				{
					colour[0] = ValCondInv(1.0f, selected);
					colour[1] = ValCondInv(0.0f, selected);
					colour[2] = ValCondInv(0.0f, selected);
					colour[3] = 0.25f;
				}

				glColor4f(colour[0], colour[1], colour[2], colour[3]);

				if (m_DrawKeyframeAxis)
				{
					glBegin(GL_LINES);

					// x / forward line:

					glVertex3f((float)(cpv.X - c_CampathCrossRadius), (float)cpv.Y, (float)cpv.Z);
					glVertex3f((float)(cpv.X + c_CampathCrossRadius), (float)cpv.Y, (float)cpv.Z);

					// y / left line:

					glVertex3f((float)cpv.X, (float)(cpv.Y - c_CampathCrossRadius), (float)cpv.Z);
					glVertex3f((float)cpv.X, (float)(cpv.Y + c_CampathCrossRadius), (float)cpv.Z);

					// z / up line:

					glVertex3f((float)cpv.X, (float)cpv.Y, (float)(cpv.Z - c_CampathCrossRadius));
					glVertex3f((float)cpv.X, (float)cpv.Y, (float)(cpv.Z + c_CampathCrossRadius));

					glEnd();
				}

				if (m_DrawKeyframeCam) DrawCamera(cpv, colour, width, height);
			}

		}

		// Draw wireframe camera:
		if (inCampath && campathCanEval)
		{
			float colourCam[4] = {
				ValCondInv(1.0f, cameraMightBeSelected),
				ValCondInv(campathEnabled ? 0.0f : 1.0f, cameraMightBeSelected),
				ValCondInv(1.0f, cameraMightBeSelected),
				0.5f
			};

			CamPathValue cpv = camPath->Eval(curTime);

			DrawCamera(cpv, colourCam, width, height);
		}
	}

	glLineWidth(1.0f);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void CCampathDrawer::DrawCamera(const CamPathValue& cpv, const float colour[4], float width, float height)
{
	glLineWidth(c_CameraPixelWidth);

	// limit to values as RenderView hook:
	double fov = min(179, max(1, cpv.Fov));

	double forward[3], right[3], up[3];
	QEulerAngles ang = cpv.R.ToQREulerAngles().ToQEulerAngles();
	MakeVectors(ang.Roll, ang.Pitch, ang.Yaw, forward, right, up);

	Vector3 vCp(cpv.X, cpv.Y, cpv.Z);
	Vector3 vForward(forward);
	Vector3 vUp(up);
	Vector3 vRight(right);

	double a = sin(fov * M_PI / 360.0) * c_CameraRadius;
	double b = a;

	double aspectRatio = width ? (double)height / (double)width : 1.0;

	b *= aspectRatio;

	Vector3 vLU = vCp + (double)c_CameraRadius * vForward - a * vRight + b * vUp;
	Vector3 vRU = vCp + (double)c_CameraRadius * vForward + a * vRight + b * vUp;
	Vector3 vLD = vCp + (double)c_CameraRadius * vForward - a * vRight - b * vUp;
	Vector3 vRD = vCp + (double)c_CameraRadius * vForward + a * vRight - b * vUp;
	Vector3 vMU = vLU + (vRU - vLU) / 2;
	Vector3 vMUU = vMU + 0.5 * b * vUp;

	glColor4f(colour[0], colour[1], colour[2], colour[3]);

	glBegin(GL_LINES);
	glVertex3f((float)vCp.X, (float)vCp.Y, (float)vCp.Z);
	glVertex3f((float)vLD.X, (float)vLD.Y, (float)vLD.Z);

	glVertex3f((float)vCp.X, (float)vCp.Y, (float)vCp.Z);
	glVertex3f((float)vRD.X, (float)vRD.Y, (float)vRD.Z);

	glVertex3f((float)vCp.X, (float)vCp.Y, (float)vCp.Z);
	glVertex3f((float)vLU.X, (float)vLU.Y, (float)vLU.Z);

	glVertex3f((float)vCp.X, (float)vCp.Y, (float)vCp.Z);
	glVertex3f((float)vRU.X, (float)vRU.Y, (float)vRU.Z);

	glVertex3f((float)vLD.X, (float)vLD.Y, (float)vLD.Z);
	glVertex3f((float)vRD.X, (float)vRD.Y, (float)vRD.Z);

	glVertex3f((float)vRD.X, (float)vRD.Y, (float)vRD.Z);
	glVertex3f((float)vRU.X, (float)vRU.Y, (float)vRU.Z);

	glVertex3f((float)vRU.X, (float)vRU.Y, (float)vRU.Z);
	glVertex3f((float)vLU.X, (float)vLU.Y, (float)vLU.Z);

	glVertex3f((float)vLU.X, (float)vLU.Y, (float)vLU.Z);
	glVertex3f((float)vLD.X, (float)vLD.Y, (float)vLD.Z);

	glVertex3f((float)vLU.X, (float)vLU.Y, (float)vLU.Z);
	glVertex3f((float)vMUU.X, (float)vMUU.Y, (float)vMUU.Z);

	glVertex3f((float)vRU.X, (float)vRU.Y, (float)vRU.Z);
	glVertex3f((float)vMUU.X, (float)vMUU.Y, (float)vMUU.Z);

	glEnd();
}

void CCampathDrawer::RamerDouglasPeucker(TempPoint* start, TempPoint* end, double epsilon)
{
	double dmax = 0;
	TempPoint* index = start;

	for (TempPoint* i = start->nextPt; i && i != end; i = i->nextPt)
	{
		double d = ShortestDistanceToSegment(i, start, end);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}

	// If max distance is greater than epsilon, recursively simplify
	if (dmax > epsilon)
	{
		RamerDouglasPeucker(start, index, epsilon);
		RamerDouglasPeucker(index, end, epsilon);
	}
	else {
		start->nextPt = end;
	}
}

double CCampathDrawer::ShortestDistanceToSegment(TempPoint* pt, TempPoint* start, TempPoint* end)
{
	double ESx = end->y.X - start->y.X;
	double ESy = end->y.Y - start->y.Y;
	double ESz = end->y.Z - start->y.Z;
	double dESdES = ESx * ESx + ESy * ESy + ESz * ESz;
	double t = dESdES ? (
		(pt->y.X - start->y.X) * ESx + (pt->y.Y - start->y.Y) * ESy + (pt->y.Z - start->y.Z) * ESz
		) / dESdES : 0.0;

	if (t <= 0.0)
		return (start->y - pt->y).Length();
	else
		if (1.0 <= t)
			return (pt->y - end->y).Length();

	return (pt->y - (start->y + t * (end->y - start->y))).Length();
}
