#include "stdafx.h"

#include <windows.h>
#include <gl/gl.h>
#include <shared/AfxDetours.h>
#include <hlsdk.h>
#include <deps/release/Detours/src/detours.h>
#include <deps/release/halflife/common/r_studioint.h>

#include "../../../hl_addresses.h"
#include "../../HookHw.h"
#include "../../hw/Host_Frame.h"
#include "../../../cmdregister.h"

#include "ViewmodelAnimationFix.h"

REGISTER_CVAR(cstrike_force_knife_righthand, "0", 0);

pfnEngSrc_pfnWeaponAnim_t g_detoured_pfnWeaponAnim = NULL;

typedef void(__cdecl* UnkCstrikeEV_FireM4A1Fn_t)(event_args_t *args);
UnkCstrikeEV_FireM4A1Fn_t g_pfnCstrike_EV_FireM4A1_Hooked_Func = NULL;

typedef void(__cdecl* UnkCstrikeEV_FireUSPFn_t)(event_args_t* args);
UnkCstrikeEV_FireUSPFn_t g_pfnCstrike_EV_FireUSP_Hooked_Func = NULL;

extern playermove_s* ppmove;
extern engine_studio_api_s* pEngStudio;

WeaponState weaponStatePrimary = WeaponState::stateDefault;
WeaponState weaponStateSecondary = WeaponState::stateDefault;

bool forceWeaponStatePrimary = false;
bool forceWeaponStateSecondary = false;
char* previousWeapon = nullptr;
bool viewmodelChanged = false;
bool animationFixEnabled = false;

std::vector<ModelSequence> cachedModelSequences;

const std::vector<AnimationSequence> AnimationActions {
	{ AnimationType::animError, "error" },
	{ AnimationType::animNone, "none"  },
	{ AnimationType::animReload, "reload" },
	{ AnimationType::animThrow, "throw" },
	{ AnimationType::animKnifeAttack, "shoot_knife" },
	{ AnimationType::animPlant, "shoot_c4"},
	{ AnimationType::animDrawGrenade, "aim_grenade" },
	{ AnimationType::animDrawGun,"aim" },
};

std::vector<std::string> GetModelSequenceStrings(void* pmodel)
{
	std::vector<std::string> sequences;

	studiohdr_t* pstudiohdr;
	pstudiohdr = (studiohdr_t*)pmodel;

	// return cached model sequence
	int cachedSize = cachedModelSequences.size();
	for (int i = 0; i < cachedSize; i++)
		if (strncmp(pstudiohdr->name, cachedModelSequences[i].modelName, sizeof(char)*64) == 0)
			return cachedModelSequences[i].sequences;

	// create model sequence if its not already cached
	mstudioseqdesc_t* pseqdesc;
	pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
		sequences.push_back(pseqdesc[i].label);

	ModelSequence modelSequence;
	strncpy_s((char*)modelSequence.modelName, sizeof(modelSequence.modelName), (char*)pstudiohdr->name, sizeof(modelSequence.modelName));
	modelSequence.sequences = sequences;
	cachedModelSequences.push_back(modelSequence);

	return sequences;
}

bool isSilencedWeapon()
{
	model_s* viewmodel = pEngfuncs->GetViewModel()->model;
	void* filePtr = pEngStudio->Mod_Extradata(viewmodel);
	bool isSilencedWeapon = false;

	std::vector<std::string> modelStrings = GetModelSequenceStrings(filePtr);

	int modelStringsSize = modelStrings.size();
	for (int i = 0; i < modelStringsSize; i++)
	{
		if (strstr(modelStrings[i].c_str(), "_unsil"))
		{
			isSilencedWeapon = true;
			break;
		}
	}

	return isSilencedWeapon;
}

WeaponType getSilencedWeaponType()
{
	model_s* viewmodel = pEngfuncs->GetViewModel()->model;
	void* filePtr = pEngStudio->Mod_Extradata(viewmodel);
	bool isSilencedWeapon = false;

	std::vector<std::string> modelStrings = GetModelSequenceStrings(filePtr);

	int modelStringsSize = modelStrings.size();
	for (int i = 0; i < modelStringsSize; i++)
	{
		if (strstr(modelStrings[i].c_str(), "_unsil"))
		{
			isSilencedWeapon = true;
			break;
		}
	}

	if (!isSilencedWeapon)
		return WeaponType::typeNotSilenced;

	if (strstr(viewmodel->name, "m4a1"))
		return WeaponType::typePrimary;

	if (strstr(viewmodel->name, "usp"))
		return WeaponType::typeSecondary;

	return WeaponType::typeError;
}

