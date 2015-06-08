#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct physmem_allocator {
	uint8_t*  mem_base;
	uint64_t* bmp_begin;
	uint64_t* bmp_end;
} physmem_allocator;

typedef struct physmem_alloc_result {
	bool success;
	uint8_t* base;
} physmem_alloc_result;

// The region of memory denoted by begin and end should be 256 kiB-aligned and
// be a non-zero multiple of 256 kiB large. The buffer should point to as many
// contiguous uint64_t objects as there are 256 kiB subregions of [begin, end).
physmem_allocator physmem_make_allocator (uint64_t* bmp_buffer, uint64_t begin, uint64_t end);

physmem_alloc_result physmem_alloc (physmem_allocator (*phy));
void physmem_free (physmem_allocator (*phy), uint8_t* base);

#endif
