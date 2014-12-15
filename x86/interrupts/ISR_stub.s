.section .text

# Declare an interrupt handler delegating to C
_ISR_entry:
        pushq %rax
        pushq %rcx
        pushq %rdx
        pushq %r8
        pushq %r9
        pushq %r10
        pushq %r11
	call ISR_entry
        popq %r11
        popq %r10
        popq %r9
        popq %r8
        popq %rdx
        popq %rcx
        popq %rax
	popq %rdi
	iretq

	.macro .isr number name
	.global \name
	.type \name, @function
\name:
        pushq %rdi
        movabsq \number, %rdi
        jmp _ISR_entry
	.endm

# Wrapped ISRs
	.isr $0x00 _ISR_00
	.isr $0x01 _ISR_01
	.isr $0x02 _ISR_02
	.isr $0x03 _ISR_03
	.isr $0x04 _ISR_04
	.isr $0x05 _ISR_05
	.isr $0x06 _ISR_06
	.isr $0x07 _ISR_07
	.isr $0x08 _ISR_08
	.isr $0x09 _ISR_09
	.isr $0x0A _ISR_0A
	.isr $0x0B _ISR_0B
	.isr $0x0C _ISR_0C
	.isr $0x0D _ISR_0D
	.isr $0x0E _ISR_0E
	.isr $0x0F _ISR_0F
	.isr $0x10 _ISR_10
	.isr $0x11 _ISR_11
	.isr $0x12 _ISR_12
	.isr $0x13 _ISR_13

	.isr $0x20 _ISR_20
	.isr $0x21 _ISR_21
	.isr $0x22 _ISR_22
	.isr $0x23 _ISR_23
	.isr $0x24 _ISR_24
	.isr $0x25 _ISR_25
	.isr $0x26 _ISR_26
	.isr $0x27 _ISR_27
	.isr $0x28 _ISR_28
	.isr $0x29 _ISR_29
	.isr $0x2A _ISR_2A
	.isr $0x2B _ISR_2B
	.isr $0x2C _ISR_2C
	.isr $0x2D _ISR_2D
	.isr $0x2E _ISR_2E
	.isr $0x2F _ISR_2F
