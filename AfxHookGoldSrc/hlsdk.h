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
#include <halflife/cl_dll/wrect.h>
#include <halflife/cl_dll/cl_dll.h>
#include <halflife/cl_dll/in_defs.h> // PITCH YAW ROLL // HL1 sdk
#include <halflife/engine/cdll_int.h>
#include <halflife/engine/progdefs.h>
#include <halflife/engine/eiface.h>
#include <halflife/engine/studio.h>
#include <halflife/common/cl_entity.h>
#include <halflife/common/com_model.h>
#include <halflife/common/cvardef.h>
#include <halflife/common/entity_state.h>
#include <halflife/common/entity_types.h>
#include <halflife/common/event_args.h>
#include <halflife/common/net_api.h>
//#include <halflife/common/r_studioint.h>
#include <halflife/pm_shared/pm_defs.h>
#include <halflife/common/r_efx.h>
#include <halflife/common/com_model.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
#pragma pop_macro("offsetof")
#pragma pop_macro("ARRAYSIZE")
// END HLSDK includes
