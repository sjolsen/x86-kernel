#include "IDT.h"
#include "8259.h"
#include "ISR_stub.h"
#include "x86/GDT.h"

static __attribute__((noinline))
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

	for (uint8_t i = 0x00; i < 0x14; ++i)
		(*idt) [i] = make_IDT_entry (_LOW_ISR(i));
	for (uint8_t i = 0x14; i < 0x20; ++i)
		(*idt) [i] = (IDT_entry) {.present = 0};
	for (uint8_t i = 0x20; i < 0x30; ++i)
		(*idt) [i] = make_IDT_entry (_HIGH_ISR(i - 0x20));

	install_IDT (idt);
}
