#include "Parser.hpp"
#include <iostream>

using Ast_Ptr = Parser::Ast_Ptr;

Parser::Parser(Lexer& lex)
	: lex{ lex }
{
	consume();
}


void Parser::consume()
{
	curr_token = lex.next();
}


Ast_Ptr Parser::parseInstr() noexcept
{
	std::unique_ptr<Ast_Instr> ast_p = std::make_unique<Ast_Instr>();
	ast_p->repeated_n = 0;
	ast_p->tok = curr_token;
	do
	{
		consume();
		ast_p->repeated_n++;
	} while (curr_token.toktype == ast_p->tok.toktype);
	return ast_p;
}


Ast_Ptr Parser::parseLoop() noexcept
{
	std::unique_ptr<Ast_Loop> ast_p = std::make_unique<Ast_Loop>();
	consume();

	while (curr_token.toktype != Token_Type::LoopEnd)
	{
		ast_p->body.push_back(parseSingle());
	}
	consume(); // consume LoopEnd
	return ast_p;
}


Ast_Ptr Parser::parseSingle() noexcept
{
	if (curr_token.toktype == Token_Type::LoopStart)
		return parseLoop();
	else if (curr_token.toktype == Token_Type::Eof)
	{
		std::cerr << "Unexcpected EOF\n";
		exit(1);
	}
	else
		return parseInstr();
}


std::vector<Ast_Ptr> Parser::parse() noexcept
{
	std::vector<Ast_Ptr> vec;

	while (curr_token.toktype != Token_Type::Eof)
	{
		vec.push_back(parseSingle());
	}
	return vec;
}