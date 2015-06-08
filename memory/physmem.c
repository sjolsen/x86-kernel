#include "physmem.h"

static inline
void mzero64 (uint64_t* begin, uint64_t* end)
{
	for (uint64_t* ptr = begin; ptr != end; ++ptr)
		*ptr = 0;
}

static inline
uint8_t lowest_nonzero_bit (uint64_t value)
{
	uint64_t result;
	__asm__ ("bsfq %1, %0" : "=g" (result) : "g" (value));
	return result;
}



physmem_allocator physmem_make_allocator (uint64_t* bmp_buffer, uint64_t begin, uint64_t end)
{
	physmem_allocator phy = {
		.mem_base  = (uint8_t*)(uintptr_t) begin,
		.bmp_begin = bmp_buffer,
		.bmp_end   = bmp_buffer + (end - begin)/(4096 * 64)
	};

	mzero64 (phy.bmp_begin, phy.bmp_end);
	return phy;
}

physmem_alloc_result physmem_alloc (physmem_allocator (*phy))
{
	for (uint64_t* block = phy->bmp_begin; block != phy->bmp_end; ++block)
		if (~*block != 0) {
			uint8_t bit = lowest_nonzero_bit (~*block);
			uint64_t index = 64 * (block - phy->bmp_begin) + bit;
			*block |= (uint64_t)1 << bit;

			return (physmem_alloc_result) {
				.success = true,
				.base = phy->mem_base + (4096 * index)
			};
		}

	return (physmem_alloc_result) {
		.success = false
	};
}

void physmem_free (physmem_allocator (*phy), uint8_t* base)
{
	uint64_t index = (base - phy->mem_base)/4096;
	uint64_t block = index / 64;
	uint8_t  bit   = index % 64;

	phy->bmp_begin [block] &= ~(1 << bit);
}
