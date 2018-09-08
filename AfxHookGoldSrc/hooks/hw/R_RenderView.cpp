#include "stdafx.h"

#include "R_RenderView.h"

#include <hl_addresses.h>

#include "../HookHw.h"
#include "../../filming.h"

#include <Windows.h>
#include <shared/Detours/src/detours.h>

// hack hack hack (make it use correct definition):
typedef float vec_t;
#ifdef DID_VEC3_T_DEFINE
#undef DID_VEC3_T_DEFINE
#undef vec3_t
#endif
#ifndef DID_VEC3_T_DEFINE
#define DID_VEC3_T_DEFINE
typedef vec_t vec3_t[3];
#endif

typedef void (*R_RenderView_t)( void );
typedef void (*R_PushDlights_t)( void );

R_RenderView_t g_Old_R_RenderView = 0;
R_PushDlights_t g_R_PushDlights = 0;
bool g_R_RenderViewCallFromEngine = true;
bool g_R_RenderViewCalledFromEngine = false;

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



void New_R_RenderView(void)
{
	refdef_t* p_r_refdef=(refdef_t*)HL_ADDR_GET(r_refdef); // pointer to r_refdef global struct

	static vec3_t oldorigin;
	static vec3_t oldangles;
	float old_fov;
	float cur_fov;
	
	// save original values
	memcpy (oldorigin,p_r_refdef->vieworg,3*sizeof(float));
	memcpy (oldangles,p_r_refdef->viewangles,3*sizeof(float));
	old_fov = *(float*)HL_ADDR_GET(g_fov);

	cur_fov = old_fov;
	g_Filming.OnR_RenderView(p_r_refdef->vieworg, p_r_refdef->viewangles, cur_fov);

	if(cur_fov != old_fov)
	{
		// Filming system changed fov.

		*(float*)HL_ADDR_GET(g_fov) = cur_fov;
	}

	g_Old_R_RenderView();

	if(g_R_RenderViewCallFromEngine) g_R_RenderViewCalledFromEngine = true;

	// restore original values
	memcpy (p_r_refdef->vieworg,oldorigin,3*sizeof(float));
	memcpy (p_r_refdef->viewangles,oldangles,3*sizeof(float));
	*(float*)HL_ADDR_GET(g_fov) = old_fov;
}

void Additional_R_RenderView(void)
{
	g_R_RenderViewCallFromEngine = false;

	if(g_R_RenderViewCalledFromEngine)
	{
		// repush dynamic lights, so flashlight won't be off:
		g_R_PushDlights();

		New_R_RenderView();
	}

	g_R_RenderViewCallFromEngine = true;
}

void Reset_R_RenderViewCalledFromEngine()
{
	g_R_RenderViewCalledFromEngine = false;
}

bool Hook_R_RenderView()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (!(HL_ADDR_GET(R_RenderView) && HL_ADDR_GET(R_PushDlights))) {
		firstResult = false;
	}
	else
	{
		g_R_PushDlights = (R_PushDlights_t)HL_ADDR_GET(R_PushDlights);

		LONG error = NO_ERROR;

		g_Old_R_RenderView = (R_RenderView_t)AFXADDR_GET(R_RenderView);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_R_RenderView, New_R_RenderView);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_RenderView");
		}
	}

	return firstResult;
}
