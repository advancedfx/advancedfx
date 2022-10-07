#pragma once

#include <vector>
#include <string>

// Description: Counter-Strike 1.6 viewmodel animation fix.

enum AnimationType { animError = -1, animDoNothing, animNone, animReload, animPlant, animThrow, animKnifeAttack, animDrawGrenade, animDrawGun };
enum WeaponState { stateError = -1, stateDefault, stateUnsilenced, stateSilenced };
enum WeaponType { typeError = -1, typeNotSilenced, typePrimary, typeSecondary };

struct AnimationSequence {
	AnimationType type;
	std::string action;
};

struct ModelSequence {
	std::vector<std::string> sequences;
	char modelName[128];
};

/// <summary>Installs the cstrike viewmodel animation fix hook.</summary>
bool Hook_Cstrike_Viewmodel_Animation_Fix();

void ApplyViewmodelAnimationFix();