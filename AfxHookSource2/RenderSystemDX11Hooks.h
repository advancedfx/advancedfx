#pragma once

#define ADVNACEDFX_STARTMOIVE_WAV_KEY "advancedfx-802bb089-972b-4841-bdf3-5108175ab59d"

bool AfxStreams_IsRcording();
const wchar_t * AfxStreams_GetTakeDir();

void Hook_RenderSystemDX11(void * hModule);
