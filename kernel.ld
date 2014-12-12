/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;

	.text32 BLOCK(4K) : ALIGN(4K)
	{
		KEEP(*(.multiboot))
		*(.multiboot)
		*(.text32)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata*)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

        _ktext_base = .;
        .text 0x40000000 : AT(_ktext_base)
        {
                _ktext_start = .;
                *(.text)
                /* _ktext_end = .; */
        }

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */

	/* Discard unneeded sections */
	/DISCARD/ :
	{
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
}
