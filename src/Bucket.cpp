#include "Bucket.hpp"
#include <cstdarg>
#include <cstdlib>
#include <algorithm>
#include <iostream>

using byte = unsigned char;

static Bucket head = {};
static Bucket* curr_buck = &head;


void alloc_bucket_mem(Bucket* buck)
{
	if (!buck)
	{
		std::cerr << "Error allocating mem for a (null)Bucket\n";
		exit(1);
	}
	buck->mem = malloc(Bucket::bucket_size);
	if (!buck->mem)
	{
		std::cerr << "Error initializing bucket mem\n";
		exit(1);
	}
}


void initialize()
{
	alloc_bucket_mem(curr_buck);
}


inline
void copy_bytes(size_t n, va_list& vl)
{
	for (int i = 0; i < n; ++i)
	{
		reinterpret_cast<byte*>(curr_buck->mem)[curr_buck->occupied++]
			= static_cast<byte>(va_arg(vl, int));
	}

}

const Bucket* get_bucket_head()
{
	return &head;
}

void write_bytes_raw(size_t n, ...)
{
	va_list vl;
	va_start(vl, n);

	auto to_write = std::min(n, Bucket::bucket_size - curr_buck->occupied);
	copy_bytes(to_write, vl);

	if (to_write < n)
	{
		Bucket* buck = curr_buck;
		curr_buck = new Bucket{};
		alloc_bucket_mem(curr_buck);

		copy_bytes(n - to_write, vl);
	}

	va_end(vl);
}
