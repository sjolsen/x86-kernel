#include "multiboot/multiboot.h"
#include "vbe/vbe.h"
#include "util/format.h"
#include "x86/interrupts/IDT.h"
#include "x86/interrupts/ISR.h"
#include "x86/interrupts/IRQ.h"
#include <stdint.h>
#include <stddef.h>

static IDT idt;
static ISR_table_t isrt;



void halt (void)
{
	__asm__ volatile (
		"cli;"
		"halt%=:"
		"hlt;"
		"jmp halt%="
		:
	);
}

void wait (void)
{
	__asm__ volatile (
		"sti;"
		"wait%=:"
		"hlt;"
		"jmp wait%="
		:
	);
}



void plot_pixel (ModeInfoBlock* mode_info,
                 uint32_t x, uint32_t y,
                 uint32_t R, uint32_t G, uint32_t B)
{
	uint8_t* base   = (uint8_t*)(uintptr_t) mode_info->PhysBasePtr;
	uint16_t pitch  = mode_info->BytesPerScanLine;
	uint16_t width  = (mode_info->BitsPerPixel + 8 - 1) / 8;
	uint32_t offset = y * pitch + x * width;

	const uint32_t cdata =
		(R & ((1 << mode_info->RedMaskSize)   - 1)) << mode_info->RedFieldPosition   |
		(G & ((1 << mode_info->GreenMaskSize) - 1)) << mode_info->GreenFieldPosition |
		(B & ((1 << mode_info->BlueMaskSize)  - 1)) << mode_info->BlueFieldPosition;
	const uint8_t* cdata_raw = (const uint8_t*) &cdata;

	for (uint8_t i = 0; i < width; ++i)
		base [offset + i] = cdata_raw [i];
}

#include "kernel.h"

void kernel_main (multiboot_info_t* info,
                  __attribute__ ((unused)) multiboot_uint32_t magic)
{
	ISR_table_initialize (&isrt, &null_ISR);
	IDT_initialize (&idt);
	IRQ_disable (IRQ_PIT);

	ModeInfoBlock* mode_info = (ModeInfoBlock*)(uintptr_t) info->vbe_mode_info;

	const uint32_t xres = mode_info->XResolution;
	const uint32_t yres = mode_info->XResolution;
	const uint32_t Rmax = (1 << mode_info->RedMaskSize)   - 1;
	const uint32_t Gmax = (1 << mode_info->GreenMaskSize) - 1;
	const uint32_t Bmax = (1 << mode_info->BlueMaskSize)  - 1;
	for (uint32_t y = 0; y < yres; ++y)
		for (uint32_t x = 0; x < xres; ++x)
			plot_pixel (mode_info, x, y,
			            (x * (Rmax+1)) / xres,
			            (y * (Gmax+1)) / yres,
			            Bmax);

	wait ();
}

#define FLAGS (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE)

__attribute__ ((aligned (MULTIBOOT_HEADER_ALIGN)))
const struct multiboot_header kernel_header = {
	.magic         = MULTIBOOT_HEADER_MAGIC,
	.flags         = FLAGS,
	.checksum      = -(MULTIBOOT_HEADER_MAGIC + FLAGS),
	.header_addr   = 0,
	.load_addr     = 0,
	.load_end_addr = 0,
	.bss_end_addr  = 0,
	.entry_addr    = 0,
	.mode_type     = 0,
	.width         = 1280,
	.height        = 1024,
	.depth         = 32,
};
