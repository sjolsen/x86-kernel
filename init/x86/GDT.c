#include "GDT.h"
#include <stdbool.h>

static inline
GDT_entry make_code_GDT (uint32_t base, uint32_t limit,
                         bool readable, bool conforming,
                         uint8_t privilege, bool page_granularity,
                         bool long_mode)
{
	return (GDT_entry) {
		.limit_low   = limit & 0xFFFF,
		.base_low    = base & 0xFFFF,
		.base_high1  = (base >> 16) & 0xFF,
		.accessed    = 0,
		.rw_bit      = readable,
		.dc_bit      = conforming,
		.ex_bit      = 1,
		.one         = 1,
		.privilege   = privilege,
		.present     = 1,
		.limit_high  = (limit >> 16) & 0xFF,
		.zero        = 0,
		.mode64      = long_mode ? 1 : 0,
		.mode32      = long_mode ? 0 : 1,
		.granularity = page_granularity,
		.base_high2  = (base >> 24) & 0xFF
	};
}

static inline
GDT_entry make_data_GDT (uint32_t base, uint32_t limit,
                         bool writable, bool downward,
                         uint8_t privilege, bool page_granularity,
                         bool long_mode)
{
	return (GDT_entry) {
		.limit_low   = limit & 0xFFFF,
		.base_low    = base & 0xFFFF,
		.base_high1  = (base >> 16) & 0xFF,
		.accessed    = 0,
		.rw_bit      = writable,
		.dc_bit      = downward,
		.ex_bit      = 0,
		.one         = 1,
		.privilege   = privilege,
		.present     = 1,
		.limit_high  = (limit >> 16) & 0xFF,
		.zero        = 0,
		.mode64      = long_mode ? 1 : 0,
		.mode32      = long_mode ? 0 : 1,
		.granularity = page_granularity,
		.base_high2  = (base >> 24) & 0xFF
	};
}

static inline
TSS_descriptor make_TSSD (TSS_64* tss, uint8_t privilege)
{
	uint32_t base  = (uint32_t) tss;
	uint32_t limit = sizeof (TSS_64) - 1;
	return (TSS_descriptor) {
		.limit_low   = limit & 0xFFFF,
		.base_low    = base & 0xFFFF,
		.base_high1  = (base >> 16) & 0xFF,
		.type        = 0b1001,
		.zero0       = 0,
		.privilege   = privilege,
		.present     = 1,
		.limit_high  = (limit >> 16) & 0xFF,
		.zero1       = 0,
		.granularity = 0,
		.base_high2  = (base >> 24) & 0xFF,
		.base_high3  = 0,
		.reserved    = 0
	};
}

extern
void install_GDT (GDT* gdt, uint16_t entries);

static inline
void reload_segments (uint16_t code_selector, uint16_t data_selector)
{
	uint16_t code = code_selector * sizeof (GDT_entry);
	uint32_t data = data_selector * sizeof (GDT_entry);
	__asm__ (
		"pushw %0\n"
		"pushl $reload_CS\n"
		"lret\n"
	"reload_CS:"
		"mov %1, %%ds\n"
		"mov %1, %%es\n"
		"mov %1, %%fs\n"
		"mov %1, %%gs\n"
		"mov %1, %%ss\n"
		:: "r" (code), "r" (data)
	);
}

static inline
void install_TSS (uint16_t selector)
{
	selector *= 8;
	__asm__ ("ltr %0" :: "r" (selector));
}

extern const void _ktext_base;


// Extern functions

void GDT_initialize (GDT* gdt, TSS_64* tss)
{
	gdt->GDTEs [KERNEL_NULL_SELECTOR] = make_data_GDT (0, 0, false, false, 0, 0, false);
	gdt->GDTEs [KERNEL_INIT_SELECTOR] = make_code_GDT (0, -1, true, false, 0, 1, false);
	gdt->GDTEs [KERNEL_DATA_SELECTOR] = make_data_GDT (0, -1, true, false, 0, 1, false);
	gdt->GDTEs [KERNEL_TEXT_SELECTOR] = make_code_GDT (0, -1, true, false, 0, 1, true);
	gdt->TSSD = make_TSSD (tss, 0);

	install_GDT (gdt, sizeof (GDT) / sizeof (GDT_entry));
	reload_segments (KERNEL_INIT_SELECTOR, KERNEL_DATA_SELECTOR);
	install_TSS (TSS_SELECTOR);
}
