#include "stdafx.h"

#include "MirvPgl.h"

#ifdef AFX_MIRV_PGL

#include "../../shared/easywsclient/easywsclient.hpp"
#include "../../shared/easywsclient/easywsclient.cpp"

#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "AfxStreams.h"

#include <math.h>

#include <string>
#include <list>
#include <queue>
#include <set>

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>


extern WrpVEngineClient * g_VEngineClient;


using namespace std::chrono_literals;

using easywsclient::WebSocket;

namespace MirvPgl
{
	const int m_CheckRestoreEveryTicks = 5000;
	const int m_ThreadSleepMsIfNoData = 1;
	const uint32_t m_Version = 2;

	CamData::CamData()
	{

	}

	CamData::CamData(float time, float xPosition, float yPosition, float zPosition, float xRotation, float yRotation, float zRotation, float fov)
	: Time(time)
	, XPosition(xPosition)
	, YPosition(yPosition)
	, ZPosition(zPosition)
	, XRotation(xRotation)
	, YRotation(yRotation)
	, ZRotation(zRotation)
	, Fov(fov)
	{
	}

	class CThreadData
	{
	public:
		void Prepare(std::vector<std::uint8_t> const & data)
		{
			m_IsCancelled = false;
			m_Data = data;
		}

		void Cancel()
		{
			m_IsCancelled = true;
		}

		bool IsCancelled()
		{
			return m_IsCancelled;
		}

		std::vector<uint8_t> & AccessData()
		{
			return m_Data;
		}

	private:
		std::atomic_bool m_IsCancelled;
		std::vector<uint8_t> m_Data;
	};

	void DrawingThread_SupplyThreadData(CThreadData * threadData);

	class CSupplyThreadData_Functor
		: public CAfxFunctor
	{
	public:
		CSupplyThreadData_Functor(CThreadData * threadData)
			: m_Value(threadData)
		{
		}

		virtual void operator()()
		{
			DrawingThread_SupplyThreadData(m_Value);
		}

	private:
		CThreadData * m_Value;
	};


	class CThreadDataPool
	{
	public:
		/// <remarks>Must only be called from main thread.</remarks>
		std::vector<std::uint8_t> & AccessNextThreadData(void)
		{
			return m_Data;
		}

		/// <remarks>Must only be called from main thread.</remarks>
		CThreadData * Acquire(void)
		{
			std::unique_lock<std::mutex> lock(m_ThreadDataQueueMutex);

			CThreadData * threadData;
			
			if (m_ThreadDataAvailable.empty())
			{
				threadData = new CThreadData();
			}
			else
			{
				threadData = m_ThreadDataAvailable.front();
				m_ThreadDataAvailable.pop();
			}

			threadData->Prepare(m_Data);

			m_Data.clear();

			m_ThreadDataInUse.insert(threadData);

			return threadData;
		}

		void Return(CThreadData * threadData)
		{
			threadData->Cancel();

			std::unique_lock<std::mutex> lock(m_ThreadDataQueueMutex);

			m_ThreadDataInUse.erase(threadData);

			m_ThreadDataAvailable.push(threadData);
		}

		void Cancel(void)
		{
			std::unique_lock<std::mutex> lock(m_ThreadDataQueueMutex);

			for (std::set<CThreadData *>::iterator it = m_ThreadDataInUse.begin(); it != m_ThreadDataInUse.end(); ++it)
			{
				(*it)->Cancel();
			}

			m_Data.clear();
		}

	private:
		std::vector<std::uint8_t> m_Data;
		std::queue<CThreadData *> m_ThreadDataAvailable;
		std::set<CThreadData *> m_ThreadDataInUse;
		std::mutex m_ThreadDataQueueMutex;
	} m_ThreadDataPool;

	bool m_WsaActive = false;

	WebSocket * m_Ws = 0;
	bool m_WantWs = false;
	std::string m_WsUrl("ws://host:port/path");
	std::mutex m_WsMutex;

	std::list<std::string> m_Commands;
	std::mutex m_CommandsMutex;

	std::vector<uint8_t> m_SendThreadTempData;

	std::vector<uint8_t> m_DataForSendThread;
	std::condition_variable m_DataForSendThreadAvailableCondition;
	std::mutex m_DataForSendThreadMutex;

	bool m_DataActive = false;

	std::thread * m_Thread = 0;
	bool m_WantClose = false;

	DWORD m_LastCheckRestoreTick = 0;

	CThreadData * m_DrawingThread_ThreadData = 0;

	std::string m_CurrentLevel;

	void AppendCString(char const * cstr, std::vector<uint8_t> &outVec)
	{
		std::string str(cstr);
		outVec.insert(outVec.end(), str.begin(), str.end());
		outVec.push_back(static_cast<uint8_t>('\0'));
	}

