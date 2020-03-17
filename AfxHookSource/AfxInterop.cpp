#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvTime.h"
#include "MirvCalcs.h"
#include "AfxCommandLine.h"
#include "AfxStreams.h"

#include <Windows.h>

#include <set>
#include <queue>

#include <atomic>
#include <mutex>
#include <shared_mutex>

#include <memory>

// TODO: very fast disconnects and reconnects will cause invalid state probably.

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;
extern SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo;

namespace AfxInterop {
	IAfxInteropSurface* m_Surface = NULL;

	class CInteropClient
	{
	public:
		CInteropClient(const char * pipeName) : m_EnginePipeName(pipeName)
		{

		}

		/// <remarks>Threadsafe.</reamarks>
		void AddRef()
		{
			++m_RefCount;
		}

		/// <remarks>Threadsafe.</reamarks>
		void Release()
		{
			--m_RefCount;
			if (0 == m_RefCount) delete this;
		}

		void BeforeFrameStart()
		{
			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_BeforeFrameStart)) { errorLine = __LINE__; goto error; }

			if (!SendCommands(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			UINT32 commandCount;

			if (!ReadCompressedUInt32(m_hEnginePipe, commandCount)) { errorLine = __LINE__; goto error; }

			for (UINT32 i = 0; i < commandCount; ++i)
			{
				std::string command;

				if (!ReadStringUTF8(m_hEnginePipe, command)) { errorLine = __LINE__; goto error; }

				g_VEngineClient->ExecuteClientCmd(command.c_str());
			}

			return;

		error:
			Tier0_Warning("AfxInterop::BeforeFrameStart: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void BeforeFrameRenderStart()
		{
			ConnectEngine();

			if (m_EngineWantsConnect) QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CConnectFunctor(this)));

			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_BeforeFrameRenderStart)) { errorLine = __LINE__; goto error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::BeforeFrameRenderStart: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void AfterFrameRenderStart()
		{
			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_AfterFrameRenderStart)) { errorLine = __LINE__; goto error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			{
				UINT32 numCalcs;
				std::string calcName;

				std::queue<HandleCalcResult*> handleCalcResults;
				std::queue<VecAngCalcResult*> vecAngCalcResults;
				std::queue<CamCalcResult*> camCalcResults;
				std::queue<FovCalcResult*> fovCalcResults;
				std::queue<BoolCalcResult*> boolCalcResults;
				std::queue<IntCalcResult*> intCalcResults;

				// Read and compute handle calcs:

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto error; }

					if (IMirvHandleCalc* calc = g_MirvHandleCalcs.GetByName(calcName.c_str()))
					{
						SOURCESDK::CSGO::CBaseHandle handle;

						if (calc->CalcHandle(handle))
						{
							handleCalcResults.push(new HandleCalcResult(handle.ToInt()));
						}
						else handleCalcResults.push(nullptr);
					}
					else handleCalcResults.push(nullptr);
				}

