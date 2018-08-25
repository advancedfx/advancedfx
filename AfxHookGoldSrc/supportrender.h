#pragma once

// Comment:
//   Due to the OpenGL FrameBuffer contents depending on the Pixel OwnerShip
//   Test implementation of the used rendering hardware / software, we cannot
//   safely read from the usual GameWindow with glReadPixels, because window
//   parts that are not on-screen or covered by other windows may cause
//   those implementations to discard fragments for these areas, which can
//   result in black or random areas.
//
//   The CHlaeSupportRender class has been created to deal with this problem
//   by supplying additional render targets. But not all of these targets can
//   be considered to be supported by the customer's GL implementation or to
//   work as expected.
//
//   The following targets are supported by the class:
//
//   RT_GAMEWINDOW:
//     This is the default render target, and means using the original game
//     window. This will most likely cause problems with NVIDIA cards which
///    may omit covered window parts in the frame buffer.
//     However in case of problems with the other targets this is our last
//     resort.
//
//  RT_HIDDENWINDOW: // not implemented yet
//    Renders to a hidden window.
//    May still suffer from missing image aereas for some users.
//    May be slow (not accelerated) for some users.
//
//  RT_MEMORYDC:
//    This renders to a Memory Device Context's bitmap. This may work for many
//    users, but may be also very SLOW (not accelerated) for many of them.
//
//  RT_FRAMEBUFFEROBJECT:
//    http://www.opengl.org/registry/specs/EXT/framebuffer_object.txt
//    This is actually what we are longing for, however this requires the
//    EXT_framebuffers_object OpenGL extension (implemented at least to fit
//    our needs to render to RenderBuffer Objects), which is not present in
//    older implementations.
//    If present this will render to a FrameBuffer Object (to which we assign
//    a GL_RGBA and a GL_DEPTH_COMPONENT RenderBuffer Object, so no stenceling
//    is buffered at the moment).
//    If not present or not fully implemented to our needs, this target will
//    switch to RT_GAMEWINDOW (if possible) in the first hlaeMakeCurrent call,
//    currently a error box informing the user will pop up.
//    For technical reasons the drawing on the game window has to happen
//    using the texture units (if we want to avoid slow copies over system
//    memory), read "(42) What set of framebuffer targets should the initial
//    extension support?" in the EXT specs for the reasons.
//    For technical reasons this uses a hardcoded texture id: FBO_TEXUTRE_ID
//    This target also depends on the correct calling of hlaeOnFilmingStart()
//    and hlaeOnFilmingStop()

#include <windows.h>

class CHlaeSupportRender
{
public:
	// FBO texture ID: (gen textures won't do, because H-L will overwrite it
	#define FBO_TEXUTRE_ID 21337

	enum ERenderTarget
	{
		RT_NULL, // not set yet
		RT_GAMEWINDOW, // read comment at top of file
		RT_HIDDENWINDOW, // . // not implemented yet!
		RT_MEMORYDC, // .
		RT_FRAMEBUFFEROBJECT // .
	};

	enum EExtSupport
	{
		EXTS_UNKOWN, // could not be determined (yet)
		EXTS_NO, // is not supported
		EXTS_YES //is supported
	};

	CHlaeSupportRender(HWND hGameWindow, int iWidth, int iHeight);
	// defaults RenderTarget to RT_NULL
	// hGameWindow - HWND Window Handle of the GameWindow
	// iWidth - total width in pixels of the image data (GameResolution)
	// iHeight - total height in pixels of the image data (GameResolution)
	// MANDATORY for these targets: all;

	~CHlaeSupportRender();
	// if RenderTarget is different from RT_NULL hlaeDeleteContext will be implecitly called on destroy
	// MANDATORY for these targets: all;

	EExtSupport Has_EXT_FrameBufferObject();
	// will only return s.th. useful when hlaeMakeCurrent was called after
	// hlaeCreateContext was called successfully with RT_FRAMEBUFFEROBJECT as eRenderTarget
	// returns
    //   EXTS_YES     : EXT_framebuffer_object supported (doesn't mean it supports all we will need though)
	//   EXTS_NO      : not supported 
	//   EXTS_UNKOWN : was unable to determine support (error, or called to early)

	ERenderTarget GetRenderTarget();
	// returns the current RenderTarget

	HGLRC GetHGLRC();
	// returns handle to the render target's HGLRC or NULL on error

	HDC GetInternalHDC();
	// Only valid when RenderTarget is one of these: RT_MEMORYDC, RT_HIDDENWINDOW;
	// returns the handle to the internal HDC, or NULL on error

	HWND GetInternalHWND();
	// Only valid when RenderTarget is one of these: RT_HIDDENWINDOW;
	// returns the handle to the internal HWND, or NULL on error

	//
	// Functions that need to be called by the hook:
	//   (with a few exceptions)
	//