WeaponState getWeaponState()
{
	switch (getSilencedWeaponType())
	{
		default:
			return WeaponState::stateDefault;
		case WeaponType::typePrimary:
			return weaponStatePrimary;
		case WeaponType::typeSecondary:
			return weaponStateSecondary;
	}
}

int ApplyWeaponStateToSequence(int sequence, WeaponState state)
{
	model_s* viewmodel = pEngfuncs->GetViewModel()->model;
	void* filePtr = pEngStudio->Mod_Extradata(viewmodel);

	std::vector<std::string> modelStrings = GetModelSequenceStrings(filePtr);
	std::string sequenceString = modelStrings[sequence];

	// not an m4a1/usp
	if (!isSilencedWeapon())
		return sequence;

	switch (state)
	{
		case WeaponState::stateSilenced:
			if (strstr(sequenceString.c_str(), "_unsil"))
				sequenceString.replace(sequenceString.find("_unsil"), 6, "");
			break;
		case WeaponState::stateUnsilenced:
			// only add if sequence string doesn't already contain _unsil
			if (!strstr(sequenceString.c_str(), "_unsil"))
				sequenceString += "_unsil";
			break;
		default:
			return sequence;
	}

	int modelStringsSize = modelStrings.size();
	for (auto i = 0; i < modelStringsSize; i++)
		if (_stricmp(modelStrings[i].c_str(), sequenceString.c_str()) == 0)
			return i;

	return -1;
}

int AnimationLookupSequence(const char* label, cl_entity_t* entity)
{
	void* filePtr = pEngStudio->Mod_Extradata(entity->model);
	int animationSequence = -1;

	std::vector<std::string> modelStrings = GetModelSequenceStrings(filePtr);

	int modelStringsSize = modelStrings.size();
	for (auto i = 0; i < modelStringsSize; i++)
	{
		if (strstr(label, modelStrings[i].c_str()))
		{
			WeaponType weaponType = getSilencedWeaponType();
			animationSequence = ApplyWeaponStateToSequence(i, getWeaponState());
		}
	}

	return animationSequence;
}

AnimationType GetAnimationAction(int sequence, cl_entity_s* entity)
{
	if (pEngfuncs->GetViewModel()->model == nullptr)
		return animNone;

	void* filePtr = pEngStudio->Mod_Extradata(entity->model);
	AnimationType anim = AnimationType::animNone;

	std::vector<std::string> modelStrings = GetModelSequenceStrings(filePtr);
	std::string sequenceName = modelStrings[sequence];

	int animationActionsSize = AnimationActions.size();
	for (auto i = 0; i < animationActionsSize; i++)
	{
		if (strstr(sequenceName.c_str(), AnimationActions[i].action.c_str()))
		{
			anim = AnimationActions[i].type;
			break;
		}
	}

	return anim;
}

void ViewmodelPlayAnimation(int sequence)
{
	g_detoured_pfnWeaponAnim(sequence, 0);
}

void Touring_pfnWeaponAnim(int sequence, int idx)
{
	if (previousWeapon != pEngfuncs->GetViewModel()->model->name)
		viewmodelChanged = true;

	g_detoured_pfnWeaponAnim(sequence, idx);
}

void ForceWeaponState(int* param, WeaponState state, WeaponType type)
{
	switch (type)
	{
		default:
			break;
		case WeaponType::typePrimary:
			if (forceWeaponStatePrimary)
			{
				*param = state - WeaponState::stateUnsilenced;
				return;
			}

			weaponStatePrimary = static_cast<WeaponState>(*param + stateUnsilenced);
			break;
		case WeaponType::typeSecondary:
			if (forceWeaponStateSecondary)
			{
				*param = state - WeaponState::stateUnsilenced;
				return;
			}

			weaponStateSecondary = static_cast<WeaponState>(*param + stateUnsilenced);
			break;
	}
}

// https://github.com/thomaseichhorn/cs16-client/blob/d09d4ace1e3978b6fce244e463d69c935f813d05/cl_dll/events/event_m4a1.cpp#L55
void __cdecl EV_FireM4A1(event_args_t* args)
{
	if (pEngfuncs->IsSpectateOnly() && (args->entindex == pEngfuncs->GetViewModel()->index))
		ForceWeaponState(&args->bparam1, weaponStatePrimary, WeaponType::typePrimary);

	g_pfnCstrike_EV_FireM4A1_Hooked_Func(args);
}

