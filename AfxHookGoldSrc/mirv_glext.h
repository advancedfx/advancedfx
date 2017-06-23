#pragma once

#include <windows.h>
#include <gl\gl.h>
#include <shared\ogl\glext.h>

//
//
extern bool g_Has_All_Gl_Extensions;
bool Install_All_Gl_Extensions();

//
// GL_ARB_texture_env_combine

extern bool g_Has_GL_ARB_texture_env_combine;
bool Install_GL_ARB_texture_env_combine();

//
// GL_ARB_multitexture Extension

extern bool g_Has_GL_ARB_multitexture;
bool Install_GL_ARB_multitexture();

extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;

