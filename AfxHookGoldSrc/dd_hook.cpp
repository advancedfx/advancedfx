/*
File        : dd_hook.cpp
Started     : 2007-09-09 11:53:06
Project     : Mirv Demo Tool
Authors     : Dominik Tugend // add yourself here if u change s.th.
Description : See dd_hook.h
*/

#include "mdt_debug.h"

#include <hooks/shared/detours.h>

#include "dd_hook.h"
#include <ddraw.h>
#include <stdio.h>

// includes and externs for half-life:

// BEGIN HLSDK includes
//
// HACK: prevent cldll_int.h from messing the HSPRITE definition,
// HLSDK's HSPRITE --> MDTHACKED_HSPRITE
#pragma push_macro("HSPRITE")
#define HSPRITE MDTHACKED_HSPRITE
//
#include <hlsdk/multiplayer/cl_dll/wrect.h>
#include <hlsdk/multiplayer/cl_dll/cl_dll.h>
#include <hlsdk/multiplayer/engine/cdll_int.h>
#include <hlsdk/multiplayer/common/cvardef.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
// END HLSDK includes

extern cl_enginefuncs_s *pEngfuncs;


typedef HRESULT (WINAPI *DirectDrawCreate_t) ( GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter);
DirectDrawCreate_t g_oldDirectDrawCreate=NULL; // stores our OldDirectDraw addres

LPDDENUMMODESCALLBACK g_oldEnumModesCallback=NULL;
bool g_bEnumModesCallbackFirst=false;

bool	g_dd_hook_bForceChecked = false;
bool	g_dd_hook_bForceEnabled = false;
int		g_dd_hook_iWidth  = 640;
int		g_dd_hook_iHeight = 480;
int		g_dd_hook_iBpp    = 32;

void FixCallBackFunction (DWORD *HalfLifeCallBackAddr)
// this fixes the HL Callback function in order to circumvent the most checks
{
	// write at HalfLifeCallBackAddr+(0x001dc5a40-0x001dc5a30)==HalfLifeCallBackAddr+0x10
	// JMP forward (01dc5a78-01dc5a40-2 ==0x36) Bytes --> BYTECODE: EB 36

	static char	jmp36 [2]	= {(CHAR)0xEB,(CHAR)0x36};
	DWORD		*writeAddr	= HalfLifeCallBackAddr+0x04; // 4*0x04 = 0x10 
	MdtMemBlockInfos mbis;

	MdtMemAccessBegin((DWORD *)writeAddr,2, &mbis);
	memcpy((DWORD *)writeAddr,jmp36,2);
	MdtMemAccessEnd(&mbis);
}

HRESULT WINAPI myEnumModesCallback(
  LPDDSURFACEDESC lpDDSurfaceDesc, 
  LPVOID lpContext
)
// FixCallBackFunction has removed some checks for us
// the only checks that are left are:
// - if the height supplied in lpDDSurfaceDesc->dwHeight is >=0x1E0
// - if bitdepth supplied in lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == *lpContext
// HL will use lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->dwWidht as Width, lpDDSurfaceDesc->dwHeight as Height and (lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount or *lpContext) as BitsPerPixel
{
	HRESULT hResult=NULL;
	//static char test[100];

	lpDDSurfaceDesc->dwWidth=g_dd_hook_iWidth;
	lpDDSurfaceDesc->dwHeight=g_dd_hook_iHeight;
	lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount=g_dd_hook_iBpp;

	//if (g_bEnumModesCallbackFirst)
	//{
		//g_bEnumModesCallbackFirst=false;
		//sprintf(test,"Address: 0x%08x\nlpContext: 0x%08x",(DWORD)g_oldEnumModesCallback,(DWORD)lpContext); // 0x01dc5a30 0x0248aba4
		//MessageBox(NULL,test,"MDT EnumModesCallBack hook",MB_OK);
	//}

	// now call the old one:
	memcpy(lpContext,&(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount),sizeof(DWORD));
	g_oldEnumModesCallback(lpDDSurfaceDesc,lpContext);

	return hResult;
}