// https://github.com/thomaseichhorn/cs16-client/blob/d09d4ace1e3978b6fce244e463d69c935f813d05/cl_dll/events/event_usp.cpp#L69
void __cdecl EV_FireUSP(event_args_t* args)
{
	if (pEngfuncs->IsSpectateOnly() && (args->entindex == pEngfuncs->GetViewModel()->index))
		ForceWeaponState(&args->bparam2, weaponStateSecondary, WeaponType::typeSecondary);
	
	g_pfnCstrike_EV_FireUSP_Hooked_Func(args);
}

void ApplyViewmodelAnimationFix()
{
	if (!animationFixEnabled || !pEngfuncs->IsSpectateOnly())
		return;

	cl_entity_t* viewmodel = pEngfuncs->GetViewModel();
	static bool switchedPlayers = false;
	static int previousSpectatedEntity = 0;
	static int previousSequence = 0;

	static cvar_s* righthand = nullptr;
	if (righthand == nullptr)
		righthand = pEngfuncs->pfnGetCvarPointer("cl_righthand");

	if (viewmodel->model == nullptr)
		return;

	if (previousSpectatedEntity != viewmodel->index) {
		switchedPlayers = true;
		viewmodelChanged = false;
		righthand->value = 1;

		// force idle animation of pistol / m4a1 to be silenced or not
		if (isSilencedWeapon())
			ViewmodelPlayAnimation(AnimationLookupSequence((getWeaponState() == WeaponState::stateSilenced) ? "idle" : "idle_unsil", viewmodel));
	}

	previousSpectatedEntity = viewmodel->index;
	cl_entity_t* currentSpectatedEntity = pEngfuncs->GetEntityByIndex(viewmodel->index);
	
	if (currentSpectatedEntity != nullptr && currentSpectatedEntity->player) 
	{
		AnimationType anim = GetAnimationAction(currentSpectatedEntity->curstate.sequence, currentSpectatedEntity);

		if (cstrike_force_knife_righthand->value && (viewmodelChanged || switchedPlayers)) {
			char* viewmodelString = viewmodel->model->name;
			
			if (!strstr(viewmodelString, "shield"))
				righthand->value = strstr(viewmodelString, "knife") ? 0.0f : 1.0f;
		}

		if (previousSequence != currentSpectatedEntity->curstate.sequence) {
			switch (anim)
			{
				default:
					break;
				case AnimationType::animReload:
					ViewmodelPlayAnimation(AnimationLookupSequence("reload", viewmodel));
					break;
				case AnimationType::animThrow:
					ViewmodelPlayAnimation(AnimationLookupSequence("throw", viewmodel));
					break;
				case AnimationType::animPlant:
					ViewmodelPlayAnimation(AnimationLookupSequence("pressbutton", viewmodel));
					break;
				case AnimationType::animKnifeAttack:
					ViewmodelPlayAnimation(AnimationLookupSequence("midslash1", viewmodel));
					break;
			}
		}

		if (viewmodelChanged && !switchedPlayers)
		{
			bool isGrenade = (strstr(viewmodel->model->name, "grenade") || strstr(viewmodel->model->name, "flashbang"));
			ViewmodelPlayAnimation(AnimationLookupSequence(isGrenade ? "deploy" : "draw", viewmodel));
		}

		previousWeapon = viewmodel->model->name;
		previousSequence = currentSpectatedEntity->curstate.sequence;
		switchedPlayers = false;
		viewmodelChanged = false;
	}
}

