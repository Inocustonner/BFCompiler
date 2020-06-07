#include "Bucket.hpp"
#include <cstdarg>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <iostream>
using byte = unsigned char;

static Bucket head = {};
static Bucket* curr_buck = &head;
size_t bytes_written = 0;

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


Bucket* get_bucket_head()
{
	return &head;
}

size_t get_size()
{
	return bytes_written;
}


void set_size(size_t new_size)
{
	bytes_written = new_size;
}


void write_bytes(size_t n, const void* src)
{
	auto to_write = std::min(n, Bucket::bucket_size - curr_buck->occupied);
	memcpy(reinterpret_cast<char*>(curr_buck->mem) + curr_buck->occupied, src, to_write);
	curr_buck->occupied += to_write;
	bytes_written += to_write;

	if (to_write < n)
	{
		Bucket* buck = curr_buck;
		curr_buck = new Bucket{};
		alloc_bucket_mem(curr_buck);

		memcpy(reinterpret_cast<char*>(curr_buck->mem) + curr_buck->occupied, src, n - to_write);
		curr_buck->occupied += to_write;
		bytes_written += to_write;
	}
}


BuckWalker::BuckWalker(Bucket* bucket, int curr_addr):
	buck{ bucket }
{
	fptr = reinterpret_cast<char*>(buck->mem) + curr_addr;
	endptr = reinterpret_cast<char*>(buck->mem) + bucket->occupied;
	addr = curr_addr;
}


BuckWalker::BuckWalker(Bucket* bucket, char* start_ptr, int curr_addr):
	buck{ bucket }, fptr{ start_ptr }, addr{ curr_addr }
{
	if (fptr == nullptr) fptr = reinterpret_cast<char*>(buck->mem);
	endptr = reinterpret_cast<char*>(buck->mem) + bucket->occupied;
	assert(buck->mem <= fptr && fptr < endptr);
}


void BuckWalker::advance(int n)
{
	addr += n;
	if (fptr + n > endptr)
	{
		n = n - n % static_cast<int>(endptr - fptr);

		int nm = n / Bucket::bucket_size;
		for (int nm = n / Bucket::bucket_size; nm > 0; --nm)
			buck = buck->next;

		n = n % Bucket::bucket_size;
		fptr = static_cast<char*>(buck->mem) + n;
		endptr = static_cast<char*>(buck->mem) + Bucket::bucket_size;
	}
	else
	{
		fptr += n;
	}
}


std::string BuckWalker::read_n(int n)
{
	char* buffer = reinterpret_cast<char*>(calloc(sizeof(char), n));

	int written = 0;
	char *sptr = fptr, *eptr = endptr;

	int remdr = n % (static_cast<int>(eptr - sptr) + 1);
	if (remdr)
	{
		memcpy(buffer, sptr, remdr);
		written += remdr;
	}
	n -= remdr;

	if (n == 0)
		goto ret;

	Bucket* tbucket = buck;
	tbucket = tbucket->next;
	sptr = static_cast<char*>(tbucket->mem);

	int bucks = n / Bucket::bucket_size;
	n -= Bucket::bucket_size * bucks;
	for (;bucks > 0; --bucks)
	{
		memcpy(buffer + written, sptr, Bucket::bucket_size);
		written += Bucket::bucket_size;

		tbucket = buck->next;
		sptr = static_cast<char*>(tbucket->mem);
	}

	n %= Bucket::bucket_size;
	if (n)
	{
		memcpy(buffer + written, sptr, n);
		written += n;
	}
ret:

	std::string str(buffer, written);
	free(buffer);
	return str;
}


std::string BuckWalker::read_cstring()
{
	int to_read = 0;			// the least we can read is terminated null
	BuckWalker walker{ *this };

	while (true)
	{
		int it_read = std::min(static_cast<int>(walker.endptr - walker.fptr), static_cast<int>(walker.buck->occupied)); // read for current iteration
		for (int i = 0; i < it_read; ++i)
			if (*(walker.fptr + i) == '\0')
			{
				to_read += i;
				return read_n(to_read);
			}
		to_read += it_read;

		assert(walker.buck->next != nullptr);
		walker.advance(it_read);
	}
}


int BuckWalker::read_int()
{
	int res;
	char *buffer = reinterpret_cast<char*>(&res);

	int n = sizeof(int);

	int written = 0;
	char *sptr = fptr, *eptr = endptr;

	int remdr = n % static_cast<int>(eptr - sptr);
	if (remdr)
	{
		memcpy(buffer, sptr, remdr);
		written += remdr;
	}

	n -= remdr;
	if (n)
	{
		memcpy(buffer + written, buck->next->mem, n);
	}

	return res;
}


void BuckWalker::write_n(const void* src, int n)
{
	const char *csrc = reinterpret_cast<const char*>(src);

	int written = 0;

	int remdr = n % static_cast<int>(endptr - fptr);
	if (remdr)
	{
		memcpy(fptr, csrc, remdr);
		written += remdr;
	}
	advance(remdr);

	int bucks = (n -= remdr) / Bucket::bucket_size;
	n -= Bucket::bucket_size * bucks;
	for (;bucks > 0; --bucks)
	{
		memcpy(fptr, csrc + written, Bucket::bucket_size);
		written += Bucket::bucket_size;
		advance(Bucket::bucket_size);
	}

	n %= Bucket::bucket_size;
	if (n)
	{
		memcpy(fptr, csrc + written, n);
		advance(n);
	}
}


void BuckWalker::write_int(const int i)
{
	const char *csrc = reinterpret_cast<const char*>(&i);

	int written = 0;

	int n = sizeof(int);

	int remdr = n % static_cast<int>(endptr - fptr);
	if (remdr)
	{
		memcpy(fptr, csrc, remdr);
		written += remdr;
	}
	advance(remdr);

	n -= remdr;
	if (n)
	{
		memcpy(fptr, csrc, n);
		advance(n);
	}
}


char& BuckWalker::operator*()
{
	return *fptr;
}


BuckWalker& BuckWalker::operator+=(int n)
{
	advance(n);
	return *this;
}
