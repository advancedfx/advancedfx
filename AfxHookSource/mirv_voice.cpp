#include "stdafx.h"

#include "mirv_voice.h"

#include "addresses.h"
#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvWav.h"

#include <shared/AfxDetours.h>
#include <shared/StringTools.h>

#include <list>
#include <set>
#include <map>
#include <sstream>

std::set<int> m_VoiceBlocks;

namespace SOURCESDK {
namespace CSGO{

class CVoiceChannel;
class INetChannel;

class CNetMessage
{
public:
	// [...]

protected:
	bool				m_bReliable;	// true if message should be send reliable
	INetChannel			*m_NetChannel;	// netchannel this message is from/for
};

class SVC_VoiceData : public CNetMessage
{
	// [...]

public:
	int	m_nFromClient; // client who has spoken
	// [...]

};

} // namespace CSGO{
} // namespace SOURCESDK {

using namespace SOURCESDK::CSGO;

class CMirvVoiceWriter
{
public:
	void OnAfterFrameRenderEnd()
	{
		if (m_Recording)
		{
			if (WrpGlobals * glob = g_Hook_VClient_RenderView.GetGlobals())
			{
				double frameTime = glob->absoluteframetime_get();
				Mix(frameTime);
				m_Time += frameTime;
			}
		}
	}

	void Start(const wchar_t * directoryPath)
	{
		Stop();

		m_Recording = true;
		m_Time = 0 < m_TimePadding ? m_TimePadding : 0;
		m_Directory = directoryPath;
	}

	void Stop()
	{
		m_EntityMixData.clear();

		m_Recording = false;
	}


	void SetEntityIndex(int value)
	{
		m_EntityIndex = value;
	}

	void SupplyData(const byte *data, size_t datalen)
	{
		if (!m_Recording)
			return;

		std::map<int, CEntityMixData>::iterator it = m_EntityMixData.find(m_EntityIndex);
		if (it == m_EntityMixData.end())
		{
			std::wostringstream os;
			os << m_Directory << L"\\entity_" << m_EntityIndex << L".wav";
			std::wstring fileName = os.str();

			it = m_EntityMixData.emplace(std::piecewise_construct, std::forward_as_tuple(m_EntityIndex), std::forward_as_tuple(m_Time, fileName.c_str())).first;
		}

		it->second.AddData((const unsigned short * )data, datalen >> 1);
	}

	void TimePadding_set(double value)
	{
		m_TimePadding = (float)value;
	}

	double TimePadding_get(void)
	{
		return m_TimePadding;
	}

private:
	static const size_t m_OutSampleRate = 22050;
	static const size_t m_VoiceSampleRate = 22050;
	float m_TimePadding = 0.2f;
	bool m_Recording = false;
	int m_EntityIndex = 0;
	double m_Time = 0;
	std::wstring m_Directory;

	class CMixData
	{
	public:
		CMixData(const unsigned short * data, size_t samples)
			: m_Samples(samples)
			, m_Time(0)
		{
			m_Buffer = (unsigned short *)malloc(sizeof(unsigned short)*samples);
			memcpy(m_Buffer, data, sizeof(unsigned short)*samples);
		}

		~CMixData()
		{
			free(m_Buffer);
		}

		bool IsFinished(void)
		{
			return IsFinishedSamplePos(GetSamplePosition(m_Time));
		}

		WORD SampleAdvance(double time)
		{
			WORD result = 0;

			size_t samplePos = GetSamplePosition(m_Time);

			if (!IsFinishedSamplePos(samplePos))
			{
				result = m_Buffer[samplePos];
			}

			m_Time += time;

			return result;
		}

	private:
		double m_Time;
		size_t m_Samples;
		unsigned short * m_Buffer;

		size_t GetSamplePosition(double time)
		{
			double position = time * m_VoiceSampleRate;
			if (0 <= position)
			{
				size_t pos = (size_t)position;
				if (pos < m_Samples)
					return pos;
			}

			return m_Samples;
		}

		bool IsFinishedSamplePos(size_t samplePos)
		{
			return 0 >= m_Samples - samplePos;
		}

	};

