# Set up space for the stack.
.section .bootstrap_stack, "aw", @nobits
        .global _stack_bottom
        .global _stack_top
_stack_bottom:
	.skip 65536 # 16 KiB
_stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded.
.section .text
        .code32
	.global _start
_start:
	# Initialize the stack.
	movl $_stack_top, %esp
	movl $_stack_top, %ebp

	# Save the arguments supplied by the bootloader and set up long mode.
	pushl %eax
	pushl %ebx
	call init
        popl %ebx
        popl %eax

        # Go to long mode
        ljmp _longmode_start

        .code64
        .global _longmode_start
_longmode_start:
        # Do longmode stuff.
        movl %eax, %edi
        movl %ebx, %esi
	call kernel_main

	# Hang indefinitely.
	cli
.Lhang:
	hlt
	jmp .Lhang
	.size _start, . - _start
