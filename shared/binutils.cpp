#include "stdafx.h"

#include "binutils.h"

#include <algorithm>

#define PtrFromRva( base, rva ) ( ( ( PBYTE ) base ) + rva )

namespace Afx {
namespace BinUtils {

////////////////////////////////////////////////////////////////////////////////

// TODO: optimize this.
MemRange FindBytes(MemRange memRange, char const * pattern, size_t patternSize)
{
	size_t matchDepth = 0;
	size_t oldMemRangeStart = memRange.Start;

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

MemRange FindBytesReverse(MemRange memRange, char const * pattern, size_t patternSize)
{
	size_t matchDepth = 0;
	size_t oldMemRangeEnd = memRange.End;

	if(!pattern)
		return MemRange(max(oldMemRangeEnd, memRange.Start), oldMemRangeEnd);

	if(1 > patternSize)
		return MemRange(max(oldMemRangeEnd -1, memRange.Start), oldMemRangeEnd);

	for(;memRange.Start < memRange.End;)
	{
		memRange.End--;
		char cur = *(char const *)memRange.End;

		if(cur == pattern[patternSize - matchDepth - 1])
			matchDepth++;
		else
		{
			memRange.End += matchDepth;
			matchDepth = 0;
			continue;
		}

		if(matchDepth == patternSize)
			return MemRange(memRange.End, memRange.End+matchDepth);
	}

	return MemRange(max(oldMemRangeEnd, memRange.Start), oldMemRangeEnd);
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
	size_t matchDepth = 0;
	size_t oldMemRangeStart = memRange.Start;
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

size_t FindClassVtable(HMODULE hModule, const char * name, DWORD rttiBaseClassArrayOffset, DWORD completeObjectLocatorOffset)
{
	ImageSectionsReader imageSectionReader(hModule);

	if (imageSectionReader.Eof())
		return 0;

	imageSectionReader.Next();

	MemRange data2Range = imageSectionReader.GetMemRange();

	if (imageSectionReader.Eof())
		return 0;

	imageSectionReader.Next();

	MemRange data3Range = imageSectionReader.GetMemRange();

	MemRange rangeName = FindCString(data3Range, name);

	if (rangeName.IsEmpty())
		return 0;

	MemRange rangeRttiTypeDescriptor = data3Range.And(MemRange::FromSize((size_t)(rangeName.Start - 2*sizeof(void*)), (size_t)sizeof(DWORD)));

	if (rangeRttiTypeDescriptor.IsEmpty())
		return 0;

#ifndef _WIN64
	DWORD dwOffsetRttiTypeDescriptor = rangeRttiTypeDescriptor.Start;
#else
	DWORD dwOffsetRttiTypeDescriptor = (DWORD)(rangeRttiTypeDescriptor.Start - (size_t)hModule);
#endif

	MemRange rangeMaybeRttiBaseClassDescriptor = MemRange(data2Range.Start,data2Range.Start);
	while(true)
	{
		rangeMaybeRttiBaseClassDescriptor = FindBytes(MemRange(rangeMaybeRttiBaseClassDescriptor.End, data2Range.End), (const char *)&dwOffsetRttiTypeDescriptor, sizeof(DWORD));

		if (rangeMaybeRttiBaseClassDescriptor.IsEmpty())
			break;

#ifndef _WIN64
		DWORD dwOffsetMaybeRttiBaseClassDescriptor = rangeMaybeRttiBaseClassDescriptor.Start;
#else
		DWORD dwOffsetMaybeRttiBaseClassDescriptor = (DWORD)(rangeMaybeRttiBaseClassDescriptor.Start - (size_t)hModule);
#endif		

		MemRange rangeMaybeRttiBaseClassArrayRef = MemRange(data2Range.Start, data2Range.Start);
		while (true)
		{
			rangeMaybeRttiBaseClassArrayRef = FindBytes(MemRange(rangeMaybeRttiBaseClassArrayRef.End, data2Range.End), (char const *)&dwOffsetMaybeRttiBaseClassDescriptor, sizeof(DWORD));

			if (rangeMaybeRttiBaseClassArrayRef.IsEmpty())
				break;

			MemRange rangeMaybeRttiBaseClassArray = data2Range.And(MemRange::FromSize((size_t)(rangeMaybeRttiBaseClassArrayRef.Start - sizeof(DWORD) * rttiBaseClassArrayOffset), (size_t)sizeof(DWORD)));

			if (rangeMaybeRttiBaseClassArray.IsEmpty())
				continue;

#ifndef _WIN64
			DWORD dwOffsetMaybeRttiBaseClassArray = rangeMaybeRttiBaseClassArray.Start;
#else
			DWORD dwOffsetMaybeRttiBaseClassArray = (DWORD)(rangeMaybeRttiBaseClassArray.Start - (size_t)hModule);
#endif		


			MemRange rangeMaybeRttiClassHirachyDescriptorRef = MemRange(data2Range.Start, data2Range.Start);
			while (true)
			{
				rangeMaybeRttiClassHirachyDescriptorRef = FindBytes(MemRange(rangeMaybeRttiClassHirachyDescriptorRef.End, data2Range.End), (const char *)&dwOffsetMaybeRttiBaseClassArray, sizeof(DWORD));

				if (rangeMaybeRttiClassHirachyDescriptorRef.IsEmpty())
					break;

				MemRange rangeMaybeRttiClassHirachyDescriptor = data2Range.And(MemRange::FromSize((size_t)(rangeMaybeRttiClassHirachyDescriptorRef.Start - 0x0c), (size_t)sizeof(DWORD)));

				if (rangeMaybeRttiClassHirachyDescriptor.IsEmpty())
					continue;

#ifndef _WIN64
				DWORD dwOffsetMaybeRttiClassHirachyDescriptor = rangeMaybeRttiClassHirachyDescriptor.Start;
#else
				DWORD dwOffsetMaybeRttiClassHirachyDescriptor = (DWORD)(rangeMaybeRttiClassHirachyDescriptor.Start - (size_t)hModule);
#endif	

				MemRange rangeMaybeCompleteObjectAllocatorRef = MemRange(data2Range.Start, data2Range.Start);
				while (true)
				{
					rangeMaybeCompleteObjectAllocatorRef = FindBytes(MemRange(rangeMaybeCompleteObjectAllocatorRef.End, data2Range.End), (const char *)&dwOffsetMaybeRttiClassHirachyDescriptor, sizeof(DWORD));

					if (rangeMaybeCompleteObjectAllocatorRef.IsEmpty())
						break;

					MemRange rangeMaybeCompleteObjectAllocator = data2Range.And(MemRange::FromSize((size_t)(rangeMaybeCompleteObjectAllocatorRef.Start - 0x10), (size_t)sizeof(DWORD)));

					if (rangeMaybeCompleteObjectAllocator.IsEmpty())
						continue;

					if (completeObjectLocatorOffset != *(DWORD *)(rangeMaybeCompleteObjectAllocator.Start + 0x4))
						continue;

					MemRange rangeMaybeVTableRef = MemRange(data2Range.Start, data2Range.Start);
					while (true)
					{
						rangeMaybeVTableRef = FindBytes(MemRange(rangeMaybeVTableRef.End, data2Range.End), (const char *)&rangeMaybeCompleteObjectAllocator.Start, sizeof(size_t));

						if (rangeMaybeVTableRef.IsEmpty())
							break;

						MemRange rangeMaybeVTable = data2Range.And(MemRange::FromSize((size_t)(rangeMaybeVTableRef.Start + sizeof(void*)), (size_t)sizeof(DWORD)));

						if (rangeMaybeVTable.IsEmpty())
							continue;

						return rangeMaybeVTable.Start;
					}
				}
			}
		}
	}

	return 0;
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
	size_t startAddress = GetStartAddress();
	return MemRange(startAddress, startAddress+GetSize());
}

size_t ImageSectionsReader::GetStartAddress(void)
{
	return (size_t)PtrFromRva(m_hModule, m_Section->VirtualAddress);
}

size_t ImageSectionsReader::GetSize(void)
{
	return m_Section->Misc.VirtualSize;
}

// MemRange ////////////////////////////////////////////////////////////////////

MemRange MemRange::FromSize(size_t address, size_t size)
{
	return MemRange(address, address + size);
}

MemRange::MemRange()
{
	Start = End = 0;
}

MemRange::MemRange(size_t start, size_t end)
{
	Start = start;
	End = end;
}

bool MemRange::IsEmpty(void) const
{
	return End <= Start;
}

MemRange MemRange::And(const MemRange & range) const
{
	if (this->IsEmpty())
		return MemRange(*this);

	if (range.IsEmpty())
		return MemRange(range);

	return MemRange(max(range.Start, this->Start), min(range.End, this->End));
}


} // namespace Afx {
} // namespace BinUtils {
