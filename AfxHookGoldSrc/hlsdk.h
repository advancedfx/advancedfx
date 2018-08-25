#pragma once

// BEGIN HLSDK includes
#pragma push_macro("ARRAYSIZE")
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#pragma push_macro("offsetof")
#ifdef offsetof
#undef offsetof
#endif
#pragma push_macro("HSPRITE")
#define HSPRITE MDTHACKED_HSPRITE
//
#include <shared/halflife/cl_dll/wrect.h>
#include <shared/halflife/cl_dll/cl_dll.h>
#include <shared/halflife/cl_dll/in_defs.h> // PITCH YAW ROLL // HL1 sdk
#include <shared/halflife/engine/cdll_int.h>
#include <shared/halflife/engine/progdefs.h>
#include <shared/halflife/engine/eiface.h>
#include <shared/halflife/engine/studio.h>
#include <shared/halflife/common/cl_entity.h>
#include <shared/halflife/common/com_model.h>
#include <shared/halflife/common/cvardef.h>
#include <shared/halflife/common/entity_state.h>
#include <shared/halflife/common/entity_types.h>
#include <shared/halflife/common/event_args.h>
#include <shared/halflife/common/net_api.h>
//#include <halflife/common/r_studioint.h>
#include <shared/halflife/pm_shared/pm_defs.h>
#include <shared/halflife/common/r_efx.h>
#include <shared/halflife/common/com_model.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
#pragma pop_macro("offsetof")
#pragma pop_macro("ARRAYSIZE")
// END HLSDK includes
