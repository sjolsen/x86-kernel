#include "IRQ.h"
#include "8259.h"
#include "x86/portio.h"


// Extern functions

void IRQ_EOI_master (void)
{
	outb (PIC1_COMMAND, EOI);
}

void IRQ_EOI_slave (void)
{
	outb (PIC1_COMMAND, EOI);
	outb (PIC2_COMMAND, EOI);
}

void IRQ_disable (IRQ irq)
{
	uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
	uint8_t index = irq % 8;
	outb (port, inb (port) | (1 << index));
}

void IRQ_enable (IRQ irq)
{
	uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
	uint8_t index = irq % 8;
	outb (port, inb (port) & ~(1 << index));
}

bool IRQ_requested (IRQ irq)
{
	return read_8259_register (irq, OCW3_IRR);
}

bool IRQ_in_service (IRQ irq)
{
	return read_8259_register (irq, OCW3_ISR);
}
