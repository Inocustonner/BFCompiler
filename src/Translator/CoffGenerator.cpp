#include "../PEFileHeader.hpp" // basically coff
#include "../SectionHeader.hpp" // section headers for coff
#include "CoffGenerator.hpp"
#include "Bucket.hpp"
#include <windows.h>

void CoffGenerate(FILE* fp, const Bucket* buck, const std::vector<unresd_name>& unresolved_v)
{
	PEFileHeader header = {};
	header.numberOfSections = 1; // only .text section
	header.characteristics = PEFileHeader::RelocsStripped
		| PEFileHeader::NumsStripped
		| PEFileHeader::SymsStripped
		| PEFileHeader::DebugStripped
		| PEFileHeader::Machine32Bit;
	SectionHeader sec = {};
	const char* text = ".text";
	std::memcpy(sec.name, text, std::strlen(text));
	sec.characteristics = SectionHeader::CanExecute
		| SectionHeader::CanRead
		| SectionHeader::ContainsCode;
	sec.pointerToRawData = static_cast<dword>(std::size(header) + std::size(sec));
	sec.sizeOfRawData = get_size();
	fwrite(&header, std::size(header), 1, fp);
	fwrite(&sec, std::size(sec), 1, fp);
	for (; buck; buck = buck->next)
		fwrite(buck->mem, buck->occupied, 1, fp);
}
