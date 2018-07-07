#pragma once

bool csgo_Audio_Install(void);

bool csgo_Audio_StartRecording(const wchar_t * ansiTakeDir);
void csgo_Audio_EndRecording(void);

void csgo_Audio_FRAME_RENDEREND(void);
