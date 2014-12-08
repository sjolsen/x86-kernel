#include "GDT.h"
#include <stdbool.h>

static inline
GDT_entry make_code_GDT (uint32_t base, uint32_t limit,
                         bool readable, bool conforming,
                         uint8_t privilege, bool page_granularity)
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
		.mode32      = 1,
		.granularity = page_granularity,
		.base_high2  = (base >> 24) & 0xFF
	};
}

static inline
GDT_entry make_data_GDT (uint32_t base, uint32_t limit,
                         bool writable, bool downward,
                         uint8_t privilege, bool page_granularity)
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
		.mode32      = 1,
		.granularity = page_granularity,
		.base_high2  = (base >> 24) & 0xFF
	};
}

extern
void install_GDT (const GDT_entry GDT [static 3], uint16_t entries);

static inline
void reload_segments (uint16_t code_selector, uint16_t data_selector)
{
	uint32_t code = code_selector * sizeof (GDT_entry);
	uint32_t data = data_selector * sizeof (GDT_entry);
	__asm__ (
		"pushl %0\n"
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


// Extern functions

void GDT_initialize (GDT_entry GDT [static 3])
{
	GDT [KERNEL_NULL_SELECTOR] = make_data_GDT (0, 0, false, false, 0, 0);
	GDT [KERNEL_CODE_SELECTOR] = make_code_GDT (0, -1, true, false, 0, 1);
	GDT [KERNEL_DATA_SELECTOR] = make_data_GDT (0, -1, true, false, 0, 1);

	install_GDT (GDT, 3);
	reload_segments (KERNEL_CODE_SELECTOR, KERNEL_DATA_SELECTOR);
}
