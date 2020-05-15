#pragma once
#include "Registers.hpp"
#include <vector>
#include <memory>
#include <string>

class Ast;

enum class Instruction
{
	Add,
	Sub,
	Ld,
	Test,
	// Jmp,
	Call,						// first operand is symbol name the rest is arguments
	JmpIf,
};


enum class Jmp_Cond
{
	Eq
};


enum class Operand_Type
{
	Register,
	Immediate,
	Symbol,
};

struct INode;

// addressing: represent addressing in asm fe [eax + ebx * 2 + 3] and etc.
// if disp == -1 it is not specified
// if scale == 0 index is not specified


struct Operand
{
	Operand_Type type;
	union
	{
		addrn_t addrn;
		int immediate;
	    const char* symbol;
	};
};


struct INode
{
	Instruction instr;
	Jmp_Cond jmp_cond;			// if instr == jmpIf
	std::vector<Operand> operands;
	std::string label;          // may has or not value, if it does then is valid label

	INode* next = nullptr;
};


INode* make_intermediate(std::vector<std::unique_ptr<Ast>>& ast_v);
