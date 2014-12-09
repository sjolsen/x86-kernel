#include "8259.h"
#include "x86/portio.h"


// Extern functions

void remap_8259_PIC (uint8_t master_base, uint8_t slave_base)
{
	uint8_t master_mask = inb (PIC1_DATA);
	uint8_t slave_mask  = inb (PIC2_DATA);

	outb (PIC1_COMMAND, ICW1 | ICW1_NEED_ICW4);
	outb (PIC1_DATA, master_base);
	outb (PIC1_DATA, ICW3_MASTER);
	outb (PIC1_DATA, ICW4_8086);

	outb (PIC2_COMMAND, ICW1 | ICW1_NEED_ICW4);
	outb (PIC2_DATA, slave_base);
	outb (PIC2_DATA, ICW3_SLAVE);
	outb (PIC2_DATA, ICW4_8086);

	outb (PIC1_DATA, master_mask);
	outb (PIC1_DATA, slave_mask);
}

uint8_t read_8259_register (IRQ irq, uint8_t OCW3_register)
{
	uint16_t port = (irq < 8) ? PIC1_COMMAND : PIC2_COMMAND;
	uint8_t index = irq % 8;
	outb (port, OCW3 | OCW3_READ | OCW3_register);
	return inb (port) & (1 << index);
}
