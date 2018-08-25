/*
File        : mdt_media.cpp
Started     : 2007-08-25 21:00:00
Project     : Mirv Demo Tool
Authors     : Dominik Tugend
Description : This file implements the mirv_cameraofs_cs function to allow setting of cameraofs (can be usefull to do stereocaptures by capturing scenes twice i.e.)
*/

// I tried to do this similar to demoeditfix.cpp, but of course it is different heh

#include "wrect.h"
#include "cl_dll.h"
#include "cdll_int.h"
#include "r_efx.h"
#include "com_model.h"
#include "r_studioint.h"
#include "pm_defs.h"
#include "cvardef.h"
#include "entity_types.h"

#include "hl_addresses.h"

//
#include <windows.h> // BYTE, ...
#include "cmdregister.h" // functions to help with registering console commands
#include "ref_params.h" // hl1/common/ref_params.h, defines the ref_params_s structure supplied with each V_CalcRefDef call
#include "in_defs.h" // PITCH YAW ROLL // HL1 sdk

extern cl_enginefuncs_s *pEngfuncs;
//extern engine_studio_api_s *pEngStudio;
//extern playermove_s *ppmove;

void *HookVTentryWrapper(DWORD* tableEntry, BYTE* wrappingFunc)
// tableEntry --> Address(-pointer) of Table entry
// wrappingFUnc --> (Address) of new func
// WARNING: THIS FUNCTION IS ONLY FOR 32 BIT I386 environments!
{
	DWORD dwProt = 0; // here we will store the original protection mode of the memory we have to change

	DWORD oldentry;
	VirtualProtect(tableEntry, sizeof(DWORD), PAGE_READWRITE, &dwProt); // tell windows to give us access to the memory where the entry is stored
	
	oldentry = *tableEntry; // get original entry

	*tableEntry = (DWORD)wrappingFunc; // place the new entry

#ifdef __DEBUG
	pEngfuncs->Con_Printf("Hooked:  @0x%08x: 0x%08x --> 0x%08x (==0x%08x)\n",tableEntry,oldentry,*tableEntry,wrappingFunc);
#endif

	VirtualProtect(tableEntry, sizeof(DWORD), dwProt, 0); // try to restore the old memory protection heh
	
	return (BYTE *)oldentry; // return old entry
}

float camerofs_screenofs_right = 0;
float camerofs_screenofs_up = 0;
float camerofs_screenofs_forward = 0;

// address from client.dll's export table:

#if 0
	// not used: // also outdated now
	#define VTENTRY_V_CalcRefDef 0x02f6a98c
	#define SIZE_V_CalcRefDef 38
	#define ADDRESS_V_CalcRefDef 0x0196db70
	#define VTENTRY_CL_CameraOffset 0x02f6a980
	#define VTENTRY_CL_IsThirdPerson 0x02f6a97c
#endif

typedef void (*V_CalcRefDef_t)( struct ref_params_s *pparams );
V_CalcRefDef_t orig_V_CalcRefDef;

void hooked_V_CalcRefDef( struct ref_params_s *pparams )
// note: this is not used anmore, I just left it in in case we might need it.
{
	// call the original function
	orig_V_CalcRefDef(pparams);
}


