ENTRY(start)

SECTIONS {
    . = 1M;

    .boot :
    {
        /* ensure that the multiboot header is at the beginning */
        boot.o(.multiboot_header)
        boot.o(.text)
    }

    . = ALIGN(4096);

    .cpu2 :
    {
     cpu2.o(.text)
    }

    .text :
    {
        *(.text)
    }

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