// from ddraw.h (inside a c++ calls definition):    STDMETHOD(EnumDisplayModes)( THIS_ DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK ) PURE;
// we want to reassemble the function as c++ interface and simulate it to be sitting in a class:

typedef HRESULT (STDMETHODCALLTYPE *EnumDisplayModes_t)(DWORD *this_ptr, DWORD dwFlags,LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext,LPDDENUMMODESCALLBACK lpEnumModesCallback);
EnumDisplayModes_t  g_oldEnumDisplayModes=NULL;

HRESULT STDMETHODCALLTYPE myEnumDisplayModes(
  DWORD *this_ptr,
  DWORD dwFlags, 
  LPDDSURFACEDESC lpDDSurfaceDesc, 
  LPVOID lpContext, 
  LPDDENUMMODESCALLBACK lpEnumModesCallback
)
{
	HRESULT hResult=NULL;

	g_oldEnumModesCallback=lpEnumModesCallback; // this will be needed by our own new callback
	g_bEnumModesCallbackFirst=true; // since we have to dectet if we were called the first time or not

    // first fix the old function to allow more modes:
	FixCallBackFunction((DWORD *)g_oldEnumModesCallback);

	hResult=g_oldEnumDisplayModes(this_ptr,dwFlags,lpDDSurfaceDesc,lpContext,myEnumModesCallback);

	return hResult;
}

char g_oourIDirectDraw_Memory [sizeof(DWORD *)*IDD_IFACE_FUNCS_CNT];
IDirectDraw* g_newIDirectDraw = (IDirectDraw *)g_oourIDirectDraw_Memory;


