#include "Ast.hpp"
#include "Intermediate.hpp"

#include <iostream>
#include <cassert>

static const char* print_symbol = "print";
static const char* input_sybmol = "input";

using Ast_Ptr = std::unique_ptr<Ast>;

static INode* proceed_body(std::vector<std::unique_ptr<Ast>>& ast_v);

static INode* to_intermediate(Ast_Ptr& ast_p) noexcept
{
	INode* inode = nullptr;
	switch (ast_p->type)
	{
		case Ast_Type::Instr:
		{
			inode = new INode{};
			Ast_Instr* ast_i = reinterpret_cast<Ast_Instr*>(ast_p.get());
			inode->operands.reserve(2); // the most common instruction have 2 operands
			switch (ast_i->tok.toktype)
			{
				case Token_Type::Shl:
				{
					inode->instr = Instruction::Sub;

					Operand operd{ .type = Operand_Type::Register };
					operd.addrn = addrn_t{ .index = Register::R1 };
					inode->operands.push_back(operd);

					operd = Operand{ .type = Operand_Type::Immediate, .immediate = ast_i->repeated_n };
					inode->operands.push_back(operd);
				}break;

				case Token_Type::Shr:
				{
					inode->instr = Instruction::Add;

					Operand operd{ .type = Operand_Type::Register };
					operd.addrn = addrn_t{ .index = Register::R1 };

					inode->operands.push_back(operd);
					operd = Operand{ .type = Operand_Type::Immediate, .immediate = ast_i->repeated_n };
					inode->operands.push_back(operd);
				}break;

				case Token_Type::Inc:
				{
					inode->instr = Instruction::Add;

					Operand operd{ .type = Operand_Type::Register };
					operd.addrn = addrn_t{ .index = Register::R1, .ref = true, .ref_size = RefSize::Byte };

					inode->operands.push_back(operd);
					operd = Operand{ .type = Operand_Type::Immediate, .immediate = ast_i->repeated_n };

					inode->operands.push_back(operd);
				}break;

				case Token_Type::Dec:
				{
					inode->instr = Instruction::Sub;

					Operand operd{ Operand_Type::Register };
					operd.addrn = addrn_t{ .index = Register::R1, .ref = true, .ref_size = RefSize::Byte};
					inode->operands.push_back(operd);

					operd = Operand{ .type = Operand_Type::Immediate, .immediate = ast_i->repeated_n };
					inode->operands.push_back(operd);
				}break;

				// RE THINK
				case Token_Type::Out:
				{
					inode->instr = Instruction::Call;

					Operand operd{ .type = Operand_Type::Symbol, .symbol = print_symbol };
					inode->operands.push_back(operd);

					operd = Operand{ .type = Operand_Type::Register };
					operd.addrn = addrn_t{ .index = Register::R1 };
					inode->operands.push_back(std::move(operd));

					operd = Operand{ .type = Operand_Type::Immediate, .size = RefSize::Dword, .immediate = ast_i->repeated_n };
					inode->operands.push_back(operd);
				}break;

				case Token_Type::In:
				{
					inode->instr = Instruction::Call;

					Operand operd{ .type = Operand_Type::Immediate };
					operd.immediate = ast_i->repeated_n;
					inode->operands.push_back(operd);
				}break;
			}
		}break;
		// RETHINK
		case Ast_Type::Loop:
		{
			static int labelN = 1; // temporary name label generator

			Ast_Loop* ast_l = reinterpret_cast<Ast_Loop*>(ast_p.get());
			inode = proceed_body(ast_l->body);
			inode->label = "Loop" + std::to_string(labelN++);

			INode* curr = inode, **prev = nullptr;
			for (; curr->next != nullptr; curr = curr->next);

			prev = &curr->next;
			curr = new INode{.instr = Instruction::Test };
			*prev = curr;

			// create test for counter being 0
			Operand operd{ .type = Operand_Type::Register };
			operd.addrn = addrn_t{ .index = Register::R1, .ref = true, .ref_size = RefSize::Byte };
			curr->operands.reserve(2);
			curr->operands.push_back(operd);
			
			operd = Operand{ .type = Operand_Type::Immediate, .immediate = 0xFF };
			curr->operands.push_back(operd);

			prev = &curr->next;
			curr = new INode{ .instr = Instruction::JmpIf, .jmp_cond = Jmp_Cond::Eq };
			*prev = curr;

			operd = Operand{ .type = Operand_Type::Symbol, .symbol = inode->label.c_str() };
			curr->operands.push_back(operd);
		}break;

		default:
			std::cerr << "Invalid Ast type\n";
			assert(false);
	}
	return inode;
}


static INode* proceed_body(std::vector<std::unique_ptr<Ast>>& ast_v)
{
	INode* head = nullptr;
	INode* curr = head, **prev = &head;
	// ADD start node with loading stack addr to R1
	for (Ast_Ptr& ast_p : ast_v)
	{
		curr = to_intermediate(ast_p);
		*prev = curr;

		prev = &curr->next;
	}
	return head;
}


INode* make_intermediate(std::vector<std::unique_ptr<Ast>>& ast_v)
{
	//INode* start = new INode{ .instr = Instruction::Ld };
	INode* begin = new INode{ .instr = Instruction::Sub };
	begin->operands.reserve(2);
	begin->operands.push_back(Operand{ .type = Operand_Type::Register, .addrn = addrn_t{.index = Register::RStack } });
	begin->operands.push_back(Operand{ .type = Operand_Type::Immediate, .immediate = 512 });

	begin->next = new INode{ .instr = Instruction::Ld };
	INode* start = begin->next;
	start->operands.reserve(2);
	start->operands.push_back(Operand{ .type = Operand_Type::Register, .addrn = addrn_t{.index = Register::R1 } });
	start->operands.push_back(Operand{ .type = Operand_Type::Register, .addrn = addrn_t{.index = Register::RStack } });

	start->next = proceed_body(ast_v);
	return begin;
}
