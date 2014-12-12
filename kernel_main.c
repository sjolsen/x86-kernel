#include <stdint.h>
#include "vga/tinyvga.h"

static tinyvga vga;

void kernel_main (void)
{
	register uint64_t tmp = 0;
	__asm__ volatile (
		"movq $(1 << 48), %0;"
		"shr $32, %0;"
		: "+r" (tmp)
	);

	vga = vga_initialize ();
	vga_clear (&vga);
	vga_putline (&vga, "64-bit boot successful!");
	vga_putline (&vga, (tmp == (1 << 16) ? "GOOD" : "BAD"));
}
