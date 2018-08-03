#pragma once

#include <windows.h>

namespace Afx {
namespace BinUtils {

struct MemRange
{
	static MemRange FromSize(DWORD address, DWORD size);

	// inclusive
	DWORD Start;

	// exclusive
	DWORD End;

	MemRange();
	MemRange(DWORD start, DWORD end);

	bool IsEmpty(void) const;

	MemRange And(const MemRange & range) const;
};

class ImageSectionsReader
{
public:
	ImageSectionsReader(HMODULE hModule);

	bool Eof(void);

	void Next(void);

	/// <param name="characteristicsFilter">Minium bit mask that must be matched, should be a combination of IMAGE_SCN_*</param>
	void Next(DWORD characteristicsFilter);

	PIMAGE_SECTION_HEADER Get(void);
	MemRange GetMemRange(void);
	DWORD GetStartAddress(void);
	DWORD GetSize(void);

private:
	HMODULE m_hModule;
	PIMAGE_SECTION_HEADER m_Section;
	DWORD m_SectionsLeft;
};

/// <remarks>The memory specified by memRange must be readable.</remarks>
MemRange FindBytes(MemRange memRange, char const * pattern, DWORD patternSize);

/// <remarks>The memory specified by memRange must be readable.</remarks>
MemRange FindCString(MemRange memRange, char const * pattern);

/// <remarks>The memory specified by memRange must be readable.</remarks>
MemRange FindWCString(MemRange memRange, wchar_t const * pattern);

/// <remarks>The memory specified by memRange must be readable.</remarks>
/// <param name="hexBytePattern">
/// A pattern like &quot;00 de ?? be ef&quot;
/// Pattern is assumed to be valid, if it's not nothing will crash, but results
/// can be unexpected.
/// </param>
MemRange FindPatternString(MemRange memRange, char const * hexBytePattern);

/// <returns>0 if not found, otherwise address of vtable</returns>
DWORD FindClassVtable(HMODULE hModule, const char * name, DWORD rttiBaseClassArrayOffset, DWORD completeObjectLocatorOffset);

} // namespace Afx {
} // namespace BinUtils {
