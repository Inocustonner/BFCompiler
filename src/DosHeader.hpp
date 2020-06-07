#pragma once
#include "types.hpp"

struct DosHeader
{
	word magic = 0x5a4d;		// 'MZ'
	word lastPageSize;
	word numberOfPages;
	word numberOfRelocationEntries;
	word headerSize = sizeof(DosHeader) / 0x10;			// size of the header in paragraphs (16 byte sized blocks)
	word minExtraMemory;
	word mxExtraMemory = 0xffff;
	word stackSegmentInitialValue;
	word SPInitialValue = 0xb8;
	word complementedCheckSum;
	word IPInitialValue;
	word codeSegmentInitialValue;
	word relocationTableOffset;
	word overlayNumber;
	byte reserved1[8] = {};
	word oemId;
	word oemInfo;
	byte reserved2[20] = {};
	dword PEHeaderOffset = sizeof(DosHeader);		// file offset to PEFileHeader. I use no DOS-stub, so PEFileHeader will follow immediatly

	constexpr static size_t size()
	{
		return sizeof(DosHeader);
	}
};