/*
Finding R_RenderView in a debugger:
You can find R_RenderView very easily if you look into gl_rmain.c (it is similar to the Half-Life one).
Shortly before ret it directly pushes the string "%3ifps %3i ms  %4i wpoly %4i epoly\n"
in order to do an Con_Printf with it. just follow the ret and you get the original address (from the call).

01d50550 d9056809eb01    fld     dword ptr [launcher!CreateInterface+0xaaf579 (01eb0968)] ds:0023:01eb0968=00000000	// if (r_norefresh.value) return;
01d50556 d81d7ca0e701    fcomp   dword ptr [launcher!CreateInterface+0xa78c8d (01e7a07c)]							// .
01d5055c 83ec14          sub     esp,14h																			// .
01d5055f dfe0            fnstsw  ax																					// .
01d50561 f6c444          test    ah,44h																				// .
01d50564 0f8a1a010000    jp      launcher!CreateInterface+0x94f295 (01d50684)										// .

01d5056a a19492c102      mov     eax,dword ptr [launcher!CreateInterface+0x1817ea5 (02c19294)]						// if (!r_worldentity.model
01d5056f 85c0            test    eax,eax																			// .
01d50571 7409            je      launcher!CreateInterface+0x94f18d (01d5057c)										// .
01d50573 a17030f502      mov     eax,dword ptr [launcher!CreateInterface+0x1b51c81 (02f53070)]						// ||!cl.worldmodel)
01d50578 85c0            test    eax,eax																			// .
01d5057a 750d            jne     launcher!CreateInterface+0x94f19a (01d50589)										// .
01d5057c 686c08eb01      push    offset launcher!CreateInterface+0xaaf47d (01eb086c)								// Sys_Error ("R_RenderView: NULL worldmodel");
01d50581 e8aa110700      call    launcher!CreateInterface+0x9c0341 (01dc1730)										// .
01d50586 83c404          add     esp,4																				// .

01d50589 d905d809eb01    fld     dword ptr [launcher!CreateInterface+0xaaf5e9 (01eb09d8)]							// 	if (r_speeds.value)
01d5058f d81d7ca0e701    fcomp   dword ptr [launcher!CreateInterface+0xa78c8d (01e7a07c)]							// .
01d50595 dfe0            fnstsw  ax																					// .
01d50597 f6c444          test    ah,44h																				// .
01d5059a 7b23            jnp     launcher!CreateInterface+0x94f1d0 (01d505bf)										// .
01d5059c ff15e01b7e02    call    dword ptr [launcher!CreateInterface+0x13e07f1 (027e1be0)]							// { glFinish ();
01d505a2 e869130700      call    launcher!CreateInterface+0x9c0521 (01dc1910)										// time1 = Sys_DoubleTime ();
01d505a7 dd5c240c        fstp    qword ptr [esp+0Ch]																// .
01d505ab c705fc93c10200000000 mov dword ptr [launcher!CreateInterface+0x181800d (02c193fc)],0						// c_brush_polys = 0;
01d505b5 c7057c95c10200000000 mov dword ptr [launcher!CreateInterface+0x181818d (02c1957c)],0						// c_alias_polys = 0;
01d505bf c705c093c10200000000 mov dword ptr [launcher!CreateInterface+0x1817fd1 (02c193c0)],0						// } mirror = false;

01d505c9 e802f8ffff      call    launcher!CreateInterface+0x94e9e1 (01d4fdd0)										// R_Clear();

01d505ce a16c93c102      mov     eax,dword ptr [launcher!CreateInterface+0x1817f7d (02c1936c)]						// if (!UNKNOWN)
01d505d3 85c0            test    eax,eax																			// .
01d505d5 7505            jne     launcher!CreateInterface+0x94f1ed (01d505dc)										// .
01d505d7 e8d4e9ffff      call    launcher!CreateInterface+0x94dbc1 (01d4efb0)										// UNKNOWN() or R_RenderScene() or R_DrawViewModel();

01d505dc e83ffeffff      call    launcher!CreateInterface+0x94f031 (01d50420)										// R_RenderScene(); // with several extensions

01d505e1 a16c93c102      mov     eax,dword ptr [launcher!CreateInterface+0x1817f7d (02c1936c)]						// if (!UNKNOWN)
01d505e6 85c0            test    eax,eax																			// .
01d505e8 750a            jne     launcher!CreateInterface+0x94f205 (01d505f4)										// .
01d505ea e811e6ffff      call    launcher!CreateInterface+0x94d811 (01d4ec00)										// { UNKNOWN() or R_DrawViewModel();
01d505ef e87cebffff      call    launcher!CreateInterface+0x94dd81 (01d4f170)										// UNKNOWN(); }

01d505f4 e8d70f0500      call    launcher!CreateInterface+0x9a01e1 (01da15d0)										// UNKONW(); //  gets  snd_noextraupdate.value

01d505f9 d905d809eb01    fld     dword ptr [launcher!CreateInterface+0xaaf5e9 (01eb09d8)]							// if (r_speeds.value)
01d505ff d81d7ca0e701    fcomp   dword ptr [launcher!CreateInterface+0xa78c8d (01e7a07c)]							// .
01d50605 dfe0            fnstsw  ax																					// .
01d50607 f6c444          test    ah,44h																				// .
01d5060a 7b78            jnp     launcher!CreateInterface+0x94f295 (01d50684)										// .
01d5060c dd0518abdd02    fld     qword ptr [launcher!CreateInterface+0x19d9729 (02ddab18)]							// {
01d50612 dc2520abdd02    fsub    qword ptr [launcher!CreateInterface+0x19d9731 (02ddab20)]							//
01d50618 d95c2400        fstp    dword ptr [esp]																	//
01d5061c d9442400        fld     dword ptr [esp]																	//
01d50620 d81d7ca0e701    fcomp   dword ptr [launcher!CreateInterface+0xa78c8d (01e7a07c)]							//
01d50626 dfe0            fnstsw  ax																					//
01d50628 2500410000      and     eax,4100h																			//
01d5062d 750e            jne     launcher!CreateInterface+0x94f24e (01d5063d)										//
01d5062f d905eca5e701    fld     dword ptr [launcher!CreateInterface+0xa791fd (01e7a5ec)]							//
01d50635 d8742400        fdiv    dword ptr [esp]																	//
01d50639 d95c2400        fstp    dword ptr [esp]																	//

01d5063d e8ce120700      call    launcher!CreateInterface+0x9c0521 (01dc1910)										//
01d50642 dd5c2404        fstp    qword ptr [esp+4]																	//
01d50646 dd442404        fld     qword ptr [esp+4]																	//
01d5064a dc64240c        fsub    qword ptr [esp+0Ch]																//
01d5064e a17c95c102      mov     eax,dword ptr [launcher!CreateInterface+0x181818d (02c1957c)]						//
01d50653 8b0dfc93c102    mov     ecx,dword ptr [launcher!CreateInterface+0x181800d (02c193fc)]						//
01d50659 50              push    eax																				//
01d5065a 51              push    ecx																				//
01d5065b dc0db0a7e701    fmul    qword ptr [launcher!CreateInterface+0xa793c1 (01e7a7b0)]							//
01d50661 e86a921000      call    launcher!CreateInterface+0xa584e1 (01e598d0)										//
01d50666 d9442408        fld     dword ptr [esp+8]																	//
01d5066a dc05b8a5e701    fadd    qword ptr [launcher!CreateInterface+0xa791c9 (01e7a5b8)]							//
01d50670 50              push    eax																				//
01d50671 e85a921000      call    launcher!CreateInterface+0xa584e1 (01e598d0)										//
01d50676 50              push    eax																				//
01d50677 688c08eb01      push    offset launcher!CreateInterface+0xaaf49d (01eb088c)								// <-- "%3ifps %3i ms %4i wpoly %4i epoly\n"
01d5067c e86f86feff      call    launcher!CreateInterface+0x937901 (01d38cf0)										// <-- Con_Printf
01d50681 83c414          add     esp,14h																			// }

01d50684 83c414          add     esp,14h																			// <-- all return;s go here
01d50687 c3              ret																						// <-- true end
01d50688 90              nop
01d50689 90              nop
01d5068a 90              nop
01d5068b 90              nop
01d5068c 90              nop
01d5068d 90              nop
01d5068e 90              nop
01d5068f 90              nop


Finding the r_refdef global structure:

follow the call of R_RenderScene()

follow some of the calls where there are 4 in the row (the one to R_SetupGL

you will find s.th. like
01d4fa4d ff158c187e02    call    dword ptr [launcher!CreateInterface+0x13e049d (027e188c)] ds:0023:027e188c={opengl32!glViewport (5f0d51bc)}
01d4fa53 db05e892c102    fild    dword ptr [launcher!CreateInterface+0x1817ef9 (02c192e8)]

ok, now we should have won, since it accesses the width :
glViewport (glx + x, gly + y2, w, h);
creenaspect = (float)r_refdef.vrect.width/r_refdef.vrect.height;
--->r_refdef.vrect.width@ 02c192e8 --> 0x02c192e0 r_refdef address!


*/


