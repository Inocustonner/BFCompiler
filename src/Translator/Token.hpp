enum class Token_Type
{
	Eof = 0,

	Shl,
	Shr,
	Inc,
	Dec,
	Out,
	In,
	LoopStart,
	LoopEnd
};

struct Token
{
	Token_Type toktype;
};
