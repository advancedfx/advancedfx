#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvTime.h"
#include "MirvCalcs.h"

#include <Windows.h>

#include <set>
#include <queue>

#include <mutex>
#include <atomic>

// TODO: very fast disconnects and reconnects will cause invalid state probably.

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;
extern SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo;

namespace AfxInterop {

	const INT32 m_Version = 4;

	bool Connect();
	void Disconnect();
	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool ReadBoolean(HANDLE hFile, bool & outValue);
	bool ReadByte(HANDLE hFile, BYTE & outValue);
	bool ReadSByte(HANDLE hFile, signed char & outValue);
	bool ReadUInt32(HANDLE hFile, UINT32 & outValue);
	bool ReadCompressedUInt32(HANDLE hFile, UINT32 & outValue);
	bool ReadInt32(HANDLE hFile, INT32 & outValue);
	bool ReadCompressedInt32(HANDLE hFile, INT32 & outValue);
	bool ReadHandle(HANDLE hFile, HANDLE & outValue);
	bool ReadSingle(HANDLE hFile, FLOAT & value);
	bool ReadStringUTF8(HANDLE hFile, std::string & outValue);
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool WriteBoolean(HANDLE hFile, bool value);
	bool WriteByte(HANDLE hFile, BYTE value);
	bool WriteSByte(HANDLE hFile, signed char value);
	bool WriteUInt32(HANDLE hFile, UINT32 value);
	bool WriteCompressedUInt32(HANDLE hFile, UINT32 value);
	bool WriteInt32(HANDLE hFile, INT32 value);
	bool WriteCompressedInt32(HANDLE hFile, INT32 value);
	bool WriteSingle(HANDLE hFile, FLOAT value);
	bool WriteStringUTF8(HANDLE hFile, const std::string);
	bool WriteHandle(HANDLE hFille, HANDLE value);
	bool Flush(HANDLE hFile);

	bool m_Enabled = false;

	namespace DrawingThread {

		enum DrawingMessage
		{
			DrawingMessage_Invalid = 0,
			DrawingMessage_PreapareDraw = 1,
			DrawingMessage_Finished = 2,
			DrawingMessage_BeforeTranslucentShadow = 3,
			DrawingMessage_AfterTranslucentShadow = 4,
			DrawingMessage_BeforeTranslucent = 5,
			DrawingMessage_AfterTranslucent = 6,
			DrawingMessage_BeforeHud = 7,
			DrawingMessage_AfterHud = 8
		};

		enum PrepareDrawReply
		{
			PrepareDrawReply_Skip = 1,
			PrepareDrawReply_Retry = 2,
			PrepareDrawReply_Continue = 3
		};

		std::mutex m_ConnectMutex;
		bool m_Connecting = false;
		bool m_Connected = false;

		HANDLE m_hPipe = INVALID_HANDLE_VALUE;

		IAfxInteropSurface * m_Surface = NULL;

		bool m_Skip = true;
	}

	namespace EngineThread {

		enum EngineMessage {
			EngineMessage_LevelInitPreEntity = 1,
			EngineMessage_LevelShutDown = 2,
			EngineMessage_BeforeFrameStart = 3,
			EngineMessage_OnRenderView = 4,
			EngineMessage_AfterFrameRenderEnd = 5,
			EngineMessage_BeforeFrameRenderStart = 8,
			EngineMessage_AfterFrameRenderStart = 9,
			EngineMessage_OnViewOverride = 10
		};

		class CConsole
		{
		public:
			CConsole(IWrpCommandArgs *args)
			{
				for (int i = 0; i < args->ArgC(); ++i)
				{
					m_Args.push(args->ArgV(i));
				}
			}

			bool Send(HANDLE hPipe)
			{
				if (!WriteCompressedUInt32(hPipe, (UINT32)m_Args.size())) return false;

				bool okay = true;

				while (!m_Args.empty())
				{
					okay = okay && WriteStringUTF8(hPipe, m_Args.front().c_str());
					m_Args.pop();
				}

				return okay;
			}

		private:
			std::queue<std::string> m_Args;
		};

		bool m_Server64Bit = false;

		std::mutex m_ConnectMutex;
		bool m_WantsConnect = false;
		bool m_Connected = false;