REGISTER_CMD_FUNC(cstrike_hltv_animation_fix)
{
	int argc = pEngfuncs->Cmd_Argc();

	if (2 <= argc)
	{
		char const* arg1 = pEngfuncs->Cmd_Argv(1);

		if (0 == _stricmp("enabled", arg1))
		{
			char const* arg2 = pEngfuncs->Cmd_Argv(2);
			animationFixEnabled = atoi(arg2);

			if (argc >= 2)
				pEngfuncs->Con_Printf("enabled: %d\n", animationFixEnabled);

			return;
		}

		if (0 == _stricmp("primary", arg1))
		{
			if (3 <= argc)
			{
				char const* arg2 = pEngfuncs->Cmd_Argv(2);

				if ((atoi(arg2) <= 2) && (atoi(arg2) >= 0))
					weaponStatePrimary = (WeaponState)atoi(arg2);

				(atoi(arg2) > 0) ? forceWeaponStatePrimary = true : forceWeaponStatePrimary = false;

				if (animationFixEnabled && pEngfuncs->IsSpectateOnly() && (pEngfuncs->GetViewModel()->model != nullptr))
				{
					cl_entity_t* viewmodel = pEngfuncs->GetViewModel();
					ViewmodelPlayAnimation(AnimationLookupSequence((weaponStatePrimary == WeaponState::stateSilenced) ? "idle" : "idle_unsil", viewmodel));
				}
			}

			if (2 <= argc)
				pEngfuncs->Con_Printf("primary weapon state: %d\n", weaponStatePrimary);

			return;
		}

		if (0 == _stricmp("secondary", arg1))
		{
			if (3 <= argc)
			{
				char const* arg2 = pEngfuncs->Cmd_Argv(2);

				if ((atoi(arg2) <= 2) && (atoi(arg2) >= 0))
					weaponStateSecondary = (WeaponState)atoi(arg2);

				(atoi(arg2) > 0) ? forceWeaponStateSecondary = true : forceWeaponStateSecondary = false;

				if (animationFixEnabled && pEngfuncs->IsSpectateOnly() && (pEngfuncs->GetViewModel()->model != nullptr))
				{
					cl_entity_t* viewmodel = pEngfuncs->GetViewModel();
					ViewmodelPlayAnimation(AnimationLookupSequence((weaponStateSecondary == WeaponState::stateSilenced) ? "idle" : "idle_unsil", viewmodel));
				}
			}

			if (2 <= argc)
				pEngfuncs->Con_Printf("secondary weapon state: %d\n", weaponStateSecondary);

			return;
		}
	}

	pEngfuncs->Con_Printf(
		"Usage:\n"
		PREFIX "cstrike_hltv_animation_fix enabled 0|1 - Enable (1) or Disable (0) viewmodel animations for hltv demos.\n"
		PREFIX "cstrike_hltv_animation_fix primary 0|1|2 - Override primary weapon state.\n"
		PREFIX "cstrike_hltv_animation_fix secondary 0|1|2 - Override secondary weapon state.\n"
		"primary/secondary (0) will only remember the state the weapon was in when it previously fired.\n"
		"primary/secondary (1) will force the weapon state to be unsilenced.\n"
		"primary/secondary (2) will force the weapon state to be silenced.\n"
	);
}

bool Hook_cstrike_events_m4a1(void * pAddress) {
	static bool firstRun = true;
	static bool firstResult = false;
	if (!firstRun) return firstResult;
	firstRun = false;

	if(pAddress) {
		g_pfnCstrike_EV_FireM4A1_Hooked_Func = (UnkCstrikeEV_FireM4A1Fn_t)pAddress;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_pfnCstrike_EV_FireM4A1_Hooked_Func, EV_FireM4A1);
		firstResult = NO_ERROR == DetourTransactionCommit();
	}

	if(!firstResult) ErrorBox("Hook_cstrike_events_m4a1");

	return firstResult;
}

bool Hook_cstrike_events_usp(void * pAddress) {
	static bool firstRun = true;
	static bool firstResult = false;
	if (!firstRun) return firstResult;
	firstRun = false;

	if(pAddress) {
		g_pfnCstrike_EV_FireUSP_Hooked_Func = (UnkCstrikeEV_FireM4A1Fn_t)pAddress;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_pfnCstrike_EV_FireUSP_Hooked_Func, EV_FireUSP);
		firstResult = NO_ERROR == DetourTransactionCommit();
	}

	if(!firstResult) ErrorBox("Hook_cstrike_events_usp");

	return firstResult;
}

bool Hook_Cstrike_Viewmodel_Animation_Fix()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;
	
	if (pEngfuncs)
	{
		LONG error = NO_ERROR;

		g_detoured_pfnWeaponAnim = pEngfuncs->pfnWeaponAnim;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_detoured_pfnWeaponAnim, Touring_pfnWeaponAnim);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nViewmodelAnimationFix.cpp: Hook_Cstrike_Viewmodel_Animation_Fix");
		}
	}
	else
		firstResult = false;

	return firstResult;
}