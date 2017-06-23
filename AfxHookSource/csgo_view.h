#pragma once

#include "SourceInterfaces.h"

SOURCESDK::IViewRender_csgo * GetView_csgo(void);

bool Hook_csgo_CViewRender_ShouldForceNoVis(void);