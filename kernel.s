# Set up space for the stack.
.section .bootstrap_stack, "aw", @nobits
        .global _stack_bottom
        .global _stack_top
_stack_bottom:
	.skip 65536 # 16 KiB
_stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded.
.section .text32
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
        .code64
        movl %eax, %edi
        movl %ebx, %esi
	movw $(3 << 3), %ax
        pushw %ax
	movabsq $kernel_main, %rax
        pushq %rax
	lretq

	# Hang indefinitely.
	cli
.Lhang:
	hlt
	jmp .Lhang
