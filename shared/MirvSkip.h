#pragma once

#include "MirvCamPath.h"

class IMirvSkip_GotoDemoTick abstract {
public:
    virtual void GotoDemoTick(int tick) = 0;
};

void MirvSkip_ConsoleCommand(advancedfx::ICommandArgs * args, IMirvCampath_Time * mirvTime, IMirvSkip_GotoDemoTick * pGotoDemoTick);
