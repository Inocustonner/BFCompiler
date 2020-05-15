#pragma once
#include "Registers.hpp"

enum class x86Reg
{
	EAX = 0b000,
	ECX = 0b001,
	EDX = 0b010,
	EBX = 0b011,
	ESP = 0b100,
	EBP = 0b101,
	ESI = 0b110,
	EDI = 0b111,

	None = -1,
};

struct eff_addr_t
{			   //   Mod R/M        SIB            disp32
	byte_t raw[sizeof(byte_t) + sizeof(byte_t) + sizeof(dword_t)];
	int len;
};

// make constructor from addrn_t
struct addrn86_t
{
	union
	{
		x86Reg index;
		x86Reg dst;
	};

	x86Reg base = x86Reg::None;
	x86Reg src = x86Reg::None;

	int scale = 1;
	int disp;

	bool ref = false;
	bool dp = false;
};

// both MODR/M & SIB inlcuded
eff_addr_t make_operands(addrn86_t op1);
