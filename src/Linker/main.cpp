#include "../PEHeader.hpp"
#include "../SectionHeader.hpp"

#include <iostream>
#include <windows.h>
#include <cassert>
#define ALIGN_TO(x, r) (r * (x / r + 1))

int getPageSize()
{
	SYSTEM_INFO si = {};
	GetSystemInfo(&si);
	return si.dwPageSize;
}

static const int page_size = getPageSize();

void write_dos_header(FILE* fp)
{
	DosHeader dos = {};
	fwrite(&dos, std::size(dos), 1, fp);
}

int main(int argc, char *argv[])
{
	const char* obj_path = R"(D:\Projects\C++\Brainfuck\Compiler\test.obj)";
	const char* exe_path = R"(D:\Projects\C++\Brainfuck\Compiler\test.exe)";
	FILE *fp, *fp_src;
	fopen_s(&fp, exe_path, "wb");
	fopen_s(&fp_src, obj_path, "rb");

	if (!fp)
	{
		fprintf(stderr, "ERROR openning exe file\n");
		std::exit(1);
	}

	if (!fp_src)
	{
		fprintf(stderr, "ERROR openning obj file\n");
		std::exit(1);
	}
	// write dos header
	write_dos_header(fp);
	PEFileHeader obj_header = {};
	SectionHeader obj_section = {};

	fread(&obj_header, PEFileHeader::size(), 1, fp_src);
	fread(&obj_section, SectionHeader::size(), 1, fp_src);

	PEHeader peh = {};
	peh.file.numberOfSections = 1;
	peh.file.sizeOfOptionalHeader = std::size(peh.opt);
	peh.file.characteristics = obj_header.characteristics | PEFileHeader::Characteristic::ExecutableImage;
	// because 1 have only 1 section
	dword size_of_headers = (DosHeader::size() + PEHeader::size() + SectionHeader::size());
	dword file_align = 512;
	dword sec_align = page_size;
	dword virtual_addr = sec_align * (size_of_headers / sec_align + 1);

	dword entryPoint = virtual_addr; // bcs i have only .text section

	dword raw_addr = file_align * (size_of_headers / file_align + 1);

	SectionHeader tsec = obj_section;
	tsec.virtualSize = tsec.sizeOfRawData;
	tsec.virtualAddress = virtual_addr;
	tsec.sizeOfRawData = ALIGN_TO(obj_section.sizeOfRawData, file_align);
	tsec.pointerToRawData = raw_addr;

	raw_addr += tsec.sizeOfRawData;
	virtual_addr += ALIGN_TO(obj_section.sizeOfRawData, sec_align);
	if (tsec.characteristics & SectionHeader::Characteristic::ContainsCode)
	{
		peh.opt.baseOfCode = virtual_addr;
		peh.opt.sizeOfCode += tsec.sizeOfRawData;
	}

	peh.opt.sectionAlignment = sec_align;
	peh.opt.fileAlignment = file_align;

	peh.opt.addressOfEntryPoint = entryPoint;
	//peh.opt.baseOfData = entryPoint;			// test

	peh.opt.sizeOfImage = virtual_addr;
	peh.opt.sizeOfHeaders = ALIGN_TO(size_of_headers, peh.opt.fileAlignment);
	peh.opt.subsystem = PEOptionalHeader32::Subsystem::WindowsCui;
	peh.opt.sizeOfStackReserve = 1024;
	peh.opt.sizeOfStackCommit = 1024;

	peh.opt.majorOperatingSystemVersion = 0x4;
	peh.opt.minorOperatingSystemVersion = 0x0;
	peh.opt.majorImageVersion = 0x1;
	peh.opt.minorImageVersion = 0x0;
	peh.opt.majorSubsystemVersion = 0x4;
	peh.opt.minorSubsystemVersion = 0x0;

	peh.opt.numberOfRvaAndSizes = 0;

	fwrite(&peh, std::size(peh), 1, fp);
	fwrite(&tsec, std::size(tsec), 1, fp);

	// fill spaces
	for (dword i = size_of_headers; i < tsec.pointerToRawData; i++)
	{
		fputc(0xCC, fp);
	}

	byte* code = new byte[obj_section.sizeOfRawData];
	int rc = fread(code, 1, obj_section.sizeOfRawData, fp_src);
	assert(rc == obj_section.sizeOfRawData);
	fwrite(code, obj_section.sizeOfRawData, 1, fp);

	for (dword i = obj_section.sizeOfRawData; i < peh.opt.sizeOfCode; i++)
	{
		fputc(0xCC, fp);
	}

	fclose(fp);
	fclose(fp_src);
	return 0;
}
