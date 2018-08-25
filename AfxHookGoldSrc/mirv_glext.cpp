#include "stdafx.h"

#include "mirv_glext.h"

//
//

bool g_Has_All_Gl_Extensions = false;

bool Install_All_Gl_Extensions()
{
	static bool bTried = false;
	if(bTried)
		return g_Has_All_Gl_Extensions;
	bTried = true;

	g_Has_All_Gl_Extensions = true
		&& Install_GL_ARB_multitexture()
		//&& Install_GL_ARB_texture_env_combine()
	;

	return g_Has_All_Gl_Extensions;
}


//
// GL_ARB_texture_env_combine

bool g_Has_GL_ARB_texture_env_combine = false;

bool Install_GL_ARB_texture_env_combine()
{
	static bool bTried = false;
	if(bTried)
		return g_Has_GL_ARB_texture_env_combine;
	bTried = true;

	char *pExtStr = (char *)(glGetString( GL_EXTENSIONS ));

	if (pExtStr && strstr( pExtStr, "GL_ARB_texture_env_combine " ))
	{
		g_Has_GL_ARB_texture_env_combine = true
		;
	}

	return g_Has_GL_ARB_texture_env_combine;
}


//
// GL_ARB_multitexture Extension

bool g_Has_GL_ARB_multitexture = false;

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = 0;


bool Install_GL_ARB_multitexture()
{
	static bool bTried = false;
	if(bTried)
		return g_Has_GL_ARB_multitexture;
	bTried = true;

	char *pExtStr = (char *)(glGetString( GL_EXTENSIONS ));

	if (pExtStr && strstr( pExtStr, "GL_ARB_multitexture " ))
	{
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");

		g_Has_GL_ARB_multitexture = true
			&& glActiveTextureARB
		;
	}

	return g_Has_GL_ARB_multitexture;
}

