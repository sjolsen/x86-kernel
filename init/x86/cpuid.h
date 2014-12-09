#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>

typedef struct {
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
} CPUID_info;

static inline
CPUID_info cpuid (uint32_t EAX_in)
{
	CPUID_info info;
	__asm__ volatile (
		"mov %0, %%eax\n"
		"cpuid\n"
		: "=a" (info.EAX),
		  "=b" (info.EBX),
		  "=c" (info.ECX),
		  "=d" (info.EDX)
		: "g" (EAX_in)
	);
	return info;
}

#endif
