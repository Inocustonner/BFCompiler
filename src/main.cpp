#include "Parser.hpp"
#include "Intermediate.hpp"
#include "Addressing.hpp"
//#include "Intelx86Gen.hpp"
int main(int argc, char *argv[])
{
	// Lexer lex(R"(D:\Projects\C++\Brainfuck\Compiler\test.bf)");
	// Parser parser(lex);
	// INode* inode = make_intermediate(parser.parse());
	addrn86_t addrn = { .index = x86Reg::EAX, 
		.base = x86Reg::EBP,
		.src = x86Reg::EDX, 
		.scale = 2, 
		.disp = 127, 
		.ref = true, 
		.dp = true
	};
	auto x = make_operands(addrn);
	return 0;
}
