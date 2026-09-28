#pragma once

void Mirv_Voice_OnAfterFrameRenderEnd(void);

/// <param name="pathTemplate">Output path template where only {ENTITY_INDEX} is left to expand.</param>
bool Mirv_Voice_StartRecording(const wchar_t * pathTemplate);
void Mirv_Voice_EndRecording();