				// Read and compute vecang calcs:

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto error; }

					if (IMirvVecAngCalc* calc = g_MirvVecAngCalcs.GetByName(calcName.c_str()))
					{
						SOURCESDK::Vector vector;
						SOURCESDK::QAngle qangle;

						if (calc->CalcVecAng(vector, qangle))
						{
							vecAngCalcResults.push(new VecAngCalcResult(vector.x, vector.y, vector.z, qangle.x, qangle.y, qangle.z));
						}
						else vecAngCalcResults.push(nullptr);
					}
					else vecAngCalcResults.push(nullptr);
				}

				// Read and compute cam calcs:

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto error; }

					if (IMirvCamCalc* calc = g_MirvCamCalcs.GetByName(calcName.c_str()))
					{
						SOURCESDK::Vector vector;
						SOURCESDK::QAngle qangle;
						float fov;

						if (calc->CalcCam(vector, qangle, fov))
						{
							camCalcResults.push(new CamCalcResult(vector.x, vector.y, vector.z, qangle.x, qangle.y, qangle.z, fov));
						}
						else camCalcResults.push(nullptr);
					}
					else camCalcResults.push(nullptr);
				}

				// Read and compute fov calcs:

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto error; }

					if (IMirvFovCalc* calc = g_MirvFovCalcs.GetByName(calcName.c_str()))
					{
						float fov;

						if (calc->CalcFov(fov))
						{
							fovCalcResults.push(new FovCalcResult(fov));
						}
						else fovCalcResults.push(nullptr);
					}
					else fovCalcResults.push(nullptr);
				}

				// Read and compute bool calcs:

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto error; }

					if (IMirvBoolCalc* calc = g_MirvBoolCalcs.GetByName(calcName.c_str()))
					{
						bool result;

						if (calc->CalcBool(result))
						{
							boolCalcResults.push(new BoolCalcResult(result));
						}
						else boolCalcResults.push(nullptr);
					}
					else boolCalcResults.push(nullptr);
				}

				// Read and compute int calcs:

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto error; }

					if (IMirvIntCalc* calc = g_MirvIntCalcs.GetByName(calcName.c_str()))
					{
						int result;

						if (calc->CalcInt(result))
						{
							intCalcResults.push(new IntCalcResult(result));
						}
						else intCalcResults.push(nullptr);
					}
					else intCalcResults.push(nullptr);
				}

				// Write handle calc result:

				while (!handleCalcResults.empty())
				{
					HandleCalcResult* result = handleCalcResults.front();
					handleCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

						if (!WriteInt32(m_hEnginePipe, result->IntHandle)) { errorLine = __LINE__; goto error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
					}
				}

				// Write vec ang calc result:

				while (!vecAngCalcResults.empty())
				{
					VecAngCalcResult* result = vecAngCalcResults.front();
					vecAngCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

						if (!WriteSingle(m_hEnginePipe, result->X)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Y)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Z)) { errorLine = __LINE__; goto error; }

						if (!WriteSingle(m_hEnginePipe, result->Pitch)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Yaw)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Roll)) { errorLine = __LINE__; goto error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
					}
				}

				// Write cam calc result:

				while (!camCalcResults.empty())
				{
					CamCalcResult* result = camCalcResults.front();
					camCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

						if (!WriteSingle(m_hEnginePipe, result->X)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Y)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Z)) { errorLine = __LINE__; goto error; }

						if (!WriteSingle(m_hEnginePipe, result->Pitch)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Yaw)) { errorLine = __LINE__; goto error; }
						if (!WriteSingle(m_hEnginePipe, result->Roll)) { errorLine = __LINE__; goto error; }

						if (!WriteSingle(m_hEnginePipe, result->Fov)) { errorLine = __LINE__; goto error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
					}
				}

				// Write fov calc result:

				while (!fovCalcResults.empty())
				{
					FovCalcResult* result = fovCalcResults.front();
					fovCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

						if (!WriteSingle(m_hEnginePipe, result->Fov)) { errorLine = __LINE__; goto error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
					}
				}

				// Write bool calc result:

				while (!boolCalcResults.empty())
				{
					BoolCalcResult* result = boolCalcResults.front();
					boolCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

						if (!WriteBoolean(m_hEnginePipe, result->Result)) { errorLine = __LINE__; goto error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
					}
				}

				// Write int calc result:

				while (!intCalcResults.empty())
				{
					IntCalcResult* result = intCalcResults.front();
					intCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

						if (!WriteInt32(m_hEnginePipe, result->Result)) { errorLine = __LINE__; goto error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
					}
				}

				// Do not flush here, since we are not waiting for data.
			}

			return;

		error:
			Tier0_Warning("AfxInterop::AfterFrameRenderStart: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		bool OnViewOverride(float& Tx, float& Ty, float& Tz, float& Rx, float& Ry, float& Rz, float& Fov)
		{
			if (!m_EngineConnected) return false;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_OnViewOverride)) { errorLine = __LINE__; goto error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			{
				bool overrideView;

				if (!ReadBoolean(m_hEnginePipe, overrideView)) { errorLine = __LINE__; goto error; }

				if (overrideView)
				{
					FLOAT tTx, tTy, tTz, tRx, tRy, tRz, tFov;

					if (!ReadSingle(m_hEnginePipe, tTx)) { errorLine = __LINE__; goto error; }
					if (!ReadSingle(m_hEnginePipe, tTy)) { errorLine = __LINE__; goto error; }
					if (!ReadSingle(m_hEnginePipe, tTz)) { errorLine = __LINE__; goto error; }
					if (!ReadSingle(m_hEnginePipe, tRx)) { errorLine = __LINE__; goto error; }
					if (!ReadSingle(m_hEnginePipe, tRy)) { errorLine = __LINE__; goto error; }
					if (!ReadSingle(m_hEnginePipe, tRz)) { errorLine = __LINE__; goto error; }
					if (!ReadSingle(m_hEnginePipe, tFov)) { errorLine = __LINE__; goto error; }

					Tx = tTx;
					Ty = tTy;
					Tz = tTz;
					Rx = tRx;
					Ry = tRy;
					Rz = tRz;
					Fov = tFov;

					return true;
				}

			}

			return false;

		error:
			Tier0_Warning("AfxInterop::OnViewOverride: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return false;
		}

		bool GetActive()
		{
			return m_EngineWantsConnect;
		}

		EnabledFeatures_t& GetEnabledFeatures()
		{
			return m_EnabledFeatures;
		}

		void OnRenderView(int engineFrame, const SOURCESDK::CViewSetup_csgo& view)
		{
			m_EnabledFeatures.Clear();

			if (!m_EngineConnected) return;

			int errorLine = 0;
			{

				if (!WriteInt32(m_hEnginePipe, EngineMessage_OnRenderView)) { errorLine = __LINE__; goto error; }

				if (!WriteInt32(m_hEnginePipe, engineFrame)) { errorLine = __LINE__; goto error; }

				if (!WriteSingle(m_hEnginePipe, g_MirvTime.GetAbsoluteFrameTime())) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, g_MirvTime.GetTime())) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, g_MirvTime.GetFrameTime())) { errorLine = __LINE__; goto error; }

				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledX)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledY)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledWidth)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledHeight)) { errorLine = __LINE__; goto error; }

				SOURCESDK::VMatrix worldToView;
				SOURCESDK::VMatrix viewToProjection;
				SOURCESDK::VMatrix worldToProjection;
				SOURCESDK::VMatrix worldToPixels;

				g_pVRenderView_csgo->GetMatricesForView(view, &worldToView, &viewToProjection, &worldToProjection, &worldToPixels);

				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][3])) { errorLine = __LINE__; goto error; }

				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][3])) { errorLine = __LINE__; goto error; }

				if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

				if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.BeforeTranslucentShadow)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.AfterTranslucentShadow)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.BeforeTranslucent)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.AfterTranslucent)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.BeforeHud)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.AfterHud)) { errorLine = __LINE__; goto error; }
			}

			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CPrepareDrawFunctor(this, AfxInterop::GetFrameCount())));

			return;

		error:
			Tier0_Warning("AfxInterop::OnRenderView: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void OnRenderViewEnd()
		{
			if (!m_EngineConnected) return;

			int errorLine = 0;
			{
				if (!WriteInt32(m_hEnginePipe, EngineMessage_OnRenderViewEnd)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }
			}

			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new COnRenderViewEndFunctor(this)));

			return;

		error:
			Tier0_Warning("AfxInterop::OnRenderViewEnd: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void On_DrawTranslucentRenderables(IAfxMatRenderContext* ctx, SOURCESDK::CSGO::CRendering3dView* rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall)
		{
			if (bInSkybox) return;

			if (!m_EngineConnected) return;

			int errorLine = 0;

			{
				EngineMessage message = EngineMessage_Invalid;

				if (true == bShadowDepth)
				{
					if (false == afterCall)
					{
						if (!m_EnabledFeatures.BeforeTranslucentShadow) return;
						message = EngineMessage_BeforeTranslucentShadow;
					}
					else
					{
						if (!m_EnabledFeatures.AfterTranslucentShadow) return;
						message = EngineMessage_AfterTranslucentShadow;
					}
				}
				else
				{
					if (false == afterCall)
					{
						if (!m_EnabledFeatures.BeforeTranslucent) return;
						message = EngineMessage_BeforeTranslucent;
					}
					else
					{
						if (!m_EnabledFeatures.AfterTranslucent) return;
						message = EngineMessage_AfterTranslucent;
					}
				}

				if (!WriteInt32(m_hEnginePipe, message)) { errorLine = __LINE__; goto error; }

				SOURCESDK::CViewSetup_csgo& view = rendering3dView->AfxHackGetViewSetup();

				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledX)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledY)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledWidth)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hEnginePipe, view.m_nUnscaledHeight)) { errorLine = __LINE__; goto error; }

				SOURCESDK::VMatrix worldToView;
				SOURCESDK::VMatrix viewToProjection;
				SOURCESDK::VMatrix worldToProjection;
				SOURCESDK::VMatrix worldToPixels;

				g_pVRenderView_csgo->GetMatricesForView(view, &worldToView, &viewToProjection, &worldToProjection, &worldToPixels);

				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[0][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[1][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[2][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, worldToView.m[3][3])) { errorLine = __LINE__; goto error; }

				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[0][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[1][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[2][3])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][0])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][1])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][2])) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, viewToProjection.m[3][3])) { errorLine = __LINE__; goto error; }

				QueueOrExecute(ctx->GetOrg(), new CAfxLeafExecute_Functor(new COn_DrawTranslucentRenderablesFunctor(this, bInSkybox, bShadowDepth, afterCall)));

				return;
			}

		error:
			Tier0_Warning("AfxInterop::On_DrawingTranslucentRenderables: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void OnBeforeHud(IAfxMatRenderContext* ctx)
		{
			QueueOrExecute(ctx->GetOrg(), new CAfxLeafExecute_Functor(new CBeforeHudFunctor(this)));
		}

		void OnAfterHud(IAfxMatRenderContext* ctx)
		{
			QueueOrExecute(ctx->GetOrg(), new CAfxLeafExecute_Functor(new CAfterHudFunctor(this)));
		}

		void Shutdown() {
			DisconnectDrawing();
			DisconnectEngine();
		}

		void DrawingThreadPrepareDraw(int frameCount, IAfxInteropSurface * mainSurface)
		{
			m_DrawingSkip = true;
			m_DrawingFrameCount = frameCount;

			if (!m_DrawingConnected) return;

			if (6 == m_DrawingVersion) return;

			int errorLine = 0;

			while (true)
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_PreapareDraw)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hDrawingPipe, frameCount)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }

				INT32 prepareDrawReply;
				if (!ReadInt32(m_hDrawingPipe, prepareDrawReply)) { errorLine = __LINE__; goto error; }

				switch (prepareDrawReply)
				{
				case PrepareDrawReply_Skip:
					m_DrawingSkip = true;
					return;
				case PrepareDrawReply_Retry:
					break;
				case PrepareDrawReply_Continue:
				{
					m_DrawingSkip = false;

					bool colorTextureWasLost;
					HANDLE sharedColorTextureHandle;
					bool colorDepthTextureWasLost;
					HANDLE sharedColorDepthTextureHandle;

					if (!ReadBoolean(m_hDrawingPipe, colorTextureWasLost)) { errorLine = __LINE__; goto error; }
					if (!ReadHandle(m_hDrawingPipe, sharedColorTextureHandle)) { errorLine = __LINE__; goto error; }
					if (!ReadBoolean(m_hDrawingPipe, colorDepthTextureWasLost)) { errorLine = __LINE__; goto error; }
					if (!ReadHandle(m_hDrawingPipe, sharedColorDepthTextureHandle)) { errorLine = __LINE__; goto error; }

					m_DrawingMainSurface = mainSurface;

					if (mainSurface)
					{
						if (colorTextureWasLost) mainSurface->AfxSetReplacement(NULL);
						if (colorDepthTextureWasLost) mainSurface->AfxSetDepthSurface(NULL);

						if (colorTextureWasLost && NULL != sharedColorTextureHandle || colorDepthTextureWasLost && NULL != sharedColorDepthTextureHandle)
						{
							IDirect3DDevice9* device = NULL;
							if (SUCCEEDED(mainSurface->AfxGetSurface()->GetDevice(&device)))
							{
								D3DSURFACE_DESC desc;

								if (SUCCEEDED(mainSurface->AfxGetSurface()->GetDesc(&desc)))
								{
									if (colorTextureWasLost && NULL != sharedColorTextureHandle)
									{
										IDirect3DTexture9* replacementTexture = NULL;
										if (sharedColorTextureHandle)
										{
											if (SUCCEEDED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &replacementTexture, &sharedColorTextureHandle)))
											{
												IDirect3DSurface9* replacementSurface = NULL;

												if (SUCCEEDED(replacementTexture->GetSurfaceLevel(0, &replacementSurface)))
												{
													mainSurface->AfxSetReplacement(replacementSurface);
													mainSurface->AfxReplacementEnabled_set(true);
													replacementSurface->Release();
												}
												replacementTexture->Release();
											}
										}
									}
									if (colorDepthTextureWasLost && NULL != sharedColorDepthTextureHandle)
									{
										IDirect3DTexture9* replacementTexture = NULL;
										if (sharedColorDepthTextureHandle)
										{
											if (SUCCEEDED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &replacementTexture, &sharedColorDepthTextureHandle)))
											{
												IDirect3DSurface9* replacementSurface = NULL;

												if (SUCCEEDED(replacementTexture->GetSurfaceLevel(0, &replacementSurface)))
												{
													mainSurface->AfxSetDepthSurface(replacementSurface);
													replacementSurface->Release();
												}
												replacementTexture->Release();
											}
										}
									}
								}
								device->Release();
							}
						}
					}

					return;
				}
				default:
					{ errorLine = __LINE__; goto error; }
				}

			}

		error:
			Tier0_Warning("AfxInterop::DrawingThreadPrepareDraw: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
		}


		void DrawingThread_On_DrawTranslucentRenderables(bool bInSkybox, bool bShadowDepth, bool afterCall)
		{
			if (m_DrawingSkip) return;

			if (bInSkybox) return;

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			{
				DrawingMessage message = DrawingMessage_Invalid;

				if (true == bShadowDepth)
				{
					if (false == afterCall)
					{
						message = DrawingMessage_BeforeTranslucentShadow;
					}
					else
					{
						message = DrawingMessage_AfterTranslucentShadow;
					}
				}
				else
				{
					if (false == afterCall)
					{
						message = DrawingMessage_BeforeTranslucent;
					}
					else
					{
						message = DrawingMessage_AfterTranslucent;
					}
				}


				if (6 == m_DrawingVersion)
				{
					if(!WriteInt32(m_hDrawingPipe, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
					if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }
					if(!HandleVersion6DrawingReply()) { errorLine = __LINE__; goto error; }
				}
				else
				{
					AfxD3D_WaitForGPU();

					if (!WriteInt32(m_hDrawingPipe, message)) { errorLine = __LINE__; goto error; }

					if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }

					bool done;
					do {
						if (!ReadBoolean(m_hDrawingPipe, done)) { errorLine = __LINE__; goto error; }
					} while (!done);
				}


				return;
			}
		error:
			Tier0_Warning("AfxInterop::DrawingThread_On_DrawTanslucentRenderables: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return;
		}

		void DrawingThread_BeforeHud()
		{
			if (m_DrawingSkip) return;

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			if (6 == m_DrawingVersion)
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_BeforeHud)) { errorLine = __LINE__; goto error; }

				if (!WriteInt32(m_hDrawingPipe, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }
				if (!HandleVersion6DrawingReply()) { errorLine = __LINE__; goto error; }
			}
			else
			{
				AfxD3D_WaitForGPU();

				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_BeforeHud)) { errorLine = __LINE__; goto error; }

				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }

				bool done;
				do {
					if (!ReadBoolean(m_hDrawingPipe, done)) { errorLine = __LINE__; goto error; }
				} while (!done);
			}

			return;

		error:
			Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return;
		}

		void DrawingThread_AfterHud()
		{
			if (m_DrawingSkip) return;

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			if (6 == m_DrawingVersion)
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_AfterHud)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hDrawingPipe, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }
				if (!HandleVersion6DrawingReply()) { errorLine = __LINE__; goto error; }
			}
			else
			{
				AfxD3D_WaitForGPU();
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_AfterHud)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }
				bool done;
				do {
					if (!ReadBoolean(m_hDrawingPipe, done)) { errorLine = __LINE__; goto error; }
				} while (!done);
			}

			return;

		error:
			Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return;
		}

		void DrawingThread_OnRenderViewEnd()
		{
			if (m_DrawingSkip) return;

			if (!m_DrawingConnected) return;

			if (6 == m_DrawingVersion) return;

			int errorLine = 0;

			if (!WriteInt32(m_hDrawingPipe, DrawingMessage_OnRenderViewEnd)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::DrawingThread_OnRenderViewEnd: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return;
		}

		void DrawingThread_DeviceLost()
		{
			while (!m_SharedSurfaces.empty())
			{
				auto it = m_SharedSurfaces.begin();
				it->second.Texture->Release();
				m_SharedSurfaces.erase(it);
			}
		}

		bool ConnectDrawing() {
			{
				std::shared_lock<std::shared_timed_mutex> sharedLock(m_DrawingConnectMutex);

				if (!m_DrawingWantsConnect) {
					if (m_DrawingConnected) {
						sharedLock.unlock();
						DisconnectDrawing();
					}
					return false;
				}
				else if (m_DrawingConnected) return true;
			}

			int errorLine = 0;
			{
				std::shared_lock< std::shared_timed_mutex> engineSharedLock(m_EngineConnectMutex);
				std::unique_lock<std::shared_timed_mutex> drawingLock(m_DrawingConnectMutex);

				if (m_DrawingWantsConnect && !m_DrawingConnected)
				{
					if (!m_DrawingPreConnect)
					{
						std::string strPipeName("\\\\.\\pipe\\");
						strPipeName.append(m_DrawingPipeName);

						unsigned int sleepCount = 0;

						while (true)
						{
							m_hDrawingPipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
							if (m_hDrawingPipe != INVALID_HANDLE_VALUE)
								break;

							DWORD lastError = GetLastError();

							switch (lastError)
							{
							case ERROR_FILE_NOT_FOUND:
								if (sleepCount >= 5000)
								{
									Tier0_Warning("Could not open pipe. GLE=%d (%s)\n", lastError, strPipeName.c_str());
									{ errorLine = __LINE__; goto error; }
								}
								++sleepCount;
								Sleep(1);
								continue;
							case ERROR_PIPE_BUSY:
								break;
							default:
							{
								Tier0_Warning("Could not open pipe. GLE=%d (%s)\n", lastError, strPipeName.c_str());
								{ errorLine = __LINE__; goto error; }
							}
							}

							if (!WaitNamedPipe(strPipeName.c_str(), 5000))
							{
								Tier0_Warning("WaitNamedPipe: timed out (%s).\n", strPipeName.c_str());
								{ errorLine = __LINE__; goto error; }
							}
						}

						Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());
						m_DrawingPreConnect = true;
					}
					else if (m_EngineConnected)
					{
						m_DrawingConnected = true;
						return true;
					}

				}
			}
			return false;

		error:
			Tier0_Warning("AfxInterop::ConnectDrawing: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return false;
		}

		void DisconnectDrawing()
		{
			std::unique_lock<std::shared_timed_mutex> darwingLock(m_DrawingConnectMutex);

			DrawingThread_DeviceLost();

			if (INVALID_HANDLE_VALUE != m_hDrawingPipe)
			{
				if (!CloseHandle(m_hDrawingPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i (%s).\n", __LINE__, m_DrawingPipeName.c_str());
				}
				m_hDrawingPipe = INVALID_HANDLE_VALUE;
			}

			m_DrawingConnected = false;
			m_DrawingPreConnect = false;

			if (m_DrawingMainSurface)
			{
				m_DrawingMainSurface->AfxReplacementEnabled_set(false);
				m_DrawingMainSurface->AfxSetReplacement(NULL);
				m_DrawingMainSurface->AfxSetDepthSurface(NULL);
				m_DrawingMainSurface = nullptr;
			}
		}

		bool ConnectEngine() {

			{
				std::shared_lock<std::shared_timed_mutex> sharedLock(m_EngineConnectMutex);

				if (!m_EngineWantsConnect) {
					if (m_EngineConnected) {
						sharedLock.unlock();
						DisconnectEngine();
					}
					return false;
				}
				else if (m_EngineConnected) return true;
			}

			int errorLine = 0;
			{
				std::unique_lock<std::shared_timed_mutex> darwingLock(m_DrawingConnectMutex);
				std::unique_lock<std::shared_timed_mutex> engineLock(m_EngineConnectMutex);

				if(!m_DrawingConnected) // do not do stuff while drawing thread is still connected
				{
					if (m_EngineWantsConnect && !m_EngineConnected)
					{
						if (!m_EnginePreConnect)
						{
							std::string strPipeName("\\\\.\\pipe\\");
							strPipeName.append(m_EnginePipeName);

							m_DrawingPipeName = m_EnginePipeName;
							m_DrawingPipeName.append("_drawing");

							while (true)
							{
								m_hEnginePipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
								if (m_hEnginePipe != INVALID_HANDLE_VALUE)
									break;

								DWORD lastError = GetLastError();

								if (lastError != ERROR_PIPE_BUSY)
								{
									Tier0_Warning("Could not open pipe. GLE=%d (%s)\n", lastError, strPipeName.c_str());
									{ errorLine = __LINE__; goto error; }
								}

								if (!WaitNamedPipe(strPipeName.c_str(), 5000))
								{
									Tier0_Warning("WaitNamedPipe: timed out (%s).\n", strPipeName.c_str());
									{ errorLine = __LINE__; goto error; }
								}
							}

							Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());

							m_EnginePreConnect = true;
							m_DrawingWantsConnect = true;
						}
						else if(m_DrawingPreConnect) // do not do furhter stuff until drawing thread is preconnected (to maintain backwards compat)
						{
							if (!ReadInt32(m_hEnginePipe, m_EngineVersion)) { errorLine = __LINE__; goto error; }

							switch (m_EngineVersion)
							{
							case 5:
							case 6:
								break;
							default:
								Tier0_Warning("Version %d is not supported.\n", m_EngineVersion);
								if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto error; }
								if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }
								{ errorLine = __LINE__; goto error; }
							}

							if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto error; }

							if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

							if (!ReadBoolean(m_hEnginePipe, m_EngineServer64Bit)) { errorLine = __LINE__; goto error; }

							m_DrawingVersion = m_EngineVersion;
							m_EngineConnected = true;
							return true;
						}
					}
				}			
			}
			return false;

		error:
			Tier0_Warning("AfxInterop::Connect: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return false;
		}

		void DisconnectEngine() {
			std::unique_lock<std::shared_timed_mutex> drawingLock(m_DrawingConnectMutex);
			std::unique_lock<std::shared_timed_mutex> engineLock(m_EngineConnectMutex);

			if (INVALID_HANDLE_VALUE != m_hEnginePipe)
			{
				if (!CloseHandle(m_hEnginePipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i (%s).\n", __LINE__, m_EnginePipeName.c_str());
				}
				m_hEnginePipe = INVALID_HANDLE_VALUE;
			}

			m_EngineConnected = false;
			m_EnginePreConnect = false;
			m_DrawingWantsConnect = false;
		}

		bool Console_Edit(IWrpCommandArgs* args)
		{
			int argc = args->ArgC();

			if (2 <= argc)
			{
				const char* arg1 = args->ArgV(1);

				if (0 == _stricmp("pipeName", arg1))
				{
					if (3 <= argc)
					{
						const char* arg2 = args->ArgV(2);

						m_EnginePipeName = arg2;

						return true;
					}

					Tier0_Msg(
						"afx_interop pipeName <sName> - Set new name.\n"
						"Current value: %s\n"
						, m_EnginePipeName.c_str()
					);
					return true;
				}
				else if (0 == _stricmp("connect", arg1))
				{
					if (3 <= argc)
					{
						const char* arg2 = args->ArgV(2);

						m_EngineWantsConnect = 0 != atoi(arg2);

						return true;
					}

					Tier0_Msg(
						"afx_interop connect 0|1 - 0 = disabled (default), 1 = enabled.\n"
						"Current value: %i\n"
						, m_EngineWantsConnect ? 1 : 0
					);
					return true;
				}
				else if (0 == _stricmp("send", arg1))
				{
					CSubWrpCommandArgs subArgs(args, 2);

					AddCommand(&subArgs);

					return true;
				}
			}

			const char* arg0 = args->ArgV(0);

			Tier0_Msg("%s pipeName [...] - Name of the pipe to connect to.\n", arg0);
			Tier0_Msg("%s connect [...] - Controls if interop connection is enabled.\n", arg0);
			Tier0_Msg("%s send [<arg1>[ <arg2> [ ...]] - Queues a command to be sent to the server (lossy if connection is unstable).\n", arg0);

			return false;
		}

	protected:
		~CInteropClient()
		{
			Shutdown();
		}

	private:
		std::atomic_int m_RefCount;

		struct HandleCalcResult
		{
			INT32 IntHandle;

			HandleCalcResult(INT32 intHandle)
				: IntHandle(intHandle)
			{
			}
		};

		struct VecAngCalcResult
		{
			FLOAT X;
			FLOAT Y;
			FLOAT Z;
			FLOAT Pitch;
			FLOAT Yaw;
			FLOAT Roll;

			VecAngCalcResult(FLOAT x, FLOAT y, FLOAT z, FLOAT pitch, FLOAT yaw, FLOAT roll)
				: X(x), Y(y), Z(z), Pitch(pitch), Yaw(yaw), Roll(roll)
			{

			}
		};

		struct CamCalcResult
		{
			FLOAT X;
			FLOAT Y;
			FLOAT Z;
			FLOAT Pitch;
			FLOAT Yaw;
			FLOAT Roll;
			FLOAT Fov;

			CamCalcResult(FLOAT x, FLOAT y, FLOAT z, FLOAT pitch, FLOAT yaw, FLOAT roll, FLOAT fov)
				: X(x), Y(y), Z(z), Pitch(pitch), Yaw(yaw), Roll(roll), Fov(fov)
			{

			}

		};

		struct FovCalcResult
		{
			FLOAT Fov;

			FovCalcResult(FLOAT fov)
				: Fov(fov)
			{
			}
		};

		struct BoolCalcResult
		{
			BOOL Result;

			BoolCalcResult(BOOL result)
				: Result(result)
			{
			}
		};

		struct IntCalcResult
		{
			INT32 Result;

			IntCalcResult(INT32 result)
				: Result(result)
			{
			}
		};

		enum DrawingMessage
		{
			DrawingMessage_Invalid = 0,
			DrawingMessage_PreapareDraw = 1,
			DrawingMessage_BeforeTranslucentShadow = 2,
			DrawingMessage_AfterTranslucentShadow = 3,
			DrawingMessage_BeforeTranslucent = 4,
			DrawingMessage_AfterTranslucent = 5,
			DrawingMessage_BeforeHud = 6,
			DrawingMessage_AfterHud = 7,
			DrawingMessage_OnRenderViewEnd = 8
		};

		enum PrepareDrawReply
		{
			PrepareDrawReply_Skip = 1,
			PrepareDrawReply_Retry = 2,
			PrepareDrawReply_Continue = 3
		};



		class CConnectFunctor
			: public CAfxFunctor
		{
		public:
			CConnectFunctor(CInteropClient* client)
				: m_Client(client)
			{
				client->AddRef();
			}

			virtual void operator()()
			{
				m_Client->ConnectDrawing();
				m_Client->Release();
			}

		private:
			CInteropClient* m_Client;
		};

		class CPrepareDrawFunctor
			: public CAfxFunctor
		{
		public:
			CPrepareDrawFunctor(CInteropClient* client, int frameCount)
				: m_Client(client),
				m_FrameCount(frameCount)
			{
				client->AddRef();
			}

			virtual void operator()()
			{
				m_Client->DrawingThreadPrepareDraw(m_FrameCount, m_Surface);
				m_Client->Release();
			}

		private:
			CInteropClient* m_Client;
			int m_FrameCount;
		};

		class COn_DrawTranslucentRenderablesFunctor
			: public CAfxFunctor
		{
		public:
			COn_DrawTranslucentRenderablesFunctor(CInteropClient* client, bool bInSkybox, bool bShadowDepth, bool afterCall)
				: m_Client(client)
				, m_bInSkyBox(bInSkybox)
				, m_bShadowDepth(bShadowDepth)
				, m_bAfterCall(afterCall)
			{
				client->AddRef();
			}

			virtual void operator()()
			{
				m_Client->DrawingThread_On_DrawTranslucentRenderables(m_bInSkyBox, m_bShadowDepth, m_bAfterCall);
				m_Client->Release();
			}

		private:
			CInteropClient* m_Client;
			bool m_bInSkyBox;
			bool m_bShadowDepth;
			bool m_bAfterCall;
		};

		class COnRenderViewEndFunctor
			: public CAfxFunctor
		{
		public:
			COnRenderViewEndFunctor(CInteropClient* client)
				: m_Client(client)
			{
				client->AddRef();
			}

			virtual void operator()() override
			{
				m_Client->DrawingThread_OnRenderViewEnd();
				m_Client->Release();
			}
		private:
			CInteropClient* m_Client;
		};


		class CBeforeHudFunctor
			: public CAfxFunctor
		{
		public:
			CBeforeHudFunctor(CInteropClient* client)
				: m_Client(client)
			{
				client->AddRef();
			}

			virtual void operator()() override
			{
				m_Client->DrawingThread_BeforeHud();
				m_Client->Release();
			}
		private:
			CInteropClient* m_Client;
		};

		class CAfterHudFunctor
			: public CAfxFunctor
		{
		public:
			CAfterHudFunctor(CInteropClient* client)
				: m_Client(client)
			{
				client->AddRef();
			}

			virtual void operator()() override
			{
				m_Client->DrawingThread_AfterHud();
				m_Client->Release();
			}
		private:
			CInteropClient* m_Client;
		};

		std::shared_timed_mutex m_DrawingConnectMutex;
		std::string m_DrawingPipeName;
		bool m_DrawingWantsConnect = false;
		bool m_DrawingPreConnect = false;
		bool m_DrawingConnected = false;

		HANDLE m_hDrawingPipe = INVALID_HANDLE_VALUE;

		bool m_DrawingSkip = true;
		int m_DrawingFrameCount = -1;

		IAfxInteropSurface* m_DrawingMainSurface = nullptr;

		struct SharedSurfacesValue
		{
			HANDLE Handle = nullptr;
			IDirect3DTexture9* Texture = nullptr;
		};

		std::map<INT32, SharedSurfacesValue> m_SharedSurfaces;

		bool HandleVersion6DrawingReply()
		{
			int errorLine = 0;

			INT32 textureId;
			HANDLE sharedTextureHandle;
			UINT32 texWidth;
			UINT32 texHeight;
			UINT32 d3dFormat;

			if (!ReadInt32(m_hDrawingPipe, textureId)) { errorLine = __LINE__; goto error; }
			if (!ReadHandle(m_hDrawingPipe, sharedTextureHandle)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, texWidth)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, texHeight)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, d3dFormat)) { errorLine = __LINE__; goto error; }

			SharedSurfacesValue* val = nullptr;

			{
				auto  itOld = m_SharedSurfaces.find(textureId);
				if (itOld != m_SharedSurfaces.end())
				{
					if(itOld->second.Handle != sharedTextureHandle)
					{
						itOld->second.Texture->Release();
						m_SharedSurfaces.erase(itOld);
					}
					else
					{
						val = &(itOld->second);
					}
				}
			}

			if (sharedTextureHandle)
			{
				if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
				{
					IDirect3DTexture9* texture;

					if (SUCCEEDED(device->CreateTexture(texWidth, texHeight, 1, 0, (D3DFORMAT)d3dFormat, D3DPOOL_DEFAULT, &texture, &sharedTextureHandle)))
					{
						SharedSurfacesValue& val = m_SharedSurfaces[textureId];

						val.Handle = sharedTextureHandle;
						val.Texture = texture;
					}
				}
			}

			while (true)
			{
				bool hasRect;
				if (!ReadBoolean(m_hDrawingPipe, hasRect)) { errorLine = __LINE__; goto error; }

				if (!hasRect) break;

				INT32 rectX, rectY, rectWidth, rectHeight;

				if (!ReadInt32(m_hDrawingPipe, rectX)) { errorLine = __LINE__; goto error; }
				if (!ReadInt32(m_hDrawingPipe, rectY)) { errorLine = __LINE__; goto error; }
				if (!ReadInt32(m_hDrawingPipe, rectWidth)) { errorLine = __LINE__; goto error; }
				if (!ReadInt32(m_hDrawingPipe, rectHeight)) { errorLine = __LINE__; goto error; }

				if (val)
				{
					AfxDrawRect(val->Texture, rectX, rectY, rectWidth, rectHeight, 0.5f + rectX / (float)texWidth, 0.5f + rectY / (float)texHeight, 0.5f + (rectX + rectWidth -1) / (float)texWidth, 0.5f + (rectY + rectHeight -1) / (float)texHeight);
				}
			}

			AfxD3D_WaitForGPU(); // TODO: Improve. We are slowing down more than we have to.
			if(!WriteBoolean(m_hDrawingPipe, true)) { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion6DrawingReply: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		enum EngineMessage {
			EngineMessage_Invalid = 0,
			EngineMessage_LevelInitPreEntity = 1,
			EngineMessage_LevelShutDown = 2,
			EngineMessage_BeforeFrameStart = 3,
			EngineMessage_OnRenderView = 4,
			EngineMessage_OnRenderViewEnd = 5,
			EngineMessage_BeforeFrameRenderStart = 6,
			EngineMessage_AfterFrameRenderStart = 7,
			EngineMessage_OnViewOverride = 8,
			EngineMessage_BeforeTranslucentShadow = 9,
			EngineMessage_AfterTranslucentShadow = 10,
			EngineMessage_BeforeTranslucent = 11,
			EngineMessage_AfterTranslucent = 12
		};

		class CConsole
		{
		public:
			CConsole(IWrpCommandArgs* args)
			{
				for (int i = 0; i < args->ArgC(); ++i)
				{
					m_Args.push(args->ArgV(i));
				}
			}

			size_t GetArgCount() const
			{
				return m_Args.size();
			}

			const char* GetArgFront() const
			{
				return m_Args.front().c_str();
			}

			void PopArgFront()
			{
				m_Args.pop();
			}

			bool HasArg()
			{
				return !m_Args.empty();
			}

		private:
			std::queue<std::string> m_Args;
		};

		bool m_EngineServer64Bit = false;

		std::shared_timed_mutex m_EngineConnectMutex;
		bool m_EngineWantsConnect = false;
		bool m_EnginePreConnect = false;
		bool m_EngineConnected = false;

		HANDLE m_hEnginePipe = INVALID_HANDLE_VALUE;

		std::string m_EnginePipeName;

		std::queue<CConsole> m_Commands;
		
		EnabledFeatures_t m_EnabledFeatures;

		int m_EngineVersion = 5;
		int m_DrawingVersion = 5;

		bool ConsoleSend(HANDLE hPipe, CConsole & command)
		{
			if (!WriteCompressedUInt32(hPipe, (UINT32)command.GetArgCount())) return false;

			bool okay = true;

			while (command.HasArg())
			{
				okay = okay && WriteStringUTF8(hPipe, command.GetArgFront());
				command.PopArgFront();
			}

			return okay;
		}


		void AddCommand(IWrpCommandArgs* args)
		{
			m_Commands.emplace(args);
		}

		bool SendCommands(HANDLE hPipe)
		{
			if (!WriteCompressedUInt32(hPipe, (UINT32)m_Commands.size())) return false;

			bool okay = true;

			while (!m_Commands.empty())
			{
				okay = okay && ConsoleSend(hPipe, m_Commands.front());
				m_Commands.pop();
			}

			return okay;
		}

		bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
		{
			lpBuffer = &(((char*)lpBuffer)[offset]);
			BOOL success = false;

			do
			{
				DWORD bytesRead;

				success = ReadFile(hFile, lpBuffer, numBytes, &bytesRead, NULL);

				if (!success)
				{
					Tier0_Warning("!ReadBytes: GetLastError=%d\n", GetLastError());
					return false;
				}

				numBytes -= bytesRead;
				lpBuffer = &(((char*)lpBuffer)[bytesRead]);

			} while (0 < numBytes);

			return true;
		}

		bool ReadBoolean(HANDLE hFile, bool& outValue)
		{
			BYTE useVal;

			bool result = ReadBytes(hFile, &useVal, 0, sizeof(useVal));

			if (result) outValue = 0 != useVal ? true : false;

			return result;
		}

		bool ReadByte(HANDLE hFile, BYTE& outValue)
		{
			return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
		}

		bool ReadSByte(HANDLE hFile, signed char& value)
		{
			return ReadByte(hFile, (BYTE&)value);
		}

		bool ReadUInt32(HANDLE hFile, UINT32& outValue)
		{
			return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
		}

		bool ReadCompressedUInt32(HANDLE hFile, UINT32& outValue)
		{
			BYTE value;

			if (!ReadByte(hFile, value))
				return false;

			if (value < 255)
			{
				outValue = value;
				return true;
			}

			return ReadUInt32(hFile, outValue);
		}

		bool ReadInt32(HANDLE hFile, INT32& outValue)
		{
			return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
		}

		bool ReadCompressedInt32(HANDLE hFile, INT32& outValue)
		{
			signed char value;

			if (!ReadSByte(hFile, value))
				return false;

			if (value < 127)
			{
				outValue = value;
				return true;
			}

			return ReadInt32(hFile, outValue);
		}

		bool ReadHandle(HANDLE hFile, HANDLE& outValue)
		{
			DWORD value32;

			if (ReadBytes(hFile, &value32, 0, sizeof(value32)))
			{
				outValue = ULongToHandle(value32);
				return true;
			}

			return false;
		}

		bool ReadSingle(HANDLE hFile, FLOAT& outValue)
		{
			return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
		}

		bool ReadStringUTF8(HANDLE hFile, std::string& outValue)
		{
			UINT32 length;

			if (!ReadCompressedUInt32(hFile, length)) return false;

			outValue.resize(length);

			if (!ReadBytes(hFile, &outValue[0], 0, length)) return false;

			return true;
		}

		bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
		{
			DWORD bytesWritten;

			if (!WriteFile(hFile, &(((char*)lpBuffer)[offset]), numBytes, &bytesWritten, NULL) || numBytes != bytesWritten)
				return false;

			return true;
		}

		bool WriteBoolean(HANDLE hFile, bool value) {

			BYTE useVal = value ? 1 : 0;

			return WriteBytes(hFile, &useVal, 0, sizeof(useVal));
		}

		bool WriteByte(HANDLE hFile, BYTE value) {

			return WriteBytes(hFile, &value, 0, sizeof(value));
		}

		bool WriteSByte(HANDLE hFile, signed char value)
		{
			return WriteByte(hFile, (BYTE)value);
		}

		bool WriteUInt32(HANDLE hFile, UINT32 value) {
			return WriteBytes(hFile, &value, 0, sizeof(value));
		}

		bool WriteCompressedUInt32(HANDLE hFile, UINT32 value)
		{
			if (0 <= value && value <= 255 - 1)
				return WriteByte(hFile, (BYTE)value);

			return WriteByte(hFile, 255) && WriteUInt32(hFile, value);
		}

		bool WriteInt32(HANDLE hFile, INT32 value) {
			return WriteBytes(hFile, &value, 0, sizeof(value));
		}

		bool WriteCompressedInt32(HANDLE hFile, INT32 value)
		{
			if (-128 <= value && value <= 127 - 1)
				return WriteSByte(hFile, (signed char)value);

			return WriteSByte(hFile, 127)
				&& WriteUInt32(hFile, value);
		}

		bool WriteSingle(HANDLE hFile, FLOAT value)
		{
			return WriteBytes(hFile, &value, 0, sizeof(value));
		}

		bool WriteStringUTF8(HANDLE hFile, const std::string value)
		{
			UINT32 length = (UINT32)value.length();

			return WriteCompressedUInt32(hFile, length)
				&& WriteBytes(hFile, (LPVOID)value.c_str(), 0, length);
		}

		bool WriteHandle(HANDLE hFile, HANDLE value)
		{
			DWORD value32 = HandleToULong(value);

			return WriteBytes(hFile, &value32, 0, sizeof(value32));
		}

		bool Flush(HANDLE hFile)
		{
			if (!FlushFileBuffers(hFile))
				return false;

			return true;
		}
	};

	class CInteropClientRef
	{
	public:
		CInteropClientRef(CInteropClient* client)
			: m_Client(client)
		{
			client->AddRef();
		}

		CInteropClientRef(const CInteropClientRef& client)
			: m_Client(client.m_Client)
		{
			m_Client->AddRef();
		}

		~CInteropClientRef()
		{
			m_Client->Release();
		}

		CInteropClient* Get() const
		{
			return m_Client;
		}

	private:
		CInteropClient* m_Client;
	};

	int m_EngineFrame = -1;
	//bool m_Suspended = false;

	std::shared_timed_mutex m_ClientsMutex;
	std::list<CInteropClientRef> m_Clients;

	bool m_Enabled = false;
	bool m_MainEnabled = false;

	void DllProcessAttach() {
		m_MainEnabled = 0 != g_CommandLine->FindParam(L"-afxInterop");
		m_Enabled = m_MainEnabled || 0 != g_CommandLine->FindParam(L"-afxInteropLight");

		if (!m_Enabled) return;
	}

	int GetFrameCount() {
		return m_EngineFrame;
	}

	void BeforeFrameStart()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->BeforeFrameStart();
		}
	}

	void BeforeFrameRenderStart()
	{
		if (!m_Enabled) return;

		++m_EngineFrame;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->BeforeFrameRenderStart();
		}
	}

	void AfterFrameRenderStart()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->AfterFrameRenderStart();
		}
	}

	bool OnViewOverride(float & Tx, float & Ty, float & Tz, float & Rx, float & Ry, float & Rz, float & Fov)
	{
		if (!m_Enabled) return false;

		bool overriden = false;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->Get()->OnViewOverride(Tx, Ty, Tz, Rx, Ry, Rz, Fov)) overriden = true;
		}

		return overriden;
	}

	void OnRenderView(const SOURCESDK::CViewSetup_csgo & view, EnabledFeatures_t & outEnabled)
	{
		if (!m_Enabled) return;

		outEnabled.Clear();

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->OnRenderView(m_EngineFrame, view);
		
			outEnabled.Or(it->Get()->GetEnabledFeatures());
		}
	}

	void OnRenderViewEnd()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->OnRenderViewEnd();
		}
	}

	void On_DrawTranslucentRenderables(IAfxMatRenderContext* ctx, SOURCESDK::CSGO::CRendering3dView * rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall)
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->On_DrawTranslucentRenderables(ctx, rendering3dView, bInSkybox, bShadowDepth, afterCall);
		}
	}

	void OnBeforeHud(IAfxMatRenderContext* ctx) {
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->OnBeforeHud(ctx);
		}
	}

	void OnAfterHud(IAfxMatRenderContext* ctx) {
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->OnAfterHud(ctx);
		}
	}


	void Shutdown()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		m_Clients.clear();
	}

	void LevelInitPreEntity(char const* pMapName) {
		if (!m_Enabled) return;

	}

	void LevelShutdown() {
		if (!m_Enabled) return;

	}

	bool Enabled() {
		return m_Enabled;
	}

	bool MainEnabled() {
		return m_MainEnabled;
	}

	bool Active() {
		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->Get()->GetActive()) return true;
		}

		return false;
	}

	void DrawingThread_DeviceLost()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->DrawingThread_DeviceLost();
		}
	}

	void OnCreatedSurface(IAfxInteropSurface * surface)
	{
		if (m_Surface)
		{
			OnReleaseSurface(m_Surface);
		}

		m_Surface = surface;
	}

	void OnReleaseSurface(IAfxInteropSurface * surface)
	{
		if (surface)
		{
			surface->AfxReplacementEnabled_set(false);
			surface->AfxSetReplacement(NULL);
			surface->AfxSetDepthSurface(NULL);

			m_Surface = NULL;
		}
	}

	/// <param name="info">can be nullptr</param>
	void OnSetRenderTarget(DWORD RenderTargetIndex, IAfxInteropSurface * surface)
	{
		if (0 == RenderTargetIndex)
		{
			// Hm.
		}
	}

}

