#include "Token.hpp"
#include <vector>
#include <memory>

enum class Ast_Type
{
	Instr,
	Loop
};

class Ast
{
public:
	Ast_Type type;
};

class Ast_Instr : public Ast
{
public:
	Ast_Instr()
		: Ast{ Ast_Type::Instr }, repeated_n{ 0 }
	{}

	Token tok;
	int repeated_n;
};

class Ast_Loop : public Ast
{
public:
	Ast_Loop()
		: Ast{ Ast_Type::Loop }
	{}

	std::vector<std::unique_ptr<Ast>> body;
};
