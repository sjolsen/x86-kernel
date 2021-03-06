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
	pushl %ebx
	pushl %eax
	call init
        popl %eax
        popl %ebx

        # Go to long mode
        ljmpl $(3 << 3), $_longmode_trampoline

_longmode_trampoline:
	.code64

        movl %ebx, %edi
        movl %eax, %esi
        movabsq $kernel_main, %rax
        jmp *%rax
