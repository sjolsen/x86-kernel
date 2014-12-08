#include "kernel.h"
#include "vga/tinyvga.h"
#include "x86/cpuid.h"
#include "x86/GDT.h"
#include "x86/paging.h"
#include <stdbool.h>

static tinyvga vga;
static GDT_entry GDT [3];
static PML4_table pml4_table;
static PDP_table pdp_table;

static inline
bool capable_64 (void)
{
	return cpuid (0x80000001).EDX & (1 << 29);
}

static inline
void paging_initialize (void)
{
	pml4_table [0] = (PML4E) {
		.present         = 1,
		.writable        = 1,
		.PDPT_address    = (uint32_t) &pdp_table,
		.execute_disable = 0
	};
	pdp_table [0] = (PDPTE) {
		.direct = {
			.present = 1,
			.writable = 1,
			.page_size = 1,
			.page_address = (uint64_t) 0,
			.execute_disable = 0,
		}
	};
}

static inline
void enable_PAE (void)
{
	register uint32_t tmp = 0;
	__asm__ (
		"mov %%cr4, %0;"
		"or (1<<5), %0;"
		"mov %0, %%cr4"
		: "+r" (tmp)
	);
}

static inline
void load_PML4 (PML4_table* table)
{
	register uint32_t tmp = (uint32_t) &(*table)[0];
	__asm__ (
		"movl %0, %%cr3"
		:: "r" (tmp)
	);
}

static inline
void enable_LM (void)
{
	__asm__ (
		"mov $0xC0000080, %ecx;" // EFER MSR
		"rdmsr;"
		"or (1<<8), %eax;"
		"wrmsr"
	);
}

static inline
void enable_paging (void)
{
	register uint32_t tmp = 0;
	__asm__ (
		"mov %%cr0, %0;"
		"or (1<<31), %0;"
		"mov %0, %%cr0"
		: "+r" (tmp)
	);
}

static inline
bool LMA_set (void)
{
	register uint32_t tmp = 0;
	__asm__ (
		"mov $0xC0000080, %%ecx;" // EFER MSR
		"rdmsr;"
		"and (1<<10), %0;"
		: "=a" (tmp)
	);
	return tmp != 0;
}

void kernel_main (__attribute__ ((unused)) multiboot_info_t* info,
                  __attribute__ ((unused)) multiboot_uint32_t magic)
{
	vga = vga_initialize ();
	vga_clear (&vga);

	if (capable_64 ())
		vga_putline (&vga, "64-bit capable.");
	else {
		vga_putline (&vga, "Not 64-bit capable.");
		return;
	}

	GDT_initialize (GDT);
	vga_putline (&vga, "GDT initialized.");

	paging_initialize ();
	vga_putline (&vga, "Page table initialized.");

	enable_PAE ();
	vga_putline (&vga, "PAE bit set.");

	load_PML4 (&pml4_table);
	vga_putline (&vga, "PML4 table loaded.");

	enable_LM ();
	vga_putline (&vga, "Long mode enabled.");

	enable_paging ();
	vga_putline (&vga, "Paging enabled.");

	if (LMA_set ())
		vga_putline (&vga, "LMA set.");
	else
		vga_putline (&vga, "LMA not set.");
}
