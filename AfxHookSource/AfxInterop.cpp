#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvTime.h"
#include "MirvCalcs.h"
#include "AfxCommandLine.h"

#include <Windows.h>

#include <set>
#include <queue>

#include <atomic>
#include <mutex>
#include <shared_mutex>

// TODO: very fast disconnects and reconnects will cause invalid state probably.

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;
extern SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo;

namespace AfxInterop {

	const INT32 m_Version = 5;

	class CInteropClient
	{
	public:
		CInteropClient(const char * pipeName) : m_EnginePipeName(pipeName)
		{

		}

		void BeforeFrameStart()
		{
			if (!Connect()) return;

			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_BeforeFrameStart)) { errorLine = __LINE__; goto locked_error; }

			if (!SendCommands(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }

			UINT32 commandCount;

			if (!ReadCompressedUInt32(m_hEnginePipe, commandCount)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < commandCount; ++i)
			{
				std::string command;

				if (!ReadStringUTF8(m_hEnginePipe, command)) { errorLine = __LINE__; goto locked_error; }

				g_VEngineClient->ExecuteClientCmd(command.c_str());
			}

			return;

		locked_error:
			Tier0_Warning("AfxInterop::BeforeFrameStart: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void BeforeFrameRenderStart()
		{
			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_BeforeFrameRenderStart)) { errorLine = __LINE__; goto locked_error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }

			return;

		locked_error:
			Tier0_Warning("AfxInterop::BeforeFrameRenderStart: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void AfterFrameRenderStart()
		{
			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_AfterFrameRenderStart)) { errorLine = __LINE__; goto locked_error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }

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

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto locked_error; }

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

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto locked_error; }

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

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto locked_error; }

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

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto locked_error; }

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

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto locked_error; }

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

				if (!ReadCompressedUInt32(m_hEnginePipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < numCalcs; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, calcName)) { errorLine = __LINE__; goto locked_error; }

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
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteInt32(m_hEnginePipe, result->IntHandle)) { errorLine = __LINE__; goto locked_error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
					}
				}

				// Write vec ang calc result:

				while (!vecAngCalcResults.empty())
				{
					VecAngCalcResult* result = vecAngCalcResults.front();
					vecAngCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteSingle(m_hEnginePipe, result->X)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Y)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Z)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteSingle(m_hEnginePipe, result->Pitch)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Yaw)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Roll)) { errorLine = __LINE__; goto locked_error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
					}
				}

				// Write cam calc result:

				while (!camCalcResults.empty())
				{
					CamCalcResult* result = camCalcResults.front();
					camCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteSingle(m_hEnginePipe, result->X)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Y)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Z)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteSingle(m_hEnginePipe, result->Pitch)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Yaw)) { errorLine = __LINE__; goto locked_error; }
						if (!WriteSingle(m_hEnginePipe, result->Roll)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteSingle(m_hEnginePipe, result->Fov)) { errorLine = __LINE__; goto locked_error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
					}
				}

				// Write fov calc result:

				while (!fovCalcResults.empty())
				{
					FovCalcResult* result = fovCalcResults.front();
					fovCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteSingle(m_hEnginePipe, result->Fov)) { errorLine = __LINE__; goto locked_error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
					}
				}

				// Write bool calc result:

				while (!boolCalcResults.empty())
				{
					BoolCalcResult* result = boolCalcResults.front();
					boolCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteBoolean(m_hEnginePipe, result->Result)) { errorLine = __LINE__; goto locked_error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
					}
				}

				// Write int calc result:

				while (!intCalcResults.empty())
				{
					IntCalcResult* result = intCalcResults.front();
					intCalcResults.pop();

					if (result)
					{
						if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

						if (!WriteInt32(m_hEnginePipe, result->Result)) { errorLine = __LINE__; goto locked_error; }

						delete result;
					}
					else
					{
						if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
					}
				}

				// Do not flush here, since we are not waiting for data.
			}

			return;

		locked_error:
			Tier0_Warning("AfxInterop::AfterFrameRenderStart: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		bool OnViewOverride(float& Tx, float& Ty, float& Tz, float& Rx, float& Ry, float& Rz, float& Fov)
		{
			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineConnected) return false;

			int errorLine = 0;

			if (!WriteInt32(m_hEnginePipe, EngineMessage_OnViewOverride)) { errorLine = __LINE__; goto locked_error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }

			{
				bool overrideView;

				if (!ReadBoolean(m_hEnginePipe, overrideView)) { errorLine = __LINE__; goto locked_error; }

				if (overrideView)
				{
					FLOAT tTx, tTy, tTz, tRx, tRy, tRz, tFov;

					if (!ReadSingle(m_hEnginePipe, tTx)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadSingle(m_hEnginePipe, tTy)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadSingle(m_hEnginePipe, tTz)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadSingle(m_hEnginePipe, tRx)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadSingle(m_hEnginePipe, tRy)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadSingle(m_hEnginePipe, tRz)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadSingle(m_hEnginePipe, tFov)) { errorLine = __LINE__; goto locked_error; }

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

		locked_error:
			Tier0_Warning("AfxInterop::OnViewOverride: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return false;
		}

		EnabledFeatures_t& GetEnabled()
		{
			return m_Enabled;
		}

		void OnRenderView(int engineFrame, const SOURCESDK::CViewSetup_csgo& view)
		{
			m_Enabled.Clear();

			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

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

				if (!ReadBoolean(m_hEnginePipe, m_Enabled.BeforeTranslucentShadow)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_Enabled.AfterTranslucentShadow)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_Enabled.BeforeTranslucent)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_Enabled.AfterTranslucent)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_Enabled.BeforeHud)) { errorLine = __LINE__; goto error; }
				if (!ReadBoolean(m_hEnginePipe, m_Enabled.AfterHud)) { errorLine = __LINE__; goto error; }
			}

			return;

		error:
			Tier0_Warning("AfxInterop::OnRenderView: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void OnRenderViewEnd()
		{
			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineConnected) return;

			int errorLine = 0;
			{
				if (!WriteInt32(m_hEnginePipe, EngineMessage_OnRenderViewEnd)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }
			}

			return;

		error:
			Tier0_Warning("AfxInterop::OnRenderViewEnd: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void On_DrawTranslucentRenderables(SOURCESDK::CSGO::CRendering3dView* rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall)
		{
			if (bInSkybox) return;

			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineConnected) return;

			int errorLine = 0;

			{
				EngineMessage message = EngineMessage_Invalid;

				if (true == bShadowDepth)
				{
					if (false == afterCall)
					{
						message = EngineMessage_BeforeTranslucentShadow;
					}
					else
					{
						message = EngineMessage_AfterTranslucentShadow;
					}
				}
				else
				{
					if (false == afterCall)
					{
						message = EngineMessage_BeforeTranslucent;
					}
					else
					{
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

				return;
			}

		error:
			Tier0_Warning("AfxInterop::On_DrawingTranslucentRenderables: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void Shutdown() {
			Disconnect();
		}

		void DrawingThreadPrepareDraw(int frameCount, IAfxInteropSurface * mainSurface)
		{
			m_DrawingSkip = true;

			std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

			if (!(m_DrawingConnected || m_DrawingConnecting)) return;

			if (m_DrawingConnecting)
			{
				m_DrawingConnecting = false;
				m_DrawingConnected = true;
			}

			int errorLine = 0;

			while (true)
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_PreapareDraw)) { errorLine = __LINE__; goto locked_error; }
				if (!WriteInt32(m_hDrawingPipe, frameCount)) { errorLine = __LINE__; goto locked_error; }
				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto locked_error; }

				INT32 prepareDrawReply;
				if (!ReadInt32(m_hDrawingPipe, prepareDrawReply)) { errorLine = __LINE__; goto locked_error; }

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

					if (!ReadBoolean(m_hDrawingPipe, colorTextureWasLost)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadHandle(m_hDrawingPipe, sharedColorTextureHandle)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadBoolean(m_hDrawingPipe, colorDepthTextureWasLost)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadHandle(m_hDrawingPipe, sharedColorDepthTextureHandle)) { errorLine = __LINE__; goto locked_error; }

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
					{ errorLine = __LINE__; goto locked_error; }
				}

			}

		locked_error:
			Tier0_Warning("AfxInterop::DrawingThreadPrepareDraw: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
		}


		void DrawingThread_On_DrawTranslucentRenderables(bool bInSkybox, bool bShadowDepth, bool afterCall)
		{
			if (m_DrawingSkip) return;

			if (bInSkybox) return;

			std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			{
				AfxD3D_WaitForGPU();

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

				if (!WriteInt32(m_hDrawingPipe, message)) { errorLine = __LINE__; goto locked_error; }

				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto locked_error; }

				bool done;
				do {
					if (!ReadBoolean(m_hDrawingPipe, done)) { errorLine = __LINE__; goto locked_error; }
				} while (!done);

				return;
			}
		locked_error:
			Tier0_Warning("AfxInterop::DrawingThread_On_DrawTanslucentRenderables: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void DrawingThread_BeforeHud(IAfxMatRenderContextOrg* context)
		{
			if (m_DrawingSkip) return;

			std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			AfxD3D_WaitForGPU();

			if (!WriteInt32(m_hDrawingPipe, DrawingMessage_BeforeHud)) { errorLine = __LINE__; goto locked_error; }
			if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto locked_error; }

			bool done;
			do {
				if (!ReadBoolean(m_hDrawingPipe, done)) { errorLine = __LINE__; goto locked_error; }
			} while (!done);

			return;

		locked_error:
			Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void DrawingThread_AfterHud(IAfxMatRenderContextOrg* context)
		{
			if (m_DrawingSkip) return;

			std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			AfxD3D_WaitForGPU();

			if (!WriteInt32(m_hDrawingPipe, DrawingMessage_AfterHud)) { errorLine = __LINE__; goto locked_error; }
			if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto locked_error; }

			bool done;
			do {
				if (!ReadBoolean(m_hDrawingPipe, done)) { errorLine = __LINE__; goto locked_error; }
			} while (!done);

			return;

		locked_error:
			Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		void DrawingThread_OnRenderViewEnd()
		{
			if (m_DrawingSkip) return;

			std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

			if (!m_DrawingConnected) return;

			int errorLine = 0;

			if (!WriteInt32(m_hDrawingPipe, DrawingMessage_OnRenderViewEnd)) { errorLine = __LINE__; goto locked_error; }

			return;

		locked_error:
			Tier0_Warning("AfxInterop::DrawingThread_OnRenderViewEnd: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return;
		}

		bool Connect() {

			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			if (!m_EngineWantsConnect) {

				if (m_EngineConnected)
				{
					lock.unlock();
					Disconnect();
				}

				return false;
			}
			else if (m_EngineConnected) return true;

			int errorLine = 0;
			{
				std::string strPipeName("\\\\.\\pipe\\");
				strPipeName.append(m_EnginePipeName);

				while (true)
				{
					m_hEnginePipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
					if (m_hEnginePipe != INVALID_HANDLE_VALUE)
						break;

					DWORD lastError = GetLastError();

					if (lastError != ERROR_PIPE_BUSY)
					{
						Tier0_Warning("Could not open pipe. GLE=%d (%s)\n", lastError, strPipeName.c_str());
						{ errorLine = __LINE__; goto locked_error; }
					}

					if (!WaitNamedPipe(strPipeName.c_str(), 5000))
					{
						Tier0_Warning("WaitNamedPipe: timed out (%s).\n", strPipeName.c_str());
						{ errorLine = __LINE__; goto locked_error; }
					}
				}

				Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());
			}

			{
				std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

				std::string strPipeName("\\\\.\\pipe\\");
				strPipeName.append(m_EnginePipeName);
				strPipeName.append("_drawing");

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
							{ errorLine = __LINE__; goto locked_error; }
						}
						++sleepCount;
						Sleep(1);
						continue;
					case ERROR_PIPE_BUSY:
						break;
					default:
					{
						Tier0_Warning("Could not open pipe. GLE=%d (%s)\n", lastError, strPipeName.c_str());
						{ errorLine = __LINE__; goto locked_error; }
					}
					}

					if (!WaitNamedPipe(strPipeName.c_str(), 5000))
					{
						Tier0_Warning("WaitNamedPipe: timed out (%s).\n", strPipeName.c_str());
						{ errorLine = __LINE__; goto locked_error; }
					}
				}

				Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());
				m_DrawingConnecting = true;
			}

			INT32 version;
			if (!ReadInt32(m_hEnginePipe, version)) { errorLine = __LINE__; goto locked_error; }

			if (m_Version != version)
			{
				Tier0_Warning("Version %d is not a supported (%d) version.\n", version, m_Version);
				if (!WriteBoolean(m_hEnginePipe, false)) { errorLine = __LINE__; goto locked_error; }
				if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }
			}

			if (!WriteBoolean(m_hEnginePipe, true)) { errorLine = __LINE__; goto locked_error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto locked_error; }

			if (!ReadBoolean(m_hEnginePipe, m_EngineServer64Bit)) { errorLine = __LINE__; goto locked_error; }

			m_EngineConnected = true;

			return true;

		locked_error:
			Tier0_Warning("AfxInterop::Connect: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			lock.unlock();
			Disconnect();
			return false;
		}

		void Disconnect() {
			std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

			{
				std::unique_lock<std::mutex> lock(m_DrawingConnectMutex);

				if (INVALID_HANDLE_VALUE != m_hDrawingPipe)
				{
					if (!CloseHandle(m_hDrawingPipe))
					{
						Tier0_Warning("AfxInterop::Disconnect: Error in line %i (%s).\n", __LINE__, m_EnginePipeName.c_str());
					}
					m_hDrawingPipe = INVALID_HANDLE_VALUE;
				}

				if (m_DrawingConnected)
				{
					m_DrawingConnected = false;
				}

				if (m_DrawingMainSurface)
				{
					m_DrawingMainSurface->AfxReplacementEnabled_set(false);
					m_DrawingMainSurface->AfxSetReplacement(NULL);
					m_DrawingMainSurface->AfxSetDepthSurface(NULL);
					m_DrawingMainSurface = nullptr;
				}
			}

			if (INVALID_HANDLE_VALUE != m_hEnginePipe)
			{
				if (!CloseHandle(m_hEnginePipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i (%s).\n", __LINE__, m_EnginePipeName.c_str());
				}
				m_hEnginePipe = INVALID_HANDLE_VALUE;
			}

			if (m_EngineConnected)
			{
				m_EngineConnected = false;
			}
		}

		bool Console_Edit(IWrpCommandArgs* args)
		{
			int argc = args->ArgC();

			if (2 <= argc)
			{
				const char* arg1 = args->ArgV(1);

				if (0 == _stricmp("pipeName", arg1))
				{
					std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

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
					std::unique_lock<std::mutex> lock(m_EngineConnectMutex);

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

	private:

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

		std::mutex m_DrawingConnectMutex;
		bool m_DrawingConnecting = false;
		bool m_DrawingConnected = false;

		HANDLE m_hDrawingPipe = INVALID_HANDLE_VALUE;

		bool m_DrawingSkip = true;

		IAfxInteropSurface* m_DrawingMainSurface = nullptr;

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

		std::mutex m_EngineConnectMutex;
		bool m_EngineWantsConnect = false;
		bool m_EngineConnected = false;

		HANDLE m_hEnginePipe = INVALID_HANDLE_VALUE;

		std::string m_EnginePipeName;

		std::queue<CConsole> m_Commands;
		
		EnabledFeatures_t m_Enabled;

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

	int m_EngineFrame = -1;
	//bool m_Suspended = false;

	std::shared_timed_mutex m_ClientsMutex;
	std::list<CInteropClient> m_Clients;

	bool m_Enabled = false;
	bool m_MainEnabled = false;

	IAfxInteropSurface* m_Surface = NULL;
	
	void DllProcessAttach() {
		m_Enabled = 0 != g_CommandLine->FindParam(L"-afxInterop");
		m_MainEnabled = m_Enabled && 0 == g_CommandLine->FindParam(L"-afxInteropNoMain");

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
			it->BeforeFrameStart();
		}
	}

	void BeforeFrameRenderStart()
	{
		if (!m_Enabled) return;

		++m_EngineFrame;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->BeforeFrameRenderStart();
		}
	}

	void AfterFrameRenderStart()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->AfterFrameRenderStart();
		}
	}

	bool OnViewOverride(float & Tx, float & Ty, float & Tz, float & Rx, float & Ry, float & Rz, float & Fov)
	{
		if (!m_Enabled) return false;

		bool overriden = false;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->OnViewOverride(Tx, Ty, Tz, Rx, Ry, Rz, Fov)) overriden = true;
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
			it->OnRenderView(m_EngineFrame, view);
		
			outEnabled.Or(it->GetEnabled());
		}
	}

	void OnRenderViewEnd()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->OnRenderViewEnd();
		}
	}

	void On_DrawTranslucentRenderables(SOURCESDK::CSGO::CRendering3dView * rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall)
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->On_DrawTranslucentRenderables(rendering3dView, bInSkybox, bShadowDepth, afterCall);
		}
	}


	void Shutdown()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Shutdown();
		}
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

	void DrawingThreadPrepareDraw(int frameCount)
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->DrawingThreadPrepareDraw(frameCount, m_Surface);
		}
	}

	void DrawingThread_On_DrawTranslucentRenderables(bool bInSkybox, bool bShadowDepth, bool afterCall)
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->DrawingThread_On_DrawTranslucentRenderables(bInSkybox, bShadowDepth, afterCall);
		}
	}
	
	void DrawingThread_BeforeHud(IAfxMatRenderContextOrg * context)
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->DrawingThread_BeforeHud(context);
		}
	}

	void DrawingThread_AfterHud(IAfxMatRenderContextOrg * context)
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->DrawingThread_AfterHud(context);
		}
	}

	void DrawingThread_OnRenderViewEnd()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->DrawingThread_OnRenderViewEnd();
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

	if (AfxInterop::m_Clients.empty()) AfxInterop::m_Clients.emplace_back("advancedfxInterop");

	{
		CSubWrpCommandArgs subArgs(args, 1);
		if (AfxInterop::m_Clients.front().Console_Edit(&subArgs)) return;
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
					AfxInterop::m_Clients.emplace_back(args->ArgV(3));
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
							it->Console_Edit(&subArgs);
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
