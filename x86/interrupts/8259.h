#ifndef _8259_H
#define _8259_H

#include "IRQ.h"
#include <stdint.h>

enum {
	// 8259A PIC I/O Ports
	PIC1_COMMAND = 0x20,
	PIC1_DATA    = 0x21,
	PIC2_COMMAND = 0xA0,
	PIC2_DATA    = 0xA1,

	// End-of-interrupt command
	EOI = 0x20,

	// 8259A Initialization Control Word 1
	ICW1           = 0x10, // Base for ICW1
	ICW1_NEED_ICW4 = 0x01, // Require ICW4
	ICW1_SINGLE    = 0x02, // Skip ICW3 (single PIC)
	ICW1_INTERVAL4 = 0x04, // 4-byte IVT packing (no effect on 8086)
	ICW1_LEVELTRIG = 0x08, // Level-triggered (default edge-triggered)

	// ICW2 specifies the 5 most-significant bits of the PIC's IVT. The low
	// three bits are ignored.

	// 8259A Initialization Control Word 3
	ICW3_MASTER = 0x04, // Slave on IRQ2 (bit 2)
	ICW3_SLAVE  = 0x02, // Slave on IRQ2 (binary 010)

	// 8259A Initialization Control Word 4
	ICW4_SFNM     = 0x10, // Special fully nested mode
	ICW4_BUFFERED = 0x08, // Buffered mode
	ICW4_MASTER   = 0x04, // Master (default slave, no effect outside of buffered mode)
	ICW4_AUTO_EOI = 0x02, // Automatic end-of-interrupt
	ICW4_8086     = 0x01, // 8086 mode (default MCS-80/85)

	// 8259A Operation Control Word 3
	OCW3      = 0x08, // Base for OCW3
	OCW3_READ = 0x04, // Read register
	OCW3_IRR  = 0x00, // Read IRR
	OCW3_ISR  = 0x01  // Read ISR
};

void remap_8259_PIC (uint8_t master_base, uint8_t slave_base);
uint8_t read_8259_register (IRQ irq, uint8_t OCW3_register);

#endif
