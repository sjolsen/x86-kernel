#include "IDT.h"
#include "8259.h"
#include "ISR_stub.h"
#include "x86/GDT.h"

static
IDT_entry make_IDT_entry (void (*address) (void))
{
	enum {
		T_LDT       = 0b0010,
		T_TSS_AVAIL = 0b1001,
		T_TSS_BUSY  = 0b1011,
		T_CALL_GATE = 0b1100,
		T_INTR_GATE = 0b1110,
		T_TRAP_GATE = 0b1111
	};

	return (IDT_entry) {
		.address_low   = (uintptr_t) address & 0xFFFF,
			.selector      = KERNEL_TEXT_SELECTOR << 3,
			.zero0         = 0,
			.type          = T_INTR_GATE,
			.zero1         = 0,
			.privilege     = 0,
			.present       = 1,
			.address_high0 = ((uintptr_t) address >> 16) & 0xFFFF,
			.address_high1 = ((uintptr_t) address >> 32) & 0xFFFFFFFF
			};
}

static
void install_IDT (IDT* idt)
{
	struct __attribute__ ((packed)) {
		uint16_t limit;
		uint64_t offset;
	} lidt_args = {
		.limit  = sizeof (IDT) - 1,
		.offset = (uint64_t) &((*idt) [0])
	};
	__asm__ volatile ("lidt %0" :: "m" (lidt_args));
}


// Extern functions

void IDT_initialize (IDT* idt)
{
	remap_8259_PIC (INT_IRQ_MBASE, INT_IRQ_SBASE);

	(*idt) [0x00] = make_IDT_entry (&_ISR_00);
	(*idt) [0x01] = make_IDT_entry (&_ISR_01);
	(*idt) [0x02] = make_IDT_entry (&_ISR_02);
	(*idt) [0x03] = make_IDT_entry (&_ISR_03);
	(*idt) [0x04] = make_IDT_entry (&_ISR_04);
	(*idt) [0x05] = make_IDT_entry (&_ISR_05);
	(*idt) [0x06] = make_IDT_entry (&_ISR_06);
	(*idt) [0x07] = make_IDT_entry (&_ISR_07);
	(*idt) [0x08] = make_IDT_entry (&_ISR_08);
	(*idt) [0x09] = make_IDT_entry (&_ISR_09);
	(*idt) [0x0A] = make_IDT_entry (&_ISR_0A);
	(*idt) [0x0B] = make_IDT_entry (&_ISR_0B);
	(*idt) [0x0C] = make_IDT_entry (&_ISR_0C);
	(*idt) [0x0D] = make_IDT_entry (&_ISR_0D);
	(*idt) [0x0E] = make_IDT_entry (&_ISR_0E);
	(*idt) [0x0F] = make_IDT_entry (&_ISR_0F);
	(*idt) [0x10] = make_IDT_entry (&_ISR_10);
	(*idt) [0x11] = make_IDT_entry (&_ISR_11);
	(*idt) [0x12] = make_IDT_entry (&_ISR_12);
	(*idt) [0x13] = make_IDT_entry (&_ISR_13);
	for (uint8_t i = 0x14; i <= 0x1F; ++i)
		(*idt) [i] = (IDT_entry) {.present = 0};
	(*idt) [0x20] = make_IDT_entry (&_ISR_20);
	(*idt) [0x21] = make_IDT_entry (&_ISR_21);
	(*idt) [0x22] = make_IDT_entry (&_ISR_22);
	(*idt) [0x23] = make_IDT_entry (&_ISR_23);
	(*idt) [0x24] = make_IDT_entry (&_ISR_24);
	(*idt) [0x25] = make_IDT_entry (&_ISR_25);
	(*idt) [0x26] = make_IDT_entry (&_ISR_26);
	(*idt) [0x27] = make_IDT_entry (&_ISR_27);
	(*idt) [0x28] = make_IDT_entry (&_ISR_28);
	(*idt) [0x29] = make_IDT_entry (&_ISR_29);
	(*idt) [0x2A] = make_IDT_entry (&_ISR_2A);
	(*idt) [0x2B] = make_IDT_entry (&_ISR_2B);
	(*idt) [0x2C] = make_IDT_entry (&_ISR_2C);
	(*idt) [0x2D] = make_IDT_entry (&_ISR_2D);
	(*idt) [0x2E] = make_IDT_entry (&_ISR_2E);
	(*idt) [0x2F] = make_IDT_entry (&_ISR_2F);

	install_IDT (idt);
}