#define ADDRESS_R_RenderView HL_ADDR_R_RenderView
// Use 20 Bytes for detouring (this is a series of asm commands that should stay together I guess)
#define DETOURSIZE_R_RenderView 0x014
#define ADDRESS_r_refdef HL_ADDR_r_refdef

// BEGIN from ID Software's Quake 1 Source:

// q1source/QW/client/mathlib.h
// our hl includes already give us that:
//typedef float vec_t;
//typedef vec_t vec3_t[3];

// q1source/QW/client/vid.h
typedef struct vrect_s
{
	int				x,y,width,height;
	struct vrect_s	*pnext;
} vrect_t;

// q1source/QW/client/render.h
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vrect_t		vrect;				// subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t		aliasvrect;			// scaled Alias version
	int			vrectright, vrectbottom;	// right & bottom screen coords
	int			aliasvrectright, aliasvrectbottom;	// scaled Alias versions
	float		vrectrightedge;			// rightmost right edge we care about,
										//  for use in edge list
	float		fvrectx, fvrecty;		// for floating-point compares
	float		fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	int			vrect_x_adj_shift20;	// (vrect.x + 0.5 - epsilon) << 20
	int			vrectright_adj_shift20;	// (vrectright + 0.5 - epsilon) << 20
	float		fvrectright_adj, fvrectbottom_adj;
										// right and bottom edges, for clamping
	float		fvrectright;			// rightmost edge, for Alias clamping
	float		fvrectbottom;			// bottommost edge, for Alias clamping
	float		horizontalFieldOfView;	// at Z = 1.0, this many X is visible 
										// 2.0 = 90 degrees
	float		xOrigin;			// should probably allways be 0.5
	float		yOrigin;			// between be around 0.3 to 0.5

	vec3_t		vieworg;
	vec3_t		viewangles;

	float		fov_x, fov_y;
	
	int			ambientlight;
} refdef_t;

