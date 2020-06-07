#include <fstream>
struct Token;

class Lexer
{
public:
	Lexer() = default;
	Lexer(const char* file_path);
	Token next();

	inline
	bool is_open()
	{
		return in_stream.is_open();
	}
private:
	void step();

private:
	unsigned long p = 0;
	unsigned long line = 0, col= 0;
	char ch;

	std::ifstream in_stream;
};
