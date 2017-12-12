ENTRY(start)

SECTIONS {
    /* All protected mode code sits above 1MB */
    . = 1M;

    .boot :
    {
        /* ensure that the multiboot header is at the beginning of the kernel */
        boot.o(.multiboot_header)
        boot.o(.text)
    }

    .text :
    {
        *(.text)
    }

    . = ALIGN(4096);
    .eh_frame :
    {
      *(.eh_frame)
    }

    .rodata :
    {
      *(.rodata) *(.rodata.*)
    }

    . = ALIGN(4096);
    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }

    . = ALIGN(4096);
    _end = .;
}
