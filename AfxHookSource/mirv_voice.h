#pragma once

void Mirv_Voice_OnAfterFrameRenderEnd(void);

bool Mirv_Voice_StartRecording(const wchar_t * directoryPath);
void Mirv_Voice_EndRecording();