// END from ID Software's Quake 1 Source.

// from HL1SDK/multiplayer/common/mathlib.h:
#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#include "detours.h" // we want to use Detourapply

typedef void (*R_RenderView_t)( void );
R_RenderView_t detoured_R_RenderView;

// please note the difference of detouring and wrapping / hooking if you code in this function!

void touring_R_RenderView(void)
{
	refdef_t* p_r_refdef=(refdef_t*)ADDRESS_r_refdef; // pointer to r_refdef global struct

#if __debug
	pEngfuncs->Con_DPrintf("p_r_refdef->vieworg: (%f, %f, %f)\n",p_r_refdef->vieworg[0],p_r_refdef->vieworg[1],p_r_refdef->vieworg[2]);
	pEngfuncs->Con_DPrintf("p_r_refdef->viewangles: (%f, %f, %f)\n",p_r_refdef->viewangles[0],p_r_refdef->vieworg[1],p_r_refdef->viewangles[2]);
#endif

#if 0
	// for debug
	p_r_refdef->viewangles[0]=0;
	p_r_refdef->viewangles[1]=0;
	p_r_refdef->viewangles[2]=0;
#endif

	vec3_t oldorigin = p_r_refdef->vieworg; // save old

	// >> begin calculate transform vectors
	// we have to calculate our own transformation vectors from the angles and can not use pparams->forward etc., because in spectator mode they might be not present:
	// (adapted from HL1SDK/multiplayer/pm_shared.c/AngleVectors) and modified for quake order of angles:

	vec3_t angles;
	float forward[3],right[3],up[3];

	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angles = p_r_refdef->viewangles;

	angle = angles[YAW] * ((float)M_PI*2 / 360);
	sy = sin((float)angle);
	cy = cos((float)angle);
	angle = angles[PITCH] * ((float)M_PI*2 / 360);
	sp = sin((float)angle);
	cp = cos((float)angle);
	angle = angles[ROLL] * ((float)M_PI*2 / 360);
	sr = sin((float)angle);
	cr = cos((float)angle);

	forward[0] = cp*cy;
	forward[1] = cp*sy;
	forward[2] = -sp;

	right[0] = (-1*sr*sp*cy+-1*cr*-sy);
	right[1] = (-1*sr*sp*sy+-1*cr*cy);
	right[2] = -1*sr*cp;

	up[0] = (cr*sp*cy+-sr*-sy);
	up[1] = (cr*sp*sy+-sr*cy);
	up[2] = cr*cp;

	// << end calculate transform vectors

	// apply our values(this code is similar to HL1SDK/multiplayer/cl_dll/view.cpp/V_CalcNormalRefdef):
	for ( int i=0 ; i<3 ; i++ )
	{
		p_r_refdef->vieworg[i] += camerofs_screenofs_forward*forward[i] + camerofs_screenofs_right*right[i] + camerofs_screenofs_up*up[i];
	}

	detoured_R_RenderView();

	p_r_refdef->vieworg = oldorigin; // restore old (is this necessary? I don't know if the values are used for interpolations later or not)
}


REGISTER_CMD_FUNC(cameraofs_cs)
{
	static bool bHooked_V_CalcRefDef = false;

	if (pEngfuncs->Cmd_Argc() == 4)
	{
		if(!bHooked_V_CalcRefDef)
		{
			// Redirect to hook
			// not used anymore currently, since CS overrides several properties after the call when in ineyedemomode: //orig_V_CalcRefDef = (V_CalcRefDef_t) HookVTentryWrapper((DWORD *)VTENTRY_V_CalcRefDef,(BYTE *)hooked_V_CalcRefDef);
			detoured_R_RenderView= (R_RenderView_t) DetourApply((BYTE *)ADDRESS_R_RenderView, (BYTE *)touring_R_RenderView, (int)DETOURSIZE_R_RenderView);

			bHooked_V_CalcRefDef = true;
			pEngfuncs->Con_Printf("Installed hook.\n");
		}

		camerofs_screenofs_right = atof(pEngfuncs->Cmd_Argv(1));
		camerofs_screenofs_up = atof(pEngfuncs->Cmd_Argv(2));
		camerofs_screenofs_forward = atof(pEngfuncs->Cmd_Argv(3));
		// pEngfuncs->Con_Printf("Set new x y and z.\n");
	}
	else
		pEngfuncs->Con_Printf("Usage: " PREFIX "cameraofs_cs <right> <up> <forward>\nThis function probably currently only works with Counter-Strike 1.6.\n");
}