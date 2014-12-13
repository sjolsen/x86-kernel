#include <stdint.h>
#include <stddef.h>

void kernel_main (void)
{
	const char* str = "Success.";

	volatile uint16_t* vga_buffer = (volatile uint16_t*) 0xB8000;
	size_t i = 0;
	for (; i < 8; ++i)
		vga_buffer [i] = (uint16_t)(str[i]) | ((uint16_t)7 << 8) | ((uint16_t)0) << 12;
	for (; i < 80 * 25; ++i)
		vga_buffer [i] = (uint16_t)' ' | ((uint16_t)7 << 8) | ((uint16_t)0) << 12;
}
