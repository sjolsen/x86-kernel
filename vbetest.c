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

#include "kernel.h"

void kernel_main (multiboot_info_t* info,
                  __attribute__ ((unused)) multiboot_uint32_t magic)
{
	ISR_table_initialize (&isrt, &null_ISR);
	IDT_initialize (&idt);
	IRQ_disable (IRQ_PIT);

	ModeInfoBlock* mode_info = (ModeInfoBlock*)(uintptr_t) info->vbe_mode_info;
	uint8_t* ptr = (uint8_t*)(uintptr_t) mode_info->PhysBasePtr;
	for (int i = 0; i < 3*1024; i+=3) {
		ptr[i+0] = 0xFF;
		ptr[i+1] = 0xFF;
		ptr[i+2] = 0x00;
	}

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
	.width         = 1024,
	.height        = 768,
	.depth         = 32,
};
