#include "stdafx.h"

// Hint: for now commands are registered upon the first client.dll CreateInterface() call

#include <shared/StringTools.h>

#include "RenderView.h"
#include "MirvTime.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "csgo_CHudDeathNotice.h"
#include "csgo_SndMixTimeScalePatch.h"
#include <shared/hooks/gameOverlayRenderer.h>
#include "AfxStreams.h"
#include "addresses.h"
#include "CampathDrawer.h"
#include "csgo_S_StartSound.h"
#include "d3d9Hooks.h"
#include "aiming.h"
#include "../shared/CommandSystem.h"
#include <shared/binutils.h>
#include "csgo/ClientToolsCSgo.h"
#include "csgo_CBasePlayer.h"
#include "csgo_CCSGameMovement.h"
#include "csgo_vphysics.h"
#include "csgo_c_baseentity.h"
#include "csgo_c_baseanimatingoverlay.h"
#include "../shared/FovScaling.h"
//#include "csgo_CDemoFile.h"
#include "csgo_Stdshader_dx9_Hooks.h"
#include "csgo_models_replace.h"
//#include <csgo/sdk_src/public/tier0/memalloc.h>
#include <shared/MirvCampath.h>
#include <shared/AfxDetours.h>
#include "../shared/MirvSkip.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include <sstream>
#include <iomanip>

extern WrpVEngineClient * g_VEngineClient;

HMODULE g_H_EngineDll = 0;


#if AFXSTREAMS_REFTRACKER

CON_COMMAND(__mirv_streams_ref, "")
{
	Tier0_Msg("Current tracked count: %i\n", AfxStreams_RefTracker_Get());
}

#endif

#ifdef _DEBUG
CON_COMMAND(__mirv_dumpmemoryleaks, "")
{
	_CrtDumpMemoryLeaks();
}

CON_COMMAND(__mirv_memdiff, "")
{
	static int state = 0;
	static _CrtMemState s1;
	static _CrtMemState s2;
	_CrtMemState s3;

	switch (state)
	{
	case 0:
		_CrtMemCheckpoint(&s1);
		state = 1;
		break;
	case 1:
		_CrtMemCheckpoint(&s2);
		if (_CrtMemDifference(&s3, &s1, &s2))
			_CrtMemDumpStatistics(&s3);
		state = 2;
		break;
	case 2:
		_CrtMemCheckpoint(&s1);
		if (_CrtMemDifference(&s3, &s2, &s1))
			_CrtMemDumpStatistics(&s3);
		state = 1;
		break;
	}
}

#endif

void PrintInfo();

CON_COMMAND(__mirv_info, "")
{
	PrintInfo();
}


void mirv_test_msg_parallel_func() {
	for (size_t i = 0; i < 150; i++) {
		Sleep(150);
		Tier0_Warning("Thread: %i @ %i\n", i, GetTickCount());
	}
}

CON_COMMAND(__mirv_test_msg_parallel, "") {
	Tier0_Warning("Main: START @ %i\n", GetTickCount());
	std::thread thread(mirv_test_msg_parallel_func);
	for (size_t i = 0; i < 150; i++) {
		Sleep(100);
		Tier0_Warning("Main: %i @ %i\n", i, GetTickCount());
	}
	Tier0_Warning("Main: JOIN @ %i\n", GetTickCount());
	thread.join();
	Tier0_Warning("Main: FINISH @ %i\n", GetTickCount());
}

CON_COMMAND(__mirv_test6, "")
{
	int argc = args->ArgC();

	if (2 > argc)
		return;


	int idx = atoi(args->ArgV(1));
	
	SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(idx);
	SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

	if (be)
	{
		static float old_m_flAnimTime = 0;
		static float old_m_flSimualtionTime = 0;
		static float old_curtime = 0;

		float m_flAnimTime = *(float *)((char *)be +0x25c);
		float m_flSimulationTime = *(float *)((char *)be + 0x264);
		float curtime = g_Hook_VClient_RenderView.GetGlobals()->curtime_get();

		float delta_m_flAnimTime = m_flAnimTime - old_m_flAnimTime;
		float delta_m_flSimualtionTime = m_flSimulationTime - old_m_flSimualtionTime;
		float delta_curtime = curtime - old_curtime;

		bool oneIsNeg = delta_m_flAnimTime < 0 || delta_m_flSimualtionTime < 0 || delta_curtime < 0;

		if (!oneIsNeg)
			Tier0_Msg(
				"%i: animTime=%f (%f), simulationTime=%f (%f) | curTime=%f (%f)\n"
				, idx, m_flAnimTime, delta_m_flAnimTime, m_flSimulationTime, delta_m_flSimualtionTime, curtime, delta_curtime
			);
		else
			Tier0_Warning(
				"%i: animTime=%f (%f), simulationTime=%f (%f) | curTime=%f (%f)\n"
				, idx, m_flAnimTime, delta_m_flAnimTime, m_flSimulationTime, delta_m_flSimualtionTime, curtime, delta_curtime
			);

		old_m_flAnimTime = m_flAnimTime;
		old_m_flSimualtionTime = m_flSimulationTime;
		old_curtime = curtime;
	}
}

CON_COMMAND(__mirv_ct, "")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("debug", cmd1) && 3 <= argc)
		{
			if(CClientToolsCsgo * instance = CClientToolsCsgo::Instance()) instance->DebugEntIndex(atoi(args->ArgV(2)));
		}

	}
}


CON_COMMAND(__mirv_addr, "")
{
	int argc = args->ArgC();

	if (3 <= argc)
	{
		HMODULE hModule = GetModuleHandleA(args->ArgV(1));

		if (!hModule)
		{
			Tier0_Warning("Invalid module name.\n");
			return;
		}

		char const * cmd1 = args->ArgV(2);

		Afx::BinUtils::ImageSectionsReader sections(hModule);
		if (!sections.Eof())
		{
			Afx::BinUtils::MemRange search = sections.GetMemRange();

			Afx::BinUtils::MemRange result;

			int numResults = 0;

			while (!search.IsEmpty())
			{
				result = FindPatternString(search, cmd1);

				if (result.IsEmpty())
					break;

				Tier0_Msg(
					"%i: [0x%08x,0x%08x)\n",
					numResults,
					result.Start - (DWORD)hModule,
					result.End - (DWORD)hModule
				);

				search = Afx::BinUtils::MemRange(result.End, search.End);

				++numResults;

				if (100 < numResults)
				{
					Tier0_Msg("RESULT LIMIT REACHED\n");
					break;
				}
			}
		}
	}
	else
		Tier0_Msg("__mirv_addr <moduleName> <hexPatternString>\n");
}

CON_COMMAND(__mirv_test2, "")
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();

	if(di && g_Hook_VClient_RenderView.GetGlobals())
	{
		Tier0_Msg(
			"GetDemoRecordingTick=%i\n"
			"GetDemoPlaybackTick=%i\n"
			"GetDemoPlaybackStartTick=%i\n"
			"GetDemoPlaybackTimeScale=%f\n"
			"GetDemoPlaybackTotalTicks=%i\n",
			di->GetDemoRecordingTick(),
			di->GetDemoPlaybackTick(),
			di->GetDemoPlaybackStartTick(),
			di->GetDemoPlaybackTimeScale(),
			di->GetDemoPlaybackTotalTicks()
		);

		double curTime = g_Hook_VClient_RenderView.GetGlobals()->curtime_get();
		int client_current_tick = di->GetDemoPlaybackTick();
		double tick_interval = g_Hook_VClient_RenderView.GetGlobals()->interval_per_tick_get();
		double interpolation_amount = g_Hook_VClient_RenderView.GetGlobals()->interpolation_amount_get();

		double result = client_current_tick * tick_interval + interpolation_amount* tick_interval;

		Tier0_Msg(
			"curtime(%f) == client_current_tick(%i) * tick_interval(%f) +interpolation_amount(%f) * tick_interval == %f\n",
			curTime,
			client_current_tick,
			tick_interval,
			interpolation_amount,
			result
		);
	}
}

bool g_bD3D9DebugPrint = false;

CON_COMMAND(__mirv_test, "")
{
	g_bD3D9DebugPrint = true;
}

CON_COMMAND(__mirv_skyboxscale, "print skyboxscale in CS:GO")
{
	if(AFXADDR_GET(csgo_C_BasePlayer_OFS_m_skybox3d_scale) != (AfxAddr)-1) {
		if(auto pLocalPlayer = CClientToolsCsgo::GetLocalPlayer())
		{
			int skyBoxScale = *(int *)((unsigned char *)pLocalPlayer +AFXADDR_GET(csgo_C_BasePlayer_OFS_m_skybox3d_scale));

			Tier0_Msg("skyBoxScale: %i\n", skyBoxScale);
			return;
		}
	}
	Tier0_Msg("skyBoxScale: n/a\n");
}

CON_COMMAND(__mirv_show_renderview_count, "") {
	int argc = args->ArgC();

	if (2 <= argc) {
		g_AfxStreams.Console_ShowRenderViewCountSet(0 != atoi(args->ArgV(1)));
		return;
	}

	Tier0_Msg(
		"%s 0|1\n"
		"Current value: %i\n",
		args->ArgV(0),
		g_AfxStreams.Console_ShowRenderViewCountGet() ? 1 : 0
	);
}

CON_COMMAND(mirv_streams, "Access to streams system.")
{
	bool bIsCsgo = g_SourceSdkVer == SourceSdkVer::SourceSdkVer_CSGO || g_SourceSdkVer == SourceSdkVer_CSCO;
	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if(bIsCsgo && 0 == _stricmp(cmd1, "add"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "normal"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add normal <name> - Add a normal stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "baseFx"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddBaseFxStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add baseFx <name> - Add a baseFx stream with name <name>.\n"
					);
					return;
				}
				else if (!_stricmp(cmd2, "hudWhite"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddHudWhiteStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add hudWhite <name> - Add a hud stream on white background with name <name>.\n"
					);
					return;
				}
				else if (!_stricmp(cmd2, "hudBlack"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddHudBlackStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add hudBlack <name> - Add a hud stream on black background with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "depth"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddDepthStream(cmd3, !(5 <= argc && 0 == _stricmp(args->ArgV(4), "draw")));

						return;
					}

					Tier0_Msg(
						"mirv_streams add depth <name> [draw]- Add a depth stream with name <name>. Draw to force the old method.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "matteWorld"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddMatteWorldStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add matteWorld <name> - Add a matte world stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "depthWorld"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddDepthWorldStream(cmd3, !(5 <= argc && 0 == _stricmp(args->ArgV(4), "draw")));

						return;
					}

					Tier0_Msg(
						"mirv_streams add depthWorld <name> [draw] - Add a depth world stream with name <name>. Draw to force the old method.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "matteEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddMatteEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add matteEnitity <name> - Add a matte entity stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "depthEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddDepthEntityStream(cmd3, !(5 <= argc && 0 == _stricmp(args->ArgV(4), "draw")));

						return;
					}

					Tier0_Msg(
						"mirv_streams add depthEntity <name> [draw] - Add a depth entity stream with name <name>. Draw to force the old method.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "alphaMatteEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaMatteEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaMatteEntity <name> - DEPRECATED: Add an alpha matte entity stream with name <name>.\n"
					);
					return;
				}				
				else
				if(!_stricmp(cmd2, "alphaMatte"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaMatteStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaMatte <name> - DEPRECATED: Add an alpha matte stream with name <name>.\n"
					);
					return;
				}				
				else
				if(!_stricmp(cmd2, "alphaEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaEntity <name> -DEPRECATED:  Add an alpha entity stream with name <name>.\n"
					);
					return;
				}				
				else
				if(!_stricmp(cmd2, "alphaWorld"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaWorldStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaWorld <name> - DEPRECATED: Add an alpha world stream with name <name>.\n"
					);
					return;
				}	
				else if(!_stricmp(cmd2, "matte"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddMatteStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add matte <name> - Add a matte stream with name <name>.\n"
					);
					return;
				}	
			}

			Tier0_Msg(
				"mirv_streams add normal [...] - Add a normal stream.\n"
				"mirv_streams add baseFx [...] - Add a stream that allows effects, but should not look different from normal stream unless you edit its settings.\n"
				//"mirv_streams add hud [...] - Add a HUD stream (this is two streams (hudWhite / hudBlack) combined into one (RGBA)).\n"
				"mirv_streams add hudWhite [...] - Add a HUD stream on white background.\n"
				"mirv_streams add hudBlack [...] - Add a HUD stream on black background.\n"
				"mirv_streams add depth [...] - Add a depth stream.\n"
				"mirv_streams add matteWorld [...] - Add a matte world stream.\n"
				"mirv_streams add depthWorld [...] - Add a depth world stream.\n"
				"mirv_streams add matteEntity [...] - Add a matte entity stream.\n"
				"mirv_streams add depthEntity [...] - Add a depth entity stream.\n"
				"mirv_streams add matte [...] - Add a matte stream (use drawMatte action to set what shall be drawn inside the matte).\n"
			);
			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "edit"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				CSubWrpCommandArgs subArgs(args, 3);

				g_AfxStreams.Console_EditStream(cmd2, &subArgs);
				return;
			}

			Tier0_Msg(
				"mirv_streams edit <streamName> [...] - Edit the stream with name <streamName>, you can get the value from mirv_streams print. <streamName> can match multiple streams when using wildcards (\\* = wildcard and \\\\ = \\), however this will only do something useful when the streams matched are compatible!\n"
			);
			return;
		}
		else if (bIsCsgo && 0 == _stricmp(cmd1, "move"))
		{
			CSubWrpCommandArgs subArgs(args, 2);

			g_AfxStreams.Console_MoveStream(&subArgs);
			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "remove"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);
				
				g_AfxStreams.Console_RemoveStream(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_streams remove <streamName> - Remove a stream with name <streamName>, you can get the name from mirv_streams print.\n"
			);
			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "preview"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (4 <= argc)
				{
					g_AfxStreams.Console_PreviewStream(cmd2, atoi(args->ArgV(3)));
					return;
				}
				
				g_AfxStreams.Console_PreviewStream(cmd2, 0);
				return;
			}

			Tier0_Msg(
				"mirv_streams preview <streamName> [<iSlot>]- Preview the stream with name <streamName>, you can get the name from mirv_streams print. To end previewing enter \"\" (empty string) for <streamName>!\n"
			);
			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "previewEnd"))
		{
			if (3 <= argc)
			{
				g_AfxStreams.Console_PreviewStream("", atoi(args->ArgV(2)));
				return;
			}

			g_AfxStreams.Console_PreviewStream("", -1);
			return;
		}
		else
		if (bIsCsgo && 0 == _stricmp(cmd1, "previewSuspend"))
		{
			if (3 <= argc)
			{
				g_AfxStreams.Console_PreviewSuspend_set(0 != atoi(args->ArgV(2)));
				return;
			}

			Tier0_Msg(
				"mirv_streams previewSuspend 0|1 - If to suspend preview of all streams.\n"
				"Current value: %i\n",
				g_AfxStreams.Console_PreviewSuspend_get() != 0 ? 1 : 0);
			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "print"))
		{
			g_AfxStreams.Console_PrintStreams();

			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "print2"))
		{
			g_AfxStreams.Console_PrintStreams2();

			return;
		}
		else
		if(!_stricmp(cmd1, "record"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "name"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_RecordName_set(cmd3);
						return;
					}

					Tier0_Msg(
						"mirv_streams record name <name> - Set record name to <name>.\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_RecordName_get()
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "start"))
				{
					g_AfxStreams.Console_Record_Start();
					return;
				}
				else
				if(!_stricmp(cmd2, "end"))
				{
					g_AfxStreams.Console_Record_End();
					return;
				}
				else
				if(!_stricmp(cmd2, "format"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_RecordFormat_set(cmd3);
						return;
					}

					Tier0_Msg(
						"mirv_streams record format tga|bmp - Set record format to tga or bmp.\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_RecordFormat_get()
					);
					return;
				}
				else
				if(bIsCsgo && 0 == _stricmp(cmd2, "presentOnScreen"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_PresentRecordOnScreen_set(0 != atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record presentOnScreen 0|1 - Whether to show recording on screen (where possible) (1) [May cause epileptic seizures!] or not (0).\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_PresentRecordOnScreen_get() ? "1" : "0"
					);
					return;
				}
				else if (bIsCsgo && 0 == _stricmp(cmd2, "matPostprocessEnable"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_MatPostprocessEnable_set(atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record matPostprocessEnable <iValue> - Positive value: Force integer value <iValue> for mat_postprocess_enable during recording (changing not recommended).\n"
						"Current value: %i.\n",
						g_AfxStreams.Console_MatPostprocessEnable_get()
					);
					return;
				}
				else if (bIsCsgo && 0 == _stricmp(cmd2, "matDynamicTonemapping"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_MatDynamicToneMapping_set(atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record matDynamicTonemapping <iValue> - Positive value: Force integer value <iValue> for mat_dynamic_tonemapping during recording (changing not recommended).\n"
						"Current value: %i.\n",
						g_AfxStreams.Console_MatDynamicToneMapping_get()
					);
					return;
				}
				else if (bIsCsgo && 0 == _stricmp(cmd2, "matMotionBlurEnabled"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_MatMotionBlurEnabled_set(atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record matMotionBlurEnabled <iValue> - Positive value: Force integer value <iValue> for mat_motion_blur_enabled during recording (changing not recommended).\n"
						"Current value: %i.\n",
						g_AfxStreams.Console_MatMotionBlurEnabled_get()
					);
					return;
				}
				else if(bIsCsgo && 0 == _stricmp(cmd2, "matForceTonemapScale"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_MatForceTonemapScale_set((float)atof(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record matForceTonemapScale <fValue> - Positive value: Force floating point value <fValue> for mat_force_tonemap_scale during recording (can fix random bomb plant brightness if enabled, but breaks auto brightness adjustment).\n"
						"Current value: %f.\n",
						g_AfxStreams.Console_MatForceTonemapScale_get()
					);
					return;
				}
				else if (0 == _stricmp(cmd2, "screen"))
				{
					CSubWrpCommandArgs subArgs(args, 3);

					g_AfxStreams.Console_RecordScreen(&subArgs);
					return;
				}
				else
				if (!_stricmp(cmd2, "startMovieWav"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_StartMovieWav_set(0 != atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record startMovieWav 0|1 - Whether to record WAV audio (1) or not (0).\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_StartMovieWav_get() ? "1" : "0"
					);
					return;
				}
				else if (bIsCsgo && 0 == _stricmp(cmd2, "voices"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_RecordVoices_set(0 != atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record voices 0|1 - Whether to record voice WAV audio into separate files (1) or not (0).\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_RecordVoices_get() ? "1" : "0"
					);
					return;
				}
				else
				if (0 == _stricmp(cmd2, "bvh"))
				{
					CSubWrpCommandArgs subArgs(args, 3);

					g_AfxStreams.Console_Bvh(&subArgs);
					return;
				}
				else if(0 == _stricmp(cmd2,"campath")) {
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						if (!_stricmp("enabled", cmd3))
						{
							if (5 <= argc)
							{
								g_AfxStreams.SetCampathAutoSave(0 != atoi(args->ArgV(4)));
								return;
							}

							Tier0_Msg(
								"mirv_streams record campath enabled 0|1 - Disable (0) or enable (1).\n"
								"Current value: %i\n"
								, g_AfxStreams.GetCampathAutoSave() ? 1 : 0
							);
							return;
						}
					}

					Tier0_Msg(
						"mirv_streams record campath enabled [...]\n"
					);
					return;					
				}
				else
				if (0 == _stricmp(cmd2, "cam"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						if (!_stricmp("enabled", cmd3))
						{
							if (5 <= argc)
							{
								g_AfxStreams.CamExport_set(0 != atoi(args->ArgV(4)));
								return;
							}

							Tier0_Msg(
								"mirv_streams record cam enabled 0|1 - Disable (0) or enable (1).\n"
								"Current value: %i\n"
								, g_AfxStreams.CamExport_get() ? 1 : 0
							);
							return;
						}
					}

					Tier0_Msg(
						"mirv_streams record cam enabled [...]\n"
					);
					return;
				}
				else
				if (!_stricmp(cmd2, "agr"))
				{
					CSubWrpCommandArgs subArgs(args, 3);

					g_AfxStreams.Console_GameRecording(&subArgs);
					return;
				}
				else if (!_stricmp(cmd2, "fps")) {
					if (4 <= argc)
					{
						char const* cmd3 = args->ArgV(3);
						if (0 == _stricmp(cmd3, "default")) {
							g_AfxStreams.SetOverrideFps(false);
							return;
						}
						else if (!StringIsAlphas(cmd3)) {
							g_AfxStreams.SetOverrideFpsValue((float)atof(cmd3));
							g_AfxStreams.SetOverrideFps(true);
							return;
						}
					}

					Tier0_Msg(
						"mirv_streams record fps default|<fValue>\n"
					);
					if (g_AfxStreams.GetOverrideFps()) {
						Tier0_Msg(
							"Current value: %f\n", g_AfxStreams.GetOverrideFpsValue()
						);
					}
					else {
						Tier0_Msg(
							"Current value: default\n"
						);
					}
					return;
				}
			}

			Tier0_Msg(
				"mirv_streams record name [...] - Set/get record name.\n"
				"mirv_streams record start - Begin recording.\n"
				"mirv_streams record end - End recording.\n"
				"mirv_streams record format [...] - Set/get file format.\n"
				"mirv_streams record fps [...] - Allows to override input FPS for games where we can not detect it (not needed for CS:GO).\n"
			);
			if (bIsCsgo) Tier0_Msg(
				"mirv_streams record presentOnScreen [...] - Controls screen presentation during recording.\n"
				"mirv_streams record matPostprocessEnable [...] - Control forcing of mat_postprocess_enable.\n"
				"mirv_streams record matDynamicTonemapping [...] - Control forcing of mat_dynamic_tonemapping.\n"
				"mirv_streams record matMotionBlurEnabled [...] - Control forcing of mat_motion_blur_enabled.\n"
				"mirv_streams record matForceTonemapScale [...] - Control forcing of mat_force_tonemap_scale.\n"
			);
			Tier0_Msg(
				"mirv_streams record screen [...] - Controls capturing the game content drawn to screen right before being presented.\n"
				"mirv_streams record startMovieWav [...] - Controls WAV audio recording.\n"
			);
			if (bIsCsgo) Tier0_Msg(
				"mirv_streams record voices [...] - Controls voice WAV audio recording.\n"
			);
			Tier0_Msg(
				"mirv_streams record bvh [...] - Controls the HLAE/BVH camera motion data capture output.\n"
				"mirv_streams record cam [...] - Controls the camera motion data capture output (can be imported with mirv_camio).\n"
				"mirv_streams record campath [...] - Save current campath into take folder (if not empty).\n"
			);
			Tier0_Msg(
				"mirv_streams record agr [...] - Controls afxGameRecord (.agr) game state recording.\n"
			);
			return;
		}
		else
		if(bIsCsgo && 0 == _stricmp(cmd1, "actions"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "add"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						
						if(!_stricmp(cmd3, "replace"))
						{
							CSubWrpCommandArgs subArgs(args, 4);

							CAfxBaseFxStream::Console_AddReplaceAction(&subArgs);
							return;
						}
						else if (!_stricmp(cmd3, "glowColorMap"))
						{
							CSubWrpCommandArgs subArgs(args, 4);

							CAfxBaseFxStream::Console_AddGlowColorMapAction(&subArgs);
							return;
						}
					}

					Tier0_Msg(
						"mirv_streams actions add replace [...] - Add replace action.\n"
						"mirv_streams actions add glowColorMap [...] - Add glowColorMap action (can be edited after adding it).\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "print"))
				{
					CAfxBaseFxStream::Console_ListActions();
					return;
				}
				else
				if(!_stricmp(cmd2, "remove"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						CAfxBaseFxStream::RemoveAction(CAfxBaseFxStream::CActionKey(cmd3));
						return;
					}
				}
				else if(!_stricmp(cmd2, "edit"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						if (CAfxBaseFxStream::CAction* action = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey(cmd3)))
						{
							CSubWrpCommandArgs subArgs(args, 4);
							action->Console_Edit(&subArgs);
							return;
						}
						Tier0_Warning("No action with key \"%s\" found.\n", cmd3);
						return;
					}
				}
			}
			Tier0_Msg(
				"mirv_streams actions add [...] - Add an action.\n"
				"mirv_streams actions print - Print available actions.\n"
				"mirv_streams actions remove <actionName> - Remove an action named <actionName>.\n"
				"mirv_streams actions edit <actionName> [...] - Edit action named <actionName>.\n"
			);
			return;
		}
		else if (0 == _stricmp("settings", cmd1))
		{
			CSubWrpCommandArgs subArgs(args, 2);
			advancedfx::CRecordingSettings::Console(&subArgs);
			return;
		}
		else if (bIsCsgo && 0 == _stricmp("mainStream", cmd1))
		{
			CSubWrpCommandArgs subArgs(args, 2);
			g_AfxStreams.Console_MainStream(&subArgs);
			return;
		}
	}

	if (bIsCsgo) Tier0_Msg(
		"mirv_streams add [...]- Add a stream.\n"
		"mirv_streams edit [...]- Edit a stream.\n"
		"mirv_streams move [...] - Move a stream in the list.\n"
		"mirv_streams remove [...] - Remove a stream.\n"
		"mirv_streams preview [...] - Preview a stream.\n"
		"mirv_streams previewEnd [<iSlot>] - End preview.\n"
		"mirv_streams previewSuspend [...] - Suspend all previews.\n"
		"mirv_streams print - Print current streams.\n"
		"mirv_streams print2 - Print current streams.\n"
	);
	Tier0_Msg(
		"mirv_streams record [...] - Recording control.\n"
	);
	if (bIsCsgo) Tier0_Msg(
		"mirv_streams actions [...] - Actions control (for baseFx based streams).\n"
	);
	Tier0_Msg(
		"mirv_streams settings [...] - Recording settings.\n"
	);
	if (bIsCsgo) Tier0_Msg(
		"mirv_streams mainStream [...] - Controls which stream is the main stream for caching full-scene state (default is first).\n"
	);
	return;
}

void ReplaceAll(std::string & str, const std::string & from, const std::string & to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

CON_COMMAND(mirv_exec, "command execution")
{
	SOURCESDK::CSGO::ICvar * pCvar = WrpConCommands::GetVEngineCvar_CSGO();
	if (!pCvar)
	{
		Tier0_Warning("Error: No suitable Cvar interface found.\n");
		return;
	}

	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (auto cmd = pCvar->FindCommand(arg1))
		{
			const char **ppargV = (const char **)malloc(sizeof(char *)*(argC - 1));

			std::string * strings = new std::string[argC - 1];

			for (int i = 1; i < argC; ++i)
			{
				std::string & str = strings[i - 1];

				str = args->ArgV(i);

				ReplaceAll(str, "{QUOTE}", "\"");
				ReplaceAll(str, "{QUOTE2}", "{QUOTE}");
				ReplaceAll(str, "{QUOTE3}", "{QUOTE2}");
				ReplaceAll(str, "{QUOTE4}", "{QUOTE3}");
				ReplaceAll(str, "{QUOTE5}", "{QUOTE4}");
				ReplaceAll(str, "{QUOTE6}", "{QUOTE5}");
				ReplaceAll(str, "{QUOTE7}", "{QUOTE6}");
				ReplaceAll(str, "{QUOTE8}", "{QUOTE7}");
				ReplaceAll(str, "{QUOTE9}", "{QUOTE8}");
				ReplaceAll(str, "{QUOTE10}", "{QUOTE9}");
				ReplaceAll(str, "\\{", "{");
				ReplaceAll(str, "\\}", "}");

				ppargV[i-1] = str.c_str();
			}

			SOURCESDK::CSGO::CCommand ccmd(argC - 1, ppargV);

			cmd->Dispatch(ccmd);

			free(ppargV);
			return;
		}
		else
		{
			Tier0_Warning("AFXERROR: Command %s not found.\n", arg1);
			return;
		}
	}

	Tier0_Msg(
		"%s <sCommandName> \"<arg1>\" ... \"<argN>\" - Pass arguments <arg1> to <argN> to command named <sCommandName>,  use {QUOTE} for \", \\{ for {, \\} for }.\n"
		, arg0
	);
}

CON_COMMAND(__mirv_exec, "client command execution: __mirv_exec <as you would have typed here>") {
	unsigned int len=0;
	char *ttt, *ct;

	for(int i=0; i<args->ArgC(); i++)
	{
		len += strlen(args->ArgV(i))+1;
	}

	if(len<1) len=1;

	ct = ttt = (char *)malloc(sizeof(char)*len);

	for(int i=1; i<args->ArgC(); i++) {
		char const * cur = args->ArgV(i);
		unsigned int lcur = strlen(cur);
		
		if(1<i) {
			strcpy(ct, " ");
			ct++;
		}

		strcpy(ct, cur);
		ct += lcur;
	}

	*ct = 0;

	g_VEngineClient->ExecuteClientCmd(ttt);

	free(ttt);
}

bool GetCurrentDemoTick(int &outTick)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();

	if(di)
	{
		outTick = di->GetDemoPlaybackTick();
		return true;
	}

	return false;
}

bool GetDemoTimeFromClientTime(double curTime, double clientTime, double &outDemoTime)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();
	WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();
	if(di && gl)
	{
		int current_tick = di->GetDemoPlaybackTick();
		double tick_interval = gl->interval_per_tick_get();
		outDemoTime = clientTime - (curTime - current_tick * tick_interval);
		return true;
	}
	return false;
}


