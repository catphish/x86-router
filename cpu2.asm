[bits 16]
global ap_asm_length
global ap_asm_start
extern gdt_data
extern ap_c_entry

ap_asm_start:
  jmp past_header

  align 8
  ap_asm_length: dd ap_asm_end - ap_asm_start
  dd 0

  dw 0
gdt_pointer:
  dw 23 ; limit (Size of GDT)
  dd gdt_data                  ; base of GDT

past_header:
  cli
  ; enable protected mode in the cr0 register
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax

  mov esp, stack_top

  lgdt [cs:gdt_pointer - ap_asm_start]
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  jmp 0x8:dword ap_c_entry
ap_asm_end:

[bits 32]

section .bss
align 4096
stack_bottom:
    resb 1024
stack_top:
