#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef enum {
	// Exceptions
	INT_divide_by_zero          = 0x00,
	INT_debugger                = 0x01,
	INT_NMI                     = 0x02,
	INT_breakpoint              = 0x03,
	INT_overflow                = 0x04,
	INT_bounds                  = 0x05,
	INT_invalid_opcode          = 0x06,
	INT_coprocessor_unavailable = 0x07,
	INT_double_fault            = 0x08,
	INT_invalid_TSS             = 0x0A,
	INT_segment_missing         = 0x0B,
	INT_stack_fault             = 0x0C,
	INT_protection_fault        = 0x0D,
	INT_page_fault              = 0x0E,
	INT_math_fault              = 0x10,
	INT_alignment_check         = 0x11,
	INT_machine_check           = 0x12,
	INT_SIMD_exception          = 0x13,

	// IRQs
	INT_PIT      = 0x20,
	INT_keyboard = 0x21,
	INT_cascade  = 0x22,
	INT_COM2     = 0x23,
	INT_COM1     = 0x24,
	INT_LPT2     = 0x25,
	INT_floppy   = 0x26,
	INT_LPT1     = 0x27,
	INT_spurious = 0x27,
	INT_CMOS_RTC = 0x28,
	INT_misc1    = 0x29,
	INT_misc2    = 0x2A,
	INT_misc3    = 0x2B,
	INT_mouse    = 0x2C,
	INT_FPU      = 0x2D,
	INT_HDD1     = 0x2E,
	INT_HDD2     = 0x2F,

	// Limits
	INT_IRQ_MBASE = 0x20,
	INT_IRQ_SBASE = 0x28,
	INT_LIMIT     = 0x30
} INT_index;

typedef void (*ISR_t) (INT_index);
extern ISR_t ISR_table [INT_LIMIT];

void null_ISR (INT_index interrupt);

void ISR_table_initialize (ISR_t default_ISR);

#endif
