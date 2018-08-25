//
// Copyright by Half-Life Advanced Effects project
//
#include <wrect.h>
#include <cl_dll.h>
#include <cdll_int.h>
#include <r_efx.h>
#include <com_model.h>
#include <r_studioint.h>
#include <pm_defs.h>
#include <cvardef.h>
#include <entity_types.h>

#include <triangleapi.h>

#include "cmdregister.h"
#include "detours.h"

extern cl_enginefuncs_s *pEngfuncs;
extern engine_studio_api_s *pEngStudio;
extern playermove_s* ppmove;



typedef void ( *triapi_Begin_t )( int primitiveCode );
typedef void ( *triapi_End_t ) ( void );
typedef void ( *triapi_Vertex3f_t ) ( float x, float y, float z );


triapi_Begin_t g_dtourd_triapi_Begin = NULL;
triapi_End_t g_dtourd_triapi_End = NULL;

bool g_bBlockedTriApi = false;

void New_triapi_Begin( int primitiveCode )
{
	pEngfuncs->Con_DPrintf("called\n");
	if( !g_bBlockedTriApi )
		g_dtourd_triapi_Begin( primitiveCode );
}

void New_triapi_End( void )
{
	//if( !g_bBlockedTriApi ) g_dtourd_triapi_End();
}


REGISTER_CMD_FUNC(fx_flashhack)
{
	if( !g_dtourd_triapi_Begin )
	{
		pEngfuncs->Con_DPrintf("0x%08x 0x%08x\n",pEngfuncs->pTriAPI->Begin,pEngfuncs->pTriAPI->End);
		//g_dtourd_triapi_Begin = (triapi_Begin_t)DetourApply((BYTE *)pEngfuncs->pTriAPI->Begin, (BYTE *)New_triapi_Begin,0x05);
		//g_dtourd_triapi_End = (triapi_End_t)DetourApply((BYTE *)pEngfuncs->pTriAPI->End, (BYTE *)New_triapi_End,0x05);
	}

	if (pEngfuncs->Cmd_Argc() != 2)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "Please refer to the changelog / HLAEwiki for more information.");
		return;
	} else {
		int i = atoi(pEngfuncs->Cmd_Argv(1));
		g_bBlockedTriApi = ( 1==i );
	}

};