		HANDLE m_hPipe = INVALID_HANDLE_VALUE;

		std::string m_PipeName("advancedfxInterop");

		int m_Frame = -1;
		bool m_Suspended = false;

		std::queue<CConsole> m_Commands;

		void AddCommand(IWrpCommandArgs *args)
		{
			m_Commands.emplace(args);
		}

		bool SendCommands(HANDLE hPipe)
		{
			if (!WriteCompressedUInt32(hPipe, (UINT32)m_Commands.size())) return false;

			bool okay = true;

			while (!m_Commands.empty())
			{
				okay = okay && m_Commands.front().Send(hPipe);
				m_Commands.pop();
			}

			return okay;
		}
	}
	
	void DllProcessAttach() {
		m_Enabled = wcsstr(GetCommandLineW(), L"-afxInterop");

		if (!m_Enabled) return;
	}

	int GetFrameCount() {
		return EngineThread::m_Frame;
	}

	void BeforeFrameStart()
	{
		if (!m_Enabled) return;

		if (!Connect()) return;

		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		if (!EngineThread::m_Connected) return;

		int errorLine = 0;

		if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_BeforeFrameStart)) { errorLine = __LINE__; goto locked_error; }

		if (!EngineThread::SendCommands(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		UINT32 commandCount;

		if(!ReadCompressedUInt32(EngineThread::m_hPipe, commandCount)) { errorLine = __LINE__; goto locked_error; }

		for (UINT32 i = 0; i < commandCount; ++i)
		{
			std::string command;

			if (!ReadStringUTF8(EngineThread::m_hPipe, command)) { errorLine = __LINE__; goto locked_error; }

			g_VEngineClient->ExecuteClientCmd(command.c_str());
		}

		return;

	locked_error:
		Tier0_Warning("AfxInterop::BeforeFrameStart: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

	void BeforeFrameRenderStart()
	{
		if (!m_Enabled) return;

		++EngineThread::m_Frame;

		//

		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		if (!EngineThread::m_Connected) return;

		int errorLine = 0;

		if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_BeforeFrameRenderStart)) { errorLine = __LINE__; goto locked_error; }

		if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		return;

	locked_error:
		Tier0_Warning("AfxInterop::BeforeFrameRenderStart: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

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


	void AfterFrameRenderStart()
	{
		if (!m_Enabled) return;

		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		if (!EngineThread::m_Connected) return;

		int errorLine = 0;

		if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_AfterFrameRenderStart)) { errorLine = __LINE__; goto locked_error; }

		if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		{
			UINT32 numCalcs;
			std::string calcName;

			std::queue<HandleCalcResult *> handleCalcResults;
			std::queue<VecAngCalcResult *> vecAngCalcResults;
			std::queue<CamCalcResult *> camCalcResults;
			std::queue<FovCalcResult *> fovCalcResults;
			std::queue<BoolCalcResult *> boolCalcResults;
			std::queue<IntCalcResult *> intCalcResults;

			// Read and compute handle calcs:

			if(!ReadCompressedUInt32(EngineThread::m_hPipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < numCalcs; ++i)
			{
				if(!ReadStringUTF8(EngineThread::m_hPipe, calcName)) { errorLine = __LINE__; goto locked_error; }

				if (IMirvHandleCalc * calc = g_MirvHandleCalcs.GetByName(calcName.c_str()))
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

			if (!ReadCompressedUInt32(EngineThread::m_hPipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < numCalcs; ++i)
			{
				if (!ReadStringUTF8(EngineThread::m_hPipe, calcName)) { errorLine = __LINE__; goto locked_error; }

				if (IMirvVecAngCalc * calc = g_MirvVecAngCalcs.GetByName(calcName.c_str()))
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

			if (!ReadCompressedUInt32(EngineThread::m_hPipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < numCalcs; ++i)
			{
				if (!ReadStringUTF8(EngineThread::m_hPipe, calcName)) { errorLine = __LINE__; goto locked_error; }

				if (IMirvCamCalc * calc = g_MirvCamCalcs.GetByName(calcName.c_str()))
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

			if (!ReadCompressedUInt32(EngineThread::m_hPipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < numCalcs; ++i)
			{
				if (!ReadStringUTF8(EngineThread::m_hPipe, calcName)) { errorLine = __LINE__; goto locked_error; }

				if (IMirvFovCalc * calc = g_MirvFovCalcs.GetByName(calcName.c_str()))
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

			if (!ReadCompressedUInt32(EngineThread::m_hPipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < numCalcs; ++i)
			{
				if (!ReadStringUTF8(EngineThread::m_hPipe, calcName)) { errorLine = __LINE__; goto locked_error; }

				if (IMirvBoolCalc * calc = g_MirvBoolCalcs.GetByName(calcName.c_str()))
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

			if (!ReadCompressedUInt32(EngineThread::m_hPipe, numCalcs)) { errorLine = __LINE__; goto locked_error; }

			for (UINT32 i = 0; i < numCalcs; ++i)
			{
				if (!ReadStringUTF8(EngineThread::m_hPipe, calcName)) { errorLine = __LINE__; goto locked_error; }

				if (IMirvIntCalc * calc = g_MirvIntCalcs.GetByName(calcName.c_str()))
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
				HandleCalcResult * result = handleCalcResults.front();
				handleCalcResults.pop();

				if (result)
				{
					if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteInt32(EngineThread::m_hPipe, result->IntHandle)) { errorLine = __LINE__; goto locked_error; }

					delete result;
				}
				else
				{
					if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
				}
			}

			// Write vec ang calc result:

			while (!vecAngCalcResults.empty())
			{
				VecAngCalcResult * result = vecAngCalcResults.front();
				vecAngCalcResults.pop();

				if (result)
				{
					if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteSingle(EngineThread::m_hPipe, result->X)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Y)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Z)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteSingle(EngineThread::m_hPipe, result->Pitch)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Yaw)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Roll)) { errorLine = __LINE__; goto locked_error; }

					delete result;
				}
				else
				{
					if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
				}
			}

			// Write cam calc result:

			while (!camCalcResults.empty())
			{
				CamCalcResult * result = camCalcResults.front();
				camCalcResults.pop();

				if (result)
				{
					if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteSingle(EngineThread::m_hPipe, result->X)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Y)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Z)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteSingle(EngineThread::m_hPipe, result->Pitch)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Yaw)) { errorLine = __LINE__; goto locked_error; }
					if (!WriteSingle(EngineThread::m_hPipe, result->Roll)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteSingle(EngineThread::m_hPipe, result->Fov)) { errorLine = __LINE__; goto locked_error; }

					delete result;
				}
				else
				{
					if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
				}
			}

			// Write fov calc result:

			while (!fovCalcResults.empty())
			{
				FovCalcResult * result = fovCalcResults.front();
				fovCalcResults.pop();

				if (result)
				{
					if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteSingle(EngineThread::m_hPipe, result->Fov)) { errorLine = __LINE__; goto locked_error; }

					delete result;
				}
				else
				{
					if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
				}
			}

			// Write bool calc result:

			while (!boolCalcResults.empty())
			{
				BoolCalcResult * result = boolCalcResults.front();
				boolCalcResults.pop();

				if (result)
				{
					if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteBoolean(EngineThread::m_hPipe, result->Result)) { errorLine = __LINE__; goto locked_error; }

					delete result;
				}
				else
				{
					if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
				}
			}

			// Write int calc result:

			while (!intCalcResults.empty())
			{
				IntCalcResult * result = intCalcResults.front();
				intCalcResults.pop();

				if (result)
				{
					if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

					if (!WriteInt32(EngineThread::m_hPipe, result->Result)) { errorLine = __LINE__; goto locked_error; }

					delete result;
				}
				else
				{
					if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
				}
			}

			// Do not flush here, since we are not waiting for data.
		}
	
		return;

	locked_error:
		Tier0_Warning("AfxInterop::AfterFrameRenderStart: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

	bool OnViewOverride(float & Tx, float & Ty, float & Tz, float & Rx, float & Ry, float & Rz, float & Fov)
	{

		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		if (!EngineThread::m_Connected) return false;

		int errorLine = 0;

		if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_OnViewOverride)) { errorLine = __LINE__; goto locked_error; }

		if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		{
			bool overrideView;

			if(!ReadBoolean(EngineThread::m_hPipe, overrideView)) { errorLine = __LINE__; goto locked_error; }

			if (overrideView)
			{
				FLOAT tTx, tTy, tTz, tRx, tRy, tRz, tFov;

				if (!ReadSingle(EngineThread::m_hPipe, tTx)) { errorLine = __LINE__; goto locked_error; }
				if (!ReadSingle(EngineThread::m_hPipe, tTy)) { errorLine = __LINE__; goto locked_error; }
				if (!ReadSingle(EngineThread::m_hPipe, tTz)) { errorLine = __LINE__; goto locked_error; }
				if (!ReadSingle(EngineThread::m_hPipe, tRx)) { errorLine = __LINE__; goto locked_error; }
				if (!ReadSingle(EngineThread::m_hPipe, tRy)) { errorLine = __LINE__; goto locked_error; }
				if (!ReadSingle(EngineThread::m_hPipe, tRz)) { errorLine = __LINE__; goto locked_error; }
				if (!ReadSingle(EngineThread::m_hPipe, tFov)) { errorLine = __LINE__; goto locked_error; }

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
		Tier0_Warning("AfxInterop::OnViewOverride: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return false;
	}

	void OnRenderView(const SOURCESDK::CViewSetup_csgo & view, EnabledFeatures_t & outEnabled)
	{
		if (!m_Enabled) return;

		outEnabled.Clear();

		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		if (!EngineThread::m_Connected) return;

		int errorLine = 0;
		{

			if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_OnRenderView)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(EngineThread::m_hPipe, EngineThread::m_Frame)) { errorLine = __LINE__; goto error; }

			if (!WriteSingle(EngineThread::m_hPipe, g_MirvTime.GetAbsoluteFrameTime())) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, g_MirvTime.GetTime())) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, g_MirvTime.GetFrameTime())) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(EngineThread::m_hPipe, view.width)) { errorLine = __LINE__; goto error; }
			if (!WriteInt32(EngineThread::m_hPipe, view.height)) { errorLine = __LINE__; goto error; }

			SOURCESDK::VMatrix worldToView;
			SOURCESDK::VMatrix viewToProjection;
			SOURCESDK::VMatrix worldToProjection;
			SOURCESDK::VMatrix worldToPixels;

			g_pVRenderView_csgo->GetMatricesForView(view, &worldToView, &viewToProjection, &worldToProjection, &worldToPixels);

			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][3])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][3])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][3])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][3])) { errorLine = __LINE__; goto error; }

			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][3])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][3])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][3])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][0])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][1])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][2])) { errorLine = __LINE__; goto error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][3])) { errorLine = __LINE__; goto error; }

			if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto error; }

			if (!ReadBoolean(EngineThread::m_hPipe, outEnabled.BeforeTranslucentShadow)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(EngineThread::m_hPipe, outEnabled.AfterTranslucentShadow)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(EngineThread::m_hPipe, outEnabled.BeforeTranslucent)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(EngineThread::m_hPipe, outEnabled.AfterTranslucent)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(EngineThread::m_hPipe, outEnabled.BeforeHud)) { errorLine = __LINE__; goto error; }
			if (!ReadBoolean(EngineThread::m_hPipe, outEnabled.AfterHud)) { errorLine = __LINE__; goto error; }
		}

		return;

	error:
		Tier0_Warning("AfxInterop::BeforeHud: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

	void Shutdown() {
		if (!m_Enabled) return;

		Disconnect();
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

	void DrawingThreadPrepareDraw(int frameCount)
	{
		if (!m_Enabled) return;

		DrawingThread::m_Skip = true;

		std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

		if (!(DrawingThread::m_Connected || DrawingThread::m_Connecting)) return;

		if (DrawingThread::m_Connecting)
		{
			DrawingThread::m_Connecting = false;
			DrawingThread::m_Connected = true;
		}

		int errorLine = 0;

		while (true)
		{
			if (!WriteInt32(DrawingThread::m_hPipe, DrawingThread::DrawingMessage_PreapareDraw)) { errorLine = __LINE__; goto locked_error; }
			if (!WriteInt32(DrawingThread::m_hPipe, frameCount)) { errorLine = __LINE__; goto locked_error; }
			if (!Flush(DrawingThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

			INT32 prepareDrawReply;
			if(!ReadInt32(DrawingThread::m_hPipe, prepareDrawReply)) { errorLine = __LINE__; goto locked_error; }

			switch (prepareDrawReply)
			{
			case DrawingThread::PrepareDrawReply_Skip:
				DrawingThread::m_Skip = true;
				return;
			case DrawingThread::PrepareDrawReply_Retry:
				break;
			case DrawingThread::PrepareDrawReply_Continue:
				{
					DrawingThread::m_Skip = false;

					bool colorTextureWasLost;
					HANDLE sharedColorTextureHandle;
					bool colorDepthTextureWasLost;
					HANDLE sharedColorDepthTextureHandle;

					if (!ReadBoolean(DrawingThread::m_hPipe, colorTextureWasLost)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadHandle(DrawingThread::m_hPipe, sharedColorTextureHandle)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadBoolean(DrawingThread::m_hPipe, colorDepthTextureWasLost)) { errorLine = __LINE__; goto locked_error; }
					if (!ReadHandle(DrawingThread::m_hPipe, sharedColorDepthTextureHandle)) { errorLine = __LINE__; goto locked_error; }

					if (DrawingThread::m_Surface)
					{
						if (colorTextureWasLost) DrawingThread::m_Surface->AfxSetReplacement(NULL);
						if (colorDepthTextureWasLost) DrawingThread::m_Surface->AfxSetDepthSurface(NULL);

						if(sharedColorTextureHandle != DrawingThread::m_Surface->AfxGetReplacement() || sharedColorDepthTextureHandle != DrawingThread::m_Surface->AfxGetDepthSurface())
						{
							IDirect3DDevice9 * device = NULL;
							if (SUCCEEDED(DrawingThread::m_Surface->AfxGetSurface()->GetDevice(&device)))
							{
								D3DSURFACE_DESC desc;

								if (SUCCEEDED(DrawingThread::m_Surface->AfxGetSurface()->GetDesc(&desc)))
								{
									if(sharedColorTextureHandle != DrawingThread::m_Surface->AfxGetReplacement())
									{
										IDirect3DTexture9 * replacementTexture = NULL;
										if (sharedColorTextureHandle)
										{
											if (SUCCEEDED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &replacementTexture, &sharedColorTextureHandle)))
											{
												IDirect3DSurface9 * replacementSurface = NULL;

												if (SUCCEEDED(replacementTexture->GetSurfaceLevel(0, &replacementSurface)))
												{
													DrawingThread::m_Surface->AfxSetReplacement(replacementSurface);
													DrawingThread::m_Surface->AfxReplacementEnabled_set(true);
													replacementSurface->Release();
												}
												replacementTexture->Release();
											}
										}
									}
									if(sharedColorDepthTextureHandle != DrawingThread::m_Surface->AfxGetDepthSurface())
									{
										IDirect3DTexture9 * replacementTexture = NULL;
										if (sharedColorDepthTextureHandle)
										{
											if (SUCCEEDED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &replacementTexture, &sharedColorDepthTextureHandle)))
											{
												IDirect3DSurface9 * replacementSurface = NULL;

												if (SUCCEEDED(replacementTexture->GetSurfaceLevel(0, &replacementSurface)))
												{
													DrawingThread::m_Surface->AfxSetDepthSurface(replacementSurface);
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
		Tier0_Warning("AfxInterop::DrawingThreadPrepareDraw: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
	}

	void DrawingThreadFinished()
	{
		if (!m_Enabled) return;

		std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

		if (!DrawingThread::m_Connected) return;

		int errorLine = 0;

		if (!WriteInt32(DrawingThread::m_hPipe, DrawingThread::DrawingMessage_Finished)) { errorLine = __LINE__; goto locked_error; }
		if (!Flush(DrawingThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		bool done;
		do {
			if (!ReadBoolean(DrawingThread::m_hPipe, done)) { errorLine = __LINE__; goto locked_error; }
		} while (!done);

		return;

	locked_error:
		Tier0_Warning("AfxInterop::DrawingThreadFinished: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

	void DrawingThread_On_DrawTranslucentRenderables(IAfxMatRenderContextOrg * context, bool bInSkybox, bool bShadowDepth, bool afterCall)
	{
		if (!m_Enabled) return;

		if (DrawingThread::m_Skip) return;

		if (bInSkybox) return;

		std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

		if (!DrawingThread::m_Connected) return;

		int errorLine = 0;

		{

			AfxD3D_WaitForGPU(); // TODO: This is only required if Unity wants to render shit.

			DrawingThread::DrawingMessage message = DrawingThread::DrawingMessage_Invalid;

			if (true == bShadowDepth)
			{
				if (false == afterCall)
				{
					message = DrawingThread::DrawingMessage_BeforeTranslucentShadow;
				}
				else
				{
					message = DrawingThread::DrawingMessage_AfterTranslucentShadow;
				}
			}
			else
			{
				if (false == afterCall)
				{
					message = DrawingThread::DrawingMessage_BeforeTranslucent;
				}
				else
				{
					message = DrawingThread::DrawingMessage_AfterTranslucent;
				}
			}

			if (!WriteInt32(DrawingThread::m_hPipe, message)) { errorLine = __LINE__; goto locked_error; }

			int x, y, width, height;
			context->GetViewport(x, y, width, height);

			SOURCESDK::VMatrix worldToView;
			SOURCESDK::VMatrix viewToProjection;

			context->GetMatrix(SOURCESDK::MATERIAL_VIEW, &worldToView);
			context->GetMatrix(SOURCESDK::MATERIAL_PROJECTION, &viewToProjection);

			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[0][3])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[1][3])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[2][3])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, worldToView.m[3][3])) { errorLine = __LINE__; goto locked_error; }

			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[0][3])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[1][3])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[2][3])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][0])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][1])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][2])) { errorLine = __LINE__; goto locked_error; }
			if (!WriteSingle(EngineThread::m_hPipe, viewToProjection.m[3][3])) { errorLine = __LINE__; goto locked_error; }

			if (!Flush(DrawingThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

			bool done;
			do {
				if (!ReadBoolean(DrawingThread::m_hPipe, done)) { errorLine = __LINE__; goto locked_error; }
			} while (!done);

			return;
		}
	locked_error:
		Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}
	
	void DrawingThread_BeforeHud(IAfxMatRenderContextOrg * context)
	{
		if (!m_Enabled) return;

		if (DrawingThread::m_Skip) return;

		std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

		if (!DrawingThread::m_Connected) return;

		int errorLine = 0;

		AfxD3D_WaitForGPU(); // TODO: This is only required if Unity wants to render shit.

		if (!WriteInt32(DrawingThread::m_hPipe, DrawingThread::DrawingMessage_BeforeHud)) { errorLine = __LINE__; goto locked_error; }
		if (!Flush(DrawingThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		bool done;
		do {
			if (!ReadBoolean(DrawingThread::m_hPipe, done)) { errorLine = __LINE__; goto locked_error; }
		} while (!done);

		return;

	locked_error:
		Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

	void DrawingThread_AfterHud(IAfxMatRenderContextOrg * context)
	{
		if (!m_Enabled) return;

		if (DrawingThread::m_Skip) return;

		std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

		if (!DrawingThread::m_Connected) return;

		int errorLine = 0;

		AfxD3D_WaitForGPU(); // TODO: This is only required if Unity wants to render shit.

		if (!WriteInt32(DrawingThread::m_hPipe, DrawingThread::DrawingMessage_AfterHud)) { errorLine = __LINE__; goto locked_error; }
		if (!Flush(DrawingThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		bool done;
		do {
			if (!ReadBoolean(DrawingThread::m_hPipe, done)) { errorLine = __LINE__; goto locked_error; }
		} while (!done);

		return;

	locked_error:
		Tier0_Warning("AfxInterop::DrawingThread_BeforeHud: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return;
	}

	void OnCreatedSurface(IAfxInteropSurface * surface)
	{
		if (DrawingThread::m_Surface)
		{
			OnReleaseSurface(DrawingThread::m_Surface);
		}

		DrawingThread::m_Surface = surface;
	}

	void OnReleaseSurface(IAfxInteropSurface * surface)
	{
		if (surface)
		{
			surface->AfxReplacementEnabled_set(false);
			surface->AfxSetReplacement(NULL);
			surface->AfxSetDepthSurface(NULL);

			DrawingThread::m_Surface = NULL;
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

	bool Connect() {

		if (!m_Enabled) return false;

		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		if (!EngineThread::m_WantsConnect) {

			if (EngineThread::m_Connected)
			{
				lock.unlock();
				Disconnect();
			}

			return false;
		}
		else if (EngineThread::m_Connected) return true;

		int errorLine = 0;
		{
			std::string strPipeName("\\\\.\\pipe\\");
			strPipeName.append(EngineThread::m_PipeName);

			while (true)
			{
				EngineThread::m_hPipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (EngineThread::m_hPipe != INVALID_HANDLE_VALUE)
					break;

				DWORD lastError = GetLastError();

				if (lastError != ERROR_PIPE_BUSY)
				{
					Tier0_Warning("Could not open pipe. GLE=%d\n", lastError);
					{ errorLine = __LINE__; goto locked_error; }
				}

				if (!WaitNamedPipe(strPipeName.c_str(), 5000))
				{
					Tier0_Warning("WaitNamedPipe: timed out.\n");
					{ errorLine = __LINE__; goto locked_error; }
				}
			}

			Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());
		}

		{
			std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

			std::string strPipeName("\\\\.\\pipe\\");
			strPipeName.append(EngineThread::m_PipeName);
			strPipeName.append("_drawing");

			unsigned int sleepCount = 0;

			while (true)
			{
				DrawingThread::m_hPipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (DrawingThread::m_hPipe != INVALID_HANDLE_VALUE)
					break;

				DWORD lastError = GetLastError();

				switch (lastError)
				{
				case ERROR_FILE_NOT_FOUND:
					if (sleepCount >= 5000)
					{
						Tier0_Warning("Could not open pipe. GLE=%d\n", lastError);
						{ errorLine = __LINE__; goto locked_error; }
					}
					++sleepCount;
					Sleep(1);
					continue;
				case ERROR_PIPE_BUSY:
					break;
				default:
					{
						Tier0_Warning("Could not open pipe. GLE=%d\n", lastError);
						{ errorLine = __LINE__; goto locked_error; }
					}
				}

				if (!WaitNamedPipe(strPipeName.c_str(), 5000))
				{
					Tier0_Warning("WaitNamedPipe: timed out.\n");
					{ errorLine = __LINE__; goto locked_error; }
				}
			}

			Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());
			DrawingThread::m_Connecting = true;
		}

		INT32 version;
		if (!ReadInt32(EngineThread::m_hPipe, version)) { errorLine = __LINE__; goto locked_error; }
		
		if (m_Version != version)
		{
			Tier0_Warning("Version %d is not a supported (%d) version.\n", version, m_Version);
			if (!WriteBoolean(EngineThread::m_hPipe, false)) { errorLine = __LINE__; goto locked_error; }
			if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }
		}

		if (!WriteBoolean(EngineThread::m_hPipe, true)) { errorLine = __LINE__; goto locked_error; }

		if(!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

		if(!ReadBoolean(EngineThread::m_hPipe, EngineThread::m_Server64Bit)) { errorLine = __LINE__; goto locked_error; }

		EngineThread::m_Connected = true;

		return true;

	locked_error:
		Tier0_Warning("AfxInterop::Connect: Error in line %i.\n", errorLine);
		lock.unlock();
		Disconnect();
		return false;
	}

	void Disconnect() {
		std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

		{
			std::unique_lock<std::mutex> lock(DrawingThread::m_ConnectMutex);

			if (INVALID_HANDLE_VALUE != DrawingThread::m_hPipe)
			{
				if (!CloseHandle(DrawingThread::m_hPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
				}
				DrawingThread::m_hPipe = INVALID_HANDLE_VALUE;
			}

			if (DrawingThread::m_Connected)
			{
				DrawingThread::m_Connected = false;
			}

			if (DrawingThread::m_Surface)
			{
				DrawingThread::m_Surface->AfxReplacementEnabled_set(false);
				DrawingThread::m_Surface->AfxSetReplacement(NULL);
				DrawingThread::m_Surface->AfxSetDepthSurface(NULL);
			}
		}

		if (INVALID_HANDLE_VALUE != EngineThread::m_hPipe)
		{
			if (!CloseHandle(EngineThread::m_hPipe))
			{
				Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
			}
			EngineThread::m_hPipe = INVALID_HANDLE_VALUE;
		}

		if (EngineThread::m_Connected)
		{
			EngineThread::m_Connected = false;
		}
	}


	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
	{
		lpBuffer = &(((char *)lpBuffer)[offset]);
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
			lpBuffer = &(((char *)lpBuffer)[bytesRead]);

		} while (0 < numBytes);

		return true;
	}

	bool ReadBoolean(HANDLE hFile, bool & outValue)
	{
		BYTE useVal;

		bool result =  ReadBytes(hFile, &useVal, 0, sizeof(useVal));
		
		if (result) outValue = 0 != useVal ? true : false;

		return result;
	}

	bool ReadByte(HANDLE hFile, BYTE & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadSByte(HANDLE hFile, signed char & value)
	{
		return ReadByte(hFile, (BYTE &)value);
	}

	bool ReadUInt32(HANDLE hFile, UINT32 & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadCompressedUInt32(HANDLE hFile, UINT32 & outValue)
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

	bool ReadInt32(HANDLE hFile, INT32 & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadCompressedInt32(HANDLE hFile, INT32 & outValue)
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

	bool ReadHandle(HANDLE hFile, HANDLE & outValue)
	{
		DWORD value32;

		if (ReadBytes(hFile, &value32, 0, sizeof(value32)))
		{
			outValue = ULongToHandle(value32);
			return true;
		}

		return false;
	}

	bool ReadSingle(HANDLE hFile, FLOAT & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadStringUTF8(HANDLE hFile, std::string & outValue)
	{
		UINT32 length;

		if (!ReadCompressedUInt32(hFile, length)) return false;

		outValue.resize(length);

		if (!ReadBytes(hFile,&outValue[0],0,length)) return false;

		return true;
	}
	
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
	{
		DWORD bytesWritten;

		if (!WriteFile(hFile, &(((char *)lpBuffer)[offset]), numBytes, &bytesWritten, NULL) || numBytes != bytesWritten)
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

}

CON_COMMAND(afx_interop, "Controls advancedfxInterop (i.e. with Unity engine).")
{
	if (!AfxInterop::m_Enabled)
	{
		Tier0_Warning("Error: afx_interop command requires -afxInterop launch option!\n");
		return;
	}

	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("pipeName", arg1))
		{
			std::unique_lock<std::mutex> lock(AfxInterop::EngineThread::m_ConnectMutex);

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);
				
				AfxInterop::EngineThread::m_PipeName = arg2;

				return;
			}

			Tier0_Msg(
				"afx_interop pipeName <sName> - Set new name.\n"
				"Current value: %s\n"
				, AfxInterop::EngineThread::m_PipeName
			);
			return;
		}
		else if (0 == _stricmp("connect", arg1))
		{
			std::unique_lock<std::mutex> lock(AfxInterop::EngineThread::m_ConnectMutex);

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				AfxInterop::EngineThread::m_WantsConnect = 0 != atoi(arg2);

				return;
			}

			Tier0_Msg(
				"afx_interop connect 0|1 - 0 = disabled (default), 1 = enabled.\n"
				"Current value: %i\n"
				, AfxInterop::EngineThread::m_WantsConnect ? 1 : 0
			);
			return;
		}
		else if (0 == _stricmp("send", arg1))
		{
			CSubWrpCommandArgs subArgs(args, 2);

			AfxInterop::EngineThread::AddCommand(&subArgs);

			return;
		}
	}

	Tier0_Msg(
		"afx_interop pipeName [...] - Name of the pipe to connect to.\n"
		"afx_interop connect [...] - Controls if interop connection is enabled.\n"
		"afx_interop send [<arg1>[ <arg2> [ ...]] - Queues a command to be sent to the server (lossy if connection is unstable).\n"
	);
}

#endif
