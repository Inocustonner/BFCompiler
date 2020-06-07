#include "Lexer.hpp"
#include "Ast.hpp"
#include <memory>
#include <vector>

class Ast;
class Parser
{
public:
	using Ast_Ptr = std::unique_ptr<Ast>;

	Parser(Lexer& lex);
	std::vector<Ast_Ptr> parse() noexcept;

private:
	void consume();
	Ast_Ptr parseInstr() noexcept;
	Ast_Ptr parseLoop() noexcept;

	Ast_Ptr	parseSingle() noexcept;
private:
	Lexer& lex;
	Token curr_token;
};
