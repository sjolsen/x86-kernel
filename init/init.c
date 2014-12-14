#include "x86/GDT.h"
#include "x86/paging.h"
#include <stdbool.h>

static PML4_table pml4_table;
static PDP_table pdp_table;
static Page_directory page_directory;
static Page_directory high_page_directory;

static inline
bool capable_64 (void)
{
	register uint32_t tmp = 0;
	__asm__ volatile (
		"mov $0x80000001, %%eax;"
		"cpuid;"
		: "=d" (tmp)
	);
	return tmp & (1 << 29);
}

extern const void _ktext_base;
extern const void _ktext_start;
extern const void _ktext_end;

static inline
void paging_initialize (void)
{
	pml4_table [0] = (PML4E) {
		.present         = 1,
		.writable        = 1,
		.user            = 0,
		.write_through   = 0,
		.cache_disable   = 0,
		.accessed        = 0,
		.reserved        = 0, // Must be 0
		.PDPT_address    = (uint32_t) &pdp_table >> 12,
		.execute_disable = 0
	};
	// Identity-map most of the kernel
	pdp_table [0] = (PDPTE) {
		.indirect = {
			.present         = 1,
			.writable        = 1,
			.user            = 0,
			.write_through   = 0,
			.cache_disable   = 0,
			.accessed        = 0,
			.page_size       = 0, // Must be 0
			.PD_address      = (uint32_t) &page_directory >> 12,
			.execute_disable = 0,
		}
	};
	page_directory [0] = (PDE) {
		.direct = {
			.present         = 1,
			.writable        = 1,
			.user            = 0,
			.write_through   = 0,
			.cache_disable   = 0,
			.accessed        = 0,
			.dirty           = 0,
			.page_size       = 1, // Must be 1
			.global          = 1,
			.PAT             = 0,
			.reserved        = 0,
			.page_address    = (uint32_t) 0 >> 21,
			.execute_disable = 0
		}
	};
	// Map the 64-bit code at 0x40000000
	pdp_table [1] = (PDPTE) {
		.indirect = {
			.present         = 1,
			.writable        = 1,
			.user            = 0,
			.write_through   = 0,
			.cache_disable   = 0,
			.accessed        = 0,
			.page_size       = 0, // Must be 0
			.PD_address      = (uint32_t) &high_page_directory >> 12,
			.execute_disable = 0,
		}
	};
	for (int32_t i = 0; i < ((&_ktext_end - &_ktext_start + 0x001FFFFF) >> 21); ++i)
		high_page_directory [i] = (PDE) {
			.direct = {
				.present         = 1,
				.writable        = 1,
				.user            = 0,
				.write_through   = 0,
				.cache_disable   = 0,
				.accessed        = 0,
				.dirty           = 0,
				.page_size       = 1, // Must be 1
				.global          = 1,
				.PAT             = 0,
				.reserved        = 0,
				.page_address    = ((uint32_t) &_ktext_base >> 21) + i,
				.execute_disable = 0
			}
		};
}

static inline
void enable_PAE (void)
{
	register uint32_t tmp = 0;
	__asm__ volatile (
		"mov %%cr4, %0;"
		"or $(1<<5), %0;"
		"mov %0, %%cr4"
		: "+r" (tmp)
	);
}

static inline
void load_PML4 (PML4_table* table)
{
	register uint32_t tmp = (uint32_t) &(*table)[0];
	__asm__ volatile (
		"movl %0, %%cr3"
		:: "r" (tmp)
	);
}

static inline
void enable_LM (void)
{
	__asm__ volatile (
		"mov $0xC0000080, %ecx;" // EFER MSR
		"rdmsr;"
		"or $(1<<8), %eax;"
		"wrmsr"
	);
}

static inline
void enable_paging (void)
{
	register uint32_t tmp = 0;
	__asm__ volatile (
		"mov %%cr0, %0;"
		"or $(1<<31), %0;"
		"mov %0, %%cr0"
		: "+r" (tmp)
	);
}

void init (void)
{
	if (!capable_64 ())
		__asm__ volatile (
			".halt:"
			"cli;"
			"hlt;"
			"jmp .halt"
		);

	GDT_initialize ();
	paging_initialize ();
	enable_PAE ();
	load_PML4 (&pml4_table);
	enable_LM ();
	enable_paging ();
}
