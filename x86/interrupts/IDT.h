#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "ISR.h"

typedef struct __attribute__ ((packed)) __attribute__ ((aligned (8))) {
	uint16_t address_low;
	uint16_t selector;
	uint8_t  stack_table : 3;
	uint8_t  zero0       : 5; // Must be 0
	uint8_t  type        : 4;
	uint8_t  zero1       : 1; // Must be 0
	uint8_t  privilege   : 2;
	uint8_t  present     : 1;
	uint16_t address_high0;
	uint32_t address_high1;
	uint32_t reserved; // Must be 0
} IDT_entry;
_Static_assert (sizeof (IDT_entry) == 16, "IDT not packed");

typedef IDT_entry IDT [INT_LIMIT];

void IDT_initialize (IDT* idt);

#endif
