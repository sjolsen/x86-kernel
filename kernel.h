#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

extern const void _ktext32_start;
extern const void _ktext32_end;
extern const void _ktext32_size;

extern const void _krodata_start;
extern const void _krodata_end;
extern const void _krodata_size;

extern const void _kdata_start;
extern const void _kdata_end;
extern const void _kdata_size;

extern const void _kbss_start;
extern const void _kbss_end;
extern const void _kbss_size;

extern const void _ktext_lma;
extern const void _ktext_start;
extern const void _ktext_end;
extern const void _ktext_size;

extern const void _kernel_start;
extern const void _kernel_end;
extern const void _kernel_size;

extern const void _isr_size;

#define _linkaddr(sym) ((uintptr_t)&sym)

#endif
