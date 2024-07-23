#pragma once

//
class CMaterialSystemFunctor abstract {
public:
    virtual ~CMaterialSystemFunctor() {};
    virtual void operator()() = 0;
};
//

#include "AfxShaders.h"
#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../shared/CamPath.h"

#include <d3d11.h>
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

#include <queue>
#include <list>
#include <atomic>
#include <mutex>

// TODO: most line vertices can be cached if the colouring is done through the vertex shader.
class CCampathDrawer
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

	void BeginDevice(ID3D11Device * device);
	void EndDevice();
	void Reset();

	void OnRenderThread_Draw(ID3D11DeviceContext * pImmediateContext, const D3D11_VIEWPORT * pViewPort, ID3D11RenderTargetView * pRenderTargetView2, ID3D11DepthStencilView *pDepthStencilView2);
	void OnRenderThread_Present();

	void OnEngineThread_SetupViewDone();
	void OnEngineThread_EndFrame();

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

	std::mutex m_FunctorMutex;
	std::list<CMaterialSystemFunctor*> m_Functors;

	bool m_DrawKeyframeAxis = false;
	bool m_DrawKeyframeCam = true;

	ID3D11Device * m_Device = nullptr;
	ID3D11DeviceContext * m_DeviceContext = nullptr;
	ID3D11DeviceContext * m_ImmediateContext = nullptr;
	const D3D11_VIEWPORT * m_pViewPort = nullptr;
	ID3D11RenderTargetView * m_Rtv2 = nullptr;
	ID3D11DepthStencilView * m_Dsv2 = nullptr;

	ID3D11DepthStencilState * m_DepthStencilStateDigits = nullptr;
	ID3D11DepthStencilState * m_DepthStencilStateLines = nullptr;
	ID3D11SamplerState * m_SamplerState = nullptr;
	ID3D11RasterizerState * m_RasterizerStateDigits = nullptr;
	ID3D11RasterizerState * m_RasterizerStateLines = nullptr;
	ID3D11BlendState * m_BlendState = nullptr;
	ID3D11InputLayout * m_InputLayoutDigits = nullptr;
	ID3D11InputLayout * m_InputLayoutLines = nullptr;

	struct VS_CONSTANT_BUFFER {
		DirectX::XMFLOAT4X4 matrix;
		DirectX::XMFLOAT4 plane0;
		DirectX::XMFLOAT4 paneN;
		DirectX::XMFLOAT4 screenInfo;
	};
	ID3D11Buffer * m_ConstantBuffer = nullptr;

	struct VS_CONSTANT_BUFFER_WIDTH {
		DirectX::XMFLOAT4 width;
	};
	ID3D11Buffer * m_ConstantBufferWidth = nullptr;

	bool m_Draw = false;
	DWORD m_OldCurrentColor;
	Vector3 m_OldPreviousPolyLinePoint;
	ID3D11PixelShader * m_PixelShader = nullptr;
	bool m_PolyLineStarted;
	ID3D11VertexShader * m_VertexShader = nullptr;
	ID3D11Buffer * m_VertexBuffer = nullptr;
	UINT m_VertexBufferVertexCount = 0; // c_VertexBufferVertexCount
	Vertex * m_LockedVertexBuffer = nullptr;
	float m_DrawKeyframIndex = 18.0f;
	ID3D11Texture2D* m_DigitsTexture = nullptr;
	ID3D11ShaderResourceView * m_DigitsTextureRV = nullptr;
	ID3D11VertexShader * m_DrawTextureVertexShader = nullptr;
	ID3D11PixelShader * m_DrawTextureShader = nullptr;

	CLessDynamicProperties * m_LessDynamicProperties = nullptr;

	static void CampathChangedFn(void * pUserData);

	void CamPathChanged();

	void BuildPolyLinePoint(Vector3 previous, Vector3 current, DWORD currentColor, Vector3 next, Vertex * pOutVertexData);

	void BuildSingleLine(Vector3 from, Vector3 to, Vertex * pOutVertexData);
	void BuildSingleLine(DWORD colorFrom, DWORD colorTo, Vertex * pOutVertexData);

	void BuildSingleQuad(Vector3 p0, float t0x, float t0y, Vector3 p1, float t1x, float t1y, Vector3 p2, float t2x, float t2y, Vector3 p3, float t3x, float t3y, Vertex * pOutVertexData);

	void AutoSingleLine(Vector3 from, DWORD colorFrom, Vector3 to, DWORD colorTo);
	void AutoSingleLineFlush();

	void AutoPolyLineStart();
	void AutoPolyLinePoint(Vector3 previous, Vector3 current, DWORD colorCurrent, Vector3 next);
	void AutoPolyLineFlush();

	void AutoSingleQuad(Vector3 p0, float t0x, float t0y, Vector3 p1, float t1x, float t1y, Vector3 p2, float t2x, float t2y, Vector3 p3, float t3x, float t3y);
	void AutoSingleQuadFlush();

	bool LockVertexBuffer();
	void UnlockVertexBuffer();
	void UnloadVertexBuffer();

	/// <summary>For reducing the number of points.</summary>
	static void RamerDouglasPeucker(TempPoint * start, TempPoint * end, double epsilon);
	static double ShortestDistanceToSegment(TempPoint * pt, TempPoint * start, TempPoint * end);

	void DrawCamera(const CamPathValue & cpv, DWORD colour, int screenWidth, int screenHeight);

	void OnPostRenderAllTools_DrawingThread(CDynamicProperties * dynamicPorperties, CLessDynamicProperties * lessDynamicProperties);

	void SetLineWidth(float lineWidth);
};

extern CCampathDrawer g_CampathDrawer;
