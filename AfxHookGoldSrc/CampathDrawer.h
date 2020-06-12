#pragma once

#include <shared/CamPath.h>
#include <shared/MirvCampath.h>

#include <gl/GL.h>

#include <list>

class CCampathDrawer
	: public ICamPathChanged
	, public IMirvCampath_Drawer
{
public:

	CCampathDrawer();
	~CCampathDrawer();

	void Begin();
	void End();

	void Draw(float width, float height, float origin[3], float angles[3]);

	virtual bool GetEnabled() {
		return m_Draw;
	}
	virtual void SetEnabled(bool value) {
		m_Draw = value;
	}

	virtual bool GetDrawKeyframeAxis() {
		return m_DrawKeyframeAxis;
	}

	virtual void SetDrawKeyframeAxis(bool value) {
		m_DrawKeyframeAxis = value;
	}

	virtual bool GetDrawKeyframeCam() {
		return m_DrawKeyframeCam;
	}

	virtual void SetDrawKeyframeCam(bool value) {
		m_DrawKeyframeCam = value;
	}

	float GetDrawKeyframeIndex() { return m_DrawKeyframIndex; }
	void SetDrawKeyframeIndex(float value) { m_DrawKeyframIndex = value; }

	virtual void CamPathChanged(CamPath* obj);

private:
	struct TempPoint
	{
		double t;
		Vector3 y;
		TempPoint* nextPt;
	};

	bool m_DrawKeyframeAxis = false;
	bool m_DrawKeyframeCam = true;
	float m_DrawKeyframIndex = 18.0f;

	bool m_Draw;
	bool m_RebuildDrawing;
	std::list<double> m_TrajectoryPoints;

	bool m_HasDigitsTexture = false;
	GLuint m_DigitsTexture;

	void AutoPolyLineStart();
	void AutoPolyLinePoint(Vector3 previous, Vector3 current, DWORD colorCurrent, Vector3 next);
	void AutoPolyLineFlush();

	/// <summary>For reducing the number of points.</summary>
	void RamerDouglasPeucker(TempPoint* start, TempPoint* end, double epsilon);
	double ShortestDistanceToSegment(TempPoint* pt, TempPoint* start, TempPoint* end);

	void DrawCamera(const CamPathValue& cpv, const float colour[4], float width, float height);
};

extern CCampathDrawer g_CampathDrawer;