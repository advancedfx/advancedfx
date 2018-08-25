#include "stdafx.h"

#include "R_DrawSkyBoxEx.h"

#include <hl_addresses.h>
#include <shared/detours.h>


typedef void (*R_DrawSkyBoxEx_t) (void);
R_DrawSkyBoxEx_t g_Old_R_DrawSkyBoxEx = 0;

GLuint g_oldSkyTextures[6];
GLuint * g_R_DrawSkyBoxEx_NewTextures = 0;

void New_R_DrawSkyBoxEx (void)
{
	bool replaced;

	if(g_R_DrawSkyBoxEx_NewTextures)
	{
		replaced = true;

		MdtMemBlockInfos mbis;

		MdtMemAccessBegin((LPVOID)HL_ADDR_GET(skytextures), 6*sizeof(GLuint), &mbis);

		memcpy(g_oldSkyTextures, (LPVOID)HL_ADDR_GET(skytextures), 6*sizeof(GLuint));
		memcpy((LPVOID)HL_ADDR_GET(skytextures), g_R_DrawSkyBoxEx_NewTextures, 6*sizeof(GLuint));

		MdtMemAccessEnd(&mbis);
	}
	else
		replaced = false;

	g_Old_R_DrawSkyBoxEx();

	if(replaced)
	{
		MdtMemBlockInfos mbis;

		MdtMemAccessBegin((LPVOID)HL_ADDR_GET(skytextures), 6*sizeof(GLuint), &mbis);

		memcpy((LPVOID)HL_ADDR_GET(skytextures), g_oldSkyTextures, 6*sizeof(GLuint));

		MdtMemAccessEnd(&mbis);
	}
}


void Hook_R_DrawSkyBoxEx()
{
	if( !g_Old_R_DrawSkyBoxEx && 0 != HL_ADDR_GET(R_DrawSkyBoxEx) )
	{
		g_Old_R_DrawSkyBoxEx = (R_DrawSkyBoxEx_t) DetourApply((BYTE *)HL_ADDR_GET(R_DrawSkyBoxEx), (BYTE *)New_R_DrawSkyBoxEx, (int)HL_ADDR_GET(R_DrawSkyBoxEx_DSZ));
	}
}
