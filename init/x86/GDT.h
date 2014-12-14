#ifndef GDT_H
#define GDT_H

#include <stdint.h>

enum {
	KERNEL_NULL_SELECTOR = 0,
	KERNEL_INIT_SELECTOR = 1,
	KERNEL_DATA_SELECTOR = 2,
	KERNEL_TEXT_SELECTOR = 3,
	TSS_SELECTOR = 4
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
_Static_assert (sizeof (GDT_entry) == 8, "GDT_entry not packed");

typedef struct __attribute__ ((packed)) __attribute__ ((aligned (8))) {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t  base_high1;
	uint8_t  type        : 4;
	uint8_t  zero0       : 1; // Must be zero
	uint8_t  privilege   : 2;
	uint8_t  present     : 1;
	uint8_t  limit_high  : 4;
	uint8_t              : 1;
	uint8_t  zero1       : 2; // Must be zero
	uint8_t  granularity : 1;
	uint8_t  base_high2;
	uint32_t base_high3;
	uint32_t reserved; // Must be zero
} TSS_descriptor;

typedef struct __attribute__ ((packed)) {
	GDT_entry GDTEs [4];
	TSS_descriptor TSSD;
} GDT;
_Static_assert (sizeof (GDT) == (4 * sizeof (GDT_entry) + sizeof (TSS_descriptor)), "GDT not packed");

typedef struct __attribute__ ((packed)) {
	uint8_t data [104];
} TSS_64;

void GDT_initialize (GDT* gdt, TSS_64* tss);

#endif
