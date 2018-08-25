#include "stdafx.h"

#include <windows.h>
#include <gl\gl.h>

#include <hlsdk.h>

// Own includes:
#include "cmdregister.h"
#include "hl_addresses.h"


extern cl_enginefuncs_s* pEngfuncs;
extern playermove_s* ppmove;


REGISTER_CMD_FUNC(whereami)
{
	float angles[3];
	pEngfuncs->GetViewAngles(angles);
	pEngfuncs->Con_Printf("Location: %fx %fy %fz\nAngles: %fx %fy %fz\n", ppmove->origin.x, ppmove->origin.y, ppmove->origin.z, angles[0], angles[1], angles[2]);
}

void PrintDebugPlayerInfo(cl_entity_s *pl,int itrueindex)
{
	static hud_player_info_t m_hpinfo;

	memset(&m_hpinfo,0,sizeof(hud_player_info_t));
	pEngfuncs->pfnGetPlayerInfo(pl->index,&m_hpinfo);
	pEngfuncs->Con_Printf("%i (%s): %i, %s, %s, %i, %i, %i, %i, %i, %i\n",itrueindex,(pl->curstate.effects & EF_NODRAW) ? "y" : "n",pl->index,m_hpinfo.name,m_hpinfo.model,m_hpinfo.ping,m_hpinfo.packetloss,m_hpinfo.topcolor,m_hpinfo.bottomcolor,m_hpinfo.spectator,m_hpinfo.thisplayer);
}

REGISTER_CMD_FUNC(listplayers)
{
	bool bLocalListed=false;
	int iLocalIndex=-1;
	cl_entity_s *plocal = pEngfuncs->GetLocalPlayer();

	pEngfuncs->Con_Printf("Listing Players (max: %i)\ntrue index (EF_NODRAW): index, name, model, ping, packetloss, topcolor, bottomcolor, spectator, thisplayer\n",pEngfuncs->GetMaxClients());
	for (int i = 0; i <= pEngfuncs->GetMaxClients(); i++)
	{
		cl_entity_t *e = pEngfuncs->GetEntityByIndex(i);
		if (e && e->player)
		{
			PrintDebugPlayerInfo(e,i);
			//if (e->index==plocal->index)
			if (e == plocal)
			{
				bLocalListed=true;
				iLocalIndex=i;//e->index;
			}
		} else if (e == plocal) iLocalIndex=i;
	}

	if (bLocalListed)
	{
		pEngfuncs->Con_Printf("The local player is index %i.\n",iLocalIndex);
	} else {
		pEngfuncs->Con_Printf("The local player is hidden (not flagged as player):\n",iLocalIndex);
		PrintDebugPlayerInfo(plocal,iLocalIndex);
	}

}

// _mirv_info - Print some informations into the console that might be usefull. when people want to report problems they should copy the console output of the command.
REGISTER_DEBUGCMD_FUNC(info)
{
	GLint gi;
	pEngfuncs->Con_Printf(">>>> >>>> >>>> >>>>\n");
	pEngfuncs->Con_Printf("MDT_DLL_VERSION: %s\n", __DATE__);
	pEngfuncs->Con_Printf("GL_VENDOR: %s\n",glGetString(GL_VENDOR));
	pEngfuncs->Con_Printf("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
	pEngfuncs->Con_Printf("GL_VERSION: %s\n",glGetString(GL_VERSION));
	pEngfuncs->Con_Printf("GL_EXTENSIONS: %s\n",glGetString(GL_EXTENSIONS));
	glGetIntegerv(GL_PACK_ROW_LENGTH,&gi);
	pEngfuncs->Con_Printf("GL_PACK_ROW_LENGTH: %i\n",gi);
	glGetIntegerv(GL_PACK_ALIGNMENT,&gi);
	pEngfuncs->Con_Printf("GL_PACK_ALIGNMENT: %i\n",gi);
	pEngfuncs->Con_Printf("<<<< <<<< <<<< <<<<\n");
}

REGISTER_DEBUGCMD_FUNC(list_addresses) {
	unsigned int cnt = AfxAddr_Debug_GetCount();
	unsigned int zcnt = 0;
	for(unsigned int i=0; i<cnt; i++) {
		char const * pName;
		unsigned long addr;

		if(!AfxAddr_Debug_GetAt(i, addr, pName)) {
			pEngfuncs->Con_Printf("Error.\n");
			break; // error
		}

		if(!addr) zcnt++;

		pEngfuncs->Con_Printf("%s = 0x%08x\n", pName, addr);
	}
	pEngfuncs->Con_Printf("%u / %u were NULL.\n", zcnt, cnt);
}

/*


REGISTER_DEBUGCMD_FUNC(debug_devicecontext)
{
	HDC hdcResult;

	hdcResult=GetDC(g_HL_MainWindow);

	PIXELFORMATDESCRIPTOR *pPfd = new PIXELFORMATDESCRIPTOR;

	if (DescribePixelFormat( hdcResult, GetPixelFormat(hdcResult),sizeof(PIXELFORMATDESCRIPTOR),pPfd))
	{

		pEngfuncs->Con_Printf("DescribePixelFormat says:\nnSize:%u\nnVersion:%u\ndwFlags:0x%08x\niPixelType:%i\ncColorBits:%u\ncRedBits:%u\ncRedShift:%u\ncGreenBits:%u\ncGreenShift:%u\ncBlueBits:%u\ncBlueShift:%u\ncAlphaBits:%u\ncAlphaShift:%u\ncAccumBits:%u\ncAccumRedBits:%u\ncAccumGreenBits:%u\ncAccumBlueBits:%u\ncAccumAlphaBits:%u\ncDepthBits:%u\ncStencilBits:%u\ncAuxBuffers:%u\niLayerType:%i\nbReserved:%u\ndwLayerMask:%u\ndwVisibleMask:%u\ndwDamageMask:%u",
			pPfd->nSize, pPfd->nVersion, pPfd->dwFlags, pPfd->iPixelType,
			pPfd->cColorBits, pPfd->cRedBits, pPfd->cRedShift, pPfd->cGreenBits,
			pPfd->cGreenShift, pPfd->cBlueBits, pPfd->cBlueShift, pPfd->cAlphaBits,
			pPfd->cAlphaShift, pPfd->cAccumBits, pPfd->cAccumRedBits, pPfd->cAccumGreenBits,
			pPfd->cAccumBlueBits, pPfd->cAccumAlphaBits, pPfd->cDepthBits, pPfd->cStencilBits,
			pPfd->cAuxBuffers, pPfd->iLayerType, pPfd->bReserved, pPfd->dwLayerMask,
			pPfd->dwVisibleMask, pPfd->dwDamageMask);

	} else pEngfuncs->Con_Printf("DescribePixelFormat failed");

	delete pPfd;

	ReleaseDC(g_HL_MainWindow,hdcResult);
}



REGISTER_DEBUGCMD_FUNC(debug_wndproc_isontop)
{
	if( Hooking_WndProc != (WNDPROC)GetWindowLong( (HWND)g_HL_MainWindow, GWL_WNDPROC ))
		pEngfuncs->Con_Printf("S.th. hooked us!\n");
	else
		pEngfuncs->Con_Printf("We seem to be at top.\n");
}

REGISTER_DEBUGCMD_FUNC(debug_wndproc_rehook)
{
	SetWindowLongPtr( (HWND)g_HL_MainWindow, GWL_WNDPROC, (LONG)Hooking_WndProc );
}


*/


