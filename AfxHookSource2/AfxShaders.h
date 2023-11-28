#pragma once

void * LoadShaderFileInMemory(const wchar_t * fileName, size_t & outSize);

void * LoadFromAcsShaderFileInMemory(const wchar_t * fileName, int combo, size_t & outSize);
