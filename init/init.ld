SECTIONS
{
	.multiboot BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
	}

	.text32 BLOCK(4K) : ALIGN(4K)
	{
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
