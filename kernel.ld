PHDRS
{
        lowmem PT_LOAD;
        highmem PT_LOAD;
        /* If LD is permitted to pick its own PHDRS, the 4 kiB alignment of the
         * page table structures in .bss will also be applied to .rodata, .text,
         * etc., causing unnecessary padding to be inserted into the ELF file.
         */
        bss PT_NULL;
}

ENTRY(_start)

SECTIONS
{
        . = 1M;
        _kernel_start = .;

        .rodata :
        {
                _krodata_start = .;

                KEEP(*(.multiboot))
                *(.multiboot)
                *(.rodata*)

                _krodata_end = .;
        } :lowmem

        .text32 :
        {
                _ktext32_start = .;
                *(.text32)
                _ktext32_end = .;
        } :lowmem

        .data :
        {
                _kdata_start = .;
                *(.data)
                _kdata_end = .;
        } :lowmem

        .bss :
        {
                _kbss_start = .;

                *(COMMON)
                *(.bss)
                *(.bootstrap_stack)

                _kbss_end = .;
        } :bss

        _ktext_lma = ALIGN(2M);
        .text 0xFFFF800000000000 : AT(_ktext_lma)
        {
                _ktext_start = .;
                *(.text)
                _ktext_end = .;
        } :highmem

        . = _ktext_lma + SIZEOF(.text);
        _kernel_end = .;
}
