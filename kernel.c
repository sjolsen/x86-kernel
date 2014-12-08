#include "kernel.h"
#include "vga/tinyvga.h"
#include "x86/cpuid.h"

void kernel_main (__attribute__ ((unused)) multiboot_info_t* info,
                  __attribute__ ((unused)) multiboot_uint32_t magic)
{
	tinyvga vga = vga_initialize ();
	vga_clear (&vga);

	if (cpuid (0x80000001).EDX & (1 << 29))
		vga_putline (&vga, "64-bit capable.");
	else
		vga_putline (&vga, "Not 64-bit capable.");
}
