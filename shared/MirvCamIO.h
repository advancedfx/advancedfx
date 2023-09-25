#pragma once

#include "AfxConsole.h"
#include "CamIO.h"

typedef double (*MirvCamIO_GetTimeFn_t)(void);

void MirvCamIO_ConsoleCommand(advancedfx::ICommandArgs * args, CamImport * & refPCamImport, CamExport * & refPCamExport, MirvCamIO_GetTimeFn_t fnGetTime);
