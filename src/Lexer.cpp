#include "Lexer.hpp"
#include "Token.hpp"

#include <iostream>
#define EOF_CHAR '\0'


Lexer::Lexer(const char* file_path)
	: in_stream(file_path)
{
	std::ios::sync_with_stdio(false);

	if (!in_stream)
	{
		std::cerr << "Error opening '" << file_path << "'\n";
	}
}


void Lexer::step()
{
	ch = in_stream.get();
	if (!in_stream.eof())
	{
		if (ch == '\n')
		{
			line++;
			col = 1;
		}
		else
		{
			col++;
		}
	}
	else
	{
		ch = EOF_CHAR;
	}
}


Token Lexer::next()
{
	Token tok;

	do step();
	while (isspace(ch));

	for (; ch != EOF_CHAR;)
	{
		switch (ch)
		{
			case '<':
				tok.toktype = Token_Type::Shl;
				return tok;
			case '>':
				tok.toktype = Token_Type::Shr;
				return tok;
			case '+':
				tok.toktype = Token_Type::Inc;
				return tok;
			case '-':
				tok.toktype = Token_Type::Dec;
				return tok;
			case '.':
				tok.toktype = Token_Type::Out;
				return tok;
			case ',':
				tok.toktype = Token_Type::In;
				return tok;
			case '[':
				tok.toktype = Token_Type::LoopStart;
				return tok;
			case ']':
				tok.toktype = Token_Type::LoopEnd;
				return tok;
			default:
				std::cerr << "Invalid token " << ch << '\n';
				exit(1);
		}
	}
	tok.toktype = Token_Type::Eof;
	return tok;
}