bool GetDemoTickFromDemoTime(double curTime, double demoTime, int& outTick)
{
	WrpVEngineClientDemoInfoEx* di = g_VEngineClient->GetDemoInfoEx();
	WrpGlobals* gl = g_Hook_VClient_RenderView.GetGlobals();

	if (di && gl)
	{
		double tick_interval = gl->interval_per_tick_get();
		outTick = (int)round(demoTime / tick_interval);

		return true;
	}

	return false;
}

bool GetCurrentDemoTime(double &outDemoTime)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();
	WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();

	if(gl)
	{
		int client_current_tick = di->GetDemoPlaybackTick();

		double tick_interval = gl->interval_per_tick_get();

		double interpolation_amount = gl->interpolation_amount_get();

		double demoTime = (client_current_tick +interpolation_amount) * tick_interval;

		outDemoTime = demoTime;

		return true;
	}

	return false;
}

bool GetDemoTickFromClientTime(double curTime, double targetTime, int& outTick)
{
	double demoTime;
	return GetDemoTimeFromClientTime(curTime, targetTime, demoTime) && GetDemoTickFromDemoTime(curTime, demoTime, outTick);
}

extern class CExecuteClientCmdForCommandSystem : public IExecuteClientCmdForCommandSystem {
public:
    virtual void ExecuteClientCmd(const char * value) {
        if(g_VEngineClient) g_VEngineClient->ClientCmd_Unrestricted(value); // // We don't use ExecuteCliendCmd here, because it might be executed at awkward times that make the engine crash (e.g. when using playdemo).
    }
} g_ExecuteClientCmdForCommandSystem;

class CMirvCampath_Time : public IMirvCampath_Time
{
public:
	virtual double GetTime() {
		return g_MirvTime.GetTime();
	}
	virtual double GetCurTime() {
		return g_MirvTime.GetCurTime();
	}
	virtual bool GetCurrentDemoTick(int& outTick) {
		return ::GetCurrentDemoTick(outTick);
	}
	virtual bool GetCurrentDemoTime(double& outDemoTime) {
		return ::GetCurrentDemoTime(outDemoTime);
	}
	virtual bool GetDemoTickFromDemoTime(double curTime, double time, int& outTick) {
		return ::GetDemoTickFromDemoTime(curTime, time, outTick);
	}
	virtual bool GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime) {
		return ::GetDemoTimeFromClientTime(curTime, time, outDemoTime);
	}
    virtual bool GetDemoTickFromClientTime(double curTime, double targetTime, int& outTick) {
        return ::GetDemoTickFromClientTime(curTime, targetTime, outTick);
    }
} g_MirvCampath_Time;

class CMirvSkip_GotoDemoTick : public IMirvSkip_GotoDemoTick {
	virtual void GotoDemoTick(int tick) {
        std::ostringstream oss;
        oss << "demo_gototick " << tick;
        g_VEngineClient->ExecuteClientCmd(oss.str().c_str());
	}
} g_MirvSkip_GotoDemoTick;

CON_COMMAND(mirv_skip, "for skipping through demos (uses demo_gototick)")
{
    MirvSkip_ConsoleCommand(args, &g_MirvCampath_Time, &g_MirvSkip_GotoDemoTick);
}

class CMirvCampath_Camera : public IMirvCampath_Camera
{
public:
	virtual SMirvCameraValue GetCamera() {
		return SMirvCameraValue(
			g_Hook_VClient_RenderView.LastCameraOrigin[0],
			g_Hook_VClient_RenderView.LastCameraOrigin[1],
			g_Hook_VClient_RenderView.LastCameraOrigin[2],
			g_Hook_VClient_RenderView.LastCameraAngles[0],
			g_Hook_VClient_RenderView.LastCameraAngles[1],
			g_Hook_VClient_RenderView.LastCameraAngles[2],
			g_Hook_VClient_RenderView.LastCameraFov
		);
	}
} g_MirvCampath_Camera;


class CMirvCampath_Drawer : public IMirvCampath_Drawer
{
public:
	virtual bool GetEnabled() {
		return g_CampathDrawer.Draw_get();
	}
	virtual void SetEnabled(bool value) {
		g_CampathDrawer.Draw_set(value);
	}
	virtual bool GetDrawKeyframeAxis() {
		return g_CampathDrawer.GetDrawKeyframeAxis();
	}
	virtual void SetDrawKeyframeAxis(bool value) {
		g_CampathDrawer.SetDrawKeyframeAxis(value);
	}
	virtual bool GetDrawKeyframeCam() {
		return g_CampathDrawer.GetDrawKeyframeCam();
	}
	virtual void SetDrawKeyframeCam(bool value) {
		g_CampathDrawer.SetDrawKeyframeCam(value);
	}

	virtual float GetDrawKeyframeIndex() { return g_CampathDrawer.GetDrawKeyframeIndex(); }
	virtual void SetDrawKeyframeIndex(float value) { g_CampathDrawer.SetDrawKeyframeIndex(value); }

} g_MirvCampath_Drawer;
 
CON_COMMAND(mirv_campath, "camera paths")
{
	if (!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	MirvCampath_ConCommand(args, Tier0_Msg, Tier0_Warning, &g_Hook_VClient_RenderView.m_CamPath, &g_MirvCampath_Time, &g_MirvCampath_Camera, &g_MirvCampath_Drawer);
}

CON_COMMAND(mirv_camexport, "controls camera motion data export") {
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);
		if(0 == _stricmp("stop", arg1))
		{
			g_Hook_VClient_RenderView.ExportEnd();
			return;
		}
		else
		if(0 == _stricmp("start", arg1) && 4 <= argc) {
			char const * fileName = args->ArgV(2);
			double fps = atof(args->ArgV(3));
			if(fps < 0.1f) fps = 0.1f;

			std::wstring wideFileName;
			if(
				!UTF8StringToWideString(fileName, wideFileName)
				|| !g_Hook_VClient_RenderView.ExportBegin(wideFileName.c_str(), 1.0/fps)
			)
				Tier0_Msg("Error: exporting failed.");

			return;
		}
		if(0 == _stricmp("timeinfo", arg1))
		{			
			Tier0_Msg("Current (interpolated client) time: %f\n", g_MirvTime.GetTime());
			return;
		}

	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_camexport start <filename> <fps>\n"
		"mirv_camexport stop\n"
		"mirv_camexport timeinfo\n"
	);
}

CON_COMMAND(mirv_camimport, "controls camera motion data import") {
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);
		if(0 == _stricmp("stop", arg1)) {
			g_Hook_VClient_RenderView.ImportEnd();
			return;
		}
		else
		if(0 == _stricmp("start", arg1) && 3 <= argc) {
			char const * fileName = args->ArgV(2);
			g_Hook_VClient_RenderView.SetImportBaseTime(g_MirvTime.GetTime());

			std::wstring wideFileName;
			if(!UTF8StringToWideString(fileName, wideFileName)
				|| !g_Hook_VClient_RenderView.ImportBegin(wideFileName.c_str())
			)
				Tier0_Msg("Loading failed.");
			return;
		}
		else
		if(0 == _stricmp("basetime", arg1)) {
			if(3 <= argc) {
				char const * newTime = args->ArgV(2);
				if(0 == _stricmp("current", newTime))
					g_Hook_VClient_RenderView.SetImportBaseTime(g_MirvTime.GetTime());
				else
					g_Hook_VClient_RenderView.SetImportBaseTime((float)atof(newTime));
				return;
			}

			Tier0_Msg(
				"Usage:\n"
				"mirv_camimport basetime <newTime> | current\n"
				"Current setting: %f\n",
				g_Hook_VClient_RenderView.GetImportBasteTime()
			);
			return;
		}
		else
		if(0 == _stricmp("toCamPath", arg1) && 4 <= argc)
		{
			char const * arg2 = args->ArgV(2);
			char const * arg3 = args->ArgV(3);

			if(!g_Hook_VClient_RenderView.ImportToCamPath(0 != atoi(arg2), atof(arg3)))
				Tier0_Warning("ERROR: Something went wrong when converting to mirv_campath!\n");
			return;
		}
	}
	Tier0_Msg(
		"Usage:\n"
		"mirv_camimport start <filename>\n"
		"mirv_camimport stop\n"
		"mirv_camimport basetime [...]\n"
		"mirv_camimport toCamPath 0|1 <fov> - Convert to mirv_campath, 0 = no interp adjust | 1 (recommended) = adjust campath interpolation (to linear), <fov> Field Of View value to use (90 recommended).\n"
	);
}

CON_COMMAND(mirv_cvar_unhide_all,"(CS:GO only) removes hidden and development only flags from all cvars.")
{
	SOURCESDK::CSGO::ICvar * pCvar = WrpConCommands::GetVEngineCvar_CSGO();
	if(!pCvar)
	{
		Tier0_Warning("Error: No suitable Cvar interface found.\n");
		return;
	}

	SOURCESDK::CSGO::ICvar::Iterator iter(pCvar);

	int nUnhidden = 0;

	for(iter.SetFirst(); iter.IsValid(); iter.Next())
	{
		SOURCESDK::CSGO::ConCommandBase * cmd = iter.Get();

		if(cmd->IsFlagSet(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN))
			nUnhidden++;

		cmd->RemoveFlags(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
	}

	Tier0_Msg("Removed FCVAR_DEVELOPMENTONLY or FCVAR_HIDDEN from %i ConVars.\n", nUnhidden);
}

CON_COMMAND(mirv_cvar_hack, "")
{
	int argc = args->ArgC();

	if(3 <= argc)
	{
		char const * cvarName = args->ArgV(1);
		float cvarValue = (float)atof(args->ArgV(2));

		WrpConVarRef * cvar = new WrpConVarRef(cvarName);

		cvar->SetDirectHack(cvarValue);

		delete cvar;

		return;
	}

	Tier0_Msg(
		"mirv_cvar_hack <cvarName> <floatValue> - Force value directly, this will only work for true cvars (not cmds) and only for a subset of those, might have unwanted side effects - use with caution!\n"
	);
}

CON_COMMAND(mirv_deathmsg, "controls death notification options")
{
	csgo_CHudDeathNotice_Console(args);
}

CON_COMMAND(mirv_fov,"allows overriding FOV (Field Of View) of the camera")
{
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(!_stricmp(arg1,"default"))
		{
			g_Hook_VClient_RenderView.FovDefault();
			return;
		}
		else
		if(!_stricmp(arg1,"handleZoom"))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if(!_stricmp(arg2, "enabled"))
				{
					if(4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						g_Hook_VClient_RenderView.handleZoomEnabled = 0 != atoi(arg3);
						return;
					}

					Tier0_Msg(
						"Usage:\n"
						"mirv_fov handleZoom enabled 0|1 - Enable (1), disable (0).\n"
						"Current value: %s\n",
						g_Hook_VClient_RenderView.handleZoomEnabled ? "1" : "0"
					);
					return;
				}
				else
				if(!_stricmp(arg2, "minUnzoomedFov"))
				{
					if(4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						if(!_stricmp(arg3, "current"))
						{
							g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov = g_Hook_VClient_RenderView.LastCameraFov;
							return;
						}
						else if (0 == _stricmp("real", arg3) && 5 <= argc)
						{
							char const * arg4 = args->ArgV(4);

							g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov = Auto_InverseFovScaling(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, atof(arg4));
							return;
						}
						else if(StringIsEmpty(arg3) || !StringIsAlphas(arg3))
						{
							g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov = atof(arg3);
							return;
						}
					}

					Tier0_Msg(
						"Usage:\n"
						"mirv_fov handleZoom minUnzoomedFov current - Set current fov as threshold.\n"
						"mirv_fov handleZoom minUnzoomedFov [real] <f> - Set floating point value <f> as threshold.\n"
						"Current value: %f\n",
						g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov
					);
					return;
				}
			}

			Tier0_Msg(
				"Usage:\n"
				"mirv_fov handleZoom enabled [...] - Whether to enable zoom handling (if enabled mirv_fov is only active if it's not below minUnzoomedFov (not zoomed)).\n"
				"mirv_fov handleZoom minUnzoomedFov [...] - Zoom detection threshold.\n"
			);
			return;
		}
		else if (0 == _stricmp("real", arg1) && 3 <= argc)
		{
			char const * arg2 = args->ArgV(2);


			g_Hook_VClient_RenderView.FovOverride(Auto_InverseFovScaling(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, atof(arg2)));
			return;
		}
		if(StringIsEmpty(arg1) || !StringIsAlphas(arg1))
		{
			g_Hook_VClient_RenderView.FovOverride(atof(arg1));
			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_fov [real] <f> - Override fov with given floating point value <f>.\n"
		"mirv_fov default - Revert to the game's default behaviour.\n"
		"mirv_fov handleZoom [...] - Handle zooming (e.g. AWP in CS:GO).\n"
	);
	{
		Tier0_Msg("Current value: ");

		double fovValue;
		if(!g_Hook_VClient_RenderView.GetFovOverride(fovValue))
			Tier0_Msg("default (currently: %f)\n", g_Hook_VClient_RenderView.LastCameraFov);
		else
			Tier0_Msg("%f\n", fovValue);
	}
}

CON_COMMAND(mirv_replace_name, "allows replacing player names")
{
	csgo_ReplacePlayerName_Console(args);
}

CON_COMMAND(mirv_replace_team_name, "allows replacing player team names")
{
	csgo_ReplaceTeamName_Console(args);
}

CON_COMMAND(mirv_input, "Input mode configuration.")
{
	g_Hook_VClient_RenderView.Console_MirvInput(args);
}

CON_COMMAND(mirv_snd_timescale, "(CS:GO only) allows to override host_timescale value for sound system.")
{
	if(!Hook_csgo_SndMixTimeScalePatch())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if(0 == _stricmp("default", arg1))
		{
			csgo_SndMixTimeScalePatch_enable = false;
			return;
		}
		else
		{
			csgo_SndMixTimeScalePatch_enable = true;
			csgo_SndMixTimeScalePatch_value = (float)atof(arg1);
			return;
		}
	}
	Tier0_Msg(
		"Usage:\n"
		"mirv_snd_timescale <fValue> - override sound system host_timescale value with floating point value <fValue>.\n"
		"mirv_snd_timescale default - don't override.\n"
	);
}

CON_COMMAND(mirv_gameoverlay, "GameOverlayRenderer control.")
{
	int argc = args->ArgC();

	if(3 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if( 0 == _stricmp("enabled", arg1)
			|| 0 == _stricmp("enable", arg1))
		{
			bool value = 0 != atoi(args->ArgV(2));
			Tier0_Msg(
				"%s %s.\n",
				value ? "Enable" : "Disable",
				GameOverlay_Enable(value) ? "OK" : "FAILED"
			);
			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_gameoverlay enabled 0|1 - Disable/Enable the GameOverlay (will only do s.th. useful when it was enabled initially).\n"
	);
}

CON_COMMAND(mirv_snd_filter, "Sound control (e.g. blocking sounds).")
{
	if(!csgo_S_StartSound_Install())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("block", arg1))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				csgo_S_StartSound_Block_Add(arg2);
				return;
			}
		}
		else
		if(0 == _stricmp("print", arg1))
		{
			csgo_S_StartSound_Block_Print();
			return;
		}
		else
		if(0 == _stricmp("remove", arg1))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				csgo_S_StartSound_Block_Remove(atoi(arg2));
				return;
			}
		}
		else
		if(0 == _stricmp("clear", arg1))
		{
			csgo_S_StartSound_Block_Clear();
			return;
		}
		else
		if(0 == _stricmp("debug", arg1))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				g_csgo_S_StartSound_Debug = 0 != atoi(arg2);
				return;
			}
			
			Tier0_Msg("Current value: %s\n", g_csgo_S_StartSound_Debug ? "1" : "0");
			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_snd_filter block <mask> - Blocks given <mask> string (for format see below).\n"
		"mirv_snd_filter print - Prints current blocks.\n"
		"mirv_snd_filter remove <index> - Removes the block with index <index> (You can get that from the print sub-command).\n"
		"mirv_snd_filter clear - Clears all blocks.\n"
		"mirv_snd_filter debug 0|1 - Print sounds played into console.\n"
		"<mask> - string to match, where \\* = wildcard and \\\\ = \\\n"
	);
}

typedef struct MirvListEntitiesEntry_s {
	int idx;
	double dist;
	SOURCESDK::C_BaseEntity_csgo * be;
	
	MirvListEntitiesEntry_s(int _idx, double _dist, SOURCESDK::C_BaseEntity_csgo * _be)
		: idx(_idx)
		, dist(_dist)
		, be(_be)
	{

	}

} MirvListEntitiesEntry_t;

bool mirv_listentities_dist_compare(const MirvListEntitiesEntry_t & first, const MirvListEntitiesEntry_t & second)
{
	return first.dist < second.dist;
}

void mirv_print_entity(int index, double dist, SOURCESDK::C_BaseEntity_csgo * be)
{
	const char * className = be->GetClassname();
	if (!className) className = "[NULL]";

	const char * entName = be->GetEntityName();
	if (!entName) entName = "[NULL]";

	Tier0_Msg(
		"%i (%f): %s::%s :%i\n"
		, index
		, dist
		, className
		, entName
		, be->GetRefEHandle().ToInt()
	);

	if (be->IsPlayer() && g_VEngineClient)
	{
		if (SOURCESDK::IVEngineClient_014_csgo * pEngineCsgo = g_VEngineClient->GetVEngineClient_csgo())
		{

			SOURCESDK::player_info_t_csgo pInfo;
			if (pEngineCsgo->GetPlayerInfo(index, &pInfo))
			{
				std::ostringstream oss;

				oss << pInfo.xuid;

				Tier0_Msg(
					"\tplayerInfo.xuid: %s\n"
					"\tplayerInfo.name: %s\n"
					"\tplayerInfo.userID: %i\n"
					"\tplayerInfo.guid: %s\n"
					, oss.str().c_str()
					, pInfo.name
					, pInfo.userID
					, pInfo.guid
				);
			}
		}
	}
}


CON_COMMAND(mirv_listentities, "Print info about currently active entites. (CS:GO only)")
{
	if(!SOURCESDK::g_Entitylist_csgo)
	{
		Tier0_Warning("Not supported for your engine!\n");
		return;
	}

	bool onlyPlayer = false;
	bool sortByDistance = false;
	const char * classWildCard = "\\*";
	
	for (int i = 1; i < args->ArgC(); ++i)
	{
		const char * argI = args->ArgV(i);
		size_t argILen = strlen(argI);

		if (0 == _stricmp(argI, "isPlayer=1"))
		{
			onlyPlayer = true;
		}
		else if (0 == _stricmp(argI, "sort=distance"))
		{
			sortByDistance = true;
		}
		else if (StringIBeginsWith(argI, "class="))
		{
			classWildCard = argI + strlen("class=");
		}
	}

	std::list<MirvListEntitiesEntry_t> result;

	Vector3 cameraOrigin(
		g_Hook_VClient_RenderView.LastCameraOrigin[0],
		g_Hook_VClient_RenderView.LastCameraOrigin[1],
		g_Hook_VClient_RenderView.LastCameraOrigin[2]
	);

	int imax = SOURCESDK::g_Entitylist_csgo->GetHighestEntityIndex();

	for (int i = 0; i <= imax; ++i)
	{
		SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
		SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

		if (be)
		{
		

			if ((!onlyPlayer || be->IsPlayer()))
			{
				const char * className = be->GetClassname();
				if (!className) className = "[NULL]";

				if (StringWildCard1Matched(classWildCard, className))
				{

					SOURCESDK::Vector vEntOrigin = be->GetAbsOrigin();
					Vector3 entOrigin(vEntOrigin.x, vEntOrigin.y, vEntOrigin.z);

					double dist = (entOrigin - cameraOrigin).Length();

					result.emplace_back(i, dist, be);
				}
			}
		}
	}

	if(sortByDistance)
		result.sort(mirv_listentities_dist_compare);

	Tier0_Msg(
		"Results:\n"
		"index (distance): className::entityName :entityHandle\n"
	);


	for(std::list<MirvListEntitiesEntry_t>::iterator it = result.begin(); it != result.end(); ++it)
	{
		int i = it->idx;
		SOURCESDK::C_BaseEntity_csgo * be = it->be;
		double dist = it->dist;

		mirv_print_entity(i, dist, be);
	}

	Tier0_Msg(
		"\n"
		"Usage:\n"
		"mirv_listentities [isPlayer=1] [sort=distance] [class=<wildCardString(\\* = wildcard, \\\\ = \\)>]\n"
	);


}

extern SOURCESDK::CSGO::IEngineTrace * g_pClientEngineTrace;

class CMirvTraceFilter : public SOURCESDK::CSGO::CTraceFilter
{
public:
	CMirvTraceFilter(const SOURCESDK::CSGO::IHandleEntity *passentity);
	virtual bool ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask);

private:
	const SOURCESDK::CSGO::IHandleEntity *m_pPassEnt;
};

CMirvTraceFilter::CMirvTraceFilter(const SOURCESDK::CSGO::IHandleEntity *passedict)
{
	m_pPassEnt = passedict;
}

bool CMirvTraceFilter::ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask)
{
	return pHandleEntity && pHandleEntity != m_pPassEnt;
}

CON_COMMAND(mirv_traceentity, "Trace entity from current view position")
{
	if (!(SOURCESDK::g_Entitylist_csgo && CClientToolsCsgo::Instance() && g_pClientEngineTrace))
	{
		Tier0_Warning("Not supported for your engine / missing hooks,!\n");
		return;
	}

	double forward[3], right[3], up[3];

	double Rx = (g_Hook_VClient_RenderView.LastCameraAngles[0]);
	double Ry = (g_Hook_VClient_RenderView.LastCameraAngles[1]);
	double Rz = (g_Hook_VClient_RenderView.LastCameraAngles[2]);
	
	MakeVectors(Rz, Rx, Ry, forward, right, up);

	double Tx = (g_Hook_VClient_RenderView.LastCameraOrigin[0] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[0] - 0 * right[0] + 0 * up[0]);
	double Ty = (g_Hook_VClient_RenderView.LastCameraOrigin[1] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[1] - 0 * right[1] + 0 * up[1]);
	double Tz = (g_Hook_VClient_RenderView.LastCameraOrigin[2] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[2] - 0 * right[2] + 0 * up[2]);

	SOURCESDK::Vector vecAbsStart;
	vecAbsStart.Init((float)g_Hook_VClient_RenderView.LastCameraOrigin[0], (float)g_Hook_VClient_RenderView.LastCameraOrigin[1], (float)g_Hook_VClient_RenderView.LastCameraOrigin[2]);

	SOURCESDK::Vector vecAbsEnd;
	vecAbsEnd.Init((float)Tx, (float)Ty, (float)Tz);

	SOURCESDK::CSGO::Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);
	
	SOURCESDK::C_BaseEntity_csgo * localPlayer = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(CClientToolsCsgo::Instance()->GetClientToolsInterface()->GetLocalPlayer());
	CMirvTraceFilter traceFilter(localPlayer);

	SOURCESDK::CSGO::trace_t tr;

	g_pClientEngineTrace->TraceRay(ray, SOURCESDK_CSGO_MASK_ALL, &traceFilter, &tr);

	Tier0_Msg(
		"Result:\n"
		"index (distance): className::entityName :entityHandle\n"
	);


	if (tr.DidHit() && tr.m_pEnt)
	{
		SOURCESDK::Vector vecSE;

		SOURCESDK::CSGO::VectorSubtract(tr.endpos, vecAbsStart, vecSE);

		mirv_print_entity(tr.GetEntityIndex(), vecSE.LengthSqr(), tr.m_pEnt);
	}
}

CON_COMMAND(mirv_aim, "Aiming system control.")
{
	if(!g_Hook_VClient_RenderView.GetGlobals())
	{
		Tier0_Warning("Error: Required hooks missing.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("active", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.Active = 0 != atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim active 0|1 - Whether aiming is active (1) or not (0).\n"
				"Current value: %s\n",
				g_Aiming.Active ? "1" : "0"
			);
			return;
		}
		else
		if(0 == _stricmp("softDeactivate", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.SoftDeactivate = 0 != atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim softDeactivate 0|1 - Whether to support soft deactivation (1) or not (0).\n"
				"Current value: %s\n",
				g_Aiming.SoftDeactivate ? "1" : "0"
			);
			return;
		}
		else if (0 == _stricmp("calcVecAng", arg1))
		{
			if (3 <= argc)
			{
				IMirvVecAngCalc * vecAng = g_MirvVecAngCalcs.GetByName(args->ArgV(2));

				if (!vecAng)
					Tier0_Warning("No vecAng calc \"%s\" exists.\n", args->ArgV(2));

				g_Aiming.Source_set(vecAng);

				return;
			}

			IMirvVecAngCalc * vecAng = g_Aiming.Source_get();

			Tier0_Msg(
				"mirv_aim calcVecAng <sClacVecAngName> - Calc to use as source (<sClacVecAngName> is name from mirv_calcs vecAng).\n"
				"Current value: %s\n"
				, vecAng ? "" : "(none)"
			);

			if (vecAng) { vecAng->Console_PrintBegin(); vecAng->Console_PrintEnd(); }
			Tier0_Msg("\n");

			return;
		}
		else if (0 == _stricmp("calcVecAngClear", arg1))
		{
			g_Aiming.Source_set(0);
		}
		else
		if(0 == _stricmp("entityIndex", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.EntityIndex = atoi(arg2);
				g_Aiming.RebuildCalc();
				return;
			}

			Tier0_Msg(
				"mirv_aim entityIndex <n> - Entity index to aim after (use mirv_listentities to get one). Use invalid index (i.e. -1) to deactivate re-targeting.\n"
				"Current value: %i\n"
				, g_Aiming.EntityIndex
			);

			return;
		}
		else
		if(0 == _stricmp("point", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("cam", arg2))
				{
					Vector3 offset(0, 0, 0);

					if(6 <= argc)
					{
						char const * arg3 = args->ArgV(3);
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);

						offset.X = atof(arg3);
						offset.Y = atof(arg4);
						offset.Z = atof(arg5);
					}

					g_Aiming.TargetPoint(
						Vector3(g_Hook_VClient_RenderView.LastCameraOrigin[0],
							g_Hook_VClient_RenderView.LastCameraOrigin[1],
							g_Hook_VClient_RenderView.LastCameraOrigin[2])
						+ offset);
					return;
				}
				else
				if(0 == _stricmp("last", arg2))
				{
					Vector3 offset(0, 0, 0);

					if(6 <= argc)
					{
						char const * arg3 = args->ArgV(3);
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);

						offset.X = atof(arg3);
						offset.Y = atof(arg4);
						offset.Z = atof(arg5);
					}

					g_Aiming.TargetPointFromLast(offset);
					return;
				}
				else
				if(0 == _stricmp("abs", arg2) && 6 <= argc)
				{
					Vector3 offset(0, 0, 0);

					{
						char const * arg3 = args->ArgV(3);
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);

						offset.X = atof(arg3);
						offset.Y = atof(arg4);
						offset.Z = atof(arg5);
					}

					g_Aiming.TargetPoint(offset);
					return;
				}
			}

			Tier0_Msg(
				"mirv_aim point abs <fX> <fY> <fz> - Absolute point in world coordinates.\n"
				"mirv_aim point cam [<fOffsetX> <fOffsetY> <fOffsetZ>] - Aim at current camera position with optional offset in world coordinates.\n"
				"mirv_aim point last [<fOffsetX> <fOffsetY> <fOffsetZ>] - Aim at last target position with optional offset in world coordinates.\n"
			);
			return;
		}
		else
		if(0 == _stricmp("offset", arg1))
		{
			if(5 <= argc)
			{
				char const * arg2 = args->ArgV(2);
				char const * arg3 = args->ArgV(3);
				char const * arg4 = args->ArgV(4);

				g_Aiming.OffSet = Vector3(
					atof(arg2), atof(arg3), atof(arg4)
				);
				return;
			}

			Tier0_Msg(
				"mirv_aim offset <x> <y> <z> - Offset in target local space to aim at (3 floating point values).\n"
				"Current value: %f %f %f\n",
				g_Aiming.OffSet.X, g_Aiming.OffSet.Y, g_Aiming.OffSet.Z
			);
			return;
		}
		else
		if(0 == _stricmp("snapTo", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.SnapTo = 0 != atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim snapTo 0|1 - Whether to aim non-soft (1) or not (0).\n"
				"Current value: %s\n",
				g_Aiming.SnapTo ? "1" : "0"
			);
			return;
		}
		else
		if(0 == _stricmp("velLimit", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.LimitVelocity = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim velLimit <fValue> - Max velocity (in angle degrees per second^1) possible (floating point value).\n"
				"Current value: %f\n",
				g_Aiming.LimitVelocity
			);
			return;
		}
		else
		if(0 == _stricmp("accelLimit", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.LimitAcceleration = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim accelLimit <fValue> - Max acceleration (in angle degrees per second^2) possible (floating point value).\n"
				"Current value: %f\n",
				g_Aiming.LimitAcceleration
			);
			return;
		}
/*
		else
		if(0 == _stricmp("jerkLimit", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.LimitJerk = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim jerkLimit <fValue> - Max jerk (in angle degrees per second^3) possible (floating point value).\n"
				"Current value: %f\n",
				g_Aiming.LimitJerk
			);
			return;
		}
*/
		else
		if(0 == _stricmp("origin", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("net", arg2))
				{
					g_Aiming.Origin = Aiming::O_Net;
					g_Aiming.RebuildCalc();
					return;
				}
				else
				if(0 == _stricmp("view", arg2))
				{
					g_Aiming.Origin = Aiming::O_View;
					g_Aiming.RebuildCalc();
					return;
				}
			}

			char const * curValue = "[unknown]";
			switch(g_Aiming.Origin)
			{
			case Aiming::O_Net:
				curValue = "net";
				break;
			case Aiming::O_View:
				curValue = "view";
				break;
			}

			Tier0_Msg(
				"mirv_aim origin net|view - Target origin to use.\n"
				"Current value: %s\n",
				curValue
			);
			return;
		}
		else
		if(0 == _stricmp("angles", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("net", arg2))
				{
					g_Aiming.Angles = Aiming::A_Net;
					g_Aiming.RebuildCalc();
					return;
				}
				else
				if(0 == _stricmp("view", arg2))
				{
					g_Aiming.Angles = Aiming::A_View;
					g_Aiming.RebuildCalc();
					return;
				}
			}

			char const * curValue = "[unknown]";
			switch(g_Aiming.Angles)
			{
			case Aiming::A_Net:
				curValue = "net";
				break;
			case Aiming::A_View:
				curValue = "view";
				break;
			}

			Tier0_Msg(
				"mirv_aim angles net|view - Target angles to use.\n"
				"Current value: %s\n",
				curValue
			);
			return;
		}
		else
		if(0 == _stricmp("up", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("input", arg2))
				{
					g_Aiming.Up = Aiming::U_Input;
					return;
				}
				else
				if(0 == _stricmp("world", arg2))
				{
					g_Aiming.Up = Aiming::U_World;
					return;
				}
			}

			char const * curValue = "[unknown]";
			switch(g_Aiming.Up)
			{
			case Aiming::U_Input:
				curValue = "input";
				break;
			case Aiming::U_World:
				curValue = "world";
				break;
			}

			Tier0_Msg(
				"mirv_aim up input|world - Camera up direction to use.\n"
				"Current value: %s\n",
				curValue
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_aim active [...] - Whether aiming is active.\n"
		"mirv_aim softDeactivate [...] - Whether to support soft deactivation (for snapTo 0).\n"
		"mirv_aim calcVecAng [...] - Source for target (overrides entityIndex, point, origin, angles).\n"
		"mirv_aim calcVecAngClear - Clears source for target (no target) (overrides entityIndex, point, origin, angles).\n"
		"mirv_aim entityIndex [...] - Entity index to aim after (use mirv_listentities to get one).\n"
		"mirv_aim point [...] - Point to aim after.\n"
		"mirv_aim origin [...] - Target origin to use.\n"
		"mirv_aim angles [...] - Target angles to use.\n"
		"mirv_aim offset [...] - Offset in target space to aim at.\n"
		"mirv_aim up [...] - How to determine the camera up direction.\n"
		"mirv_aim snapTo [...] - Whether to aim non-soft or soft.\n"
		"mirv_aim velLimit [...] - Max velocity possible (for snapTo 0).\n"
		"mirv_aim accelLimit [...] - Max acceleration possible (for snapTo 0).\n"
		// does not work atm // "mirv_aim jerkLimit [...] - Max jerk possible (for snapTo 0).\n"
	);
}

extern class CommandSystem g_CommandSystem;

CON_COMMAND(mirv_cmd, "Command system (for scheduling commands).")
{
	g_CommandSystem.Console_Command(args);
}

extern int g_iForcePostDataUpdateChanged;
extern bool g_b_Suppress_csgo_engine_Do_CCLCMsg_FileCRCCheck;
extern bool Install_csgo_engine_Do_CCLCMsg_FileCRCCheck();

extern float g_Mirv_Pov_Interp_OrgFac[2];
extern float g_Mirv_Pov_Interp_PingFac[2];
extern float g_Mirv_Pov_Interp_Offset[2];

extern float g_panorama_fps;


bool g_bBlockHostError = false;

typedef void (*Host_Error_t)(const char * fmt, ...);
Host_Error_t g_Org_HostError = nullptr;
void New_HostError(const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buff[1024];
	vsnprintf(buff,1023,fmt,args);
	buff[1023] = 0;
	advancedfx::Warning("Suppressing Host_Error: %s\n",buff);
	va_end(args);
}

CON_COMMAND(mirv_fix, "Various fixes")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("physicsMaxFps", cmd1))
		{
			if (!Hook_csgo_vphsyics_frametime_lowerlimit())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				csgo_vphysics_SetMaxFps(atof(cmd2));
				return;
			}

			Tier0_Msg(
				"mirv_fix physicsMaxFps <floatFPSval> - Set the FPS limit for physics.\n"
				"Current value: %f\n",
				csgo_vphysics_GetMaxFps()
			);
			return;
		}
		else
			if (!_stricmp("blockObserverTarget", cmd1))
		{
			if (!Hook_csgo_C_BasePlayer_RecvProxy_ObserverTarget())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				g_csgo_Block_C_BasePlayer_RecvProxy_ObserverTarget = 0 != atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_fix blockObserverTarget 0|1 - Fixes unwanted player switching e.g. upon bomb plant (blocks C_BasePlayer::RecvProxy_ObserverTarget).\n"
				"Current value: %i\n",
				g_csgo_Block_C_BasePlayer_RecvProxy_ObserverTarget ? 1 : 0
			);
			return;
		}
		else
		if (!_stricmp("oldDuckFix", cmd1))
		{
			if (!Hook_csgo_CCSGameMovement_DuckFix())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				g_Enable_csgo_CCSGameMovement_DuckFix = 0 != atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_fix oldDuckFix 0|1 - Can fix player stuck in duck for old demos.\n"
				"Current value: %i\n",
				g_Enable_csgo_CCSGameMovement_DuckFix ? 1 : 0
			);
			return;
		}
		else
		if (!_stricmp("playerAnimState", cmd1))
		{
			if (!Hook_csgo_PlayerAnimStateFix())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				Enable_csgo_PlayerAnimStateFix_set(atoi(cmd2));

				if(args->ArgC() >= 4)
					g_csgo_mystique_annimation_factor = (float)atof(args->ArgV(3));

				return;
			}

			Tier0_Msg(
				"mirv_fix playerAnimState 0|1|2|3- Fixes twitching of player arms, see https://github.com/advancedfx/advancedfx/wiki/Source%%3ASmoother-Demos , 0 - disabled, 1 - enabled, 2/3 - debug.\n"
				"Current value: %i\n",
				Enable_csgo_PlayerAnimStateFix_get()
			);
			return;
		}
		/*
		else if (0 == _stricmp("demoIndexTicks", cmd1))
		{
			if (!Hook_csgo_DemoFile())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				g_bFixCDemoFileTicks = atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_fix demoIndexTicks <iValue> - Set to 1 or greater to enable indexing (sane values are 1920 ticks or greater), set 0 or less to disable.\n"
				"Current value: %i\n",
				g_bFixCDemoFileTicks
			);
			return;
		}
		*/
		else if (0 == _stricmp("selectedPlayerGlow", cmd1) && 3 == argc)
		{
			if (!AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS))
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			MdtMemBlockInfos mdtInfos;

			MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS), 7 * sizeof(unsigned char), &mdtInfos);

			int iArg2 = atoi(args->ArgV(2));

			if (2 == iArg2)
			{
				// Always glow selected.
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 0) = 0x90;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 1) = 0x90;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 5) = 0x90;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 6) = 0x90;
			}
			else if (0 != iArg2)
			{
				// Normal mode.
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 0) = 0x75;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 1) = 0x05;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 5) = 0x74;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 6) = 0x16;
			}
			else
			{
				// Never glow slected.
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 0) = 0x90;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 1) = 0x90;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 5) = 0xEB;
				*((unsigned char*)AFXADDR_GET(csgo_GlowCurrentPlayer_JMPS) + 6) = 0x16;
			}

			MdtMemAccessEnd(&mdtInfos);

			return;
		}
		else if (0 == _stricmp("forcePostDataUpdateChanged", cmd1))
		{
			if (3 <= argc)
			{
				g_iForcePostDataUpdateChanged = atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"mirv_fix forcePostDataUpdateChanged -1|<iEntityIndex>\n"
				"Current value: %i\n"
				, g_iForcePostDataUpdateChanged);

			return;
		}
		else if (0 == _stricmp("forceDoAnimationEvents", cmd1))
		{
			if (3 <= argc)
			{
				if (AFXADDR_GET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ)) {

					bool bEnable = 0 != atoi(args->ArgV(2));

					MdtMemBlockInfos mdtInfos;

					MdtMemAccessBegin((LPVOID)AFXADDR_GET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ), 2 * sizeof(unsigned char), &mdtInfos);

					if (bEnable)
					{
						*((unsigned char*)AFXADDR_GET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ) + 0) = 0x90;
						*((unsigned char*)AFXADDR_GET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ) + 1) = 0x90;
					}
					else
					{
						*((unsigned char*)AFXADDR_GET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ) + 0) = 0x75;
						*((unsigned char*)AFXADDR_GET(csgo_client_C_TEPlayerAnimEvent_PostDataUpdate_NewModelAnims_JNZ) + 1) = 0x14;
					}

					MdtMemAccessEnd(&mdtInfos);

					return;
				}
				else {
					Tier0_Warning("Error: Required hooks not installed.\n");
					return;
				}
			}
		}
		else if (0 == _stricmp("suppressFileCRCCheck", cmd1))
		{
			if(!Install_csgo_engine_Do_CCLCMsg_FileCRCCheck()) {
				Tier0_Warning("Error: Missing hooks.\n");
			} else if(argc < 3) {
				Tier0_Msg("mirv_fix suppressFileCRCCheck 0|1\nCurrent Value: %i\n", g_b_Suppress_csgo_engine_Do_CCLCMsg_FileCRCCheck?1:0);
			} else {
				g_b_Suppress_csgo_engine_Do_CCLCMsg_FileCRCCheck = 0 != atoi(args->ArgV(2));
			}

			return;
		}
		else if (!_stricmp("panoramaTiming", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);
				if(0 == _stricmp("factor", cmd2) && 4 <= argc) {
					float value = (float)atof(args->ArgV(3));
					if(0 < value)
						g_panorama_fps = -value;
					else
						Tier0_Warning("Error: <fFactor> must be greater than 0.\n");
					return;
				}
				else if(0 == _stricmp("framerate", cmd2) && 4 <= argc) {
					float value = (float)atof(args->ArgV(3));
					if(0 < value)
						g_panorama_fps = value;
					else
						Tier0_Warning("Error: <fFrameRateOrFps> must be greater than 0.\n");
					return;
				}
				else if(0 == _stricmp("disable", cmd2)) {
					g_panorama_fps = 0;
					return;
				}
				else if(0 == _stricmp("1", cmd2)) {
					g_panorama_fps = -1;
					return;
				}
				else if(0 == _stricmp("0", cmd2)) {
					g_panorama_fps = 0;
					return;
				}
			}

			Tier0_Msg(
				"mirv_fix panoramaTiming 0 - Shortcut for mirv_fix panoramaTiming disable.\n"
				"mirv_fix panoramaTiming 1 - Shortcut for mirv_fix panoramaTiming factor 1.\n"
				"mirv_fix panoramaTiming factor <fFactor> - Use realtime multiplied with <fFactor>.\n"
				"mirv_fix panoramaTiming framerate <fFrameRateOrFps> - Fix Panorama UI timing (default: 1).\n"
				"mirv_fix panoramaTiming disable - Restore original broken behavior.\n"
				"Default: factor 1\n"
				"Current value: "
			);
			if(0 == g_panorama_fps)
				Tier0_Msg("disable");
			else if(g_panorama_fps < 0) {
				Tier0_Msg("factor %f", -g_panorama_fps);
			}
			else {
				Tier0_Msg("framerate %f", g_panorama_fps);
			}
			Tier0_Msg("\n");
			return;
		}
		else if (0 == _stricmp("suppressHostError", cmd1) && 3 <= argc)
		{
			if (!AFXADDR_GET(engine_HostError))
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}
			
			bool isOn = 0 != atoi(args->ArgV(2));
			bool wasOn = nullptr != g_Org_HostError;

			if(wasOn != isOn) {
				if(isOn) {
					g_Org_HostError = (Host_Error_t)AFXADDR_GET(engine_HostError);
					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());
					DetourAttach(&(PVOID&)g_Org_HostError, New_HostError);
					if(NO_ERROR != DetourTransactionCommit()) {
						advancedfx::Warning("DetourAttach failed\n");
						g_Org_HostError = nullptr;
					}
				} else if(g_Org_HostError) {
					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());
					DetourDetach(&(PVOID&)g_Org_HostError, New_HostError);
					if(NO_ERROR != DetourTransactionCommit()) {
						advancedfx::Warning("DetourDetach failed\n");
					}
					g_Org_HostError = nullptr;
				}
			}

			return;
		}
	}

	Tier0_Msg(
		"mirv_fix physicsMaxFps [...] - Can raise the FPS limit for physics (e.g. rag dolls, so they don't freeze upon high host_framerate).\n"
		"mirv_fix blockObserverTarget [...] - Fixes unwanted player switching, e.g. upon bomb plant (blocks C_BasePlayer::RecvProxy_ObserverTarget).\n"
		"mirv_fix oldDuckFix [...] - Can fix player stuck in duck for old demos.\n"
		"mirv_fix playerAnimState [...] - Fixes twitching of player arms/legs, see https://github.com/advancedfx/advancedfx/wiki/Source%3ASmoother-Demos\n"
		//"mirv_fix demoIndexTicks [...] - Tries to make backward skipping faster in demos.\n"
		"mirv_fix selectedPlayerGlow 0|1|2 - 1: Game default, 2: Always glow, 0 : never glow.\n"
		"mirv_fix forcePostDataUpdateChanged [...].\n"
		"mirv_fix forceDoAnimationEvents 0|1 - Only useful in combination with replacing old models with new ones for forcing animation events to be played, defaut is 0 (off).\n"
		"mirv_fix suppressFileCRCCheck 0|1 - This is only useful with HLAE special builds and it's on by default.\n"
	);
	Tier0_Msg(
		"mirv_fix panoramaTiming [...]\n"
	);
	if(AFXADDR_GET(engine_HostError)) Tier0_Msg(
		"mirv_fix suppressHostError 0|1"
	);
}

extern SOURCESDK::CSGO::vgui::IPanel * g_pVGuiPanel_csgo;
extern SOURCESDK::CSGO::vgui::ISurface *g_pVGuiSurface_csgo;

bool MirvFindVPanel(SOURCESDK::CSGO::vgui::VPANEL panel, char const * panelName, SOURCESDK::CSGO::vgui::VPANEL * outPanel)
{
	if (!outPanel)
		return false;

	if (!strcmp(panelName, g_pVGuiPanel_csgo->GetName(panel)))
	{
		*outPanel = panel;
		return true;
	}			

	for (int i = 0; i < g_pVGuiPanel_csgo->GetChildCount(panel); ++i)
	{
		if (MirvFindVPanel(g_pVGuiPanel_csgo->GetChild(panel, i), panelName, outPanel))
			return true;
	}

	return false;
}

bool MirvVPanelSetVisible(char const * panelName, bool visible)
{
	SOURCESDK::CSGO::vgui::VPANEL panel;

	if (MirvFindVPanel(g_pVGuiSurface_csgo->GetEmbeddedPanel(), panelName, &panel))
	{
		g_pVGuiPanel_csgo->SetVisible(panel, visible);
		return true;
	}

	return false;
}

void MirvVPanelOnCommand(SOURCESDK::CSGO::vgui::Panel * panel, char const * command)
{
	int * vtable = *(int**)panel;

	void * onCommand = (void *)vtable[96];

	__asm push command
	__asm mov ecx, panel
	__asm call onCommand
}

void MirvDoVPanelOnCommand(char const * panelName, char const * destinationModule, char const * panelCommand)
{
	if (!(g_pVGuiSurface_csgo && g_pVGuiPanel_csgo))
	{
		Tier0_Warning("Error: Missing dependencies.\n");
		return;
	}

	SOURCESDK::CSGO::vgui::VPANEL vpanel;

	if (MirvFindVPanel(g_pVGuiSurface_csgo->GetEmbeddedPanel(), panelName, &vpanel))
	{
		SOURCESDK::CSGO::vgui::Panel * panel = g_pVGuiPanel_csgo->GetPanel(vpanel, destinationModule);
		if (panel)
			MirvVPanelOnCommand(panel, panelCommand);
		else
			Tier0_Warning("Error: GetPanel failed for %s\n", panelName);
	}
	else
		Tier0_Warning("Error: Invalid panel name %s\n", panelName);
}

CON_COMMAND(mirv_vpanel, "VGUI Panel access")
{
	if (!(g_pVGuiSurface_csgo && g_pVGuiPanel_csgo))
	{
		Tier0_Warning("Error: Missing dependencies.\n");
		return;
	}

	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);


		if (!_stricmp("hide", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (!MirvVPanelSetVisible(cmd2, false))
				{
					Tier0_Warning("Error: Invalid panel name %s\n", cmd2);
				}		
				return;
			}

			Tier0_Msg(
				"mirv_vpanel hide <panelName>\n"
			);
			return;
		}
		else
		if (!_stricmp("show", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (!MirvVPanelSetVisible(cmd2, true))
				{
					Tier0_Warning("Error: Invalid panel name %s\n", cmd2);
				}
				return;
			}

			Tier0_Msg(
				"mirv_vpanel show <panelName>\n"
			);
			return;
		}
		else if (!_stricmp("command", cmd1))
		{
			if (5 <= argc)
			{
				char const * panelName = args->ArgV(2);
				char const * panelModule = args->ArgV(3);
				char const * panelCommand = args->ArgV(4);

				MirvDoVPanelOnCommand(panelName, panelModule, panelCommand);
				return;
			}

			Tier0_Msg(
				"mirv_vpanel command <panelName> <sModule> <comand> - Execute <command> on panel with name <panelName> in module <sModule> (options are case-sensitive, <sModule> can e.g. be BaseUI or ClientDLL).\n"
				"Example: mirv_vpanel command DemoUIPanel BaseUI pause\n"
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_vpanel hide [...]\n"
		"mirv_vpanel show [...]\n"
		"mirv_vpanel command [...]\n"
		"Hint: To find panel names use vgui_drawtree 1.\n"
	);
}

CON_COMMAND(mirv_camio, "New camera motion data import / export.")
{
	g_Hook_VClient_RenderView.Console_CamIO(args);
}

CON_COMMAND(mirv_loadlibrary, "Load a DLL.")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		std::wstring wCmd1;
		if (UTF8StringToWideString(cmd1, wCmd1))
		{

			if (0 != LoadLibraryW(wCmd1.c_str()))
			{
				Tier0_Msg("LoadLibraryA OK.\n");
			}
			else
			{
				Tier0_Warning("LoadLibraryA failed.\n");
			}
		}
		else
		{
			Tier0_Warning("Failed to convert \"%s\" from UFT8 to UTF-16.\n", cmd1);
		}

		return;
	}

	Tier0_Msg(
		"mirv_loadlibrary <sDllFilePath> - Load DLL at given path.\n"
	);
}

CON_COMMAND(mirv_time, "time control")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * cmd1 = args->ArgV(1);

		if (0 == _stricmp("mode", cmd1))
		{
			if(3 <= argc)
			{
				const char * cmd2 = args->ArgV(2);

				g_MirvTime.SetMode(0 == _stricmp("resumePaused", cmd2) ? CMirvTime::TimeMode_ResumePaused : CMirvTime::TimeMode_CurTime);
				return;
			}

			Tier0_Msg(
				"mirv_time mode curTime|resumePaused - Time mode, curTime pauses when the demo pauses, resumePaused resumes even then. Default is curTime.\n"
				"Current value: %s\n"
				, CMirvTime::TimeMode_ResumePaused == g_MirvTime.GetMode() ? "resumePaused" : "curTime"
			);
			return;
		}
		else if (0 == _stricmp("pausedTime", cmd1))
		{
			if (3 <= argc)
			{
				const char * cmd2 = args->ArgV(2);

				g_MirvTime.SetPausedTime((float)atof(cmd2));
				return;
			}

			Tier0_Msg(
				"mirv_time pausedTime <fSeconds> - Set currently added paused time.\n"
				"Current value: %f\n"
				, g_MirvTime.GetPausedTime()
			);
			return;
		}
		else if (0 == _stricmp("drive", cmd1))
		{
			if (3 <= argc)
			{
				const char* cmd2 = args->ArgV(2);

				if (0 == _stricmp("default", cmd2))
				{
					g_MirvTime.SetDriveTimeEnabled(false);
				}
				else
				{

					float value = (float)atof(cmd2);
					float minValue = AFX_MATH_EPS;
					float maxValue = 1.0f / AFX_MATH_EPS;

					if (value < minValue)
					{
						value = minValue;
						Tier0_Warning("AFXWARNING: Limiting to minimum: %f\n.", minValue);
					}
					else if (maxValue < value)
					{
						value = maxValue;
						Tier0_Warning("AFXWARNING: Limiting to maximum: %f\n.", maxValue);
					}

					g_MirvTime.SetDriveTimeFactor(value);
					g_MirvTime.SetDriveTimeEnabled(true);
				}
				return;
			}

			Tier0_Msg(
				"mirv_time drive <fFactor> - Set the time drive factor.\n"
				"mirv_time drive default - Disable time drive.\n"
				"Current value: "
			);
			if (g_MirvTime.GetDriveTimeEnabled()) Tier0_Msg("%f\n", g_MirvTime.GetDriveTimeFactor());
			else Tier0_Msg("default\n");
			return;
		}
	}

	Tier0_Msg(
		"mirv_time mode [...]\n"
		"mirv_time pausedTime [...]\n"
		"mirv_time drive [...]\n"
	);
}

void Mirv_Pov_Interp_CompensateLatencyOn();
void Mirv_Pov_Interp_CompensateLatencyOff();
void Mirv_Pov_Interp_Default();

extern bool g_csgo_spectatortools_extend_overviewmap;

CON_COMMAND(mirv_cfg, "general HLAE configuration")
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if(0 == _stricmp("mirvForceSpectatorToolsMapOverviewShowAll", arg1))
		{
			if (3 <= argC)
			{
				g_csgo_spectatortools_extend_overviewmap = 0 != atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s mirvForceSpectatorToolsMapOverviewShowAll 0|1\n"
				"Current value: %i\n"
				, arg0
				, g_csgo_spectatortools_extend_overviewmap ? 1 : 0
			);
			return;
		}
		else if (0 == _stricmp("fovScaling", arg1))
		{
			CSubWrpCommandArgs subArgs(args, 2);
			Console_MirvFovScaling(&subArgs);
			return;
		}
		else if (0 == _stricmp("forceViewOverride", arg1))
		{
			if (3 <= argC)
			{
				g_Hook_VClient_RenderView.ForceViewOverride = 0 != atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s forceViewOverride 0|1\n"
				"Current value: %i\n"
				, arg0
				, g_Hook_VClient_RenderView.ForceViewOverride ? 1 : 0
			);
			return;
		}
		else if (0 == _stricmp("forceViewOverrideHltv", arg1))
		{
			if (3 <= argC)
			{
				g_Hook_VClient_RenderView.ForceViewOverrideHltv = 0 != atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s forceViewOverrideHltv 0|1\n"
				"Current value: %i\n"
				, arg0
				, g_Hook_VClient_RenderView.ForceViewOverrideHltv ? 1 : 0
			);
			return;
		}
		else if (0 == _stricmp("viewOverrideReset", arg1))
		{
			if (3 <= argC)
			{
				g_Hook_VClient_RenderView.ViewOverrideReset = 0 != atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s viewOverrideReset 0|1\n"
				"Current value: %i\n"
				, arg0
				, g_Hook_VClient_RenderView.ViewOverrideReset ? 1 : 0
			);
			return;
		}
		else if(0 == _stricmp("mirvPov", arg1)) {
			if(3 <= argC) {
				const char * cmd2 = args->ArgV(2);
				
				if(0 == _stricmp("interpDefault", cmd2)) {
					Mirv_Pov_Interp_Default();
					return;
				}
				else if(0 == _stricmp("interpCompensateLatencyOn", cmd2)) {
					Mirv_Pov_Interp_CompensateLatencyOn();
					return;
				}
				else if(0 == _stricmp("interpCompensateLatencyOff", cmd2)) {
					Mirv_Pov_Interp_CompensateLatencyOff();
					return;
				}
				else if(4 <= argC) {
					const char * cmd3 = args->ArgV(3);

					int iDim = 0 == _stricmp("local", cmd3) ? 1 : 0;
					if(iDim ||0 == _stricmp("other", cmd3))  {
						if(0 == _stricmp("interpPingFac", cmd2)) {
							if(5 <= argC)
							{
								g_Mirv_Pov_Interp_PingFac[iDim] = (float)atof(args->ArgV(4));
								return;
							}

							Tier0_Msg(
								"%s interpPingFac pingFac %s <fValue>\n"
								"Current value: %f\n"
								, arg0
								, cmd3
								, g_Mirv_Pov_Interp_PingFac[iDim]
							);
							return;
						}
						else if(0 == _stricmp("interpOffset", cmd2)) {
							if(5 <= argC)
							{
								g_Mirv_Pov_Interp_Offset[iDim] = (float)atof(args->ArgV(4));
								return;
							}

							Tier0_Msg(
								"%s mirvPovInterp interpOffset %s <fValue>\n"
								"Current value: %f\n"
								, arg0
								, cmd3
								, g_Mirv_Pov_Interp_Offset[iDim]
							);
							return;
						}
						else if(0 == _stricmp("interpFacOrg", cmd2)) {
							if(5 <= argC)
							{
								g_Mirv_Pov_Interp_OrgFac[iDim] = (float)atof(args->ArgV(4));
								return;
							}

							Tier0_Msg(
								"%s mirvPovInterp interpFacOrg %s <fValue>\n"
								"Current value: %f\n"
								, arg0
								, cmd3
								, g_Mirv_Pov_Interp_OrgFac[iDim]
							);
							return;
						}
					}
				}
			}
			
			Tier0_Msg(
				"%s mirvPov interpDefault - Restore default interp settings.\n"
				"%s mirvPov interpCompensateLatencyOn - Enable (approximate) latency compensation interp settings.\n"
				"%s mirvPov interpCompensateLatencyOff - Disable (approximate) latency compensation interp settings.\n"
				"%s mirvPov interpPingFac local|other [...] - Default: local=1 and other=0. Factor multiplied with ping / 1000.0 to delay interp for non-POV player-local entities.\n"
				"%s mirvPov interpOffset local|other [...] - Default: local=0 and other=0. Constant factor to add to interp.\n"
				"%s mirvPov interpFacOrg local|other [...] - Default: local=1 and other=1. Factor multiplied with original value by engine (depends on your cl_interp value!).\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
	}

	Tier0_Msg(
		"%s fovScaling [...] - Set default fov scaling.\n"
		"%s forceViewOverride [...] - If to force the view override onto the local player, can fix a few bugs (CS:GO only).\n"
		"%s forceViewOverrideHltv [...] - If to force the view override onto the HLTVCamera (e.g. for GOTV). (CS:GO only).\n"
		"%s viewOverrideReset [...] - If to reset roll to 0 and fov to 90 (unscaled) after ending a view override (CS:GO only).\n"
		"%s mirvForceSpectatorToolsMapOverviewShowAll [...] - If to extend map overivew in mirv_force_spectatortools.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
	Tier0_Msg(
		"%s mirvPov [...] - Tampers with the interp to get more accurate POV view on average.\n"
		, arg0
	);

}

CON_COMMAND(mirv_guides, "Draw guides on screen (CS:GO).")
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (3 <= argC)
	{
		const char * arg1 = args->ArgV(1);
		const char * arg2 = args->ArgV(2);

		if (0 == _stricmp("enabled", arg2))
		{
			if (0 == _stricmp("phiGrid", arg1))
			{
				if (4 <= argC)
				{
					g_AfxStreams.DrawPhiGrid = 0 != atoi(args->ArgV(3));
					return;
				}

				Tier0_Msg(
					"%s phiGrid enabled 0|1\n"
					"Current value: %i\n"
					, arg0
					, g_AfxStreams.DrawPhiGrid ? 1 : 0
				);
				return;
			}
			else if (0 == _stricmp("ruleOfThirds", arg1))
			{
				if (4 <= argC)
				{
					g_AfxStreams.DrawRuleOfThirds = 0 != atoi(args->ArgV(3));
					return;
				}

				Tier0_Msg(
					"%s ruleOfThirds enabled 0|1\n"
					"Current value: %i\n"
					, arg0
					, g_AfxStreams.DrawRuleOfThirds ? 1 : 0
				);
				return;
			}
		}
	}

	Tier0_Msg(
		"%s phiGrid enabled [...]\n"
		"%s ruleOfThirds enabled [...]\n"
		, arg0
		, arg0
	);
}

extern bool csgo_CNetChan_ProcessMessages_Install(void);

CON_COMMAND(mirv_models, "model tools") {
	if (!g_CCsgoModelsReplace.InstallHooks()) {
		Tier0_Warning("AFXERROR: Missing hooks.\n");
		return;
	}

	int argC = args->ArgC();
	const char* arg0 = args->ArgV(0);

	if (2 <= argC) {
		const char* arg1 = args->ArgV(1);

		if (0 == _stricmp("replace", arg1)) {

			if (3 <= argC) {
				const char* arg2 = args->ArgV(2);

				if (0 == _stricmp("byWcName", arg2)) {

					if (4 <= argC) {
						const char* arg3 = args->ArgV(3);

						if (0 == _stricmp(arg3, "add") && 6 <= argC) {
							g_CCsgoModelsReplace.Add(args->ArgV(4), args->ArgV(5), 7 <= argC ? (0 != atoi(args->ArgV(6))) : true);
							return;
						}
						else if (0 == _stricmp(arg3, "remove") && 5 <= argC) {
							g_CCsgoModelsReplace.Remove(atoi(args->ArgV(4)));
							return;
						}
						else if (0 == _stricmp(arg3, "move") && 6 <= argC) {
							g_CCsgoModelsReplace.MoveIndex(atoi(args->ArgV(4)), atoi(args->ArgV(5)));
							return;
						}
						else if (0 == _stricmp(arg3, "print")) {
							g_CCsgoModelsReplace.Print();
							return;
						}
						else if (0 == _stricmp(arg3, "clear")) {
							g_CCsgoModelsReplace.Clear();
							return;
						}
						else if (0 == _stricmp(arg3, "debug")) {
							if (5 <= argC) {
								g_CCsgoModelsReplace.SetDebug(atoi(args->ArgV(4)));
								return;
							}

							Tier0_Msg(
								"%s replace byWcName debug 0|1\n"
								"Current value: %i\n",
								arg0, g_CCsgoModelsReplace.GetDebug() ? 1 : 0
							);
							return;
						}
					}

					Tier0_Msg(
						"%s replace byWcName add <sWildCardNameSource> <sNameTarget> [<iTransparent=0|1>] - Add a replacement, <iTransparent> defaults to 1 if omitted and tries to hide the replacement a bit more from the engine.\n"
						"%s replace byWcName remove <iIndex>\n"
						"%s replace byWcName move <iSourceIndex> <iTargetIndex>\n"
						"%s replace byWcName print\n"
						"%s replace byWcName clear\n"
						"%s replace byWcName debug [...]\n"
						"Wildcard strings: \\* = wildcard and \\\\ = \\\n"
						, arg0
						, arg0
						, arg0
						, arg0
						, arg0
						, arg0
					);

					return;
				}
			}

			Tier0_Msg(
				"%s replace byWcName [...] - Replace by wildcard string name.\n"
				, arg0
			);

			return;
		}
	}

	Tier0_Msg(
		"%s replace [...] - Replaces models (be aware of model caching, might need restart of game is already loaded!)\n"
		, arg0
	);
}

extern SOURCESDK::C_BaseEntity_csgo * GetSpectatorPlayerForKey(int key);

CON_COMMAND(mirv_spec_player_key, "Spectate player by spectatorslot index") {

	int argC = args->ArgC();
	const char* arg0 = args->ArgV(0);

	if (2 <= argC) {
		if(SOURCESDK::C_BaseEntity_csgo * be = GetSpectatorPlayerForKey(atoi(args->ArgV(1)))) {
			std::string str("spec_player ");
			str.append(std::to_string(be->entindex()));
			g_VEngineClient->ExecuteClientCmd(str.c_str());
		}
		return;
	}
	Tier0_Msg(
		"%s <iKeyNumber> - Spectate player for given spectator slot key number.\n"
		, arg0
	);	
}
