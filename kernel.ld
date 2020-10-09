ENTRY(_start)

SECTIONS
{
        . = 1M;
        _kernel_start = .;

        .rodata : ALIGN(4k)
        {
                _krodata_start = .;

                KEEP(*(.multiboot))
                *(.multiboot)
                *(.rodata*)

                _krodata_end = .;
        }

        .text32 : ALIGN(4k)
        {
                _ktext32_start = .;
                *(.text32)
                _ktext32_end = .;
        }

        .data : ALIGN(4k)
        {
                _kdata_start = .;
                *(.data)
                _kdata_end = .;
        }

        .bss : ALIGN(4k)
        {
                _kbss_start = .;

                *(COMMON)
                *(.bss)
                *(.bootstrap_stack)

                _kbss_end = .;
        }

        _ktext_lma = ALIGN(2M);
        .text 0xFFFF800000000000 : AT(_ktext_lma)
        {
                _ktext_start = .;
                *(.text)
                _ktext_end = .;
        }

        . = _ktext_lma + SIZEOF(.text);
        _kernel_end = .;

        /* Discard unneeded sections */
        /DISCARD/ :
        {
                *(.eh_frame)
                *(.note)
                *(.comment)
        }
}
