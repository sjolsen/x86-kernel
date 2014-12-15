#include "multiboot/multiboot.h"
#include "vga/tinyvga.h"
#include "util/format.h"
#include <stdint.h>
#include <stddef.h>

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

void kernel_main (multiboot_info_t* info,
                  multiboot_uint32_t magic)
{
	tinyvga vga = vga_initialize ();
	vga_clear (&vga);
	vga_putline (&vga, "Success.");

	char buffer [17];
	vga_put (&vga, "info:  0x");
	vga_putline (&vga, format_uint (buffer, (uint64_t) info, 16, 16));
	vga_put (&vga, "magic: 0x");
	vga_putline (&vga, format_uint (buffer, magic, 8, 16));
	halt ();
}
