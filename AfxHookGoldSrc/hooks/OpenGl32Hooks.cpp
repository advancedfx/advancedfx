#include "stdafx.h"

#include "OpenGl32Hooks.h"
#include "user32Hooks.h"

#ifdef AFX_GUI
#include "../gui/Gui.h"
#endif

#include "../AfxSettings.h"
#include "../aiming.h"
#include "../cmdregister.h"
#include "../filming.h"
#include "../GlPrimMods.h"
#include "../mirv_glext.h"
#include "../supportrender.h"
#include "../zooming.h"

#ifdef AFX_SCRIPT
#include "../scripting.h"
#endif

#include "HookHw.h"
#include "hw/Host_Frame.h"
#include "hw/R_RenderView.h"
#include "hw/UnkDrawHud.h"

#include <hlsdk.h>

REGISTER_CVAR(disableautodirector, "0", 0);

REGISTER_DEBUGCVAR(gl_force_noztrick, "1", 0);
REGISTER_DEBUGCVAR(gl_previewclear, "1", 0);

struct {
	bool restore;

	// Matte key:
	GLboolean b_GL_DEPTH_TEST;
	GLint i_GL_DEPTH_FUNC;
	GLboolean b_ColorWriteMask[4];

	// Mate alpha:
	GLboolean old_enabled;
	GLint old_texture;
	GLint old_active_texture;
	GLint old_env_param;

} g_ModeKey_saved;


bool ModeKey_Begin(GLenum mode)
{
	g_ModeKey_saved.restore=false;

	Filming::DRAW_RESULT res = g_Filming.shouldDraw(mode);

	if (res == Filming::DR_HIDE)
	{
		return false;
	}
	else if (res == Filming::DR_MASK)
	{
		if(Filming::MS_ENTITY == g_Filming.GetMatteStage())
		{
			g_ModeKey_saved.restore = true;
			glGetBooleanv(GL_DEPTH_TEST,&(g_ModeKey_saved.b_GL_DEPTH_TEST));
			glGetIntegerv(GL_DEPTH_FUNC,&(g_ModeKey_saved.i_GL_DEPTH_FUNC));
			glGetBooleanv(GL_COLOR_WRITEMASK, g_ModeKey_saved.b_ColorWriteMask);

			glColorMask(FALSE, FALSE, FALSE, TRUE);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_DEPTH_TEST);
		}
	}

	return true;
}


void ModeKey_End()
{
	if (g_ModeKey_saved.restore)
	{
		g_ModeKey_saved.restore = false;
		if(!g_ModeKey_saved.b_GL_DEPTH_TEST)
			glDisable(GL_DEPTH_TEST);
		glDepthFunc(g_ModeKey_saved.i_GL_DEPTH_FUNC);
		glColorMask(g_ModeKey_saved.b_ColorWriteMask[0], g_ModeKey_saved.b_ColorWriteMask[1], g_ModeKey_saved.b_ColorWriteMask[2], g_ModeKey_saved.b_ColorWriteMask[3]);
	}
}

struct {
	GlPrimMod::Replace replaceBlack;
	GlPrimMod::Replace replaceWhite;
	GlPrimMod::Color   color;
	bool restore;
	bool wasWhite;
} g_ModeAlpha;

bool ModeAlpha_Begin(GLenum mode)
{
	static bool firstRun = true;

	if(firstRun)
	{
		firstRun = false;

		g_ModeAlpha.replaceBlack.SetRgb(0, 0, 0);

		g_ModeAlpha.replaceWhite.SetRgb(255, 255, 255);
	}

	g_ModeAlpha.restore=false;

	if (Filming::MS_ENTITY == g_Filming.GetMatteStage())
	{
		Filming::DRAW_RESULT res = g_Filming.shouldDraw(mode);

		if(Filming::DR_NORMAL == res)
		{
			// positive (white).

			g_ModeAlpha.restore = true;
			g_ModeAlpha.wasWhite = true;

			g_ModeAlpha.replaceWhite.OnGlBegin(mode);

			g_ModeAlpha.color.SetRgba(1, 1, 1, -1);
			g_ModeAlpha.color.OnGlBegin(mode);
		}
		else if(Filming::DR_MASK == res)
		{
			// negative (black).

			g_ModeAlpha.restore = true;
			g_ModeAlpha.wasWhite = false;

			g_ModeAlpha.replaceBlack.OnGlBegin(mode);

			g_ModeAlpha.color.SetRgba(0, 0, 0, -1);
			g_ModeAlpha.color.OnGlBegin(mode);
		}
		else
		{
			// (x-ray).
			return false;
		}
	}

	return true;
}

void ModeAlpha_End()
{
	if (g_ModeAlpha.restore)
	{
		g_ModeAlpha.restore = false;

		g_ModeAlpha.color.OnGlEnd();

		if(g_ModeAlpha.wasWhite)
			g_ModeAlpha.replaceWhite.OnGlEnd();
		else
			g_ModeAlpha.replaceBlack.OnGlEnd();
	}
}


