#pragma once

#include <string>

bool SuggestTakePath(wchar_t const * takePath, int takeDigits, std::wstring & outPath);

bool CreatePath(wchar_t const * path, std::wstring & outPath, bool noErrorIfExits = false);
