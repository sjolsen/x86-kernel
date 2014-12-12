#include "vga/tinyvga.h"
#include "x86/cpuid.h"
#include "x86/GDT.h"
#include "x86/paging.h"
#include "x86/interrupts/ISR.h"
#include "x86/interrupts/IDT.h"
#include "x86/interrupts/IRQ.h"
#include <stdbool.h>

static tinyvga vga;
static GDT gdt;
static TSS_64 tss;
static PML4_table pml4_table;
static PDP_table pdp_table;

static inline
bool capable_64 (void)
{
	return cpuid (0x80000001).EDX & (1 << 29);
}

extern const void _ktext_base;
extern const void _ktext_start;

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
		.PDPT_address    = (uint32_t) &pdp_table,
		.execute_disable = 0
	};
	pdp_table [1] = (PDPTE) {
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
			.page_address    = (uint32_t) &_ktext_base,
			.execute_disable = 0,
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

static
void debug_halt_ISR (__attribute__ ((unused)) INT_index interrupt)
{
	static const char* name [] = {
		[0x00] = "divide_by_zero",
		[0x01] = "debugger",
		[0x02] = "NMI",
		[0x03] = "breakpoint",
		[0x04] = "overflow",
		[0x05] = "bounds",
		[0x06] = "invalid_opcode",
		[0x07] = "coprocessor_unavailable",
		[0x08] = "double_fault",
		[0x0A] = "invalid_TSS",
		[0x0B] = "segment_missing",
		[0x0C] = "stack_fault",
		[0x0D] = "protection_fault",
		[0x0E] = "page_fault",
		[0x10] = "math_fault",
		[0x11] = "alignment_check",
		[0x12] = "machine_check",
		[0x13] = "SIMD_exception",

		[0x20] = "IRQ_PIT",
		[0x21] = "IRQ_keyboard",
		[0x22] = "IRQ_cascade",
		[0x23] = "IRQ_COM2",
		[0x24] = "IRQ_COM1",
		[0x25] = "IRQ_LPT2",
		[0x26] = "IRQ_floppy",
		[0x27] = "IRQ_LPT1",
		[0x28] = "IRQ_CMOS_RTC",
		[0x29] = "IRQ_misc1",
		[0x2A] = "IRQ_misc2",
		[0x2B] = "IRQ_misc3",
		[0x2C] = "IRQ_mouse",
		[0x2D] = "IRQ_FPU",
		[0x2E] = "IRQ_HDD1",
		[0x2F] = "IRQ_HDD2",
	};

	vga_put (&vga, "Interrupt: ");
	vga_putline (&vga, name [interrupt]);

	__asm__ volatile (
		".halt:"
			"cli;"
			"hlt;"
			"jmp .halt"
	);
}

void initialize_interrupts (void)
{
	IDT_initialize ();
	ISR_table_initialize (&debug_halt_ISR);
	IRQ_disable (IRQ_PIT);
	__asm__ ("sti");
}

void init (void)
{
	vga = vga_initialize ();
	vga_clear (&vga);
	#define vga_putline(...)
	if (capable_64 ())
		vga_putline (&vga, "64-bit capable.");
	else {
		vga_putline (&vga, "Not 64-bit capable.");
		return;
	}

	GDT_initialize (&gdt, &tss);
	vga_putline (&vga, "GDT initialized.");

	initialize_interrupts ();
	vga_putline (&vga, "Interrupt table initialized.");

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

	/* for (int i = 0; i < 100; ++i) */
	/* { */
	/* 	static const char tbl [] = "0123456789ABCDEF"; */

	/* 	const uint8_t b = ((uint8_t*)&_ktext_start) [i]; */
	/* 	const uint8_t b0 = b & 0xF; */
	/* 	const uint8_t b1 = b >> 4; */
	/* 	vga_putchar (&vga, tbl [b1]); */
	/* 	vga_putchar (&vga, tbl [b0]); */
	/* 	vga_putchar (&vga, ' '); */
	/* } */
}
