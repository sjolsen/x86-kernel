#ifndef KERNEL_H
#define KERNEL_H

#include "multiboot.h"

extern const void _kimage_start;
extern const void _stack_bottom;
extern const void _stack_top;

void kernel_main (multiboot_info_t* info, multiboot_uint32_t magic);

#endif
