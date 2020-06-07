#include "Intelx86Gen.hpp"
#include "Addressing.hpp"
#include <cassert>
#include <iostream>
#include <map>

static std::map<std::string, size_t> lo_map; // label->byte_offset
static std::vector<std::pair<size_t, Instruction>> to_res; // jmps and calls to resolve
static std::vector<std::pair<std::string, size_t>> external_symbols = {};

static x86Reg map_reg(Register r)
{
	switch (r)
	{
		case Register::R1:
			return x86Reg::EAX;

		case Register::RStack:
			return x86Reg::ESP;

		case Register::None:
			return x86Reg::None;

		default:
			assert(false);
	}
};


inline bool is_byte_bounded(int n)
{
	return n >= -128 && n <= 127;
}


inline
addrn86_t make_addrn86(const addrn_t& op1, const addrn_t& op2)
{
	addrn86_t a86;
	assert(!op2.ref);
	a86.ref = op1.ref;
	a86.dp = op1.dp;
	a86.base = map_reg(op1.base);
	a86.index = map_reg(op1.index);
	a86.scale = op1.scale;
	a86.disp = op1.disp;

	a86.src = map_reg(op2.index);
	return a86;
}

inline
addrn86_t make_addrn86(const addrn_t& op)
{
	addrn_t op2 = op;
	op2.ref = false;
	return make_addrn86(op, op2);
}


static eff_addr_t make_addr(const Operand& op1, const Operand& op2)
{
	if (op1.type == op2.type && op2.type == Operand_Type::Register)
		return make_operands(make_addrn86(op1.addrn, op2.addrn));
	else if (op1.type == Operand_Type::Register && op2.type == Operand_Type::Immediate)
	{
		addrn86_t a86 = make_addrn86(op1.addrn);
		eff_addr_t addr = make_operands(a86);
		addr.raw[addr.len] = op2.immediate;
		if (op1.addrn.ref)
			switch (op1.addrn.ref_size)
			{
				case RefSize::Byte:
					addr.len += sizeof(byte_t);
					break;
				case RefSize::Dword:
					addr.len += sizeof(dword_t);
					break;
				default:
					assert(false);
			}
		else
			addr.len += sizeof(dword_t); // because i still use only 32bit register 
		return addr;
	}
	else
		assert(false);
}


static void generate(const INode* inode)
{
	if (std::size(inode->label))
	{
		lo_map.insert(std::make_pair(inode->label, get_size()));
	}

	Operand op1 = {}, op2 = {};
	if (std::size(inode->operands) == 2)
	{
		op1 = inode->operands[0];
		op2 = inode->operands[1];
	}
	else if (std::size(inode->operands) == 1)
		op1 = inode->operands[0];

	switch (inode->instr)
	{
		case Instruction::Add:
		{
			byte_t instr[1];
			if (op1.addrn.ref && op1.addrn.ref_size == RefSize::Byte)
			{
				instr[0] = 0x80;
			}
			else
			{
				instr[0] = 0x81;
			}
			eff_addr_t ea = make_addr(op1, op2);
			write_bytes(std::size(instr), instr);
			write_bytes(ea.len, ea.raw);
		}break;

		case Instruction::Sub:
		{
			byte_t instr[1];
			if (op1.addrn.ref && op1.addrn.ref_size == RefSize::Byte)
			{
				instr[0] = 0x80;
			}
			else
			{
				instr[0] = 0x81;
			}

			assert(op2.type == Operand_Type::Immediate);
			addrn86_t a86 = { .index = map_reg(op1.addrn.index),
							  .base = map_reg(op1.addrn.base), .src = x86Reg::EBP,
							  .scale = op1.addrn.scale, .disp = op1.addrn.disp,
							  .ref = op1.addrn.ref, .dp = op1.addrn.dp };
			eff_addr_t ea = make_operands(a86);

			if (instr[0] == 0x81)
				*reinterpret_cast<dword_t*>(ea.raw + ea.len) = op2.immediate;
			else
				ea.raw[ea.len] = op2.immediate;

			// calc operand size
			if (op1.addrn.ref)
				switch (op1.addrn.ref_size)
				{
				case RefSize::Byte:
					ea.len += sizeof(byte_t);
					break;
				case RefSize::Dword:
					ea.len += sizeof(dword_t);
					break;
				default:
					assert(false);
				}
			else
				ea.len += sizeof(dword_t); // because i still use only 32bit register 

			write_bytes(std::size(instr), instr);
			write_bytes(ea.len, ea.raw);
		}break;

		case Instruction::Ld:
		{
			byte_t instr[1];
			// made for only mov r32, r32
			instr[0] = 0x89;
			eff_addr_t ea = make_addr(op1, op2);
			write_bytes(std::size(instr), instr);
			write_bytes(ea.len, ea.raw);
		}break;

		case Instruction::Test:
		{
			addrn86_t a86 = make_addrn86(op1.addrn);
			assert(op2.type == Operand_Type::Immediate);
			if (!a86.ref && a86.index == x86Reg::EAX)
			{
				byte_t instr[5] = { 0x9A };
				*reinterpret_cast<int*>(&instr[1]) = op2.immediate;
				write_bytes(std::size(instr), instr);
			}
			else
			{
				byte_t instr[] = { 0xF6 };
				eff_addr_t ea = make_addr(op1, op2);
				write_bytes(std::size(instr), instr);
				write_bytes(ea.len, ea.raw);
			}
		}break;

		case Instruction::JmpIf:
		{
			switch (inode->jmp_cond)
			{
				case Jmp_Cond::Eq:
				{
					assert(inode->operands[0].type == Operand_Type::Symbol);
					const std::string_view label(inode->operands[0].symbol);
					byte_t instr[1] = { 0x75 };

					to_res.push_back(std::make_pair(get_size(), inode->instr));

					write_bytes(std::size(instr), instr);
					write_bytes(std::size(label) + 1, label.data());
				}break;
				default:
					assert(false);
			}
		}break;

		case Instruction::Call:
		{
			assert(inode->operands[0].type == Operand_Type::Symbol);

			// push
			for (auto it = std::crbegin(inode->operands); it != std::crend(inode->operands) - 1; ++it)
			{
				switch (it->type)
				{
				case Operand_Type::Immediate:
				{
					byte_t instr[5] = { 0x68 };
					assert(it->size == RefSize::Dword);
					*reinterpret_cast<int*>(instr + 1) = it->immediate;
					write_bytes(1 + static_cast<size_t>(it->size), instr);
				}break;

				case Operand_Type::Register:
				{
					// no check for register addressing so we assume that we push gp reg
					assert(it->addrn.ref == false);
					byte_t instr[1] = { static_cast<byte_t>(0x50 + static_cast<int>(map_reg(it->addrn.index))) };
					write_bytes(std::size(instr), instr);
				}break;

				default:
					assert(false);
				}
			}

			const std::string label(inode->operands[0].symbol);
			byte_t instr[1] = { 0xE8 };

			to_res.push_back(std::make_pair(get_size(), inode->instr));

			write_bytes(std::size(instr), instr);
			write_bytes(std::size(label) + 1, label.c_str());

			for (auto it = std::cbegin(inode->operands) + 1; it != std::cend(inode->operands); ++it)
			{
				// bcs we assumed that we pushed only dword values, just flush them to trash registers
				byte_t instrs[1] = { 0x58 + static_cast<byte_t>(x86Reg::EDX)};
				write_bytes(std::size(instrs), instrs);
			}

		}break;
		default:
			assert(false);
	}
}


