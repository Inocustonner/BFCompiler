#include <type_traits>

struct Bucket
{
	void* mem = nullptr;
	size_t occupied = 0;
	constexpr static int bucket_size = 65536;
	Bucket* next = nullptr;
};

void initialize();
// a list of integers which values are byte bounded

void write_bytes_raw(size_t n, ...);

template <typename... BytesT>
void write_bytes(BytesT... bytes)
{
	write_bytes_raw(sizeof...(bytes), bytes...);
}

const Bucket* get_bucket_head();
