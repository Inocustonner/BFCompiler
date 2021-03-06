#pragma once
using byte_t = unsigned char;
using word_t = unsigned short;
using dword_t = unsigned int;

using int_t = byte_t;

// kinda virtual registers
enum class Register
{
	R1,
	RStack, // register that has stack addr, ebp basically

	None = -1,
};

enum class RefSize
{
	Byte = 1,
	Word = 2,
	Dword = 4,
	Qword = 8,

	None = -1
};

struct addrn_t
{
	Register base = Register::None;
	union
	{
		Register index;
		Register dst;
	};
	int scale = 1;
	int disp;


	bool ref = false;
	bool dp = false;			// is displacement presented

	RefSize ref_size = RefSize::None;
};
