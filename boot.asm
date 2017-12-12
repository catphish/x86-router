MBALIGN  equ  1<<0              ; align loaded modules on page boundaries
MEMINFO  equ  1<<1              ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot

section .multiboot_header
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 4096
global page_directory
page_directory:
    resb 4096
stack_bottom:
    resb 1024
stack_top:

section .text
bits 32
global start
start:
    mov esp, stack_top

    call disable_pic
    call set_up_page_tables
    call enable_paging
    extern set_up_gdt
    call set_up_gdt

    push ebx
    ;sti
    extern kernel_main
    call kernel_main

    ;cli
forever:
    hlt
    jmp forever

set_up_page_tables:
    mov ecx, 0         ; counter variable

.map_page_directory:
    ; map page directory entry directly to 4MB of RAM
    mov eax, 0x400000
    mul ecx
    or eax, 0b10000011 ; present + writable + large
    mov [page_directory + ecx * 4], eax

    inc ecx            ; increase counter
    cmp ecx, 1024      ; if counter == 1024, the whole table is mapped
    jne .map_page_directory  ; else map the next entry

    ret

enable_paging:
    ; load page_directory to cr3 register
    mov eax, page_directory
    mov cr3, eax

    ; enable PSE for 4MB pages
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

disable_pic:
    mov al, 0xff
    out 0xa1, al
    out 0x21, al
    ret
