#pragma once

#include "SourceInterfaces.h"
#include "AfxShaders.h"
#include "MaterialSystemHooks.h"

#include <shared/CamPath.h>
#include <d3d9.h>
#include <list>
#include <atomic>

#define CCampathDrawer_VertexFVF D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(1) | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(2)

// TODO: most line vertices can be cached if the colouring is done through the vertex shader.
class CCampathDrawer
: public ICamPathChanged
{
public:

	CCampathDrawer();
	~CCampathDrawer();

	void Begin();
	void End();

	void Draw_set(bool value);
	bool Draw_get(void);

	void SetDrawKeyframeAxis(bool value) { m_DrawKeyframeAxis = value; }
	bool GetDrawKeyframeAxis() { return m_DrawKeyframeAxis;  }

	void SetDrawKeyframeCam(bool value) { m_DrawKeyframeCam = value; }
	bool GetDrawKeyframeCam() { return m_DrawKeyframeCam; }

	float GetDrawKeyframeIndex() { return m_DrawKeyframIndex; }
	void SetDrawKeyframeIndex(float value) { m_DrawKeyframIndex = value; }

	void BeginDevice(IDirect3DDevice9 * device);
	void EndDevice();
	void Reset();

	void OnPostRenderAllTools_EngineThread();

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

	class CDynamicProperties {
	public:
		CDynamicProperties(CCampathDrawer * drawer);

		double GetCurTime() const {
			return m_CurTime;
		}

		bool GetInCampath() const {
			return m_InCampath;
		}

		bool GetCampathEnabled() const {
			return m_CampathEnabled;
		}

		int GetScreenWidth() const {
			return m_ScreenWidth;
		}

		int GetScreenHeight() const {
			return m_ScreenHeight;
		}

		const SOURCESDK::VMatrix & GetWorldToScreenMatrix() const {
			return m_WorldToScreenMatrix;
		}

		const Vector3 & GetPlaneOrigin() const {
			return m_PlaneOrigin;
		}

		const Vector3 & GetPlaneNormal() const {
			return m_PlaneNormal;
		}
	
		const Vector3 & GetPlaneRight() const {
			return m_PlaneRight;
		}

		const Vector3 & GetPlaneUp() const {
			return m_PlaneUp;
		}

		float GetDrawKeyframeIndex() const {
			return m_DrawKeyFrameIndex;
		}

		bool GetDrawKeyframeAxis() const {
			return m_DrawKeyframeAxis;
		}

		bool GetDrawKeyframeCam() const {
			return m_DrawKeyframeCam;
		}

		const CamPathValue & GetCurrentValue() const {
			return m_CurrentValue;
		}

	private:
		double m_CurTime;
		bool m_InCampath;
		bool m_CampathEnabled;
		int m_ScreenWidth;
		int m_ScreenHeight;
		SOURCESDK::VMatrix m_WorldToScreenMatrix;
		Vector3 m_PlaneOrigin;
		Vector3 m_PlaneNormal;
		Vector3 m_PlaneRight;
		Vector3 m_PlaneUp;

		float m_DrawKeyFrameIndex;
		bool m_DrawKeyframeAxis;
		bool m_DrawKeyframeCam;

		CamPathValue m_CurrentValue;
	};

	class CLessDynamicProperties {
	public:
		void AddRef() {
			++m_RefCount;
		}

		void Release() {
			int value = --m_RefCount;
			if(0 == value) {
				delete this;
			}
		}

		class CKeyframe {
		public:
			CKeyframe(double time, const CamPathValue & value)
			: m_Time(time)
			, m_Value(value) {

			}

			double GetTime() const {
				return m_Time;
			}

			const CamPathValue & GetValue() const {
				return m_Value;
			}

		private:
			double m_Time;
			CamPathValue m_Value;
		};

		CLessDynamicProperties(CCampathDrawer * drawer);

		bool GetCampathCanEval() const {
			return m_CampathCanEval;
		}

		std::list<CKeyframe>::const_iterator GetKeyframesBegin() const {
			return m_Keyframes.cbegin();
		}

		std::list<CKeyframe>::const_iterator GetKeyframesEnd() const {
			return m_Keyframes.cend();
		}

		size_t GetKeyframesSize() const {
			return m_Keyframes.size();
		}

		std::list<CKeyframe>::const_iterator GetTrajectoryPointsBegin() const {
			return m_TrajectoryPoints.cbegin();
		}

		std::list<CKeyframe>::const_iterator GetTrajectoryPointsEnd() const {
			return m_TrajectoryPoints.cend();
		}

	private:
		std::atomic_int m_RefCount = 0;
		bool m_CampathCanEval;
		std::list<CKeyframe> m_Keyframes;
		std::list<CKeyframe> m_TrajectoryPoints;
	};

	class CCampathDrawerFunctor : public CMaterialSystemFunctor {
	public:
		CCampathDrawerFunctor(CCampathDrawer * drawer) {
			m_DynamicProperties = new CDynamicProperties(drawer);
			m_LessDynamicProperties = drawer->m_LessDynamicProperties;
			m_LessDynamicProperties->AddRef();
		}

		virtual ~CCampathDrawerFunctor() {
			m_LessDynamicProperties->Release();
			delete m_DynamicProperties;
		}

		virtual void operator()();

	private:
		CDynamicProperties * m_DynamicProperties;
		CLessDynamicProperties * m_LessDynamicProperties;
	};

	bool m_DrawKeyframeAxis = false;
	bool m_DrawKeyframeCam = true;

	IDirect3DDevice9 * m_Device;
	bool m_Draw;
	DWORD m_OldCurrentColor;
	Vector3 m_OldPreviousPolyLinePoint;
	IAfxPixelShader * m_PixelShader;
	bool m_PolyLineStarted;
	IAfxVertexShader * m_VertexShader;
	IDirect3DVertexBuffer9 * m_VertexBuffer;
	UINT m_VertexBufferVertexCount; // c_VertexBufferVertexCount
	Vertex * m_LockedVertexBuffer;
	float m_DrawKeyframIndex = 18.0f;
	IDirect3DTexture9* m_DigitsTexture = nullptr;
	IAfxPixelShader* m_DrawTextureShader = nullptr;

	CLessDynamicProperties * m_LessDynamicProperties = nullptr;

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
	static void RamerDouglasPeucker(TempPoint * start, TempPoint * end, double epsilon);
	static double ShortestDistanceToSegment(TempPoint * pt, TempPoint * start, TempPoint * end);

	void DrawCamera(const CamPathValue & cpv, DWORD colour, FLOAT screenInfo[4], int screenWidth, int screenHeight);

	void OnPostRenderAllTools_DrawingThread(CDynamicProperties * dynamicPorperties, CLessDynamicProperties * lessDynamicProperties);
};

extern CCampathDrawer g_CampathDrawer;
