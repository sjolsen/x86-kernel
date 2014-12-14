#ifndef GDT_H
#define GDT_H

#include <stdint.h>

enum {
	KERNEL_NULL_SELECTOR = 0,
	KERNEL_INIT_SELECTOR = 1,
	KERNEL_DATA_SELECTOR = 2,
	KERNEL_TEXT_SELECTOR = 3,
};

void GDT_initialize (void);

#endif