CON_COMMAND(afx_interop, "Controls advancedfxInterop (i.e. with Unity engine).")
{
	if (!AfxInterop::m_Enabled)
	{
		Tier0_Warning("Error: afx_interop command requires -afxInterop launch option!\n");
		return;
	}

	std::unique_lock<std::shared_timed_mutex> clientsLock(AfxInterop::m_ClientsMutex);

	if (AfxInterop::m_Clients.empty()) AfxInterop::m_Clients.emplace_back(new AfxInterop::CInteropClient("advancedfxInterop"));

	{
		CSubWrpCommandArgs subArgs(args, 1);
		if (AfxInterop::m_Clients.front().Get()->Console_Edit(&subArgs)) return;
	}

	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("clients", arg1))
		{
			if (3 <= argc)
			{
				const char* arg2 = args->ArgV(2);

				if (0 == _stricmp(arg2, "add") && 4 == argc)
				{
					AfxInterop::m_Clients.emplace_back(new AfxInterop::CInteropClient(args->ArgV(3)));
				}
				else if (0 == _stricmp(arg2, "clear") && 4 == argc)
				{
					AfxInterop::m_Clients.clear();
				}
				if (0 == _stricmp(arg2, "remove") && 4 == argc)
				{
					int target = atoi(args->ArgV(3));
					int idx = 0;
					for (auto it = AfxInterop::m_Clients.begin(); it != AfxInterop::m_Clients.end(); ++it)
					{
						if (target == idx)
						{
							AfxInterop::m_Clients.erase(it);
							return;
						}

						++idx;
					}

					Tier0_Warning("Invalid index %i\n", idx);
					return;
				}
				else if (0 == _stricmp(arg2, "edit") && 4 == argc)
				{
					int target = atoi(args->ArgV(3));
					int idx = 0;
					for (auto it = AfxInterop::m_Clients.begin(); it != AfxInterop::m_Clients.end(); ++it)
					{
						if (target == idx)
						{
							CSubWrpCommandArgs subArgs(args, 4);
							it->Get()->Console_Edit(&subArgs);
							return;
						}

						++idx;
					}

					Tier0_Warning("Invalid index %i\n", idx);
					return;
				}
			}


			Tier0_Msg(
				"afx_interop clients add <sPipeName> - Add client to back of list and set its pipeName (0 is default client, first added will be 1).\n"
				"afx_interop clients clear <sPipeName> - Clear clients (including default).\n"
				"afx_interop clients remove  <iNumber> - Remove client.\n"
				"afx_interop clients edit <iNumber> [...] - Edit client (0 is default client).\n"
			);
			return;
		}

	}

	Tier0_Msg(
		"afx_interop clients [...] - Edit client list directly.\n"
	);
}

#endif
