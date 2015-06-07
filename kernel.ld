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

                ktext32_base = .;
                LONG(_ktext32_start)
                ktext32_size = .;
                LONG(_ktext32_end - _ktext32_start)

                krodata_base = .;
                LONG(_krodata_start)
                krodata_size = .;
                LONG(_krodata_end - _krodata_start)

                kdata_base = .;
                LONG(_kdata_start)
                kdata_size = .;
                LONG(_kdata_end - _kdata_start)

                kbss_base = .;
                LONG(_kbss_start)
                kbss_size = .;
                LONG(_kbss_end - _kbss_start)

                ktext_base = .;
                LONG(_ktext_base)
                ktext_size = .;
                LONG(_ktext_end - _ktext_start)

                kernel_base = .;
                LONG(_kernel_start)
                kernel_size = .;
                LONG(_kernel_end - _kernel_start)

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

        _ktext_base = ALIGN(2M);
        .text 0xFFFF800000000000 : AT(_ktext_base)
        {
                _ktext_start = .;
                *(.text)
                _ktext_end = .;
        }

        . = _ktext_base + SIZEOF(.text);
        _kernel_end = .;

        /* Discard unneeded sections */
        /DISCARD/ :
        {
                *(.eh_frame)
                *(.note)
                *(.comment)
        }
}
