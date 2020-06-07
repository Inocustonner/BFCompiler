#include "Parser.hpp"
#include "Intermediate.hpp"
#include "Addressing.hpp"
#include "Intelx86Gen.hpp"
#include "CoffGenerator.hpp"
#include <fstream>

int main(int argc, char *argv[])
{
	Lexer lex(R"(D:\Projects\C++\Brainfuck\Compiler\test.bf)");
	Parser parser(lex);
	INode* inode = make_intermediate(parser.parse());
	auto buck_head = generate_code(inode);

	const std::vector<unresd_name>& exsymbols = get_external_symbols();

	FILE* fp;
	fopen_s(&fp, R"(D:\Projects\C++\Brainfuck\Compiler\test.obj)", "wb");
	CoffGenerate(fp, buck_head, exsymbols);
	fclose(fp);
	return 0;
}
