#pragma once

#include "SourceInterfaces.h"
#include "AfxShaders.h"
#include <shared/CamPath.h>
#include <d3d9.h>
#include <list>

#define CCampathDrawer_VertexFVF D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(1) | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(2)

// TODO: most line vertices can be cached if the colouring is done through the vertex shader.
class CCampathDrawer
: public ICamPathChanged
{
public:

	CCampathDrawer();
	~CCampathDrawer();

	void Draw_set(bool value);
	bool Draw_get(void);

	void SetDrawKeyframeAxis(bool value) { m_DrawKeyframeAxis = value; }
	bool GetDrawKeyframeAxis() { return m_DrawKeyframeAxis;  }

	void SetDrawKeyframeCam(bool value) { m_DrawKeyframeCam = value; }
	bool GetDrawKeyframeCam() { return m_DrawKeyframeCam; }

	void BeginDevice(IDirect3DDevice9 * device);
	void EndDevice();
	void Reset();

	void OnPostRenderAllTools();

	virtual void CamPathChanged(CamPath * obj);

private:
	struct Vertex
	{
		FLOAT x, y, z; // Position of current line point
		DWORD diffuse; // Diffuse color of current line point
		FLOAT t0u, t0v, t0w; // Extrusion direction from current line point (-1/1), reserved
		FLOAT t1u, t1v, t1w; // Unit vector pointing to previous line point
		FLOAT t2u, t2v, t2w; // Unit vector pointing to next line point
	};

	struct TempPoint
	{
		double t;
		Vector3 y;
		TempPoint * nextPt;
	};

	bool m_DrawKeyframeAxis = false;
	bool m_DrawKeyframeCam = true;

	IDirect3DDevice9 * m_Device;
	bool m_Draw;
	DWORD m_OldCurrentColor;
	Vector3 m_OldPreviousPolyLinePoint;
	IAfxPixelShader * m_PixelShader;
	bool m_PolyLineStarted;
	bool m_RebuildDrawing;
	IAfxVertexShader * m_VertexShader;
	SOURCESDK::VMatrix m_WorldToScreenMatrix;
	IDirect3DVertexBuffer9 * m_VertexBuffer;
	UINT m_VertexBufferVertexCount; // c_VertexBufferVertexCount
	Vertex * m_LockedVertexBuffer;
	std::list<double> m_TrajectoryPoints;

	void BuildPolyLinePoint(Vector3 previous, Vector3 current, DWORD currentColor, Vector3 next, Vertex * pOutVertexData);

	void BuildSingleLine(Vector3 from, Vector3 to, Vertex * pOutVertexData);
	void BuildSingleLine(DWORD colorFrom, DWORD colorTo, Vertex * pOutVertexData);

	void AutoSingleLine(Vector3 from, DWORD colorFrom, Vector3 to, DWORD colorTo);
	void AutoSingleLineFlush();

	void AutoPolyLineStart();
	void AutoPolyLinePoint(Vector3 previous, Vector3 current, DWORD colorCurrent, Vector3 next);
	void AutoPolyLineFlush();

	bool LockVertexBuffer();
	void UnlockVertexBuffer();
	void UnloadVertexBuffer();

	/// <summary>For reducing the number of points.</summary>
	void RamerDouglasPeucker(TempPoint * start, TempPoint * end, double epsilon);
	double ShortestDistanceToSegment(TempPoint * pt, TempPoint * start, TempPoint * end);

	void DrawCamera(const CamPathValue & cpv, DWORD colour, FLOAT screenInfo[4]);
};

extern CCampathDrawer g_CampathDrawer;
