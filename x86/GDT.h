#ifndef GDT_H
#define GDT_H

#include <stdint.h>

enum {
	KERNEL_NULL_SELECTOR = 0,
	KERNEL_CODE_SELECTOR = 1,
	KERNEL_DATA_SELECTOR = 2
};

typedef struct __attribute__ ((packed)) __attribute__ ((aligned (8))) {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t  base_high1;
	uint8_t  accessed    : 1;
	uint8_t  rw_bit      : 1;
	uint8_t  dc_bit      : 1;
	uint8_t  ex_bit      : 1;
	uint8_t  one         : 1; // Unused
	uint8_t  privilege   : 2;
	uint8_t  present     : 1;
	uint8_t  limit_high  : 4;
	uint8_t  zero        : 1; // Unused
	uint8_t  mode64      : 1;
	uint8_t  mode32      : 1;
	uint8_t  granularity : 1;
	uint8_t  base_high2;
} GDT_entry;
_Static_assert (sizeof (GDT_entry) == 8, "GDT not packed");

void GDT_initialize (GDT_entry GDT [static 3]);

#endif