	HGLRC hlaeCreateContext (ERenderTarget eRenderTarget, HDC hGameWindowDC);
	// this should be placed in a wglCreateContext hook
	// ATTENTION: Make sure you use this for the right HDC and the right CreateContext call only!
	// ATTENTION: If there is already a managed render target, the call will fail!
	// returns NULL on fail, otherwise the OpenGLContext device handle
	// eRenderTarget -> read the comment at the top of this file
	// hGameWindowDC -> DC That we shall derive from
	// MANDATORY for these targets: all;
	//
	// To speed up rendering you can:
	// - avoid using RT_MEMORYDC (use RT_FRAMEBUFFEROBJECT or RT_GAMEWINDOW instead whenever possible)
	// - reduce calls to DisplayRenderTarget in case you don't need to display every frame
	
	BOOL hlaeDeleteContext (HGLRC hGlRc);
	// this should be placed in a wglDelteContext hook
	// ATTENTION: If no render target is present (RT_NULL) or hGlRc is not the one managed by this class, this call will fail.
	// may also be called ~CHlaeSupportRender() is RenderTarget is different from RT_NULL
	// MANDATORY for these targets: all;

	BOOL hlaeMakeCurrent(HDC hGameWindowDC, HGLRC hGlRc);
	// this should be placed in a wglMakeCurrent hook
	// ATTENTION: If no render target is present (RT_NULL) or hGlRc is not the one managed by this class, this call will fail.
	// MANDATORY for these targets: RT_MEMORYDC, RT_FRAMEBUFFEROBJECT;

	BOOL hlaeSwapBuffers(HDC hGameWindowDC);
	// this should be placed in a [wgl]SwapBuffers hook
	// ATTENTION: If no render target is present (RT_NULL) this call will fail.
	// ATTENTION: the caller has to make sure, that the context that is to be swapped is the right one
	// MANDATORY for these targets: RT_MEMORYDC, RT_FRAMEBUFFEROBJECT;

	void hlaeOnFilmingStart();
	// should be called by the hook filming system to switch into filming mode (might be slower)
	// ATTENTION: for technical reasons this may react delayed by one frame! So better drop one frame to be sure.
	// MANDATORY for these targets: RT_FRAMEBUFFEROBJECT;

	void hlaeOnFilmingStop();
	// should be called by the hook filming system to leave filming mode
	// MANDATORY for these targets: RT_FRAMEBUFFEROBJECT;

private:
	ERenderTarget _eRenderTarget;

	HWND _hGameWindow;
	int _iWidth;
	int _iHeight;

	EExtSupport _EExtSupported; // EXT_framebuffer_object support check

	// shared:
	HGLRC	_ownHGLRC;

	struct _FilmingState_s
	{
		bool isFilming; // this is what we are currently doing
		bool wantsFilming; // this is what the filming system says us to do
	} _FilmingState_r;

	// for FrameBufferObject only:
	struct _FrameBufferObject_s
	{
		unsigned int FBOid;
		unsigned int depthRenderBuffer;
		unsigned int rgbaRenderTexture;
	} _FrameBufferObject_r;

	// for MemoryDc only:
	struct _MemoryDc_s
	{
		HDC		ownHDC;
		HBITMAP	ownHBITMAP;
	} _MemoryDc_r;

	// for HiddenWindow only:
	struct _HiddenWindow_s
	{
		HWND	ownHWND;
		HDC		ownHDC;
		HBITMAP	ownHBITMAP;
	} _HiddenWindow_r;

	// functions:

	HGLRC	_Create_RT_GAMEWINDOW (HDC hGameWindowDC);
	BOOL	_Delete_RT_GAMEWINDOW ();

	HGLRC	_Create_RT_MEMORYDC (HDC hGameWindowDC);
	BOOL	_Delete_RT_MEMORYDC ();
	BOOL	_MakeCurrent_RT_MEMORYDC (HDC hGameWindowDC);
	BOOL	_SwapBuffers_RT_MEMORYDC (HDC hGameWindowDC);

	HGLRC	_Create_RT_HIDDENWINDOW (HDC hGameWindowDC);
	BOOL	_Delete_RT_HIDDENWINDOW ();
	BOOL	_MakeCurrent_RT_HIDDENWINDOW (HDC hGameWindowDC);
	BOOL	_SwapBuffers_RT_HIDDENWINDOW (HDC hGameWindowDC);

	HGLRC	_Create_RT_FRAMEBUFFEROBJECT (HDC hGameWindowDC);
	BOOL	_Delete_RT_FRAMEBUFFEROBJECT ();
	BOOL	_MakeCurrent_RT_FRAMEBUFFEROBJECT (HDC hGameWindowDC); // this may instantly change target back to RT_GAMEWINDOW
	BOOL	_SwapBuffers_RT_FRAMEBUFFEROBJECT (HDC hGameWindowDC);
	void	_Delete_RT_FRAMEBUFFEROBJECT_onlyFBO ();
};

extern CHlaeSupportRender * g_pSupportRender;
