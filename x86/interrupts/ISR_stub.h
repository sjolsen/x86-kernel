#ifndef ISR_STUB_H
#define ISR_STUB_H

#include "kernel.h"

extern void _ISR_00 (void);
extern void _ISR_01 (void);
extern void _ISR_02 (void);
extern void _ISR_03 (void);
extern void _ISR_04 (void);
extern void _ISR_05 (void);
extern void _ISR_06 (void);
extern void _ISR_07 (void);
extern void _ISR_08 (void);
extern void _ISR_09 (void);
extern void _ISR_0A (void);
extern void _ISR_0B (void);
extern void _ISR_0C (void);
extern void _ISR_0D (void);
extern void _ISR_0E (void);
extern void _ISR_0F (void);
extern void _ISR_10 (void);
extern void _ISR_11 (void);
extern void _ISR_12 (void);
extern void _ISR_13 (void);

extern void _ISR_20 (void);
extern void _ISR_21 (void);
extern void _ISR_22 (void);
extern void _ISR_23 (void);
extern void _ISR_24 (void);
extern void _ISR_25 (void);
extern void _ISR_26 (void);
extern void _ISR_27 (void);
extern void _ISR_28 (void);
extern void _ISR_29 (void);
extern void _ISR_2A (void);
extern void _ISR_2B (void);
extern void _ISR_2C (void);
extern void _ISR_2D (void);
extern void _ISR_2E (void);
extern void _ISR_2F (void);

#define _LOW_ISR(i) ((void (*)(void))((const char*)&_ISR_00 + (i)*_linkaddr(_isr_size)))
#define _HIGH_ISR(i) ((void (*)(void))((const char*)&_ISR_20 + (i)*_linkaddr(_isr_size)))

#endif
