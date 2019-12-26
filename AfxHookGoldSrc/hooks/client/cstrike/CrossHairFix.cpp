#include "stdafx.h"

// See also: /doc/notes_goldsrc/debug_cstrike_crosshair.txt

#include "CrossHairFix.h"

#include <windows.h>

#include <shared/AfxDetours.h>
#include <hlsdk.h>
#include "../../../hl_addresses.h"

#include "../../HookHw.h"
#include "../../hw/Host_Frame.h"

#include <gl/gl.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>


typedef void (__fastcall *UnkCstrikeCrosshairFn_t)( void * This, void * edx, float dwUnk1, DWORD dwUnk2);

bool g_Cstrike_CrossHair_Block = false;
UnkCstrikeCrosshairFn_t g_pfnCrosshairFix_Hooked_Func = NULL;
double *g_f_ch_mul_fac = NULL;
double *g_f_ch_add_fac = NULL;

double g_cstrike_ch_frameT = 1.0/100.0;


void __fastcall CrosshairFix_Hooking_Func(void * This, void * edx, float fUnkTime, DWORD dwUnkWeaponCode )
{
	static float oldClientTime = 0;
	static double deltaT = 0;

	bool freezeCh = g_Cstrike_CrossHair_Block;
	bool fix = !freezeCh && 0.0 < g_cstrike_ch_frameT;
	
	if(fix)
	{
		double frameTime = pEngfuncs->pfnGetCvarFloat("host_framerate");
		if(0 >= frameTime) frameTime = *g_phost_frametime;

		deltaT += frameTime;
		
		bool coolDown = g_cstrike_ch_frameT <= deltaT;

		if(coolDown)
		{
			// apply cooldown:
			bool doLoop;
			GLboolean oldMasks[5];
			do
			{
				deltaT -= g_cstrike_ch_frameT;

				doLoop = g_cstrike_ch_frameT <= deltaT;

				if(doLoop)
				{
					glGetBooleanv(GL_COLOR_WRITEMASK, oldMasks);
					glGetBooleanv(GL_DEPTH_WRITEMASK, &(oldMasks[4]));

					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
				}

				g_pfnCrosshairFix_Hooked_Func( This, edx, fUnkTime, dwUnkWeaponCode );

				if(doLoop)
				{
					glColorMask(oldMasks[0], oldMasks[1], oldMasks[2], oldMasks[3]);
					glDepthMask(oldMasks[4]);
				}
			}
			while(doLoop);

			return; // done.
		}
		else
			// keep it frozen:
			freezeCh = true;
	}

	if(freezeCh)
	{
		// do not apply any cool down, just make it drawn:

		MdtMemBlockInfos mbisMul, mbisAdd;
		double fOldMulFac, fOldAddFac;

		MdtMemAccessBegin(g_f_ch_mul_fac, sizeof(double), &mbisMul);
		MdtMemAccessBegin(g_f_ch_add_fac, sizeof(double), &mbisAdd);

		fOldMulFac = *g_f_ch_mul_fac;
		fOldAddFac = *g_f_ch_add_fac;

		*g_f_ch_mul_fac = 0.0f;
		*g_f_ch_add_fac = 0.0f;

		g_pfnCrosshairFix_Hooked_Func( This, edx, fUnkTime, dwUnkWeaponCode );

		*g_f_ch_mul_fac = fOldMulFac;
		*g_f_ch_add_fac = fOldAddFac;

		MdtMemAccessEnd(&mbisAdd);
		MdtMemAccessEnd(&mbisMul);
	}
	else
		// Normal (unfixed) operation.
		g_pfnCrosshairFix_Hooked_Func( This, edx, fUnkTime, dwUnkWeaponCode );
}


double Cstrike_CrossHair_Fps_get()
{
	return 0.0 != g_cstrike_ch_frameT ? 1.0 / g_cstrike_ch_frameT : 0.0;
}


void Cstrike_CrossHair_Fps_set(double value)
{
	g_cstrike_ch_frameT = 0.0 != value ? 1.0 / value : 0.0;
}


bool Hook_Cstrike_CrossHair_Fix()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	double * addrMul = (double *)HL_ADDR_GET(cstrike_UnkCrosshairFn_mul_fac);
	double * addrAdd = (double *)HL_ADDR_GET(cstrike_UnkCrosshairFn_add_fac);
	BYTE * addrFn = (BYTE *)HL_ADDR_GET(cstrike_UnkCrosshairFn);

	if (!(
		addrMul && addrAdd && addrFn
		))
	{
		firstResult = false;
	}
	else
	{
		LONG error = NO_ERROR;

		g_f_ch_mul_fac = addrMul;
		g_f_ch_add_fac = addrAdd;
		g_pfnCrosshairFix_Hooked_Func = (UnkCstrikeCrosshairFn_t)addrFn;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_pfnCrosshairFix_Hooked_Func, CrosshairFix_Hooking_Func);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_Cstrike_CrossHair_Fix()");
		}
	}

	return firstResult;
}
