ENTRY(_start)

SECTIONS
{
        _kinit_start = 0;
	. = 1M;
	.text32 :
	{
		KEEP(*(.multiboot))
		*(.multiboot)
		*(.text32)
	}

	.rodata :
	{
		*(.rodata*)

                kinit_size = .;
                LONG(_kinit_end - _kinit_start)

                ktext_base = .;
                LONG(_ktext_base)
                ktext_size = .;
                QUAD(_ktext_end - _ktext_start)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
        _kinit_end = .;

        _ktext_base = ALIGN(2M);
        .text 0xFFFF800000000000 : AT(_ktext_base)
        {
                _ktext_start = .;
                *(.text)
                _ktext_end = .;
        }

	/* Discard unneeded sections */
	/DISCARD/ :
	{
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
}