	void AppendHello(std::vector<uint8_t> &outVec)
	{
		uint8_t data[1 * sizeof(uint32_t)];
		uint32_t version = m_Version;

		AppendCString("hello", outVec);
		memcpy(&(data[0 * sizeof(uint32_t)]), &version, sizeof(uint32_t));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void AppendCamData(CamData const camData, std::vector<uint8_t> &outVec)
	{
		uint8_t data[8 * sizeof(float)];

		memcpy(&(data[0 * sizeof(float)]), &camData.Time, sizeof(float));
		memcpy(&(data[1 * sizeof(float)]), &camData.XPosition, sizeof(float));
		memcpy(&(data[2 * sizeof(float)]), &camData.YPosition, sizeof(float));
		memcpy(&(data[3 * sizeof(float)]), &camData.ZPosition, sizeof(float));
		memcpy(&(data[4 * sizeof(float)]), &camData.XRotation, sizeof(float));
		memcpy(&(data[5 * sizeof(float)]), &camData.YRotation, sizeof(float));
		memcpy(&(data[6 * sizeof(float)]), &camData.ZRotation, sizeof(float));
		memcpy(&(data[7 * sizeof(float)]), &camData.Fov, sizeof(float));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void Recv_String(const std::string & message)
	{
		// lul
	}

	void Recv_Bytes(const std::vector<uint8_t>& message)
	{
		std::vector<uint8_t>::const_iterator itBegin = message.begin();

		while (itBegin != message.end())
		{
			std::vector<uint8_t>::const_iterator itDelim = message.end();

			for (std::vector<uint8_t>::const_iterator it = itBegin; it != message.end(); ++it)
			{
				if ((uint8_t)'\0' == *it)
				{
					itDelim = it;
					break;
				}
			}

			if (message.end() != itDelim && itBegin != itDelim)
			{
				std::string strCode(itBegin, itDelim);

				char const * code = strCode.c_str();

				if (0 == strcmp("exec", code))
				{
					std::unique_lock<std::mutex> lock(m_CommandsMutex);

					std::vector<uint8_t>::const_iterator itCmdStart = itDelim + 1;
					std::vector<uint8_t>::const_iterator itCmdEnd = itCmdStart;

					bool foundDelim = false;

					for (std::vector<uint8_t>::const_iterator it = itCmdStart; it != message.end(); ++it)
					{
						if ((uint8_t)'\0' == *it)
						{
							foundDelim = true;
							itCmdEnd = it;
							break;
						}					
					}

					if (!foundDelim)
						break;

					std::string cmds(itCmdStart, itCmdEnd);

					m_Commands.push_back(cmds);

					itBegin = itCmdEnd + 1;

					continue;
				}
			}

			break;
		}
	}

	void Thread()
	{
		m_SendThreadTempData.clear();

		while (true)
		{
			{
				std::unique_lock<std::mutex> wsLock(m_WsMutex);

				if (WebSocket::CLOSED == m_Ws->getReadyState())
				{
					delete m_Ws;
					m_Ws = 0;
					break;
				}
			}

			m_Ws->poll();

			// this would eat our shit: m_Ws->dispatch(Recv_String); 
			m_Ws->dispatchBinary(Recv_Bytes);

			std::unique_lock<std::mutex> dataLock(m_DataForSendThreadMutex);
			m_DataForSendThreadAvailableCondition.wait_for(dataLock, m_ThreadSleepMsIfNoData * 1ms, [] { return !m_DataForSendThread.empty() || m_WantClose; }); // if we don't need to send data, we are a bit lazy in order to save some CPU. Of course this assumes, that the data we get from network can wait that long ;)

			if (!m_DataForSendThread.empty())
			{
				m_SendThreadTempData = std::move(m_DataForSendThread);
				m_DataForSendThread.clear();
				dataLock.unlock();

				m_Ws->sendBinary(m_SendThreadTempData);

				m_SendThreadTempData.clear();
			}
			else
				dataLock.unlock();

			if (m_WantClose)
				m_Ws->close();
		}
	}
	
	void EndThread()
	{
		if (0 != m_Thread)
		{
			m_WantClose = true;
			
			m_DataForSendThreadAvailableCondition.notify_one();

			m_Thread->join();
			
			delete m_Thread;
			m_Thread = 0;
			
			m_WantClose = false;
		}
	}

	void CreateThread()
	{
		EndThread();

		m_Thread = new std::thread(Thread);
	}


	void Init()
	{
		Shutdown();

		WSADATA wsaData;

		m_WsaActive = 0 == WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	void Shutdown()
	{
		Stop();

		if (m_WsaActive)
		{
			WSACleanup();
			m_WsaActive = false;
		}
	}

	void Url_set(char const * url)
	{
		m_WsUrl = url;
	}

	char const * Url_get(void)
	{
		return m_WsUrl.c_str();
	}

	void Start()
	{
		Stop();

		if(m_WsaActive)
		{
			m_WantWs = true;

			m_Ws = WebSocket::from_url(m_WsUrl);

			if (0 != m_Ws)
			{
				AppendHello(m_ThreadDataPool.AccessNextThreadData());

				CreateThread();
			}
		}
	}

	void Stop()
	{
		m_DataActive = false;
		m_ThreadDataPool.Cancel();
		m_ThreadDataPool.AccessNextThreadData().clear();

		EndThread();

		m_WantWs = false;
	}

	bool IsStarted()
	{
		return m_WantWs;
	}

	void DataStart()
	{
		DataStop();

		if (m_WantWs)
		{
			m_DataActive = true;

			std::vector<uint8_t> & data = m_ThreadDataPool.AccessNextThreadData();

			AppendCString("dataStart", data);

			if (!m_CurrentLevel.empty())
			{

				AppendCString("levelInit", data);
				AppendCString(m_CurrentLevel.c_str(), data);
			}
		}
	}

	void DataStop()
	{
		if (m_DataActive)
		{
			m_DataActive = false;

			m_ThreadDataPool.Cancel();

			if (m_WantWs)
			{
				AppendCString("dataStop", m_ThreadDataPool.AccessNextThreadData());
			}
		}
	}

	bool IsDataActive()
	{
		return m_DataActive;
	}

	void CheckStartedAndRestoreIfDown()
	{
		if (m_WantWs)
		{
			DWORD curTick = GetTickCount();

			if (m_CheckRestoreEveryTicks <= abs((int)m_LastCheckRestoreTick - (int)curTick))
			{
				m_LastCheckRestoreTick = curTick;

				bool needRestore = false;
				{
					std::unique_lock<std::mutex> lock(m_WsMutex);

					needRestore = 0 == m_Ws;
				}

				if (needRestore)
				{
					Start();
				}
			}
		}
	}

	void QueueThreadDataForDrawingThread(void)
	{
		if(m_DataActive || !m_ThreadDataPool.AccessNextThreadData().empty())
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CSupplyThreadData_Functor(m_ThreadDataPool.Acquire())));
	}

	void SupplyLevelInit(char const * mapName)
	{
		m_CurrentLevel = mapName;

		if (!m_DataActive)
			return;

		std::vector<uint8_t> & data = m_ThreadDataPool.AccessNextThreadData();

		AppendCString("levelInit", data);
		AppendCString(mapName, data);
	}

	void SupplyLevelShutdown()
	{
		m_CurrentLevel.clear();

		if (!m_DataActive)
			return;

		AppendCString("levelShutdown", m_ThreadDataPool.AccessNextThreadData());
	}

	void ExecuteQueuedCommands()
	{
		std::unique_lock<std::mutex> lock(m_CommandsMutex);

		while (0 < m_Commands.size())
		{
			std::string cmd = m_Commands.front();
			m_Commands.pop_front();
			lock.unlock();

			g_VEngineClient->ExecuteClientCmd(cmd.c_str());

			lock.lock();
		}
	}

	void DrawingThread_SupplyThreadData(CThreadData * threadData)
	{
		if (m_DrawingThread_ThreadData)
		{
			// should not happen.
			Assert(0);

			m_ThreadDataPool.Return(m_DrawingThread_ThreadData);
		}

		m_DrawingThread_ThreadData = threadData;
	}

	void DrawingThread_SupplyCamData(CamData const & camData)
	{
		if(m_DrawingThread_ThreadData)
		{
			std::vector<uint8_t> & data = m_DrawingThread_ThreadData->AccessData();

			AppendCString("cam", data);
			AppendCamData(camData, data);
		}
	}

	void DrawingThread_PresentedUnleashDataOnFirstCall()
	{
		if (m_DrawingThread_ThreadData && !m_DrawingThread_ThreadData->IsCancelled())
		{
			std::vector<uint8_t> & data = m_DrawingThread_ThreadData->AccessData();

			std::unique_lock<std::mutex> lock(m_DataForSendThreadMutex);

			m_DataForSendThread.insert(m_DataForSendThread.end(), data.begin(), data.end());

			lock.unlock();

			m_DataForSendThreadAvailableCondition.notify_one();

			m_ThreadDataPool.Return(m_DrawingThread_ThreadData);

			m_DrawingThread_ThreadData = 0;
		}
	}

}

CON_COMMAND(mirv_pgl, "PGL")
{
	if (!MirvPgl::m_WsaActive)
	{
		Tier0_Warning("Error: WinSock(2.2) not active, feature not available!\n");
		return;
	}

	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (0 == _stricmp("start", cmd1))
		{
			MirvPgl::Start();
			return;
		}
		else if (0 == _stricmp("stop", cmd1))
		{
			MirvPgl::Stop();
			return;
		}
		else if (0 == _stricmp("dataStart", cmd1))
		{
			MirvPgl::DataStart();
			return;
		}
		else if (0 == _stricmp("dataStop", cmd1))
		{
			MirvPgl::DataStop();
			return;
		}
		else if (0 == _stricmp("url", cmd1))
		{
			if (3 <= argc)
			{
				MirvPgl::Url_set(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"mirv_pgl url <url> - Set url to use with start.\n"
				"Current value: %s\n"
				, MirvPgl::Url_get()
			);
			return;
		}

	}

	Tier0_Msg(
		"mirv_pgl start - (Re-)Starts connectinion to server.\n"
		"mirv_pgl stop - Stops connection to server.\n"
		"mirv_pgl dataStart - Start sending data.\n"
		"mirv_pgl dataStop - Stop sending data.\n"
		"mirv_pgl url [...] - Set url to use with start.\n"
	);

}


#endif // ifdef AFX_MIRV_PGL