	class CEntityMixData
	{
	public:
		CEntityMixData(double time, const wchar_t * fileName)
			: m_Wav(fileName, 1, m_OutSampleRate)
		{
			// New file, add intital silence if requrired:
			if (0 < time)
			{
				size_t samples = (size_t)(time * m_OutSampleRate);

				for (size_t i = 0; i < samples; ++i)
				{
					WORD value = 0;

					m_Wav.Append(1, &value);
				}
			}
		}

		void AddData(const unsigned short * data, size_t samples)
		{
			m_Data.emplace_back(data, samples);
		}

		void Advance(double time)
		{
			time += m_TimeRemainder;

			if (time <= 0)
				return;

			size_t samples = (size_t)(time * m_OutSampleRate);

			double timePerSample = 1 / (double)m_OutSampleRate;

			for (size_t i = 0; i < samples; ++i)
			{
				WORD value = 0;

				if (!m_Data.empty())
				{
					CMixData & data = m_Data.front();

					value = data.SampleAdvance(timePerSample);

					if (data.IsFinished())
						m_Data.pop_front();
				}

				m_Wav.Append(1, &value);

				time -= timePerSample;
			}

			m_TimeRemainder = time;
		}

	private:
		CMirvWav m_Wav;
		std::list<CMixData> m_Data;
		double m_TimeRemainder = 0;
	};

	std::map<int, CEntityMixData> m_EntityMixData;

	void Mix(double frameTime)
	{
		for (std::map<int, CEntityMixData>::iterator it = m_EntityMixData.begin(); it != m_EntityMixData.end(); ++it)
		{
			it->second.Advance(frameTime);
		}
	}

} g_MirvVoiceWriter;

typedef void(__fastcall * CVoiceWriter_AddDecompressedData_t)(void * this_ptr, int edx, CVoiceChannel *ch, const byte *data, size_t datalen);

CVoiceWriter_AddDecompressedData_t g_Old_CVoiceWriter_AddDecompressedData;

void __fastcall New_CVoiceWriter_AddDecompressedData(void * this_ptr, int edx, CVoiceChannel *ch, const byte *data, size_t datalen)
{
	g_MirvVoiceWriter.SupplyData(data, datalen);

	g_Old_CVoiceWriter_AddDecompressedData(this_ptr, edx, ch, data, datalen);
}

typedef bool(__fastcall * CClientState_ProcessVoiceData_t)(void * this_ptr, int edx, SVC_VoiceData *msg);

CClientState_ProcessVoiceData_t g_Old_CClientState_ProcessVoiceData;

bool __fastcall New_CClientState_ProcessVoiceData(void * this_ptr, int edx, SVC_VoiceData *msg)
{
	if (msg && 0 < m_VoiceBlocks.size() && m_VoiceBlocks.find(msg->m_nFromClient + 1) != m_VoiceBlocks.end())
		return true;

	g_MirvVoiceWriter.SetEntityIndex(msg->m_nFromClient + 1);

	return g_Old_CClientState_ProcessVoiceData(this_ptr, edx, msg);
}

bool Hook_CClientState_ProcessVoiceData()
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CClientState_ProcessVoiceData))
	{
		g_Old_CClientState_ProcessVoiceData = (CClientState_ProcessVoiceData_t)DetourApply((BYTE *)AFXADDR_GET(csgo_CClientState_ProcessVoiceData), (BYTE *)New_CClientState_ProcessVoiceData, (int)AFXADDR_GET(csgo_CClientState_ProcessVoiceData_DSZ));

		firstResult = true;
	}

	return firstResult;
}

bool Hook_CVoiceWriter_AddDecompressedData()
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (Hook_CClientState_ProcessVoiceData()
		&& AFXADDR_GET(csgo_CVoiceWriter_AddDecompressedData))
	{
		g_Old_CVoiceWriter_AddDecompressedData = (CVoiceWriter_AddDecompressedData_t)DetourApply((BYTE *)AFXADDR_GET(csgo_CVoiceWriter_AddDecompressedData), (BYTE *)New_CVoiceWriter_AddDecompressedData, (int)AFXADDR_GET(csgo_CVoiceWriter_AddDecompressedData_DSZ));

		firstResult = true;
	}

	return firstResult;
}

