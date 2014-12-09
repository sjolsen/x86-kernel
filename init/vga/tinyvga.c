#include "tinyvga.h"
#include "x86/portio.h"

static inline
void vga_draw_cursor (tinyvga* vga)
{
	uint16_t position = VGA_WIDTH * vga->current_row + vga->current_column;
	outb (vga->io_base, 0x0F); // Position low byte
	outb (vga->io_base + 1, position & 0xFF);
	outb (vga->io_base, 0x0E); // Position high byte
	outb (vga->io_base + 1, position >> 8);
}

static
void vga_advance_line (tinyvga* vga)
{
	vga->current_column = 0;
	if (++vga->current_row >= VGA_HEIGHT) {
		volatile uint16_t* dst = (*vga->buffer) [0];
		volatile uint16_t* src = (*vga->buffer) [1];
		volatile uint16_t* const dst_end = dst + VGA_WIDTH * (VGA_HEIGHT - 1);
		volatile uint16_t* const src_end = src + VGA_WIDTH * (VGA_HEIGHT - 1);
		while (dst != dst_end)
			*dst++ = *src++;
		while (dst != src_end)
			*dst++ = make_vga_entry (' ', vga->current_color).value;
	}
}

static inline
void vga_advance_char (tinyvga* vga)
{
	if (++vga->current_column >= VGA_WIDTH)
		vga_advance_line (vga);
}

static inline
void vga_putraw (tinyvga* vga, char c)
{
	if (c == '\n')
		vga_advance_line (vga);
	else {
		(*vga->buffer) [vga->current_row] [vga->current_column] = make_vga_entry (c, vga->current_color).value;
		vga_advance_char (vga);
	}
}



tinyvga vga_initialize (void)
{
	return (tinyvga) {
		.buffer         = (volatile uint16_t (*) [VGA_HEIGHT] [VGA_WIDTH]) VGA_BASE,
		.io_base        = *((volatile uint16_t*) VGA_PORTADDR),
		.current_row    = 0,
		.current_column = 0,
		.current_color  = make_vga_color (COLOR_LIGHT_GREY, COLOR_BLACK)
	};
}

void vga_clear (tinyvga* vga)
{
	for (size_t row = 0; row < VGA_HEIGHT; ++row)
		for (size_t col = 0; col < VGA_WIDTH; ++col)
			(*vga->buffer) [row] [col] = make_vga_entry (' ', vga->current_color).value;
	vga->current_row = 0;
	vga->current_column = 0;
	vga_draw_cursor (vga);
}

void vga_putchar (tinyvga* vga, char c)
{
	vga_putraw (vga, c);
	vga_draw_cursor (vga);
}

void vga_put (tinyvga* vga, const char* str)
{
	for (size_t i = 0; str [i] != '\0'; ++i)
		vga_putraw (vga, str [i]);
	vga_draw_cursor (vga);
}

void vga_putline (tinyvga* vga, const char* str)
{
	vga_put (vga, str);
	vga_putchar (vga, '\n');
}
