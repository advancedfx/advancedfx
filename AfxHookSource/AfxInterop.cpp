#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvTime.h"
#include "MirvTime.h"
#include "MirvCalcs.h"
#include "AfxCommandLine.h"
#include "AfxStreams.h"
#include "csgo_GameEvents.h"

#include <Windows.h>

#include <set>
#include <queue>

#include <atomic>
#include <mutex>
#include <shared_mutex>

#include <memory>

#include <strsafe.h>

void ErrorExit(LPTSTR lpszFunction, DWORD dw)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}


// TODO: very fast disconnects and reconnects will cause invalid state probably.

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;
extern SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo;

namespace AfxInterop {
	IAfxInteropSurface* m_Surface = NULL;

	class CInteropClient : public CAfxGameEventListenerSerialzer
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

			if (!WriteUInt32(m_hEnginePipe, EngineMessage_BeforeFrameStart)) { errorLine = __LINE__; goto error; }

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

			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CConnectFunctor(this)));

			if (!m_EngineConnected) return;

			int errorLine = 0;

			if (!WriteUInt32(m_hEnginePipe, EngineMessage_BeforeFrameRenderStart)) { errorLine = __LINE__; goto error; }

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			if (7 == m_EngineVersion)
			{
				if (!ReadGameEventSettings(true)) { errorLine = __LINE__; goto error; }
			}

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

			if (!WriteUInt32(m_hEnginePipe, EngineMessage_AfterFrameRenderStart)) { errorLine = __LINE__; goto error; }

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

			if (!WriteUInt32(m_hEnginePipe, EngineMessage_OnViewOverride)) { errorLine = __LINE__; goto error; }

			if (7 == m_EngineVersion)
			{
				if (!WriteSingle(m_hEnginePipe, Tx)) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, Ty)) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, Tz)) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, Rx)) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, Ry)) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, Rz)) { errorLine = __LINE__; goto error; }
				if (!WriteSingle(m_hEnginePipe, Fov)) { errorLine = __LINE__; goto error; }
			}

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
				if (!WriteUInt32(m_hEnginePipe, EngineMessage_OnRenderView)) { errorLine = __LINE__; goto error; }

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

				if (6 == m_EngineVersion
					|| 7 == m_EngineVersion)
				{
					if (!ReadBoolean(m_hEnginePipe, m_EnabledFeatures.AfterRenderView)) { errorLine = __LINE__; goto error; }
				}
			}

			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CPrepareDrawFunctor(this, engineFrame)));

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
			if (!(m_EngineVersion == 7) || m_EnabledFeatures.AfterRenderView)
			{
				{
					if (!WriteUInt32(m_hEnginePipe, EngineMessage_OnRenderViewEnd)) { errorLine = __LINE__; goto error; }
					if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }
				}

				if (!(m_EngineVersion == 6 || m_EngineVersion == 7) || m_EnabledFeatures.AfterRenderView)
					QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new COnRenderViewEndFunctor(this)));
			}

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
			if (!m_EnabledFeatures.BeforeHud) return;

			int errorLine = 0;

			if(m_EngineVersion == 7)
			{
				if (!m_EngineConnected) return;

				if (!WriteUInt32(m_hEnginePipe, EngineMessage_OnBeforeHud)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }
			}

			QueueOrExecute(ctx->GetOrg(), new CAfxLeafExecute_Functor(new CBeforeHudFunctor(this)));

			return;

		error:
			Tier0_Warning("AfxInterop::OnBeforeHud: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void OnAfterHud(IAfxMatRenderContext* ctx)
		{
			if (!m_EnabledFeatures.AfterHud) return;

			int errorLine = 0;

			if (m_EngineVersion == 7)
			{
				if (!m_EngineConnected) return;

				if (!WriteUInt32(m_hEnginePipe, EngineMessage_OnAfterHud)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }
			}

			QueueOrExecute(ctx->GetOrg(), new CAfxLeafExecute_Functor(new CAfterHudFunctor(this)));

			return;

		error:
			Tier0_Warning("AfxInterop::OnAfterHud: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		void Shutdown() {
			DisconnectDrawing();
			DisconnectEngine();
		}

		void DrawingThreadPrepareDraw(int frameCount, IAfxInteropSurface * mainSurface)
		{
			m_DrawingSkip = true;
			m_DrawingFrameCount = frameCount;
			m_DrawingPass = 0;

			if (!m_DrawingConnected) return;

			if (6 == m_DrawingVersion || 7 == m_DrawingVersion)
			{
				m_DrawingSkip = false;
				return;
			}

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


				if (6 <= m_DrawingVersion && m_DrawingVersion <= 7)
				{
					if(!HandleDrawingMessage(message, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
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

			if (6 <= m_DrawingVersion && m_DrawingVersion <= 7)
			{
				if (!HandleDrawingMessage(DrawingMessage_BeforeHud, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
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

			if (6 <= m_DrawingVersion && m_DrawingVersion <= 7)
			{
				if (!HandleDrawingMessage(DrawingMessage_AfterHud, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
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

			int errorLine = 0;

			if (6 <= m_DrawingVersion && m_DrawingVersion <= 7)
			{
				if (!HandleDrawingMessage(DrawingMessage_OnRenderViewEnd, m_DrawingFrameCount)) { errorLine = __LINE__; goto error; }
			}
			else
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_OnRenderViewEnd)) { errorLine = __LINE__; goto error; }
			}

			return;

		error:
			Tier0_Warning("AfxInterop::DrawingThread_OnRenderViewEnd: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return;
		}

		bool Send_DrawingThread_DeviceLost_Message()
		{
			int errorLine = 0;

			if (m_DrawingVersion == 7)
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_DeviceLost)) { errorLine = __LINE__; goto error; }
			}

			return true;

		error:
			Tier0_Warning("AfxInterop::Send_DrawingThread_DeviceLost_Message: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		void DrawingThread_DeviceLost(bool disconnecting = false)
		{
			if (m_SharedSurface.Texture)
			{
				m_SharedSurface.Texture->Release();
				m_SharedSurface.Texture = nullptr;
			}

			if (disconnecting) for (auto it = m_D3d9VertexDeclarations.begin(); it != m_D3d9VertexDeclarations.end(); ++it) it->second->Release(); m_D3d9VertexDeclarations.clear();
			for (auto it = m_D3d9IndexBuffers.begin(); it != m_D3d9IndexBuffers.end(); ++it) it->second->Release(); m_D3d9IndexBuffers.clear();
			for (auto it = m_D3d9VertexBuffers.begin(); it != m_D3d9VertexBuffers.end(); ++it) it->second->Release(); m_D3d9VertexBuffers.clear();
			for (auto it = m_D3d9Textures.begin(); it != m_D3d9Textures.end(); ++it) it->second->Release(); m_D3d9Textures.clear();
			if (disconnecting) for (auto it = m_D3d9VertexShaders.begin(); it != m_D3d9VertexShaders.end(); ++it) it->second->Release(); m_D3d9VertexShaders.clear();
			if (disconnecting) for (auto it = m_D3d9PixelShaders.begin(); it != m_D3d9PixelShaders.end(); ++it) it->second->Release(); m_D3d9PixelShaders.clear();

			if (disconnecting || !m_DrawingConnected) return;

			int errorLine = 0;
			
			if (m_DrawingConnected)
			{
				if (!Send_DrawingThread_DeviceLost_Message()) { errorLine = __LINE__; goto error; }
			}

			return;

		error:
			Tier0_Warning("AfxInterop::DrawingThread_DeviceLost: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			if(!disconnecting) DisconnectDrawing();
			return;
		}

		bool HandleVersion7DrawingReply_D3d9CreateVertexDeclaration() {
			int errorLine = 0;
			void* pData = nullptr;
			D3DVERTEXELEMENT9 declEnd = D3DDECL_END();

			UINT64 index;
			UINT32 size;
			if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if(!ReadUInt32(m_hDrawingPipe, size)) { errorLine = __LINE__; goto error; }

			if(0 == (pData = malloc(size))) { errorLine = __LINE__; goto error; }

			if(!ReadBytes(m_hDrawingPipe, pData, 0, size)) { errorLine = __LINE__; goto error; }

			if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
			{
				IDirect3DVertexDeclaration9* pValue = nullptr;
				if (SUCCEEDED(device->CreateVertexDeclaration((D3DVERTEXELEMENT9 *)pData, &pValue)))
				{
					auto result = m_D3d9VertexDeclarations.emplace(index, pValue);
					if (!result.second) {
						result.first->second->Release();
						result.first->second = pValue;
					}
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			free(pData);

			return true;

		error:
			free(pData);
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_D3d9CreateVertexDeclaration: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return false;
		}

		bool HandleVersion7DrawingReply_D3d9CreateIndexBuffer() {
			int errorLine = 0;

			UINT64 index;
			UINT32 length;
			UINT32 usage;
			UINT32 format;
			UINT32 pool;
			bool hasHandle;
			HANDLE handle;

			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, length)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, usage)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, format)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, pool)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(m_hDrawingPipe, hasHandle)) { errorLine = __LINE__; goto error; }
			if (hasHandle && !ReadHandle(m_hDrawingPipe, handle)) { errorLine = __LINE__; goto error; }

			if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
			{
				IDirect3DIndexBuffer9* pValue = nullptr;
				if (SUCCEEDED(device->CreateIndexBuffer(length, usage, (D3DFORMAT)format, (D3DPOOL)pool, &pValue, hasHandle ? &handle : NULL)))
				{
					auto result = m_D3d9IndexBuffers.emplace(index,pValue);
					if (!result.second) {
						result.first->second->Release();
						result.first->second = pValue;
					}
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_D3d9CreateIndexBuffer: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		bool HandleVersion7DrawingReply_UpdateD3d9IndexBuffer() {
			int errorLine = 0;

			UINT64 index;
			UINT32 offsetToLock;
			UINT32 sizeToLock;

			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, offsetToLock)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, sizeToLock)) { errorLine = __LINE__; goto error; }
			
			auto it = m_D3d9IndexBuffers.find(index);
			if (it != m_D3d9IndexBuffers.end())
			{
				void* pData = nullptr;
				if (SUCCEEDED(it->second->Lock(offsetToLock, sizeToLock, &pData, 0)))
				{
					if (!ReadBytes(m_hDrawingPipe, pData, 0, sizeToLock)) { it->second->Unlock();  errorLine = __LINE__; goto error; }
					it->second->Unlock();
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }
			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_UpdateD3d9IndexBuffer: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		bool HandleVersion7DrawingReply_D3d9CreateVertexBuffer() {
			int errorLine = 0;

			UINT64 index;
			UINT32 length;
			UINT32 usage;
			UINT32 fvf;
			UINT32 pool;
			bool hasHandle;
			HANDLE handle;

			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, length)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, usage)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, fvf)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, pool)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(m_hDrawingPipe, hasHandle)) { errorLine = __LINE__; goto error; }
			if (hasHandle && !ReadHandle(m_hDrawingPipe, handle)) { errorLine = __LINE__; goto error; }

			if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
			{
				IDirect3DVertexBuffer9* pValue = nullptr;
				if (SUCCEEDED(device->CreateVertexBuffer(length, usage, fvf, (D3DPOOL)pool, &pValue, hasHandle ? &handle : NULL)))
				{
					auto result = m_D3d9VertexBuffers.emplace(index, pValue);
					if (!result.second) {
						result.first->second->Release();
						result.first->second = pValue;
					}
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_D3d9CreateVertexBuffer: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		bool HandleVersion7DrawingReply_UpdateD3d9VertexBuffer() {
			int errorLine = 0;
			UINT64 index;
			UINT32 offsetToLock;
			UINT32 sizeToLock;

			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, offsetToLock)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, sizeToLock)) { errorLine = __LINE__; goto error; }

			auto it = m_D3d9VertexBuffers.find(index);
			if (it != m_D3d9VertexBuffers.end())
			{
				void* pData = nullptr;
				if (SUCCEEDED(it->second->Lock(offsetToLock, sizeToLock, &pData, 0)))
				{
					if (!ReadBytes(m_hDrawingPipe, pData, 0, sizeToLock)) { it->second->Unlock(); errorLine = __LINE__; goto error; }
					it->second->Unlock();
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_UpdateD3d9VertexBuffer: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		/////

		bool HandleVersion7DrawingReply_D3d9CreateTexture() {
			int errorLine = 0;

			UINT64 index;
			UINT32 width;
			UINT32 height;
			UINT32 levels;
			UINT32 usage;
			UINT32 format;
			UINT32 pool;
			bool hasHandle;
			HANDLE handle;

			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, width)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, height)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, levels)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, usage)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, format)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, pool)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(m_hDrawingPipe, hasHandle)) { errorLine = __LINE__; goto error; }
			if (hasHandle && !ReadHandle(m_hDrawingPipe, handle)) { errorLine = __LINE__; goto error; }

			if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
			{
				IDirect3DTexture9* pValue = nullptr;
				if (SUCCEEDED(device->CreateTexture(width, height, levels, usage, (D3DFORMAT)format, (D3DPOOL)pool, &pValue, hasHandle ? &handle : NULL)))
				{
					auto result = m_D3d9Textures.emplace(index,pValue);
					if (!result.second) {
						result.first->second->Release();
						result.first->second = pValue;
					}
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_D3d9CreateTexture: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		bool HandleVersion7DrawingReply_UpdateD3d9Texture() {
			int errorLine = 0;

			UINT64 index;
			UINT32 level;
			bool hasRect;
			RECT rect;
			UINT32 rowCount;
			UINT32 rowBytes;

			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, level)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(m_hDrawingPipe, hasRect)) { errorLine = __LINE__; goto error; }
			if (hasRect)
			{
				INT32 value;
				if (!ReadInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }
				rect.left = value;
				if (!ReadInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }
				rect.top = value;
				if (!ReadInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }
				rect.right = value;
				if (!ReadInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }
				rect.bottom = value;
			}
			if (!ReadUInt32(m_hDrawingPipe, rowCount)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, rowBytes)) { errorLine = __LINE__; goto error; }


			auto it = m_D3d9Textures.find(index);
			if (it != m_D3d9Textures.end())
			{
				D3DLOCKED_RECT lockedRect;
				if (SUCCEEDED(it->second->LockRect(level, &lockedRect, hasRect ? &rect : NULL, 0)))
				{
					void* pData = lockedRect.pBits;
					for (UINT32 i = 0; i < rowCount && 0 == errorLine; ++i)
					{
						if (!ReadBytes(m_hDrawingPipe, pData, 0, rowBytes)) { it->second->UnlockRect(level); errorLine = __LINE__; goto error; }
						pData = (unsigned char*)pData + lockedRect.Pitch;
					}
					it->second->UnlockRect(level);
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_UpdateD3d9Texture: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		bool HandleVersion7DrawingReply_D3d9CreateVertexShader() {
			int errorLine = 0;
			void* pData = nullptr;

			UINT64 index;
			UINT32 size;
			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, size)) { errorLine = __LINE__; goto error; }

			if (0 == (pData = malloc(size))) { errorLine = __LINE__; goto error; }

			if (!ReadBytes(m_hDrawingPipe, pData, 0, size)) { errorLine = __LINE__; goto error; }

			if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
			{
				IDirect3DVertexShader9* pValue = nullptr;
				if (SUCCEEDED(device->CreateVertexShader((DWORD*)pData, &pValue)))
				{
					auto result = m_D3d9VertexShaders.emplace(index, pValue);
					if (!result.second) {
						result.first->second->Release();
						result.first->second = pValue;
					}
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			free(pData);

			return true;

		error:
			free(pData);
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_D3d9CreateVertexShader: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return false;
		}


		bool HandleVersion7DrawingReply_D3d9CreatePixelShader() {
			int errorLine = 0;
			void* pData = nullptr;

			UINT64 index;
			UINT32 size;
			if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
			if (!ReadUInt32(m_hDrawingPipe, size)) { errorLine = __LINE__; goto error; }

			if (0 == (pData = malloc(size))) { errorLine = __LINE__; goto error; }

			if (!ReadBytes(m_hDrawingPipe, pData, 0, size)) { errorLine = __LINE__; goto error; }

			if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
			{
				IDirect3DPixelShader9* pValue = nullptr;
				if (SUCCEEDED(device->CreatePixelShader((DWORD*)pData, &pValue)))
				{
					auto result = m_D3d9PixelShaders.emplace(index, pValue);
					if (!result.second) {
						result.first->second->Release();
						result.first->second = pValue;
					}
				}
				else { errorLine = __LINE__; goto error; }
			}
			else { errorLine = __LINE__; goto error; }

			free(pData);

			return true;

		error:
			free(pData);
			Tier0_Warning("AfxInterop::HandleVersion7DrawingReply_D3d9CreatePixelShader: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
			return false;
		}

		bool Send_DrawingThread_DeviceRestored_Message()
		{
			int errorLine = 0;

			if (m_DrawingVersion == 7)
			{
				if (!WriteInt32(m_hDrawingPipe, DrawingMessage_DeviceRestored)) { errorLine = __LINE__; goto error; }
			}

			return true;

		error:
			Tier0_Warning("AfxInterop::Send_DrawingThread_DeviceRestored_Message: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			return false;
		}

		void DrawingThread_DeviceRestored()
		{
			int errorLine = 0;

			if (m_DrawingConnected)
			{				
				if (!Send_DrawingThread_DeviceRestored_Message()) { errorLine = __LINE__; goto error; }
			}

			return;

		error:
			Tier0_Warning("AfxInterop::Send_DrawingThread_DeviceRestored_Message: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
			DisconnectDrawing();
		}

		bool ConnectDrawing() {
			{
				std::shared_lock<std::shared_timed_mutex> sharedLock(m_DrawingConnectMutex);

				if (!m_DrawingWantsConnect) {
					if (m_DrawingPreConnected || m_DrawingConnected)
					{
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

				if (m_DrawingWantsConnect)
				{
					if (!m_DrawingPreConnected && !m_DrawingConnected)
					{
						std::string strPipeName("\\\\.\\pipe\\");
						strPipeName.append(m_DrawingPipeName);

						while (true)
						{
							m_hDrawingPipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
							if (m_hDrawingPipe != INVALID_HANDLE_VALUE)
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
						m_DrawingPreConnected = true;

						return true;
					}
					else if (m_EnginePreConnected)
					{
						m_DrawingConnected = true;
						m_DrawingPreConnected = false;
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

			DrawingThread_DeviceLost(true);

			if (INVALID_HANDLE_VALUE != m_hDrawingPipe)
			{
				if (!CloseHandle(m_hDrawingPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i (%s).\n", __LINE__, m_DrawingPipeName.c_str());
				}
				m_hDrawingPipe = INVALID_HANDLE_VALUE;
			}

			m_DrawingConnected = false;
			m_DrawingWantsConnect = false;
			m_DrawingPreConnected = false;

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
					if (m_EnginePreConnected || m_EngineConnected)
					{
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


				if (m_EngineWantsConnect)
				{
					if (!m_EnginePreConnected)
					{
						if (!m_DrawingPreConnected && !m_DrawingConnected) // Don't connect while still connected in drawing.
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

							m_DrawingWantsConnect = true;
							m_DrawingVersion = m_EngineVersion;
							m_EnginePreConnected = true;
							return true;
						}
					}
					else if (m_DrawingConnected) {
						m_EngineConnected = true;
						m_EnginePreConnected = false;

						if (!ReadInt32(m_hEnginePipe, m_EngineVersion)) { errorLine = __LINE__; goto error; }

						switch (m_EngineVersion)
						{
						case 5:
						case 6:
						case 7:
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

						if (7 == m_EngineVersion)
						{
							if (!ReadGameEventSettings(false)) { errorLine = __LINE__; goto error; }
						}

						return true;
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
			std::unique_lock<std::shared_timed_mutex> engineLock(m_EngineConnectMutex);
			std::unique_lock<std::shared_timed_mutex> drwaingLockLock(m_DrawingConnectMutex);

			if (INVALID_HANDLE_VALUE != m_hEnginePipe)
			{
				if (!CloseHandle(m_hEnginePipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i (%s).\n", __LINE__, m_EnginePipeName.c_str());
				}
				m_hEnginePipe = INVALID_HANDLE_VALUE;
			}

			m_DrawingWantsConnect = false;
			m_EngineConnected = false;
			m_EnginePreConnected = false;

			g_AfxGameEvents.RemoveListener(this);
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

		virtual bool CAfxGameEventListenerSerialzer::BeginSerialize() override
		{
			if (!m_EngineConnected)
				return false;

			int errorLine = 0;
			
			if (!WriteUInt32(m_hEnginePipe, EngineMessage_OnGameEvent)) { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::BeginSerialize: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return false;
		}

		virtual void CAfxGameEventListenerSerialzer::EndSerialize() override
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if (!Flush(m_hEnginePipe)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::EndSerialize: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteCString(const char* value) override
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteCString(m_hEnginePipe, value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteCString: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteFloat(float value) override
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteSingle(m_hEnginePipe, value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteFloat: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteLong(long value) override
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteInt32(m_hEnginePipe, value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteLong: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteShort(short value) override
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteInt16(m_hEnginePipe, value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteShort: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteByte(char value)
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteByte(m_hEnginePipe, (BYTE)value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteByte: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteBoolean(bool value)
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteBoolean(m_hEnginePipe, value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteBoolean: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
		}

		virtual void CAfxGameEventListenerSerialzer::WriteUInt64(unsigned __int64 value)
		{
			if (!m_EngineConnected)
				return;

			int errorLine = 0;

			if(!WriteUInt64(m_hEnginePipe, value)) { errorLine = __LINE__; goto error; }

			return;

		error:
			Tier0_Warning("AfxInterop::CAfxGameEventListenerSerialzer::WriteUInt64: Error in line %i (%s).\n", errorLine, m_EnginePipeName.c_str());
			DisconnectEngine();
			return;
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
			DrawingMessage_OnRenderViewEnd = 8,

			DrawingMessage_DeviceLost = 9,
			DrawingMessage_DeviceRestored = 10
		};

		enum PrepareDrawReply
		{
			PrepareDrawReply_Skip = 1,
			PrepareDrawReply_Retry = 2,
			PrepareDrawReply_Continue = 3
		};

		enum class DrawingReply : unsigned int
		{
			Skip = 1,
			Retry = 2,
			Continue = 3,
			Finished = 4,

			D3d9CreateVertexDeclaration = 5,
			ReleaseD3d9VertexDeclaration = 6,

			D3d9CreateIndexBuffer = 7,
			ReleaseD3d9IndexBuffer = 8,
			UpdateD3d9IndexBuffer = 9,

			D3d9CreateVertexBuffer = 10,
			ReleaseD3d9VertexBuffer = 11,
			UpdateD3d9VertexBuffer = 12,

			D3d9CreateTexture = 13,
			ReleaseD3d9Texture = 14,
			UpdateD3d9Texture = 15,

			D3d9CreateVertexShader = 16,
			ReleaseD3d9VertexShader = 17,

			D3d9CreatePixelShader = 18,
			ReleaseD3d9PixelShader = 19,

			D3d9SetViewport = 20,
			D3d9SetRenderState = 21,
			D3d9SetSamplerState = 22,
			D3d9SetTexture = 23,
			D3d9SetTextureStageState = 24,
			D3d9SetTransform = 25,
			D3d9SetIndices = 26,
			D3d9SetStreamSource = 27,
			D3d9SetStreamSourceFreq = 28,
			D3d9SetVertexDeclaration = 29,
			D3d9SetVertexShader = 30,
			D3d9SetVertexShaderConstantF = 31,
			D3d9SetVertexShaderConstantI = 32,
			D3d9SetVertexShaderConstantB = 33,
			D3d9SetPixelShader = 34,
			D3d9SetPixelShaderConstantB = 35,
			D3d9SetPixelShaderConstantF = 36,
			D3d9SetPixelShaderConstantI = 37,
			D3d9DrawPrimitive = 38,
			D3d9DrawIndexedPrimitive = 39,

			WaitForGpu = 40,
			BeginCleanState = 41,
			EndCleanState = 42,

			D3d9UpdateTexture = 43
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
		bool m_DrawingConnected = false;
		bool m_DrawingPreConnected = false;
		int m_DrawingPass = 0;

		HANDLE m_hDrawingPipe = INVALID_HANDLE_VALUE;		

		bool m_DrawingSkip = true;
		int m_DrawingFrameCount = -1;

		std::map<UINT64, IDirect3DVertexDeclaration9*> m_D3d9VertexDeclarations;
		std::map<UINT64, IDirect3DIndexBuffer9*> m_D3d9IndexBuffers;
		std::map<UINT64, IDirect3DVertexBuffer9*> m_D3d9VertexBuffers;
		std::map<UINT64, IDirect3DTexture9*> m_D3d9Textures;
		std::map<UINT64, IDirect3DVertexShader9*> m_D3d9VertexShaders;
		std::map<UINT64, IDirect3DPixelShader9*> m_D3d9PixelShaders;

		IAfxInteropSurface* m_DrawingMainSurface = nullptr;

		struct SharedSurfacesValue
		{
			HANDLE Handle = nullptr;
			IDirect3DTexture9* Texture = nullptr;
		};

		SharedSurfacesValue m_SharedSurface;

		bool HandleDrawingMessage(DrawingMessage message, int frameCount)
		{
			int errorLine = 0;
			bool bInSafeState = false;

			while (true)
			{
				if (!WriteInt32(m_hDrawingPipe, message)) { errorLine = __LINE__; goto error; }
				if (!WriteInt32(m_hDrawingPipe, frameCount)) { errorLine = __LINE__; goto error; }
				if (7 == m_DrawingVersion) if (!WriteUInt32(m_hDrawingPipe, m_DrawingPass)) { errorLine = __LINE__; goto error; }
				if (!Flush(m_hDrawingPipe)) { errorLine = __LINE__; goto error; }

				bool loopReply = true;

				while(loopReply)
				{
					INT32 drawingReply;
					if (!ReadInt32(m_hDrawingPipe, drawingReply)) { errorLine = __LINE__; goto error; }

					switch ((DrawingReply)drawingReply)
					{
					case DrawingReply::Skip:
						m_DrawingSkip = true;
						return true;
					case DrawingReply::Retry:
						loopReply = false;
						break;
					case DrawingReply::Continue:
						loopReply = false;
							{
							HANDLE sharedTextureHandle;
							UINT32 texWidth;
							UINT32 texHeight;
							UINT32 d3dFormat;

							if (!ReadHandle(m_hDrawingPipe, sharedTextureHandle)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, texWidth)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, texHeight)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, d3dFormat)) { errorLine = __LINE__; goto error; }

							if (m_SharedSurface.Handle && m_SharedSurface.Handle != sharedTextureHandle)
							{
								if (m_SharedSurface.Texture) {
									m_SharedSurface.Texture->Release();
									m_SharedSurface.Texture = nullptr;
								}
							}

							if (sharedTextureHandle && nullptr == m_SharedSurface.Texture)
							{
								if (IDirect3DDevice9Ex* device = AfxGetDirect3DDevice9Ex())
								{
									IDirect3DTexture9* Texture = NULL;

									HRESULT hr = device->CreateTexture(texWidth, texHeight, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)d3dFormat, D3DPOOL_DEFAULT, &Texture, &sharedTextureHandle);

									if (SUCCEEDED(hr))
									{
										m_SharedSurface.Handle = sharedTextureHandle;
										m_SharedSurface.Texture = Texture;
									}
								}
							}

							if (m_SharedSurface.Handle && texWidth && texHeight)
							{
								AfxDrawRect(m_SharedSurface.Texture, 0, 0, texWidth, texHeight, 0, 0, 1, 1);
								AfxD3D_WaitForGPU(); // TODO: Improve. We are slowing down more than we have to.
							}
					
							if (!WriteBoolean(m_hDrawingPipe, true)) { errorLine = __LINE__; goto error; }
						}
						return true;

					case DrawingReply::Finished:
						if (bInSafeState)
						{
							AfxD3D9EndCleanState();
						}
						++m_DrawingPass;
						return true;

					case DrawingReply::D3d9CreateVertexDeclaration:
						if (!HandleVersion7DrawingReply_D3d9CreateVertexDeclaration()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::ReleaseD3d9VertexDeclaration:
						{
							UINT64 index;
							if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							auto it = m_D3d9VertexDeclarations.find(index);
							if (it != m_D3d9VertexDeclarations.end()) {
								it->second->Release();
								m_D3d9VertexDeclarations.erase(it);
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9CreateIndexBuffer:
						if (!HandleVersion7DrawingReply_D3d9CreateIndexBuffer()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::UpdateD3d9IndexBuffer:
						if (!HandleVersion7DrawingReply_UpdateD3d9IndexBuffer()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::ReleaseD3d9IndexBuffer:
						{
							UINT64 index;
							if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							auto it = m_D3d9IndexBuffers.find(index);
							if (it != m_D3d9IndexBuffers.end()) {
								it->second->Release();
								m_D3d9IndexBuffers.erase(it);
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9CreateVertexBuffer:
						if (!HandleVersion7DrawingReply_D3d9CreateVertexBuffer()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::UpdateD3d9VertexBuffer:
						if (!HandleVersion7DrawingReply_UpdateD3d9VertexBuffer()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::ReleaseD3d9VertexBuffer:
						{
							UINT64 index;
							if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							auto it = m_D3d9VertexBuffers.find(index);
							if (it != m_D3d9VertexBuffers.end()) {
								it->second->Release();
								m_D3d9VertexBuffers.erase(it);
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9CreateTexture:
						if (!HandleVersion7DrawingReply_D3d9CreateTexture()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::UpdateD3d9Texture:
						if (!HandleVersion7DrawingReply_UpdateD3d9Texture()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::ReleaseD3d9Texture:
						{
							UINT64 index;
							if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							auto it = m_D3d9Textures.find(index);
							if (it != m_D3d9Textures.end()) {
								it->second->Release();
								m_D3d9Textures.erase(it);
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9CreateVertexShader:
						if (!HandleVersion7DrawingReply_D3d9CreateVertexShader()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::ReleaseD3d9VertexShader:
						{
							UINT64 index;
							if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							auto it = m_D3d9VertexShaders.find(index);
							if (it != m_D3d9VertexShaders.end()) {
								it->second->Release();
								m_D3d9VertexShaders.erase(it);
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9CreatePixelShader:
						if (!HandleVersion7DrawingReply_D3d9CreatePixelShader()) { errorLine = __LINE__; goto error; }
						break;
					case DrawingReply::ReleaseD3d9PixelShader:
						{
							UINT64 index;
							if(!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							auto it = m_D3d9PixelShaders.find(index);
							if (it != m_D3d9PixelShaders.end()) {
								it->second->Release();
								m_D3d9PixelShaders.erase(it);
							}
							else { errorLine = __LINE__; goto error; }
						} break;

					case DrawingReply::D3d9SetViewport:
						{
							bool hasViewPort;
							UINT32 x;
							UINT32 y;
							UINT32 width;
							UINT32 height;
							float minZ;
							float maxZ;


							if (!ReadBoolean(m_hDrawingPipe, hasViewPort)) { errorLine = __LINE__; goto error; }

							if (hasViewPort)
							{
								if (!ReadUInt32(m_hDrawingPipe, x)) { errorLine = __LINE__; goto error; }
								if (!ReadUInt32(m_hDrawingPipe, y)) { errorLine = __LINE__; goto error; }
								if (!ReadUInt32(m_hDrawingPipe, width)) { errorLine = __LINE__; goto error; }
								if (!ReadUInt32(m_hDrawingPipe, height)) { errorLine = __LINE__; goto error; }
								if (!ReadSingle(m_hDrawingPipe, minZ)) { errorLine = __LINE__; goto error; }
								if (!ReadSingle(m_hDrawingPipe, maxZ)) { errorLine = __LINE__; goto error; }
							}

							D3DVIEWPORT9 viewPort = { x,y,width,height,minZ,maxZ };

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if(FAILED(device->SetViewport(hasViewPort ? &viewPort : NULL))) { errorLine = __LINE__; goto error; }
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetRenderState:
						{
							UINT32 state;
							UINT32 value;

							if (!ReadUInt32(m_hDrawingPipe, state)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if(FAILED(device->SetRenderState((D3DRENDERSTATETYPE)state, (DWORD)value))) { errorLine = __LINE__; goto error; }
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetSamplerState:
						{
							UINT32 sampler;
							UINT32 type;
							UINT32 value;

							if (!ReadUInt32(m_hDrawingPipe, sampler)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, type)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if(FAILED(device->SetSamplerState((DWORD)sampler, (D3DSAMPLERSTATETYPE)type, (DWORD)value))) { errorLine = __LINE__; goto error; }
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetTexture:
						{
							UINT32 stage;
							UINT64 index;

							if (!ReadUInt32(m_hDrawingPipe, stage)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
							{
								if (0 == index)
								{
									if(FAILED(device->SetTexture((DWORD)stage, NULL))) { errorLine = __LINE__; goto error; }
								}
								else
								{
									auto it = m_D3d9Textures.find(index);
									if (it == m_D3d9Textures.end()) { errorLine = __LINE__; goto error; }
									if (FAILED(device->SetTexture((DWORD)stage, it->second))) { errorLine = __LINE__; goto error; }
								}

							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetTextureStageState:
						{
							UINT32 stage;
							UINT32 type;
							UINT32 value;

							if (!ReadUInt32(m_hDrawingPipe, stage)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, type)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, value)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if(FAILED(device->SetTextureStageState((DWORD)stage, (D3DTEXTURESTAGESTATETYPE)type, (DWORD)value))) { errorLine = __LINE__; goto error; }
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetTransform:
						{
							UINT32 state;
							bool hasMatrix;
							D3DMATRIX matrix;

							if (!ReadUInt32(m_hDrawingPipe, state)) { errorLine = __LINE__; goto error; }
							if (!ReadBoolean(m_hDrawingPipe, hasMatrix)) { errorLine = __LINE__; goto error; }
							if (hasMatrix)
							{
								if (!ReadBytes(m_hDrawingPipe, &matrix, 0, sizeof(D3DMATRIX))) { errorLine = __LINE__; goto error; }
							}

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if(FAILED(device->SetTransform((D3DTRANSFORMSTATETYPE)state, hasMatrix ? &matrix : NULL))) { errorLine = __LINE__; goto error; }
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetIndices:
						{
							UINT64 index;

							if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
							{

								if (0 == index)
								{
									if(FAILED(device->SetIndices(NULL))) { errorLine = __LINE__; goto error; }
								}
								else
								{
									auto it = m_D3d9IndexBuffers.find(index);
									if(it == m_D3d9IndexBuffers.end()) { errorLine = __LINE__; goto error; }
									if (FAILED(device->SetIndices(it->second))) { errorLine = __LINE__; goto error; }
								}
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9SetStreamSource:
						{
							UINT32 streamNumber;
							UINT64 index;
							UINT32 offsetInBytes;
							UINT32 stride;

							if (!ReadUInt32(m_hDrawingPipe, streamNumber)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, offsetInBytes)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, stride)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
							{
								if (0 == index)
								{
									if (FAILED(device->SetStreamSource(streamNumber, NULL, offsetInBytes, stride))) { errorLine = __LINE__; goto error; }
								}
								else
								{
									auto it = m_D3d9VertexBuffers.find(index);
									if (it == m_D3d9VertexBuffers.end()) { errorLine = __LINE__; goto error; }
									if(FAILED(device->SetStreamSource(streamNumber, it->second, offsetInBytes, stride))) { errorLine = __LINE__; goto error; }
								}
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9SetStreamSourceFreq:
						{
							UINT32 streamNumber;
							UINT32 setting;

							if (!ReadUInt32(m_hDrawingPipe, streamNumber)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, setting)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if(FAILED(device->SetStreamSourceFreq(streamNumber, setting))) { errorLine = __LINE__; goto error; }
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetVertexDeclaration:
						{
							UINT64 index;

							if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
							{
								if (index == 0)
								{
									if (FAILED(device->SetVertexDeclaration(NULL))) { errorLine = __LINE__; goto error; }
								}
								else
								{
									auto it = m_D3d9VertexDeclarations.find(index);
									if (it == m_D3d9VertexDeclarations.end()) { errorLine = __LINE__; goto error; }
									if(FAILED(device->SetVertexDeclaration(it->second))) { errorLine = __LINE__; goto error; }
								}
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetVertexShader:
						{
							UINT64 index;

							if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {

								if (0 == index)
								{
									if (FAILED(device->SetVertexShader(NULL))) { errorLine = __LINE__; goto error; }
								}
								else
								{
									auto it = m_D3d9VertexShaders.find(index);
									if (it == m_D3d9VertexShaders.end()) { errorLine = __LINE__; goto error; }
									if(FAILED(device->SetVertexShader(it->second))) { errorLine = __LINE__; goto error; }
								}
							}
							else { errorLine = __LINE__; goto error; }

						} break;
					case DrawingReply::D3d9SetVertexShaderConstantB:
						{
							UINT32 startRegister;
							UINT32 count;

							if (!ReadUInt32(m_hDrawingPipe, startRegister)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, count)) { errorLine = __LINE__; goto error; }

							BOOL* pData = (BOOL *)malloc(sizeof(BOOL) * count);

							for (UINT32 i = 0; i < count && 0 == errorLine; ++i)
							{
								bool bValue;
								if (!ReadBoolean(m_hDrawingPipe, bValue)) errorLine = __LINE__;
								else pData[i] = bValue ? 1 : 0;
							}
							
							if (0 == errorLine)
							{
								if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
								{
									if(FAILED(device->SetVertexShaderConstantB(startRegister, pData, count))) errorLine = __LINE__;
								}
								else errorLine = __LINE__;
							}

							free(pData);

							if (errorLine) goto error;

						} break;
					case DrawingReply::D3d9SetVertexShaderConstantF:
						{
							UINT32 startRegister;
							UINT32 count4;

							if (!ReadUInt32(m_hDrawingPipe, startRegister)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, count4)) { errorLine = __LINE__; goto error; }

							float* pData = (FLOAT *)malloc(sizeof(float) * 4 * count4);

							for (UINT32 i = 0; i < 4 * count4 && 0 == errorLine; ++i)
							{
								float value;
								if (!ReadSingle(m_hDrawingPipe, value)) errorLine = __LINE__;
								else pData[i] = value;
							}
							
							if (0 == errorLine)
							{
								if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
								{
									if (FAILED(device->SetVertexShaderConstantF(startRegister, pData, count4))) errorLine = __LINE__;
								}
								else errorLine = __LINE__;
							}

							free(pData);

							if (errorLine) goto error;

						} break;
					case DrawingReply::D3d9SetVertexShaderConstantI:
						{
							UINT32 startRegister;
							UINT32 count4;

							if (!ReadUInt32(m_hDrawingPipe, startRegister)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, count4)) { errorLine = __LINE__; goto error; }

							int* pData = (int*)malloc(sizeof(int) * 4 * count4);

							for (UINT32 i = 0; i < 4 * count4 && 0 == errorLine; ++i)
							{
								int value;
								if (!ReadInt32(m_hDrawingPipe, value)) errorLine = __LINE__;
								else pData[i] = value;
							}

							if (0 == errorLine)
							{
								if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
									if (FAILED(device->SetVertexShaderConstantI(startRegister, pData, count4))) errorLine = __LINE__;
								}
								else errorLine = __LINE__;
							}

							free(pData);

						} break;
					case DrawingReply::D3d9SetPixelShader:
					{
						UINT64 index;

						if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }

						if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {

							if (0 == index)
							{
								if (FAILED(device->SetPixelShader(NULL))) { errorLine = __LINE__; goto error; }
							}
							else
							{
								auto it = m_D3d9PixelShaders.find(index);
								if (it == m_D3d9PixelShaders.end()) { errorLine = __LINE__; goto error; }
								if (FAILED(device->SetPixelShader(it->second))) { errorLine = __LINE__; goto error; }
							}
						}
						else { errorLine = __LINE__; goto error; }

					} break;
					case DrawingReply::D3d9SetPixelShaderConstantB:
					{
						UINT32 startRegister;
						UINT32 count;

						if (!ReadUInt32(m_hDrawingPipe, startRegister)) { errorLine = __LINE__; goto error; }
						if (!ReadUInt32(m_hDrawingPipe, count)) { errorLine = __LINE__; goto error; }

						BOOL* pData = (BOOL*)malloc(sizeof(BOOL) * count);

						for (UINT32 i = 0; i < count && 0 == errorLine; ++i)
						{
							bool bValue;
							if (!ReadBoolean(m_hDrawingPipe, bValue)) errorLine = __LINE__;
							else pData[i] = bValue ? 1 : 0;
						}

						if (0 == errorLine)
						{
							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
							{
								if (FAILED(device->SetPixelShaderConstantB(startRegister, pData, count))) errorLine = __LINE__;
							}
							else errorLine = __LINE__;
						}

						free(pData);

						if (errorLine) goto error;

					} break;
					case DrawingReply::D3d9SetPixelShaderConstantF:
					{
						UINT32 startRegister;
						UINT32 count4;

						if (!ReadUInt32(m_hDrawingPipe, startRegister)) { errorLine = __LINE__; goto error; }
						if (!ReadUInt32(m_hDrawingPipe, count4)) { errorLine = __LINE__; goto error; }

						float* pData = (FLOAT*)malloc(sizeof(float) * 4 * count4);

						for (UINT32 i = 0; i < 4 * count4 && 0 == errorLine; ++i)
						{
							float value;
							if (!ReadSingle(m_hDrawingPipe, value)) errorLine = __LINE__;
							else pData[i] = value;
						}

						if (0 == errorLine)
						{
							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9())
							{
								if (FAILED(device->SetPixelShaderConstantF(startRegister, pData, count4))) errorLine = __LINE__;
							}
							else errorLine = __LINE__;
						}

						free(pData);

						if (errorLine) goto error;

					} break;
					case DrawingReply::D3d9SetPixelShaderConstantI:
					{
						UINT32 startRegister;
						UINT32 count4;

						if (!ReadUInt32(m_hDrawingPipe, startRegister)) { errorLine = __LINE__; goto error; }
						if (!ReadUInt32(m_hDrawingPipe, count4)) { errorLine = __LINE__; goto error; }

						int* pData = (int*)malloc(sizeof(int) * 4 * count4);

						for (UINT32 i = 0; i < 4 * count4 && 0 == errorLine; ++i)
						{
							int value;
							if (!ReadInt32(m_hDrawingPipe, value)) errorLine = __LINE__;
							else pData[i] = value;
						}

						if (0 == errorLine)
						{
							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if (FAILED(device->SetPixelShaderConstantI(startRegister, pData, count4))) errorLine = __LINE__;
							}
							else errorLine = __LINE__;
						}

						free(pData);

					} break;
					case DrawingReply::D3d9DrawPrimitive:
						{					
							UINT32 primitiveType;
							UINT32 startVertex;
							UINT32 primitiveCount;

							if (!ReadUInt32(m_hDrawingPipe, primitiveType)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, startVertex)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, primitiveCount)) { errorLine = __LINE__; goto error; }


							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if (0 == errorLine)
								{
									if(FAILED(device->DrawPrimitive((D3DPRIMITIVETYPE)primitiveType, startVertex, primitiveCount))) { errorLine = __LINE__; goto error; }
								}
							}
							else { errorLine = __LINE__; goto error; }
						} break;
					case DrawingReply::D3d9DrawIndexedPrimitive:
						{					
							UINT32 primitiveType;
							INT32 baseVertexIndex;
							UINT32 minVertexIndex;
							UINT32 numVertices;
							UINT32 startIndex;
							UINT32 primCount;

							if (!ReadUInt32(m_hDrawingPipe, primitiveType)) { errorLine = __LINE__; goto error; }
							if (!ReadInt32(m_hDrawingPipe, baseVertexIndex)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, minVertexIndex)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, numVertices)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, startIndex)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt32(m_hDrawingPipe, primCount)) { errorLine = __LINE__; goto error; }


							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {
								if (0 == errorLine)
								{
									if(FAILED(device->DrawIndexedPrimitive((D3DPRIMITIVETYPE)primitiveType, baseVertexIndex, minVertexIndex, numVertices, startIndex, primCount))) { errorLine = __LINE__; goto error; }
								}
							}
							else { errorLine = __LINE__; goto error; }
						} break;

					case DrawingReply::WaitForGpu:
						AfxD3D_WaitForGPU();
						if (!WriteBoolean(m_hDrawingPipe, true)) { errorLine = __LINE__; goto error; }
						break;

					case DrawingReply::BeginCleanState:
						if (!bInSafeState) {
							AfxD3D9BeginCleanState();
							bInSafeState = true;
						}
						break;

					case DrawingReply::EndCleanState:
						if (bInSafeState)
						{
							AfxD3D9EndCleanState();
							bInSafeState = false;
						}
						break;

					case DrawingReply::D3d9UpdateTexture:
						{
							UINT32 stage;
							UINT64 index;
							UINT64 index2;

							if (!ReadUInt32(m_hDrawingPipe, stage)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt64(m_hDrawingPipe, index)) { errorLine = __LINE__; goto error; }
							if (!ReadUInt64(m_hDrawingPipe, index2)) { errorLine = __LINE__; goto error; }

							if (IDirect3DDevice9* device = AfxGetDirect3DDevice9()) {

								IDirect3DTexture9* tex = nullptr;
								IDirect3DTexture9* tex2 = nullptr;

								if (0 != index)
								{
									auto it = m_D3d9Textures.find(index);
									if(it == m_D3d9Textures.end()) { errorLine = __LINE__; goto error; }
									tex = it->second;
								}
								if (0 != index2)
								{
									auto it = m_D3d9Textures.find(index2);
									if (it == m_D3d9Textures.end()) { errorLine = __LINE__; goto error; }
									tex2 = it->second;
								}

								device->UpdateTexture(tex, tex2);
							}

						} break;

					default:
						{ errorLine = __LINE__; goto error; }
					}
				}
			}

		error:
			if (bInSafeState)
			{
				AfxD3D9EndCleanState();
			}
			Tier0_Warning("AfxInterop::HandleDrawingMessage: Error in line %i (%s).\n", errorLine, m_DrawingPipeName.c_str());
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
			EngineMessage_AfterTranslucent = 12,
			EngineMessage_OnBeforeHud = 13,
			EngineMessage_OnAfterHud = 14,
			EngineMessage_OnGameEvent = 15
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
		bool m_EngineConnected = false;
		bool m_EnginePreConnected = false;

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

			while (0 < numBytes)
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

			}

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

		bool ReadUInt64(HANDLE hFile, UINT64& outValue)
		{
			return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
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

		bool WriteInt16(HANDLE hFile, INT16 value) {
			return WriteBytes(hFile, &value, 0, sizeof(value));
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

		bool WriteUInt64(HANDLE hFile, uint64_t value) {
			return WriteBytes(hFile, &value, 0, sizeof(value));
		}

		bool WriteSingle(HANDLE hFile, FLOAT value)
		{
			return WriteBytes(hFile, &value, 0, sizeof(value));
		}

		bool WriteCString(HANDLE hFile, const char* value)
		{
			UINT32 length = (UINT32)(strlen(value));

			return WriteCompressedUInt32(hFile, length)
				&& WriteBytes(hFile, (LPVOID)value, 0, length);
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

		bool ReadGameEventSettings(bool delta)
		{
			bool bEnable;

			if (!ReadBoolean(m_hEnginePipe, bEnable)) return false;

			if (!bEnable)
			{
				g_AfxGameEvents.RemoveListener(this);
				return true;
			}
			
			g_AfxGameEvents.AddListener(this);
				
			if (!delta)
			{
				CAfxGameEventListenerSerialzer::Restart();
			}

			if (delta)
			{
				bool bChanged;

				// Read if any changes:
				if (!ReadBoolean(m_hEnginePipe, bChanged)) return false;

				if (!bChanged) return true;
			}

			{
				bool bValue;

				if (!ReadBoolean(m_hEnginePipe, bValue)) return false;
				TransmitClientTime = bValue;

				if (!ReadBoolean(m_hEnginePipe, bValue)) return false;
				TransmitTick = bValue;

				if (!ReadBoolean(m_hEnginePipe, bValue)) return false;
				TransmitSystemTime = bValue;
			}

			std::string eventName;
			unsigned int listSize;

			if (delta)
			{
				// Read whitelist removals:

				if (!ReadCompressedUInt32(m_hEnginePipe, listSize)) return false;

				for (unsigned int i = 0; i < listSize; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, eventName)) return false;
					CAfxGameEventListenerSerialzer::UnWhiteList(eventName.c_str());
				}
			}

			// Read whitelist additions:

			if (!ReadCompressedUInt32(m_hEnginePipe, listSize)) return false;

			for(unsigned int i = 0; i < listSize; ++i)
			{
				if (!ReadStringUTF8(m_hEnginePipe, eventName)) return false;
				CAfxGameEventListenerSerialzer::WhiteList(eventName.c_str());
			}

			if (delta)
			{
				// Read blacklist removals:

				if (!ReadCompressedUInt32(m_hEnginePipe, listSize)) return false;

				for (unsigned int i = 0; i < listSize; ++i)
				{
					if (!ReadStringUTF8(m_hEnginePipe, eventName)) return false;
					CAfxGameEventListenerSerialzer::UnBlackList(eventName.c_str());
				}
			}

			// Read blacklist additions:

			if (!ReadCompressedUInt32(m_hEnginePipe, listSize)) return false;

			for (unsigned int i = 0; i < listSize; ++i)
			{
				if (!ReadStringUTF8(m_hEnginePipe, eventName)) return false;
				CAfxGameEventListenerSerialzer::BlackList(eventName.c_str());
			}

			// Read enrichments:

			if (!ReadCompressedUInt32(m_hEnginePipe, listSize)) return false;

			for (unsigned int i = 0; i < listSize; ++i)
			{
				std::string propertyName;
				unsigned int enrichmentType;

				if (!ReadStringUTF8(m_hEnginePipe, eventName)) return false;
				if (!ReadStringUTF8(m_hEnginePipe, propertyName)) return false;
				if (!ReadUInt32(m_hEnginePipe, enrichmentType)) return false;
				
				CAfxGameEventListenerSerialzer::EnrichSet(eventName.c_str(), propertyName.c_str(), enrichmentType);
			}

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

	void DrawingThread_DeviceRestored()
	{
		if (!m_Enabled) return;

		std::shared_lock<std::shared_timed_mutex> clientsLock(m_ClientsMutex);

		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			it->Get()->DrawingThread_DeviceRestored();
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
				else if (0 == _stricmp(arg2, "edit") && 4 <= argc)
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
