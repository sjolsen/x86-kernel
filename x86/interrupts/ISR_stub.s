.section .text

# In IA32e mode, the processor pre-aligns the stack frame and pointer to 16
# bytes. It then unconditionally pushes 40 bytes onto the stack. If there is an
# error code, it pushes that code, zero-extended to 8 bytes, onto the stack. We
# can (and must) handle error codes uniformly by taking advantage of the stacks
# alignment: first, we place a zeroed quadword below the stack pointer. Then, we
# zero out the bottom four bits of the stack frame. If there was an error code,
# it now occupies the lowest 8 bytes on the stack. Otherwise, those bytes are
# occupied by the zeroed quadword placed earlier. Either way, the 40 bytes
# pushed by the processor now begin at eight bytes above the stack pointer. We
# proceed to call ISR_entry (C calling convention), then remove the bottom eight
# bytes and perform the IRET.

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
        popq %rsi
	popq %rdi
        addq $8, %rsp
	iretq

	.macro .isr number name
	.global \name
	.type \name, @function
\name:
        # Adjust for error code
        movq $0, -8(%rsp)
        andq $0xFFFFFFFFFFFFFFF0, %rsp
        # Pass interrupt number and error code
        pushq %rdi
        pushq %rsi
        movabsq \number, %rdi
        movq 16(%rsp), %rsi
        jmp _ISR_entry
        # The final jmp mnemonic above is assembled into a different encoding
        # depending on the distance between the source and destination. _ISR_2F
        # at the farthest takes 0x23 bytes, so pad to that size (that way the
	# IDT initialization routine can be implemented as a loop).
        . = \name + 0x23
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