HRESULT WINAPI myDirectDrawCreate(
  GUID FAR* lpGUID, 
  LPDIRECTDRAW FAR* lplpDD, 
  IUnknown FAR* pUnkOuter
)
{
	// if not done already check for forcing:
	if (!g_dd_hook_bForceChecked)
	{
#ifdef MDT_DEBUG
	MessageBox(0,"myDirectDrawCreate - firstcall","MDT_DEBUG",MB_OK|MB_ICONINFORMATION);
#endif

		// Check the commandline if we are ment to force a res:
		char		*pstart = NULL ;
		int i = pEngfuncs->CheckParm( "-mdtres", &pstart );
		if ( i && pstart )
		{
			char	ctmp [max(MAXLEN_HLRES,MAXLEN_HLCOLOR)+1];
			char	*dlm1=NULL;
			char	*dlm2=NULL;
			int		iw,ih,ib;
			DWORD	dlen;
//			DWORD	dlentot=strlen(pstart);
//			DWORD	dlenmax=max(MAXLEN_HLRES,MAXLEN_HLCOLOR);

			memset(ctmp,0,max(MAXLEN_HLRES,MAXLEN_HLCOLOR)+1);

			dlm1 = strchr(pstart,'x');
			if (dlm1)
				dlm2 = strchr(dlm1+1,'x');

			if(dlm1&&dlm2)
			{
				// Width:
				dlen=(DWORD)(dlm1)-(DWORD)(pstart); if (dlen>MAXLEN_HLRES) dlen=MAXLEN_HLRES;
				strncpy(ctmp,pstart,dlen); *(ctmp+dlen)=0;
				iw=atoi(ctmp); if (iw<4) iw=4;

				//MessageBox(NULL,ctmp,"MDT force width",MB_OK);

				dlm1++; // move it over the "x"

				// Height:
				dlen=(DWORD)(dlm2)-(DWORD)(dlm1); if (dlen>MAXLEN_HLRES) dlen=MAXLEN_HLRES;
				strncpy(ctmp,dlm1,dlen); *(ctmp+dlen)=0;
				ih=atoi(ctmp); if (ih<480) iw=480;

				//MessageBox(NULL,ctmp,"MDT force height",MB_OK);

				dlm2++; // move it over the "x"

				// BitsPerPixel:
				strncpy(ctmp,dlm2,MAXLEN_HLCOLOR); *(ctmp+MAXLEN_HLCOLOR)=0;
				ib=atoi(ctmp); if (ib<1) ib=1;

				//MessageBox(NULL,ctmp,"MDT force bpp",MB_OK);

				g_dd_hook_bForceEnabled = true;
				g_dd_hook_iWidth  = iw;
				g_dd_hook_iHeight = ih;
				g_dd_hook_iBpp    = ib;
			}
		}
	} // if (!g_dd_hook_bForceChecked)

	// call old DirectDrawCreate:
	HRESULT hResult=g_oldDirectDrawCreate(lpGUID,lplpDD,pUnkOuter);

	if(g_dd_hook_bForceEnabled&&(hResult==DD_OK)&&lplpDD)
	{

#ifdef MDT_DEBUG
		MessageBox(0,"myDirectDrawCreate - about to patch","MDT_DEBUG",MB_OK|MB_ICONINFORMATION);
#endif

		//static char test[100];
		//DWORD **newlpDD=(DWORD **)*lplpDD;
		//sprintf(test,"Address: 0x%08x",*(*newlpDD+8));
		//MessageBox(NULL,test,"MDT",MB_OK); // Address of EnumDisplayModes
		
		//
		// In order to under stand what we do here, you have to understand lplpDD. This is how I understand it:
		//
		// lplpDD is (a pointer to) the address of a varliable we shall call lpDD. The contents of lpDD is what Half-Life will use in order to interface DirectDraw.
		// lpDD is (a pointer to) the address of a variable that is a pointer to an IDirectDraw interface, we shall call it DD
		// so DD is our class pointer to the abstract IDirectDraw interface class that points onto the first function of the series of functions in the class
		//
		
		DWORD **lpDD=(DWORD **)*lplpDD;
		memcpy(&g_oldEnumDisplayModes,*lpDD+8,4);

		// Copy the interface:
		// The reason is that we can't place a call from an address in ddraw.dll to an address into our own (HL's) memory, that is why (I decided that) we need to have a local copy of the whole structure that we give to HL instead with only our desired func replaced.

		DWORD* newDDiface = (DWORD *)g_newIDirectDraw;
		memcpy(newDDiface,(DWORD *)*lpDD,sizeof(DWORD *)*IDD_IFACE_FUNCS_CNT); // make copy of the function table
		
		// for understanding:
		// g_newIDDirectDraw is our new class pointer now (onto the function table), that means it takes the role of DD now.
		
		// replace the old EnumDisplayMode address with our own one:

		// Don't froget that in C++ per definition (DWORD)&functionPointer == (DWORD)functionPointer:
		DWORD uDontFuckWithMe = (DWORD)myEnumDisplayModes; // not with me u bastard of a compiler thingee
		memcpy(newDDiface+8,&uDontFuckWithMe,4);

		// now replace lpDD with the address of the new function table pointer, so HL shall use our table now instead:
		memcpy(*lplpDD,&g_newIDirectDraw,sizeof(DWORD *)); // replace lpDD comment

		// for debug
		//BOOL bbb = (DWORD)&uDontFuckWithMe == (DWORD)uDontFuckWithMe;
		//sprintf(test,"Address: 0x%08x 0x%08x 0x%08x %i",(DWORD)*(*newlpDD+8),(DWORD)&uDontFuckWithMe,(DWORD)uDontFuckWithMe,bbb);
		//MessageBox(NULL,test,"MDT",MB_OK); // should carry our myEnumDisplayModes address now
	}

	return hResult;
}

FARPROC WINAPI Hook_DirectDrawCreate(FARPROC fpOldAddress)
{	
#ifdef MDT_DEBUG
	MessageBox(0,"Hook_DirectDrawCreate - called","MDT_DEBUG",MB_OK|MB_ICONINFORMATION);
#endif

	if( !fpOldAddress )
		return fpOldAddress;

	if (!g_oldDirectDrawCreate) g_oldDirectDrawCreate=(DirectDrawCreate_t)fpOldAddress; // save old address for us
	return (FARPROC)myDirectDrawCreate; // set our hooked struct
}