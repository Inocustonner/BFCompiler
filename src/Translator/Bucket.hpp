#include <type_traits>
#include <string>

struct Bucket
{
	void* mem = nullptr;
	size_t occupied = 0;
	constexpr static int bucket_size = 65536;
	Bucket* next = nullptr;
};

class BuckWalker
{
public:
	Bucket* buck;
	char *fptr = nullptr, *endptr = nullptr;
	int addr;

	BuckWalker() = default;
	BuckWalker(Bucket* bucket, int curr_addr = 0);
	BuckWalker(Bucket* bucket, char* start_ptr, int curr_addr = 0);

	void advance(int n);

	std::string read_n(int n);
	std::string read_cstring();
	int read_int();

	void write_n(const void* src, int n);
	void write_int(const int i);


	char& operator*();
	BuckWalker& operator+=(int n);
};

void initialize();
// a list of integers which values are byte bounded
Bucket* get_bucket_head();

size_t get_size();
void set_size(size_t new_size);
void write_bytes(size_t n, const void* src);

// read n consistent from bucket
