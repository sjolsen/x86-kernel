#include "x86/GDT.h"
#include "x86/paging.h"
#include "vga/tinyvga.h"
#include <stdbool.h>

static tinyvga vga;

static GDT gdt;
static TSS_64 tss;
static PML4_table pml4_table;
static PDP_table pdp_table;
static Page_directory page_directory;
static Page_directory high_page_directory;

typedef struct {
	uint16_t pml4;
	uint16_t pdp;
	uint16_t pd;
	uint16_t pt;
	uint16_t p;
} page_indices;

typedef enum {
	found             = 0,
	dunno             = 1,
	noncanonical      = 2,
	PML4E_not_present = 3,
	PDPTE_not_present = 4,
	PDE_not_present   = 5,
	PTE_not_present   = 6
} l2lstatus;

static
const char* l2lstatus_tostr (l2lstatus status)
{
	static const char* table [] = {
		[found]             = "found",
		[dunno]             = "unknown",
		[noncanonical]      = "noncanonical",
		[PML4E_not_present] = "PML4E not present",
		[PDPTE_not_present] = "PDPTE not present",
		[PDE_not_present]   = "PDE not present",
		[PTE_not_present]   = "PTE not present"
	};
	return table [status];
}

static inline
uint64_t log2linaddr (uint64_t logaddr, l2lstatus* status, page_indices* indices)
{
	*status = dunno;

	// Check for canonical addresses
	uint64_t highbits = logaddr & 0xFFFF800000000000;
	if ((highbits != 0x0000000000000000) &&
	    (highbits != 0xFFFF800000000000)) {
		*status = noncanonical;
		return 0;
	}
	logaddr &= 0x0000FFFFFFFFFFFF;

	// Grab paging indices
	page_indices i = {
		.pml4 = logaddr >> 39 & 0x01FFFF,
		.pdp  = logaddr >> 30 & 0x01FFFF,
		.pd   = logaddr >> 21 & 0x01FFFF,
		.pt   = logaddr >> 12 & 0x01FFFF
	};
	*indices = i;

	// Translate
	if (!pml4_table [i.pml4].present) {
		*status = PML4E_not_present;
		return 0;
	}

	const PDP_table* pdpt = (const PDP_table*) (uint32_t) (pml4_table [i.pml4].PDPT_address << 12);
	if (!(*pdpt) [i.pdp].present) {
		*status = PDPTE_not_present;
		return 0;
	}
	if ((*pdpt) [i.pdp].page_size == 1) {
		*status = found;
		return ((*pdpt) [i.pdp].direct.page_address << 30) | (logaddr & 0x3FFFFFFF);
	}

	const Page_directory* pd = (const Page_directory*) (uint32_t) ((*pdpt) [i.pdp].indirect.PD_address << 12);
	if (!(*pd) [i.pd].present) {
		*status = PDE_not_present;
		return 0;
	}
	if ((*pd) [i.pd].page_size == 1) {
		*status = found;
		return ((*pd) [i.pd].direct.page_address << 21) | (logaddr & 0x0001FFFFF);
	}

	const Page_table* pt = (const Page_table*) (uint32_t) ((*pd) [i.pd].indirect.PT_address << 12);
	if (!(*pt) [i.pt].present) {
		*status = PTE_not_present;
		return 0;
	}
	*status = found;
	return ((*pt) [i.pt].page_address << 12) | (logaddr & 0x00000FFF);
}

static
void print_64 (uint64_t addr)
{
	char buffer [19] = "0x";
	for (size_t i = 0; i < 16; ++i) {
		char digit = "0123456789ABCDEF" [addr % 16];
		addr /= 16;
		buffer [17 - i] = digit;
	}
	vga_put (&vga, buffer);
}

static
void print_16 (uint64_t addr)
{
	char buffer [7] = "0x";
	for (size_t i = 0; i < 4; ++i) {
		char digit = "0123456789ABCDEF" [addr % 16];
		addr /= 16;
		buffer [5 - i] = digit;
	}
	vga_put (&vga, buffer);
}

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
extern const void _ktext_size;

static __attribute__ ((noinline))
void paging_initialize (void)
{
	__asm__ volatile ("movl %0, %%eax" :: "r" (&_ktext_size));
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
	// Map the 64-bit code at 0xFFFF800000000000
	pdp_table [256] = (PDPTE) {
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
	for (uint32_t i = 0; i < (uint32_t) &_ktext_size; i += 0x200000)
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
				.page_address    = ((uint32_t) &_ktext_base + i) >> 21,
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

void halt (void)
{
		__asm__ volatile (
			".halt:"
			"cli;"
			"hlt;"
			"jmp .halt"
		);
}

static
void debug_addr (uint64_t addr)
{
	print_64 (addr);
	vga_put (&vga, ": ");

	l2lstatus status = dunno;
	page_indices indices = {
		.pml4 = 0,
		.pdp  = 0,
		.pd   = 0,
		.pt   = 0
	};
	addr = log2linaddr (addr, &status, &indices);

	if (status == found)
		print_64 (addr);
	else {
		vga_put (&vga, "not found (");
		vga_put (&vga, l2lstatus_tostr (status));
		vga_put (&vga, ": ");
		print_16 (indices.pml4);
		vga_putchar (&vga, ' ');
		print_16 (indices.pdp);
		vga_putchar (&vga, ' ');
		print_16 (indices.pd);
		vga_putchar (&vga, ' ');
		print_16 (indices.pt);
		vga_putchar (&vga, ' ');
		print_16 (indices.p);
		vga_putline (&vga, ")");
	}

	vga_putchar (&vga, '\n');
}

void init (void)
{
	if (!capable_64 ())
		halt ();

	GDT_initialize (&gdt, &tss);
	paging_initialize ();

	vga = vga_initialize ();
	debug_addr (0x0000000000100000);
	debug_addr (0xFFFF800000000000);
	halt ();

	enable_PAE ();
	load_PML4 (&pml4_table);
	enable_LM ();
	enable_paging ();
}
