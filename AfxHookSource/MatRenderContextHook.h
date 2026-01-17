#pragma once

#include "SourceInterfaces.h"
#include "csgo/AfxInterfaces.h"


/// <summary>Hooks the given material system render context, if it's not hooked yet. Returns hooked context.</summary>
IAfxMatRenderContext * MatRenderContextHook(SOURCESDK::IMatRenderContext_csgo *  ctx);

/// <summary>Hooks the current material system render context, if it's not hooked yet. Returns hooked context.</summary>
IAfxMatRenderContext * MatRenderContextHook(SOURCESDK::IMaterialSystem_csgo * materialSystem);

void MatRenderContextHook_Shutdown(void);
