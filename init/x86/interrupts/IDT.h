#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__ ((packed)) __attribute__ ((aligned (8))) {
	uint16_t address_low;
	uint16_t selector;
	uint8_t  zero1; // Unused
	uint8_t  gate_type : 4;
	uint8_t  zero2     : 1; // Unused
	uint8_t  privilege : 2;
	uint8_t  present   : 1;
	uint16_t address_high;
} IDT_entry;
_Static_assert (sizeof (IDT_entry) == 8, "IDT not packed");

void IDT_initialize (void);

#endif
