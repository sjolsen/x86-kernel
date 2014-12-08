	.global install_GDT
	.type install_GDT, @function
install_GDT: # (const GDT_entry* base, uint16_t entries)
	push	%ebp
	mov	%esp, %ebp
	sub	$6, %esp
	movw	12(%ebp), %ax
	shlw	$3, %ax # Index->offset (sizeof(GDT_entry) == 8)
	sub     $1, %ax
	movw	%ax, (%esp)
	movl	8(%ebp), %eax
	movl	%eax, 2(%esp)
	lgdt	(%esp)
	leave
	ret
	.size install_GDT, . - install_GDT
