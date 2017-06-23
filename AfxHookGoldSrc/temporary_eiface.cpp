 // 2007-12-23 21:18 UTC
// Half-Life Advanced Effects project

#include <windows.h>
#include "temporary_eiface.h"
#include "hl_addresses.h"
enginefuncs_t sHLeiface;
enginefuncs_t *pHLeiface=NULL;
enginefuncs_t *pHLeifaceHOOK=NULL;

void HLeiface_install()
{
	if (pHLeiface) return;

	// not installed yet:

	uint32 myaddr;
	myaddr = ((uint32)HL_ADDR_ENGINEFUNCS_S);
	memcpy(&sHLeiface,&myaddr,sizeof(enginefuncs_t));
	pHLeiface = (enginefuncs_t *)myaddr;
}

bool HLeiface_test(int iReceiver, int iSender, bool bListen)
{
	if (!pHLeiface) return false; // not installed yet:

	return pHLeiface->pfnVoice_SetClientListening(iReceiver,iSender, (bListen ? 1 : 0));
}

float HLeiface_gettime()
{
	if (!pHLeiface) return 0.0f; // not installed yet:

	return pHLeiface->pfnTime();
}

void HLeiface_setmodel(int i, const char *m)
{
	if (!pHLeiface) return;

	pHLeiface->pfnSetModel(pHLeiface->pfnPEntityOfEntIndex(i),m);
}

void HLeiface_test2(char *szs)
{
	if (!pHLeiface) return; // not installed yet:
	pHLeiface->pfnGetGameDir(szs);
}