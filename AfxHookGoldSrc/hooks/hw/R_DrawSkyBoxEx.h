#pragma once

#include <gl/GL.h>

/// <summary>Point this to an array of 6 GLuints to replace the sky or 0 to use the original one.</summary>
extern GLuint * g_R_DrawSkyBoxEx_NewTextures;

bool Hook_R_DrawSkyBoxEx();
