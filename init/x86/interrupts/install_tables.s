	.global install_IDT
	.type install_IDT, @function
install_IDT: # (const IDT_entry* base, uint16_t entries)
	push	%ebp
	mov	%esp, %ebp
	sub	$6, %esp
	movw	12(%ebp), %ax
	shlw	$3, %ax # Index->offset (sizeof(IDT_entry) == 8)
	movw	%ax, (%esp)
	movl	8(%ebp), %eax
	movl	%eax, 2(%esp)
	lidt	(%esp)
	leave
	ret
	.size install_IDT, . - install_IDT
