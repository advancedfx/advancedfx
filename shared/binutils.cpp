#include "stdafx.h"

#include "binutils.h"

#include <algorithm>

#define PtrFromRva( base, rva ) ( ( ( PBYTE ) base ) + rva )

namespace Afx {
namespace BinUtils {

////////////////////////////////////////////////////////////////////////////////

// TODO: optimize this.
MemRange FindBytes(MemRange memRange, char const * pattern, DWORD patternSize)
{
	DWORD matchDepth = 0;
	DWORD oldMemRangeStart = memRange.Start;

	if(!pattern)
		return MemRange(oldMemRangeStart, min(oldMemRangeStart, memRange.End));

	if(1 > patternSize)
		return MemRange(oldMemRangeStart, min(oldMemRangeStart +1, memRange.End));

	for(;memRange.Start < memRange.End;memRange.Start++)
	{
		char cur = *(char const *)memRange.Start;

		if(cur == pattern[matchDepth])
			matchDepth++;
		else
		{
			memRange.Start -= matchDepth;
			matchDepth = 0;
			continue;
		}

		if(matchDepth == patternSize)
			return MemRange(memRange.Start-matchDepth+1, memRange.Start+1);
	}

	return MemRange(oldMemRangeStart, min(oldMemRangeStart, memRange.End));
}

MemRange FindCString(MemRange memRange, char const * pattern)
{
	return FindBytes(memRange, pattern, strlen(pattern)+1);
}

MemRange FindWCString(MemRange memRange, wchar_t const * pattern)
{
	return FindBytes(memRange, (char const *)pattern, sizeof(wchar_t)*(wcslen(pattern)+1));
}

MemRange FindPatternString(MemRange memRange, char const * hexBytePattern)
{
	DWORD matchDepth = 0;
	DWORD oldMemRangeStart = memRange.Start;
	size_t patternPos = 0;

	if (!hexBytePattern)
		return MemRange(oldMemRangeStart, min(oldMemRangeStart, memRange.End));

	for (; memRange.Start < memRange.End; ++memRange.Start)
	{
		char cur = *(char const *)memRange.Start;

		char pat0;
		do
		{
			pat0 = hexBytePattern[patternPos];
			++patternPos;
		}
		while (' ' == pat0);

		char pat1 = 0;
		if (pat0)
		{
			do
			{
				pat1 = hexBytePattern[patternPos];
				++patternPos;
			} while (' ' == pat1);
		}

		bool endHi = !pat0;
		bool endLo = !pat1;

		if (endHi)
			return MemRange(memRange.Start - matchDepth + 1, memRange.Start + 1);

		bool wildHi = '?' == pat0 || endHi;
		bool wildLo = '?' == pat1 || endLo;

		pat0 = ('0' <= pat0 && pat0 <= '9') ? pat0 - '0' : ('A' <= pat0 && pat0 <= 'Z' ? pat0 - 'A' + '\xa' : pat0 - 'a' + '\xa');
		pat1 = ('0' <= pat1 && pat1 <= '9') ? pat1 - '0' : ('A' <= pat1 && pat1 <= 'Z' ? pat1 - 'A' + '\xa' : pat1 - 'a' + '\xa');

		char matchval = ((pat0 & 0xf) << 4) | (pat1 & 0xF);

		bool match = (wildHi || ((matchval & 0xf0) == (cur & 0xf0))) && (wildLo || ((matchval & 0xf) == (cur & 0xf)));

		if (match)
			++matchDepth;
		else
		{
			memRange.Start -= matchDepth;
			matchDepth = 0;
			patternPos = 0;
			continue;
		}

		if (endHi || endLo || !hexBytePattern[patternPos])
			return MemRange(memRange.Start - matchDepth + 1, memRange.Start + 1);
	}

	return MemRange(oldMemRangeStart, min(oldMemRangeStart, memRange.End));
}

// ImageSectionsReader /////////////////////////////////////////////////////////

ImageSectionsReader::ImageSectionsReader(HMODULE hModule)
{
	PIMAGE_DOS_HEADER DosHeader = ( PIMAGE_DOS_HEADER ) hModule;
	PIMAGE_NT_HEADERS NtHeader; 

	m_hModule = hModule;
	m_SectionsLeft = 0;

	NtHeader = (PIMAGE_NT_HEADERS) PtrFromRva( DosHeader, DosHeader->e_lfanew );
	if( IMAGE_NT_SIGNATURE != NtHeader->Signature )
	{
		return;		
	}

	m_SectionsLeft = NtHeader->FileHeader.NumberOfSections;
	m_Section = (PIMAGE_SECTION_HEADER) PtrFromRva( NtHeader, sizeof(IMAGE_NT_HEADERS) );
}

bool ImageSectionsReader::Eof(void)
{
	return !(0 < m_SectionsLeft);
}

void ImageSectionsReader::Next(void)
{
	if(Eof()) return;

	m_SectionsLeft--;
	m_Section = (PIMAGE_SECTION_HEADER) PtrFromRva( m_Section, sizeof(IMAGE_SECTION_HEADER) );
}

void ImageSectionsReader::Next(DWORD characteristicsFilter)
{
	for(;!Eof();Next())
	{
		if(characteristicsFilter == (m_Section->Characteristics & characteristicsFilter))
			break;
	}
}

PIMAGE_SECTION_HEADER ImageSectionsReader::Get(void)
{
	return m_Section;
}

MemRange ImageSectionsReader::GetMemRange(void)
{
	DWORD startAddress = GetStartAddress();
	return MemRange(startAddress, startAddress+GetSize());
}

DWORD ImageSectionsReader::GetStartAddress(void)
{
	return (DWORD)PtrFromRva(m_hModule, m_Section->VirtualAddress);
}

DWORD ImageSectionsReader::GetSize(void)
{
	return m_Section->Misc.VirtualSize;
}

// MemRange ////////////////////////////////////////////////////////////////////

MemRange::MemRange()
{
	Start = End = 0;
}

MemRange::MemRange(DWORD start, DWORD end)
{
	Start = start;
	End = end;
}

bool MemRange::IsEmpty(void)
{
	return End <= Start;
}


} // namespace Afx {
} // namespace BinUtils {
