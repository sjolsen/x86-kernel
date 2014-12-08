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
	.global _start
	.type _start, @function
_start:
	# Initialize the stack.
	movl $_stack_top, %esp

	# Call the C entry point with the arguments supplied by the bootloader.
	pushl %eax
	pushl %ebx
	call kernel_main

	# Hang indefinitely.
	cli
	.Lhang:
	hlt
	jmp .Lhang
	.size _start, . - _start
