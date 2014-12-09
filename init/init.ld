SECTIONS
{
	.inittext BLOCK(4K) : ALIGN(4K)
	{
		KEEP(*(.multiboot))
		*(.multiboot)
		*(.text*)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata*)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
