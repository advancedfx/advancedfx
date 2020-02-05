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
#include <deps/release/halflife/cl_dll/wrect.h>
#include <deps/release/halflife/cl_dll/cl_dll.h>
#include <deps/release/halflife/cl_dll/in_defs.h> // PITCH YAW ROLL // HL1 sdk
#include <deps/release/halflife/engine/cdll_int.h>
#include <deps/release/halflife/engine/progdefs.h>
#include <deps/release/halflife/engine/eiface.h>
#include <deps/release/halflife/engine/studio.h>
#include <deps/release/halflife/common/cl_entity.h>
#include <deps/release/halflife/common/com_model.h>
#include <deps/release/halflife/common/cvardef.h>
#include <deps/release/halflife/common/entity_state.h>
#include <deps/release/halflife/common/entity_types.h>
#include <deps/release/halflife/common/event_args.h>
#include <deps/release/halflife/common/net_api.h>
//#include <halflife/common/r_studioint.h>
#include <deps/release/halflife/pm_shared/pm_defs.h>
#include <deps/release/halflife/common/r_efx.h>
#include <deps/release/halflife/common/com_model.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
#pragma pop_macro("offsetof")
#pragma pop_macro("ARRAYSIZE")
// END HLSDK includes
