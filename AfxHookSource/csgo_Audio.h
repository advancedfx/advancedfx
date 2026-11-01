#pragma once

bool csgo_Audio_Install(void);

/// <param name="filePath">WAV output file, parent folders are created on demand.</param>
bool csgo_Audio_StartRecording(const wchar_t * filePath);
void csgo_Audio_EndRecording(void);
