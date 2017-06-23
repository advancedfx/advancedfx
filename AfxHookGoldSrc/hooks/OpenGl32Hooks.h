#pragma once

#include <windows.h>

#include <gl/gl.h>

void APIENTRY NewGlBegin(GLenum mode);

void APIENTRY NewGlEnd(void);

void APIENTRY NewGlClear(GLbitfield mask);

void APIENTRY NewGlViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void APIENTRY NewGlFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);

void APIENTRY NewGlBlendFunc (GLenum sfactor, GLenum dfactor);

typedef BOOL (APIENTRY *wglSwapBuffers_t)(HDC hDC);
extern wglSwapBuffers_t OldWglSwapBuffers;
BOOL APIENTRY NewWglSwapBuffers(HDC hDC);

HGLRC WINAPI NewWglCreateContext(HDC);

BOOL WINAPI NewWglDeleteContext(HGLRC hGlRc);

BOOL WINAPI NewWglMakeCurrent(HDC, HGLRC);
