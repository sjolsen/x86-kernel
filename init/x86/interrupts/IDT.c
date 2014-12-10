#include "IDT.h"
#include "x86/GDT.h"
#include "8259.h"
#include "isr_stub.h"
#include "ISR.h"

static
IDT_entry IDT [INT_LIMIT];

static
IDT_entry make_IDT_entry (void (*address) (void), uint16_t code_selector)
{
	enum {
		TASK_32 = 0x5,
		INTR_16 = 0x6,
		TRAP_16 = 0x7,
		INTR_32 = 0xE,
		TRAP_32 = 0xF
	};

	return (IDT_entry) {
		.address_low  = (uintptr_t) address & 0xFFFF,
		.selector     = code_selector * sizeof (IDT_entry),
		.zero1        = 0,
		.gate_type    = INTR_32,
		.zero2        = 0,
		.privilege    = 0,
		.present      = 1,
		.address_high = ((uintptr_t) address >> 16) & 0xFFFF
	};
}

void install_IDT (const IDT_entry* base, uint16_t entries);


// Extern functions

void IDT_initialize (void)
{
	remap_8259_PIC (INT_IRQ_MBASE, INT_IRQ_SBASE);

	IDT [0x00] = make_IDT_entry (&_ISR_00, KERNEL_INIT_SELECTOR);
	IDT [0x01] = make_IDT_entry (&_ISR_01, KERNEL_INIT_SELECTOR);
	IDT [0x02] = make_IDT_entry (&_ISR_02, KERNEL_INIT_SELECTOR);
	IDT [0x03] = make_IDT_entry (&_ISR_03, KERNEL_INIT_SELECTOR);
	IDT [0x04] = make_IDT_entry (&_ISR_04, KERNEL_INIT_SELECTOR);
	IDT [0x05] = make_IDT_entry (&_ISR_05, KERNEL_INIT_SELECTOR);
	IDT [0x06] = make_IDT_entry (&_ISR_06, KERNEL_INIT_SELECTOR);
	IDT [0x07] = make_IDT_entry (&_ISR_07, KERNEL_INIT_SELECTOR);
	IDT [0x08] = make_IDT_entry (&_ISR_08, KERNEL_INIT_SELECTOR);
	IDT [0x09] = make_IDT_entry (&_ISR_09, KERNEL_INIT_SELECTOR);
	IDT [0x0A] = make_IDT_entry (&_ISR_0A, KERNEL_INIT_SELECTOR);
	IDT [0x0B] = make_IDT_entry (&_ISR_0B, KERNEL_INIT_SELECTOR);
	IDT [0x0C] = make_IDT_entry (&_ISR_0C, KERNEL_INIT_SELECTOR);
	IDT [0x0D] = make_IDT_entry (&_ISR_0D, KERNEL_INIT_SELECTOR);
	IDT [0x0E] = make_IDT_entry (&_ISR_0E, KERNEL_INIT_SELECTOR);
	IDT [0x0F] = make_IDT_entry (&_ISR_0F, KERNEL_INIT_SELECTOR);
	IDT [0x10] = make_IDT_entry (&_ISR_10, KERNEL_INIT_SELECTOR);
	IDT [0x11] = make_IDT_entry (&_ISR_11, KERNEL_INIT_SELECTOR);
	IDT [0x12] = make_IDT_entry (&_ISR_12, KERNEL_INIT_SELECTOR);
	IDT [0x13] = make_IDT_entry (&_ISR_13, KERNEL_INIT_SELECTOR);
	IDT [0x14] = make_IDT_entry (&_ISR_14, KERNEL_INIT_SELECTOR);
	IDT [0x15] = make_IDT_entry (&_ISR_15, KERNEL_INIT_SELECTOR);
	IDT [0x16] = make_IDT_entry (&_ISR_16, KERNEL_INIT_SELECTOR);
	IDT [0x17] = make_IDT_entry (&_ISR_17, KERNEL_INIT_SELECTOR);
	IDT [0x18] = make_IDT_entry (&_ISR_18, KERNEL_INIT_SELECTOR);
	IDT [0x19] = make_IDT_entry (&_ISR_19, KERNEL_INIT_SELECTOR);
	IDT [0x1A] = make_IDT_entry (&_ISR_1A, KERNEL_INIT_SELECTOR);
	IDT [0x1B] = make_IDT_entry (&_ISR_1B, KERNEL_INIT_SELECTOR);
	IDT [0x1C] = make_IDT_entry (&_ISR_1C, KERNEL_INIT_SELECTOR);
	IDT [0x1D] = make_IDT_entry (&_ISR_1D, KERNEL_INIT_SELECTOR);
	IDT [0x1E] = make_IDT_entry (&_ISR_1E, KERNEL_INIT_SELECTOR);
	IDT [0x1F] = make_IDT_entry (&_ISR_1F, KERNEL_INIT_SELECTOR);
	IDT [0x20] = make_IDT_entry (&_ISR_20, KERNEL_INIT_SELECTOR);
	IDT [0x21] = make_IDT_entry (&_ISR_21, KERNEL_INIT_SELECTOR);
	IDT [0x22] = make_IDT_entry (&_ISR_22, KERNEL_INIT_SELECTOR);
	IDT [0x23] = make_IDT_entry (&_ISR_23, KERNEL_INIT_SELECTOR);
	IDT [0x24] = make_IDT_entry (&_ISR_24, KERNEL_INIT_SELECTOR);
	IDT [0x25] = make_IDT_entry (&_ISR_25, KERNEL_INIT_SELECTOR);
	IDT [0x26] = make_IDT_entry (&_ISR_26, KERNEL_INIT_SELECTOR);
	IDT [0x27] = make_IDT_entry (&_ISR_27, KERNEL_INIT_SELECTOR);
	IDT [0x28] = make_IDT_entry (&_ISR_28, KERNEL_INIT_SELECTOR);
	IDT [0x29] = make_IDT_entry (&_ISR_29, KERNEL_INIT_SELECTOR);
	IDT [0x2A] = make_IDT_entry (&_ISR_2A, KERNEL_INIT_SELECTOR);
	IDT [0x2B] = make_IDT_entry (&_ISR_2B, KERNEL_INIT_SELECTOR);
	IDT [0x2C] = make_IDT_entry (&_ISR_2C, KERNEL_INIT_SELECTOR);
	IDT [0x2D] = make_IDT_entry (&_ISR_2D, KERNEL_INIT_SELECTOR);
	IDT [0x2E] = make_IDT_entry (&_ISR_2E, KERNEL_INIT_SELECTOR);
	IDT [0x2F] = make_IDT_entry (&_ISR_2F, KERNEL_INIT_SELECTOR);

	install_IDT (IDT, INT_LIMIT);
}
