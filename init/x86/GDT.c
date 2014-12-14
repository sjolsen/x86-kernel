#include "GDT.h"
#include <stdbool.h>

typedef struct __attribute__ ((packed)) __attribute__ ((aligned (8))) {
	uint8_t data [0x20];
} GDT;

extern
void install_GDT (GDT* gdt, uint16_t entries);

static inline
void reload_segments (uint16_t code_selector, uint16_t data_selector)
{
	uint32_t code = code_selector * 8;
	uint32_t data = data_selector * 8;
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

static GDT gdt = {
	.data = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x40, 0x00,
		0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00,
		0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00,
		0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xaf, 0x00
	}
};


// Extern functions

void GDT_initialize (void)
{
	install_GDT (&gdt, 4);
	reload_segments (KERNEL_INIT_SELECTOR, KERNEL_DATA_SELECTOR);
}
