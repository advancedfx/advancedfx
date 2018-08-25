#include "stdafx.h"

// hack hack hack:
typedef float vec_t;
#ifdef DID_VEC3_T_DEFINE
#undef DID_VEC3_T_DEFINE
#undef vec3_t
#endif
#ifndef DID_VEC3_T_DEFINE
#define DID_VEC3_T_DEFINE
typedef float vec3_t[3];
#endif

#include "sv_hitboxes.h"

#include <hlsdk.h>
#include <shared/halflife/common/r_studioint.h>

#include "hooks/HookHw.h"

#include "hl_addresses.h"
#include "cmdregister.h"
#include "mirv_glext.h"

#include <GL/GL.h>
#include <list>
#include <map>

REGISTER_CVAR(draw_sv_hitboxes_enable, "0", 0);
REGISTER_CVAR(draw_sv_hitboxes_ignore1, "0", 0);
REGISTER_CVAR(draw_sv_hitboxes_width, "1.0", 0);

typedef struct temp_box_s {
	float data[8][3];
} temp_box_t;

typedef struct temp_edictboxes_s {
	bool isFresh;
	std::list<temp_box_t> tempBoxes;
} temp_edictboxes_t;	

std::map<int, temp_edictboxes_t> tempEdicts;

double hitboxesColor[3] = { 1.0, 0.0, 0.0 };
double hitboxesColorU[3] = { 1.0, 1.0, 1.0 };

void DrawBox(temp_box_t p, double red, double green, double blue)
{
	glColor3d(red, green, blue);

	glBegin(GL_LINES);

	glVertex3f(p.data[0][0], p.data[0][1], p.data[0][2]);
	glVertex3f(p.data[1][0], p.data[1][1], p.data[1][2]);

	glVertex3f(p.data[1][0], p.data[1][1], p.data[1][2]);
	glVertex3f(p.data[3][0], p.data[3][1], p.data[3][2]);

	glVertex3f(p.data[3][0], p.data[3][1], p.data[3][2]);
	glVertex3f(p.data[2][0], p.data[2][1], p.data[2][2]);

	glVertex3f(p.data[2][0], p.data[2][1], p.data[2][2]);
	glVertex3f(p.data[0][0], p.data[0][1], p.data[0][2]);

	glVertex3f(p.data[4][0], p.data[4][1], p.data[4][2]);
	glVertex3f(p.data[5][0], p.data[5][1], p.data[5][2]);

	glVertex3f(p.data[5][0], p.data[5][1], p.data[5][2]);
	glVertex3f(p.data[7][0], p.data[7][1], p.data[7][2]);

	glVertex3f(p.data[7][0], p.data[7][1], p.data[7][2]);
	glVertex3f(p.data[6][0], p.data[6][1], p.data[6][2]);

	glVertex3f(p.data[6][0], p.data[6][1], p.data[6][2]);
	glVertex3f(p.data[4][0], p.data[4][1], p.data[4][2]);

	glVertex3f(p.data[0][0], p.data[0][1], p.data[0][2]);
	glVertex3f(p.data[4][0], p.data[4][1], p.data[4][2]);

	glVertex3f(p.data[1][0], p.data[1][1], p.data[1][2]);
	glVertex3f(p.data[5][0], p.data[5][1], p.data[5][2]);

	glVertex3f(p.data[3][0], p.data[3][1], p.data[3][2]);
	glVertex3f(p.data[7][0], p.data[7][1], p.data[7][2]);

	glVertex3f(p.data[2][0], p.data[2][1], p.data[2][2]);
	glVertex3f(p.data[6][0], p.data[6][1], p.data[6][2]);

	glEnd();	
}

//
// >>>> math functions
// Copyright (c) by Valve Software.

#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])

void VectorTransform (const float *in1, float in2[3][4], float *out)
{
	out[0] = DotProduct(in1, in2[0]) + in2[0][3];
	out[1] = DotProduct(in1, in2[1]) + in2[1][3];
	out[2] = DotProduct(in1, in2[2]) + in2[2][3];
}

// <<<< math functions
//

void Draw_SV_Hitboxes(void)
{
	if(!draw_sv_hitboxes_enable->value) return;

	GLint activeTextureARB = GL_TEXTURE0_ARB;
	GLboolean arbActive;
	if(g_Has_GL_ARB_multitexture)
	{
		glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &activeTextureARB);
	}
	if(GL_TEXTURE1_ARB == activeTextureARB)
	{
		arbActive = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	GLfloat lineWidth;
	GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
	GLboolean text = glIsEnabled(GL_TEXTURE_2D);
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);


	glLineWidth(draw_sv_hitboxes_width->value);

	for(std::map<int, temp_edictboxes_t>::iterator it = tempEdicts.begin(); it != tempEdicts.end(); it++)
	{
		if(draw_sv_hitboxes_ignore1->value && 1 == it->first) {
			continue;
		}

		double red, green, blue;
		temp_edictboxes_t & tempEdict = it->second;

		if(tempEdict.isFresh)
		{
			red = hitboxesColorU[0];
			green = hitboxesColorU[1];
			blue = hitboxesColorU[2];
		}
		else
		{
			red = hitboxesColor[0];
			green = hitboxesColor[1];
			blue = hitboxesColor[2];
		}

		tempEdict.isFresh = false;

		for(std::list<temp_box_t>::iterator it2 = tempEdict.tempBoxes.begin(); it2 != tempEdict.tempBoxes.end(); it2++)
		{
			DrawBox(*it2, red, green, blue);
		}

	}

	glLineWidth(lineWidth);

	if(text) glEnable(GL_TEXTURE_2D);
	if(!depth) glDisable(GL_DEPTH_TEST);

	if(GL_TEXTURE1_ARB == activeTextureARB)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		if(arbActive) glEnable(GL_TEXTURE_2D);
	}
}