int		g_nViewports = 0;
bool	g_bIsSucceedingViewport = false;


void APIENTRY NewGlBegin(GLenum mode)
{
	if(!g_Host_Frame_Called) {
		glBegin(mode);
		return;
	}

#ifdef AFX_SCRIPT
	ScriptEvent_OnGlBegin(mode);
#endif // AFX_SCRIPT

	if (g_Filming.doWireframe(mode) == Filming::DR_HIDE) {
		return;
	}

	g_Filming.DoWorldFxBegin(mode); // WH fx

	g_Filming.DoWorldFx2(mode); // lightmap fx

	if (!g_Filming.isFilming())
	{
		glBegin(mode);
		return;
	}

	if(Filming::MM_KEY == g_Filming.GetMatteMethod())
	{
		if(!ModeKey_Begin(mode))
			return;
	}
	else
	{
		if(!ModeAlpha_Begin(mode))
			return;
	}

	glBegin(mode);
}

void APIENTRY NewGlEnd(void)
{
	if(!g_Host_Frame_Called) {
		glEnd();
		return;
	}

	glEnd();

	if(Filming::MM_KEY == g_Filming.GetMatteMethod())
		ModeKey_End();
	else
		ModeAlpha_End();

	g_Filming.DoWorldFxEnd();

#ifdef AFX_SCRIPT
	ScriptEvent_OnGlEnd();
#endif // AFX_SCRIPT
}

void APIENTRY NewGlClear(GLbitfield mask)
{
	if(!g_Host_Frame_Called) {
		glClear(mask);
		return;
	}

	// check if we want to clear (it also might set clearcolor and stuff like that):
	if (!g_Filming.checkClear(mask))
		return;

	glClear(mask);
}

void APIENTRY NewGlViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	if(!g_Host_Frame_Called) {
		glViewport(x,y,width,height);
		return;
	}

	static bool bFirstRun = true;

	g_bIsSucceedingViewport = true;

	if (bFirstRun)
	{
#if MDT_DEBUG
		MessageBox(0,"First NewGlViewport","MDT_DEBUG",MB_OK|MB_ICONINFORMATION);
#endif

		bFirstRun = false;
	}


	// Only on the first viewport
	if (g_nViewports == 0)
	{
#ifdef AFX_GUI
		AfxGui_SetViewport(width, height);
#endif

		//g_Filming.setScreenSize(width, height);

		//
		// gl_ztrick:
		// we force it to 0 by default, since we don't want the code for 1 to kick in (see Quake 1 source code for more info)
		if (gl_force_noztrick->value)
			pEngfuncs->Cvar_SetValue("gl_ztrick_old", 0);


		// Always get rid of auto_director
		if (disableautodirector->value != 0.0f)
			pEngfuncs->Cvar_SetValue("spec_autodirector", 0.0f);

		// This is called whether we're zooming or not
		g_Zooming.handleZoom();

		if (g_Aiming.isAiming())
			g_Aiming.aim();
	}

	// Not necessarily 5 viewports anymore, keep counting until reset
	// by swapbuffers hook.
	g_nViewports++;

	g_Zooming.adjustViewportParams(x, y, width, height);
	glViewport(x, y, width, height);

}

void APIENTRY NewGlFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	if(!g_Host_Frame_Called) {
		glFrustum(left, right, bottom, top, zNear, zFar);
		return;
	}

	g_Filming.SupplyZClipping(zNear, zFar);
	g_Zooming.adjustFrustumParams(left, right, top, bottom);
	glFrustum(left, right, bottom, top, zNear, zFar);
}


// TODO: for hudalpha streams color mask maybe temporary wrong
// for HUD elements that don't use blending (luckily almost all do).
void APIENTRY NewGlBlendFunc (GLenum sfactor, GLenum dfactor)
{
	if(!g_Host_Frame_Called) {
		glBlendFunc(sfactor,dfactor);
		return;
	}


	switch(g_Filming.giveHudRqState())
	{
	case Filming::HUDRQ_CAPTURE_ALPHA:
		if (dfactor == GL_ONE)//(sfactor == dfactor == GL_ONE)
		{
			// block the Alpha chan of Additive sprites
			glColorMask(TRUE, TRUE, TRUE, FALSE); // block alpha for additive HUD sprites
			glBlendFunc(sfactor,dfactor);
		}
		else
		{
			// don't let sprites lower alpha value:
			glColorMask(TRUE, TRUE, TRUE,TRUE); // allow alpha
			if (sfactor==GL_SRC_ALPHA) sfactor=GL_SRC_ALPHA_SATURATE;
			if (dfactor==GL_SRC_ALPHA) dfactor=GL_SRC_ALPHA_SATURATE;
			glBlendFunc(sfactor,dfactor);
		}
		break;
	default :
		glBlendFunc(sfactor,dfactor);
	}
}

