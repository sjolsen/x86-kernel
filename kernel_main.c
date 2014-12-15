#include "multiboot/multiboot.h"
#include "vga/tinyvga.h"
#include "util/format.h"
#include "x86/interrupts/IDT.h"
#include "x86/interrupts/ISR.h"
#include "x86/interrupts/IRQ.h"
#include <stdint.h>
#include <stddef.h>

static tinyvga vga;
static IDT idt;
static ISR_table_t isrt;

static
void halt (void)
{
		__asm__ volatile (
			"cli;"
		".halt:"
			"hlt;"
			"jmp .halt"
		);
}

static
void wait (void)
{
		__asm__ volatile (
			"sti;"
		".wait:"
			"hlt;"
			"jmp .wait"
		);
}

static
void debug_ISR (INT_index interrupt)
{
	static const char* name [] = {
		[0x00] = "divide_by_zero",
		[0x01] = "debugger",
		[0x02] = "NMI",
		[0x03] = "breakpoint",
		[0x04] = "overflow",
		[0x05] = "bounds",
		[0x06] = "invalid_opcode",
		[0x07] = "coprocessor_unavailable",
		[0x08] = "double_fault",
		[0x0A] = "invalid_TSS",
		[0x0B] = "segment_missing",
		[0x0C] = "stack_fault",
		[0x0D] = "protection_fault",
		[0x0E] = "page_fault",
		[0x10] = "math_fault",
		[0x11] = "alignment_check",
		[0x12] = "machine_check",
		[0x13] = "SIMD_exception",

		[0x20] = "IRQ_PIT",
		[0x21] = "IRQ_keyboard",
		[0x22] = "IRQ_cascade",
		[0x23] = "IRQ_COM2",
		[0x24] = "IRQ_COM1",
		[0x25] = "IRQ_LPT2",
		[0x26] = "IRQ_floppy",
		[0x27] = "IRQ_LPT1",
		[0x28] = "IRQ_CMOS_RTC",
		[0x29] = "IRQ_misc1",
		[0x2A] = "IRQ_misc2",
		[0x2B] = "IRQ_misc3",
		[0x2C] = "IRQ_mouse",
		[0x2D] = "IRQ_FPU",
		[0x2E] = "IRQ_HDD1",
		[0x2F] = "IRQ_HDD2",
	};

	char buffer [3];
	vga_put (&vga, "Interrupt 0x");
	vga_put (&vga, format_uint (buffer, interrupt, 2, 16));
	vga_put (&vga, ": ");
	vga_putline (&vga, name [interrupt]);
}

void kernel_main (__attribute__ ((unused)) multiboot_info_t* info,
                  __attribute__ ((unused)) multiboot_uint32_t magic)
{
	vga = vga_initialize ();
	vga_clear (&vga);
	vga_putline (&vga, "Success.");

	ISR_table_initialize (&isrt, &debug_ISR);
	IDT_initialize (&idt);
	IRQ_enable (IRQ_PIT);

	wait ();

	halt ();
}
