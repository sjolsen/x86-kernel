#include "kernel.h"
#include "x86/GDT.h"
#include "x86/paging.h"
#include <stdbool.h>

static GDT gdt;
static TSS_64 tss;
static PML4_table pml4_table;
static PDP_table pdp_table;
static PDP_table high_pdp_table;
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

static
void paging_initialize (void)
{
	// Identity-map kernel data and init code
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
	for (int i = 0; i < 512; ++i)
		pdp_table [i] = (PDPTE) {
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
				.page_address    = i,
				.execute_disable = 0
			}
		};

	// Map the 64-bit code at 0xFFFFFFFFC0000000
	pml4_table [511] = (PML4E) {
		.present         = 1,
		.writable        = 0,
		.user            = 0,
		.write_through   = 0,
		.cache_disable   = 0,
		.accessed        = 0,
		.reserved        = 0, // Must be 0
		.PDPT_address    = (uint32_t) &high_pdp_table >> 12,
		.execute_disable = 0
	};
	high_pdp_table [511] = (PDPTE) {
		.indirect = {
			.present         = 1,
			.writable        = 0,
			.user            = 0,
			.write_through   = 0,
			.cache_disable   = 0,
			.accessed        = 0,
			.page_size       = 0, // Must be 0
			.PD_address      = (uint32_t) &high_page_directory >> 12,
			.execute_disable = 0,
		}
	};
	for (uint32_t i = 0; i < _linkaddr(_ktext_size); i += 0x200000)
		high_page_directory [i] = (PDE) {
			.direct = {
				.present         = 1,
				.writable        = 0,
				.user            = 0,
				.write_through   = 0,
				.cache_disable   = 0,
				.accessed        = 0,
				.dirty           = 0,
				.page_size       = 1, // Must be 1
				.global          = 1,
				.PAT             = 0,
				.reserved        = 0,
				.page_address    = (_linkaddr(_ktext_lma) + i) >> 21,
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
void enable_NXE (void)
{
	__asm__ volatile (
		"mov $0xC0000080, %ecx;" // EFER MSR
		"rdmsr;"
		"or $(1<<11), %eax;"
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

static inline
void enable_write_protection (void)
{
	register uint32_t tmp = 0;
	__asm__ volatile (
		"mov %%cr0, %0;"
		"or $(1<<16), %0;"
		"mov %0, %%cr0"
		: "+r" (tmp)
	);
}

static inline
void enable_global_pages (void)
{
	register uint32_t tmp = 0;
	__asm__ volatile (
		"mov %%cr4, %0;"
		"or $(1<<7), %0;"
		"mov %0, %%cr4"
		: "+r" (tmp)
	);
}

static
void halt (void)
{
		__asm__ volatile (
		".halt:"
			"cli;"
			"hlt;"
			"jmp .halt"
		);
}

void init (void)
{
	if (!capable_64 ())
		halt ();

	GDT_initialize (&gdt, &tss);
	paging_initialize ();
	enable_PAE ();
	load_PML4 (&pml4_table);
	enable_LM ();
	enable_NXE ();
	enable_paging ();
	enable_write_protection ();
	enable_global_pages ();
}
