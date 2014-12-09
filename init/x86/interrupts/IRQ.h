#ifndef IRQ_H
#define IRQ_H

#include <stdbool.h>

typedef enum {
	IRQ_PIT      = 0x00,
	IRQ_keyboard = 0x01,
	IRQ_cascade  = 0x02,
	IRQ_COM2     = 0x03,
	IRQ_COM1     = 0x04,
	IRQ_LPT2     = 0x05,
	IRQ_floppy   = 0x06,
	IRQ_LPT1     = 0x07,
	IRQ_spurious = 0x07,
	IRQ_CMOS_RTC = 0x08,
	IRQ_misc1    = 0x09,
	IRQ_misc2    = 0x0A,
	IRQ_misc3    = 0x0B,
	IRQ_mouse    = 0x0C,
	IRQ_FPU      = 0x0D,
	IRQ_HDD1     = 0x0E,
	IRQ_HDD2     = 0x0F
} IRQ;

void IRQ_EOI_master (void);
void IRQ_EOI_slave (void);
void IRQ_disable (IRQ irq);
void IRQ_enable (IRQ irq);
bool IRQ_requested (IRQ irq);
bool IRQ_in_service (IRQ irq);

#endif