BOOL APIENTRY NewWglSwapBuffers(HDC hDC);
CAfxImportFuncHook<BOOL(APIENTRY*)(HDC)> g_Import_GDI32_SwapBuffers("SwapBuffers", NewWglSwapBuffers);
CAfxImportFuncHookBase* g_pImport_GDI32_SwapBuffers = &g_Import_GDI32_SwapBuffers;
BOOL APIENTRY NewWglSwapBuffers(HDC hDC)
{
	BOOL bResWglSwapBuffers;
	bool bRecordSwapped=false;

	// Next viewport will be the first of the new frame
	g_nViewports = 0;

#ifdef AFX_GUI
	// Give the Gui a chance to render:
	// TODO: this needs to be moved elsewhere in case we don't want to fuck up
	// the main image with the GUI.
	AfxGui_Render();
#endif

#ifdef AFX_SCRIPT
	if(ScriptEvent_OnSwapBuffers(hDC, bResWglSwapBuffers))
	{
		// reset call trackers:
		Reset_R_RenderViewCalledFromEngine();
		Reset_UnkDrawHudCalledFromEngine();

		return bResWglSwapBuffers;
	}
#endif // AFX_SCRIPT

	if (g_Filming.isFilming())
	{
		// we are filming, force buffers and capture our image:
		
		// record the selected buffer (capture):
		bRecordSwapped = g_Filming.recordBuffers(hDC, &bResWglSwapBuffers);
	}

	// do the switching of buffers as requersted:
	if (!bRecordSwapped)
	{
		if (g_pSupportRender)
			bResWglSwapBuffers = g_pSupportRender->hlaeSwapBuffers(hDC);
		else
			bResWglSwapBuffers = g_Import_GDI32_SwapBuffers.TrueFunc(hDC);
	}

	// no we have captured the image (by default from backbuffer) and display it on the front, now we can prepare the new backbuffer image if required.

	if (g_Filming.isFilming())
	{
		// we are filming, do required clearing and restore buffers:

		// carry out preparerations on the backbuffer for the next frame:
		g_Filming.FullClear();
	}
	else if(g_Host_Frame_Called && gl_previewclear->value)
		g_Filming.FullClear();

	// reset call trackers:
	Reset_R_RenderViewCalledFromEngine();
	Reset_UnkDrawHudCalledFromEngine();

	return bResWglSwapBuffers;
}


HGLRC Init_Support_Renderer(HWND hMainWindow, HDC hMainWindowDC, int iWidth, int iHeight);

HGLRC WINAPI NewWglCreateContext(HDC hDc)
{
	static int iCallCount = 0;
	iCallCount++;
	if(2 == iCallCount)
		return Init_Support_Renderer( g_GameWindow, hDc, g_AfxSettings.Width_get(), g_AfxSettings.Height_get() );

	return wglCreateContext(hDc);
}


BOOL WINAPI NewWglMakeCurrent(HDC hDc, HGLRC hGlRc)
{
	if (hGlRc && g_pSupportRender && g_pSupportRender->GetHGLRC() == hGlRc)
		return g_pSupportRender->hlaeMakeCurrent(hDc, hGlRc);

	BOOL bRet = wglMakeCurrent(hDc, hGlRc);

	return bRet;

}


BOOL WINAPI NewWglDeleteContext(HGLRC hGlRc)
{
	if (hGlRc && g_pSupportRender && g_pSupportRender->GetHGLRC() == hGlRc)
		return g_pSupportRender->hlaeDeleteContext(hGlRc);

	return wglDeleteContext(hGlRc);
}


//
// support functions:
//

HGLRC Init_Support_Renderer(HWND hMainWindow, HDC hMainWindowDC, int iWidth, int iHeight)
{
	if(g_pSupportRender)
		return NULL; // already created
	
	// determine desired target renderer:
	CHlaeSupportRender::ERenderTarget eRenderTarget = CHlaeSupportRender::RT_GAMEWINDOW;

	switch(g_AfxSettings.RenderMode_get())
	{
	case AfxSettings::RenderMode_FrameBufferObject:
		eRenderTarget = CHlaeSupportRender::RT_FRAMEBUFFEROBJECT;
		break;
	case AfxSettings::RenderMode_MemoryDC:
		eRenderTarget = CHlaeSupportRender::RT_MEMORYDC;
	}

	// Init support renderer:
	g_pSupportRender = new CHlaeSupportRender(hMainWindow, iWidth, iHeight);

	HGLRC tHGLRC;
	tHGLRC = g_pSupportRender->hlaeCreateContext(eRenderTarget,hMainWindowDC);

	if (!tHGLRC)
		MessageBoxA(0, "hlaeCreateContext failed.", "Init_Support_Renderer ERROR", MB_OK|MB_ICONERROR);

	return tHGLRC;
}
