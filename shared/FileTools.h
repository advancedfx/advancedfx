#pragma once

#include <string>

bool SuggestTakePath(wchar_t const * takePath, int takeDigits, std::wstring & outPath);

bool CreatePath(wchar_t const * path, std::wstring & outPath, bool noErrorIfExits = false);

bool GetFullPath(wchar_t const * path, std::wstring & outPath);

/// Creates the parent directories of filePath if they don't exist yet (thread-safe).
bool CreateParentPath(wchar_t const * filePath);
