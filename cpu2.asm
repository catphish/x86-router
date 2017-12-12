[bits 16]
global ap_length
global ap_entry
global far_jmp_offset
extern gdt_data
extern end_of_gdt
extern page_directory
extern cpu2

ap_entry:
  jmp past_header

  align 8
  ap_length: dd ap_end - ap_entry
  far_jmp_offset: dd far_jmp_end - ap_entry - 6
  dw 0
gdt_pointer:
  dw 23 ; limit (Size of GDT)
  dd gdt_data                  ; base of GDT
  align 8

past_header:
  cli
  mov esp, stack_top

  lgdt [cs:gdt_pointer - ap_entry]
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; enable protected mode in the cr0 register
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  jmp 0x8:dword cpu2
far_jmp_end:
ap_end:

[bits 32]

section .bss
align 4096
stack_bottom:
    resb 1024
stack_top:
