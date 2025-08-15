#include "MirvFix.h"
#include "MirvTime.h"
#include "RenderSystemDX11Hooks.h"

#undef min
#undef max

#include <mutex>
#include <algorithm>

MirvFix g_MirvFix;

typedef double (* Plat_FloatTime_t)(void);
double new_cs2_tier0_Plat_FloatTime(void);

CAfxImportFuncHook<Plat_FloatTime_t> g_Import_SceneSystem_tier0_Plat_FloatTime("Plat_FloatTime", &new_cs2_tier0_Plat_FloatTime);
CAfxImportDllHook g_Import_SceneSystem_tier0("tier0.dll", CAfxImportDllHooks({ &g_Import_SceneSystem_tier0_Plat_FloatTime }));
CAfxImportsHook g_Import_SceneSystem(CAfxImportsHooks({ &g_Import_SceneSystem_tier0 }));

CAfxImportFuncHook<Plat_FloatTime_t> g_Import_panorama_tier0_Plat_FloatTime("Plat_FloatTime", &new_cs2_tier0_Plat_FloatTime);
CAfxImportDllHook g_Import_panorama_tier0("tier0.dll", CAfxImportDllHooks({ &g_Import_panorama_tier0_Plat_FloatTime }));
CAfxImportsHook g_Import_panorama(CAfxImportsHooks({ &g_Import_panorama_tier0 }));

std::mutex g_Plat_FloatTime_mutex;

double new_cs2_tier0_Plat_FloatTime(void) {
    std::unique_lock<std::mutex> lock(g_Plat_FloatTime_mutex);

    static bool firstCall = true;
    static double lastTime = 0.0;
	static double lastTimeResult = 0.0;
    static int lastFrameCount = 0;
	static double frameTime = 0.0;

    double new_time = ((Plat_FloatTime_t)g_Import_SceneSystem_tier0_Plat_FloatTime.TrueFunc)(); 
    double delta = new_time - lastTime;

	bool auto_fps = MirvFix::Time::Mode::AUTO == g_MirvFix.time.mode && AfxStreams_IsRcording();

	if (
		g_MirvFix.time.enabled
		&& ( auto_fps || MirvFix::Time::Mode::USER == g_MirvFix.time.mode ) 
	) 
	{
		auto frame_count = g_MirvTime.framecount_get();
		if (
			(auto_fps || 0 < g_MirvFix.time.value) // drive by FPS
			&& 0 != frame_count // we are actually counting FPS and not in other awkward loading state
		) {
			// The code is a bit tricky, since we must not return a 0 delta time or panorama will go apeshit,
			/// but also not progress too much or too few time.
			if(frame_count != lastFrameCount) {
				lastTimeResult += frameTime; // catch up
				frameTime = auto_fps ? g_MirvTime.frametime_get() : 1.0f / g_MirvFix.time.value;
				lastTime = new_time;
				lastFrameCount = frame_count;
			}
			double ratio = std::min(std::max(0.0,frameTime ? delta / frameTime : 0.0), 1.0);
			double useTime = ratio * frameTime * 0.6180340;
			frameTime -= useTime;
			lastTimeResult += useTime;
		}
		else if (0 > g_MirvFix.time.value) {
			lastTimeResult += frameTime + delta * -g_MirvFix.time.value;
			frameTime = 0;
			lastTime = new_time;
			lastFrameCount = 0;
		}
		else {
			lastTimeResult += frameTime +delta;
			frameTime = 0;
			lastTime = new_time;
			lastFrameCount = 0;
		}
	}
	else {
		lastTimeResult += delta;
		lastTime = new_time;
		lastFrameCount = 0;
	}

    return lastTimeResult;
}

void* g_pCAnimGraphGameSystem = 0;

typedef void* (__fastcall * g_CreateAnimGraphSystem_t)(void * This);
g_CreateAnimGraphSystem_t g_orgCreateAnimGraphSystem = nullptr;

void* g_newCreateAnimGraphSystem (void* This)
{
	g_pCAnimGraphGameSystem = g_orgCreateAnimGraphSystem(This);
	return g_pCAnimGraphGameSystem;
}

typedef u_char* (__fastcall * g_updateAnimGraph_t)(void * This, int bClientSide);
g_updateAnimGraph_t g_orgUpdateAnimGraph = nullptr;

void updateAnimGraph() {
	if (g_orgUpdateAnimGraph && g_pCAnimGraphGameSystem && g_MirvFix.fixAnimations) {
		g_orgUpdateAnimGraph(g_pCAnimGraphGameSystem, 1);
	}
}

void HookFixClient(HMODULE clientDll)
{
	if(void ** vtable = (void **)Afx::BinUtils::FindClassVtable(clientDll, ".?AV?$CGameSystemReallocatingFactory@VCAnimGraphGameSystem@@V1@@@", 0, 0)) {
		MdtMemBlockInfos mbis;
		MdtMemAccessBegin((LPVOID)&(vtable[3]), sizeof(void*), &mbis);
		g_orgCreateAnimGraphSystem = (g_CreateAnimGraphSystem_t)vtable[3];
		vtable[3] = &g_newCreateAnimGraphSystem;
		MdtMemAccessEnd(&mbis);
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));

	// 26th in vtable for CAnimGraphGameSystem, has "AnimGraph Client Tick"
	size_t g_orgUpdateAnimGraph_addr = getAddress(clientDll, "89 54 24 ?? 55 53 57 48 8D AC 24 ?? ?? ?? ?? B8");
	if(g_orgUpdateAnimGraph_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
	}
	g_orgUpdateAnimGraph = (g_updateAnimGraph_t)(g_orgUpdateAnimGraph_addr);
}

CON_COMMAND(mirv_fix, "Various fixes")
{
	int argc = args->ArgC();
	auto arg0 = args->ArgV(0);
	if (2 <= argc)
	{
		auto arg1 = args->ArgV(1);
		if (0 == _stricmp("time", arg1))
		{
			auto arg2 = args->ArgV(2);
			if (3 <= argc)
			{
				if (0 == _stricmp("enabled", arg2)) {
					if (4 <= argc) {
						g_MirvFix.time.enabled = 0 != atoi(args->ArgV(3));
						return;
					}
					advancedfx::Message(
						"%s time enabled <0|1> - Enable (1) or disable (0) fix (default: 1).\n"
						"Current value: %d\n"
						, arg0, g_MirvFix.time.enabled 
					);
					return;
				} else if (0 == _stricmp("factor", arg2) && 4 <= argc) {
					float value = (float)atof(args->ArgV(3));
					if(0 < value)
						g_MirvFix.time.value = -value;
					else
						advancedfx::Warning("Error: <fFactor> must be greater than 0.\n");
					return;
				} else if (0 == _stricmp("framerate", arg2) && 4 <= argc) {
					float value = (float)atof(args->ArgV(3));
					if(0 < value)
						g_MirvFix.time.value = value;
					else
						advancedfx::Warning("Error: <fFps> must be greater than 0.\n");
					return;
				} else if (0 == _stricmp("mode", arg2)) {
					if (4 <= argc) {
						auto arg3 = args->ArgV(3);
						if (0 == _stricmp("auto", arg3)) {
							g_MirvFix.time.mode = MirvFix::Time::Mode::AUTO;
						} else if (0 == _stricmp("user", arg3)) {
							g_MirvFix.time.mode = MirvFix::Time::Mode::USER;
						}
						return;
					}
					advancedfx::Message(
						"%s time mode auto|user - Set mode (default: auto).\n"
						"Current value: %s\n"
						, arg0, g_MirvFix.time.mode == MirvFix::Time::Mode::AUTO ? "auto" : "user"
					);
					return;
				}
			}

			advancedfx::Message(
				"%s time enabled <0|1> - Enable (1) or disable (0) fix (default: 1).\n"
				"%s time factor <fFactor> - Use realtime multiplied with <fFactor>.\n"
				"%s time framerate <fFps> - Use time based on fps.\n"
				"%s time mode <auto|user> - Set mode (default: auto).\n"
				"\"auto\" means it will calculate time from recording settings and will apply only on recording.\n"
				, arg0, arg0, arg0, arg0
			);
			if (MirvFix::Time::Mode::AUTO == g_MirvFix.time.mode) {
				advancedfx::Message("Current value: auto\n");
			} else if (0 > g_MirvFix.time.value) {
				advancedfx::Message("Current value: factor %f\n", -g_MirvFix.time.value);
			} else {
				advancedfx::Message("Current value: framerate %f\n", g_MirvFix.time.value);
			}

			return;
		} else if (0 == _stricmp("animations", arg1)) {
			if (3 == argc) {
				g_MirvFix.fixAnimations = 0 != atoi(args->ArgV(2));
				return;
			}
			advancedfx::Message(
				"%s %s 0|1 - Enable/disable fix for smooth animations (Default: 0).\n"
				"Current value: %i\n"
				, arg0, arg1, g_MirvFix.fixAnimations ? 1 : 0
			);

			return;
		}
	}
	advancedfx::Message(
		"%s time [...] - Apply various time fixes (panorama and scene system).\n"
		"%s animations 0|1 - Enable/disable fix for smooth animations (Default: 0).\n", arg0, arg0
	);
}