inline
bool resolve_label(const std::string& label, BuckWalker& dst)
{
	if (auto it = lo_map.find(label); it != std::end(lo_map))
	{
		int diff = it->second - dst.addr - 2;
		dst.advance(1);			// move past instruction byte
		if (is_byte_bounded(diff))
		{
			dst.write_n(reinterpret_cast<const char*>(&diff), sizeof(byte_t));
		}
		else
		{
			dst.write_int(diff);
		}
		return true;
	}
	else
	{
		external_symbols.emplace_back(label, dst.addr);
		return false;
	}
}


void resolve_labels(Bucket* buck)
{
	BuckWalker dst, src;
	do
	{
		const size_t off_addr = to_res[0].first;
		src = dst = BuckWalker(buck, static_cast<int>(off_addr));
		src.advance(1);			// move past instruction byte
		const std::string label = src.read_cstring();
		to_res.erase(std::begin(to_res));		// pop front
		if (resolve_label(label, dst))
		{
			src.advance(std::size(label) + 1);
			break;
		}
	} while (true);

	for (auto it = std::cbegin(to_res); it != std::cend(to_res); ++it)
	{
		int distance = it->first - src.addr;
		const std::string code = src.read_n(distance);
		src.advance(distance + 1); // move past instr byte

		dst.write_n(code.c_str(), distance);
		const std::string label = src.read_cstring();
		resolve_label(label, dst);
	}

	// Copy the rest
	if (static_cast<int>(src.endptr - src.fptr) != 0)
	{
		// copy rest of memory
		char* buffer = nullptr;
		int size = 0;
		while (true)
		{
			int to_copy = static_cast<int>(src.endptr - src.fptr);
			if (to_copy == 0)
				break;

			if (size < to_copy)
			{
				char* resv = buffer;
				buffer = reinterpret_cast<char*>(realloc(buffer, size = to_copy));
				if (buffer == nullptr)
				{
					assert(false); // add an error
					buffer = resv;
					break;
				}
			}

			memcpy(buffer, src.fptr, to_copy);
			dst.write_n(buffer, to_copy);
			if (src.buck->next == nullptr)
				break;

			src.buck = buck->next;
			src.fptr = reinterpret_cast<char*>(buck->mem);
			src.endptr = src.fptr + buck->occupied;
		}
		free(buffer);
	}

	dst.buck->occupied = dst.addr % Bucket::bucket_size;
	set_size(dst.addr);
}


const Bucket* generate_code(const INode* inode)
{
	initialize();
	for (; inode != nullptr; inode = inode->next)
		generate(inode);
	resolve_labels(get_bucket_head());
	return get_bucket_head();
}


const std::vector<std::pair<std::string, size_t>>& get_external_symbols()
{
	return external_symbols;
}