extern enginefuncs_t g_engfuncs;

void FetchHitboxes(struct server_studio_api_s *pstudio,
	float (*bonetransform)[MAXSTUDIOBONES][3][4],
	struct model_s *pModel, float frame, int sequence, const vec3_t angles,
	const vec3_t origin, const byte *pcontroller, const byte *pblending, 
	int iBone, const edict_t *pEdict)
{
	/*pEngfuncs->Con_Printf(
		"FetchHitboxes(0x%08x): 0x%08x,0x%08x,0x%08x,%f,%i,(%f,%f,%f),(%f,%f,%f),0x%08x,0x%08x,%i,0x%08x --> %i\n",
		&FetchHitboxes,
		pstudio, bonetransform, pModel,
		frame, sequence,
		angles[0],angles[1],angles[2],
		origin[0],origin[1],origin[2],
		pcontroller, pblending,
		iBone, pEdict,
		g_engfuncs.pfnIndexOfEdict(pEdict)
	);*/

	if(!draw_sv_hitboxes_enable->value) return;

	studiohdr_t *pstudiohdr = (studiohdr_t *)pstudio->Mod_Extradata(pModel);

	if(!pstudiohdr)
	{
		pEngfuncs->Con_Printf("Error: no model for pEdict (serialnumber=%i)\n",pEdict->serialnumber);
		return;
	}

	mstudiobbox_t *pbbox;
	vec3_t tmp;
	temp_box_t p;
	int i,j;

	pbbox = (mstudiobbox_t *)((byte *)pstudiohdr+ pstudiohdr->hitboxindex);

	temp_edictboxes_t & tempEdict = tempEdicts[g_engfuncs.pfnIndexOfEdict(pEdict)];
	tempEdict.isFresh = true;
	tempEdict.tempBoxes.clear();

	for (i = 0; i < pstudiohdr->numhitboxes; i++)
	{
		/*
		pEngfuncs->Con_Printf("((%f, %f, %f),(%f, %f, %f))\n",
			pbbox[i].bbmin[0], pbbox[i].bbmin[1], pbbox[i].bbmin[2],
			pbbox[i].bbmax[0], pbbox[i].bbmax[1], pbbox[i].bbmax[2]
		);
		*/

		//get the vector positions of the 8 corners of the bounding box
		for (j = 0; j < 8; j++)
		{
			tmp[0] = (j & 1) ? pbbox[i].bbmin[0] : pbbox[i].bbmax[0];
			tmp[1] = (j & 2) ? pbbox[i].bbmin[1] : pbbox[i].bbmax[1];
			tmp[2] = (j & 4) ? pbbox[i].bbmin[2] : pbbox[i].bbmax[2];

			VectorTransform( tmp, (*bonetransform)[pbbox[i].bone], p.data[j] );
		}

		tempEdict.tempBoxes.push_back(p);
	}
}

double clamp(double i, double min, double max)
{
	if (i < min)
		return min;
	if (i > max)
		return max;
	else
		return i;
}

REGISTER_CMD_FUNC(draw_sv_hitboxes_seticolor)
{
	if (pEngfuncs->Cmd_Argc() != 4)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "draw_sv_hitboxes_seticolor <red: 0-255> <green: 0-255> <blue: 0-255>\n");
		return;
	}

	double red = (double) atoi(pEngfuncs->Cmd_Argv(1)) / 255.0;
	double green = (double) atoi(pEngfuncs->Cmd_Argv(2)) / 255.0;
	double blue = (double) atoi(pEngfuncs->Cmd_Argv(3)) / 255.0;

	// ensure that the values are within the falid range
	clamp(red, 0.0, 1.0);
	clamp(green, 0.0, 1.0);	
	clamp(blue, 0.0, 1.0);

	hitboxesColor[0] = red;
	hitboxesColor[1] = green;
	hitboxesColor[2] = blue;
}

REGISTER_CMD_FUNC(draw_sv_hitboxes_setucolor)
{
	if (pEngfuncs->Cmd_Argc() != 4)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "draw_sv_hitboxes_setucolor <red: 0-255> <green: 0-255> <blue: 0-255>\n");
		return;
	}

	double red = (double) atoi(pEngfuncs->Cmd_Argv(1)) / 255.0;
	double green = (double) atoi(pEngfuncs->Cmd_Argv(2)) / 255.0;
	double blue = (double) atoi(pEngfuncs->Cmd_Argv(3)) / 255.0;

	// ensure that the values are within the falid range
	clamp(red, 0.0, 1.0);
	clamp(green, 0.0, 1.0);	
	clamp(blue, 0.0, 1.0);

	hitboxesColorU[0] = red;
	hitboxesColorU[1] = green;
	hitboxesColorU[2] = blue;
}
