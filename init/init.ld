SECTIONS
{
	.multiboot :
	{
		*(.multiboot)
	}

	.text32 :
	{
		*(.text*)
	}

	.rodata :
	{
		*(.rodata*)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}