void Mirv_Voice_OnAfterFrameRenderEnd(void)
{
	g_MirvVoiceWriter.OnAfterFrameRenderEnd();
}

bool Mirv_Voice_StartRecording(const wchar_t * directoryPath)
{
	bool result = Hook_CVoiceWriter_AddDecompressedData();

	g_MirvVoiceWriter.Start(directoryPath);

	return result;
}

void Mirv_Voice_EndRecording()
{
	g_MirvVoiceWriter.Stop();
}

CON_COMMAND(mirv_voice, "Controls voice data related features.")
{
	int argC = args->ArgC();

	if (2 <= argC)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("block", cmd1))
		{
			if (!Hook_CClientState_ProcessVoiceData())
			{
				Tier0_Warning("Error: Hook not installed.\n");
				return;
			}

			if (3 <= argC)
			{
				char const * cmd2 = args->ArgV(2);

				if (!_stricmp("add", cmd2) && 4 <= argC)
				{
					m_VoiceBlocks.insert(atoi(args->ArgV(3)));
					return;
				}
				else if (!_stricmp("remove", cmd2) && 4 <= argC)
				{
					m_VoiceBlocks.erase(atoi(args->ArgV(3)));
					return;
				}
				else if (!_stricmp("clear", cmd2))
				{
					m_VoiceBlocks.clear();
					return;
				}
				else if (!_stricmp("list", cmd2))
				{
					int i = 0;
					for (std::set<int>::iterator it = m_VoiceBlocks.begin(); it != m_VoiceBlocks.end(); ++it)
					{
						Tier0_Msg("%s%i", 0 == i ? "" : (0 == (i % 10) ? "\n, " : ", "), *it);
						++i;
					}
					if (m_VoiceBlocks.size() <= 0)
					{
						Tier0_Msg("(empty)\n");
					}
					else
					{
						Tier0_Msg("\n");
					}
					return;
				}
			}

			Tier0_Msg(
				"mirv_voice block add <idx> - Add antity index as shown in voice_show_mute command.\n"
				"mirv_voice block remove <idx> - Remove antity index as shown in voice_show_mute command.\n"
				"mirv_voice block clear - Clear all blocks.\n"
				"mirv_voice block list - List current blocks.\n"
			);
			return;
		}

		if (!_stricmp("record", cmd1))
		{
			if (!Hook_CClientState_ProcessVoiceData())
			{
				Tier0_Warning("Error: Hook not installed.\n");
				return;
			}

			if (3 <= argC)
			{
				char const * cmd2 = args->ArgV(2);

				if (!_stricmp("timePadding", cmd2))
				{
					if (4 <= argC)
					{
						g_MirvVoiceWriter.TimePadding_set(atof(args->ArgV(3)));
						return;
					}

					Tier0_Msg(
						"mirv_voice record timePadding <fValue> - Set time padding floating point value (in seconds).\n"
						"Current value: %f\n"
						, g_MirvVoiceWriter.TimePadding_get()
					);
					return;
				}
				else if (!_stricmp("start", cmd2))
				{
					std::wstring directorPath(L"");

					if (4 <= argC)
					{
						if (!UTF8StringToWideString(args->ArgV(3), directorPath))
							Tier0_Warning("Error: Can not convert \"%s\" from UTF-8 to WideString.\n", args->ArgV(3));
					}

					Mirv_Voice_StartRecording(directorPath.c_str());
					return;
				}
				else if (!_stricmp("stop", cmd2))
				{
					Mirv_Voice_EndRecording();
					return;
				}
			}

			Tier0_Msg(
				"mirv_voice record timePadding [...] - Control TimePadding.\n"
				"mirv_voice record start [<sFolderName>] - Start recording voices into <sFolderName> (if given) or the current folder (csgo.exe folder) otherwise.\n"
				"mirv_voice record stop - Stop.\n"
			);
			return;
		}

	}

	Tier0_Msg(
		"mirv_voice block [...] - Blocking of voice data.\n"
		"mirv_voice record [...] - Recording of voice data into individual files, you can also use mirv_streams record voices instead.\n"
	);
}

