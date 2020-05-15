#include "Addressing.hpp"
#include <cassert>

#define MOD_BITS(n)   (((n) & 0xff) << 6)
#define SCALE_BITS(n) (((n) & 0xff) << 6)
#define REG_BITS(n)   (((n) & 0xfff) << 3)
#define INDEX_BITS(n) (((n) & 0xfff) << 3)
#define RM_BITS(n)    ((n) & 0xfff)
#define BASE_BITS(n)  ((n) & 0xfff)

#define MODRM_BYTE(mod, reg, rm) byte_t(MOD_BITS(mod) | REG_BITS(reg) | RM_BITS(rm))
#define SIB_BYTE(scale, index, base) byte_t(SCALE_BITS(scale) | INDEX_BITS(index) | BASE_BITS(base))

#define MRM_CAST(e) reinterpret_cast<eff_addrNS_t*>((e))
#define MRMS_CAST(e) reinterpret_cast<eff_addrS_t*>((e))
// SIB byte included
#pragma pack(push, 1)
struct eff_addrS_t
{
	byte_t modrm_b;
	byte_t sib_b;
	dword_t disp32;
};
#pragma pack(pop)
// No SIB byte included
#pragma pack(push, 1)
struct eff_addrNS_t
{
	byte_t modrm_b;
	dword_t disp32;
};
#pragma pack(pop)


inline
bool is_byte(int a)
{
	return a >= -128 && a <= 127;
}

#define EMPTY -1
// a.scale >> 1
const int scale_table[] = { 0b00, 0b01, 0b10, EMPTY, 0b11 };


eff_addr_t make_operands(addrn86_t a)
{
	union
	{
		eff_addr_t ea = {};
		eff_addrS_t eas;
		eff_addrNS_t eans;
	};
	constexpr auto modrm_sz = sizeof(eff_addrS_t::modrm_b);
	constexpr auto sib_sz = sizeof(eff_addrS_t::sib_b);

	if (a.ref)
	{
		bool sib = false;
		if (a.base == x86Reg::None)
		{
			ea.len = modrm_sz;
			eas.sib_b |= a.scale > 1 ? BASE_BITS(0b101) : BASE_BITS(0b100);
		}
		else
		{
			ea.len = modrm_sz + sib_sz;
			eas.sib_b |= BASE_BITS(static_cast<int>(a.base));
			sib = true;
		}

		if (a.scale > 1)
		{
			if (ea.len < modrm_sz + sib_sz)
				ea.len = modrm_sz + sib_sz + sizeof(eff_addrS_t::disp32);
			// MOVED UP
			//idk why but it is how it works
			// if (a.base == x86Reg::None)
			// 	eas.sib_b = BASE_BITS(0b101);
			sib = true;
		}
		eas.sib_b |= SCALE_BITS(scale_table[a.scale >> 1]) | INDEX_BITS(static_cast<int>(a.index));
		// MOD = 00 && EBP(move EBP to MOD 01)
		if (!a.dp
			|| a.dst == x86Reg::None)	// if no displacement
		{
			switch (a.index)
			{
				case x86Reg::ESP:
					assert(a.scale == 1);
					ea.len += sib_sz; 

				case x86Reg::EAX:
				case x86Reg::ECX:
				case x86Reg::EDX:
				case x86Reg::EBX:
				case x86Reg::ESI:
				case x86Reg::EDI:
				{
					if (sib)
						eas.modrm_b = MODRM_BYTE(0b00, static_cast<int>(a.src), 0b100);
					else
						eans.modrm_b = MODRM_BYTE(0b00, static_cast<int>(a.src), static_cast<int>(a.dst));
					return ea;
				}
				// BASICALLY IT IS: [EBP + 0b0]
				// MAYBE I SHOULD PUT IT IN A DIFFERENT PLACE
				case x86Reg::EBP:
				{
					if (sib)
					{
						eas.modrm_b = MODRM_BYTE(0b00, static_cast<int>(a.src), 0b100);
						eas.disp32 = 0;
					}
					else
					{
						eans.modrm_b = MODRM_BYTE(0b01, static_cast<int>(a.src), static_cast<int>(a.index));
						eans.disp32 = 0;
						ea.len += sizeof(byte_t);
					 }
					return ea;
				}
				case x86Reg::None: // displacement only
				{
					return ea;
				}
				default:
					assert(false);
			}
		}
		else if (a.dp)
		{
			ea.len += is_byte(a.disp) ? sizeof(byte_t) : sizeof(dword_t);
			if (sib || a.index == x86Reg::ESP)
				eas.disp32 = a.disp;
			else
				eans.disp32 = a.disp;

			const int mod_bits = is_byte(a.disp) ? 0b01 : 0b10;

			switch (a.index)
			{
				case x86Reg::ESP:
					assert(a.scale == 1);
					ea.len += sib_sz;

				case x86Reg::EAX:
				case x86Reg::ECX:
				case x86Reg::EDX:
				case x86Reg::EBX:
				case x86Reg::EBP:
				case x86Reg::ESI:
				case x86Reg::EDI:
				{
					if (sib)
						eans.modrm_b = MODRM_BYTE(mod_bits, static_cast<int>(a.src), 0b100);
					else
						eans.modrm_b = MODRM_BYTE(mod_bits, static_cast<int>(a.src), static_cast<int>(a.index));
					return ea;
				}
				default:
					assert(false);
			}
		}
	}
	else
	{
		eans.modrm_b = MODRM_BYTE(0b11, static_cast<int>(a.src), static_cast<int>(a.dst));
		return ea;
	}
}
