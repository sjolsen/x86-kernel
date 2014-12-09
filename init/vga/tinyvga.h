#ifndef TINYVGA_H
#define TINYVGA_H

#include <stddef.h>
#include <stdint.h>

enum {
	VGA_BASE     = 0xB8000,
	VGA_PORTADDR = 0x0463,
	VGA_WIDTH    = 80,
	VGA_HEIGHT   = 25
};

typedef enum {
	COLOR_BLACK         = 0,
	COLOR_BLUE          = 1,
	COLOR_GREEN         = 2,
	COLOR_CYAN          = 3,
	COLOR_RED           = 4,
	COLOR_MAGENTA       = 5,
	COLOR_BROWN         = 6,
	COLOR_LIGHT_GREY    = 7,
	COLOR_DARK_GREY     = 8,
	COLOR_LIGHT_BLUE    = 9,
	COLOR_LIGHT_GREEN   = 10,
	COLOR_LIGHT_CYAN    = 11,
	COLOR_LIGHT_RED     = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN   = 14,
	COLOR_WHITE         = 15,
} vga_color_code;

typedef union {
	struct {
		uint8_t fg : 4;
		uint8_t bg : 4;
	};
	uint8_t value;
} vga_color;

typedef union {
	struct {
		uint8_t character;
		vga_color color;
	};
	uint16_t value;
} vga_entry;

static inline
vga_color make_vga_color (vga_color_code fg, vga_color_code bg)
{
	return (vga_color) {{fg, bg}};
}

static inline
vga_entry make_vga_entry (uint8_t c, vga_color color)
{
	return (vga_entry) {{c, color}};
}



typedef struct {
	volatile uint16_t (*buffer) [VGA_HEIGHT] [VGA_WIDTH];
	uint16_t io_base;
	size_t current_row;
	size_t current_column;
	vga_color current_color;
} tinyvga;

tinyvga vga_initialize (void);
void vga_clear (tinyvga* vga);
void vga_putchar (tinyvga* vga, char c);
void vga_put (tinyvga* vga, const char* str);
void vga_putline (tinyvga* vga, const char* str);

#endif
