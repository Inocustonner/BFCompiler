#include "../DosHeader.hpp"
#include "../PEFileHeader.hpp"
#include "../PEOptionalHeader32.hpp"

struct PEHeader
{
	dword signature = 0x00004550;
	PEFileHeader file = {};
	PEOptionalHeader32 opt = {};
	constexpr static size_t size()
	{
		return sizeof(PEHeader);
	}
};
