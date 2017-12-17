[bits 16]
global ap_asm_length
global ap_asm_start
global ap_stack_pointer_offset
extern gdt_data
extern ap_c_entry
extern page_directory

ap_asm_start:
  jmp past_header
  align 8
  ap_asm_length: dd ap_asm_end - ap_asm_start
  ap_stack_pointer_offset: dd after_stack_pointer - ap_asm_start - 4

  dw 0         ; Padding
gdt_pointer:
  dw 23        ; limit (Size of GDT)
  dd gdt_data  ; base of GDT

pm_entry_pointer:
  dd 0         ; Far jump location
  dw 0x8       ; Far jump segment
  dw 0         ; Padding
past_header:
  ; Disable interrupts
  cli
  ; Load the GDT
  lgdt [cs:gdt_pointer - ap_asm_start]
  ; Calculate location for far jump
  xor eax,eax
  mov ax,cs
  shl eax,4
  add eax, pm_entry - ap_asm_start
  ; Place calculated location into memory
  mov [cs:pm_entry_pointer - ap_asm_start], eax
  ; Enable protected mode in the cr0 register
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  ; Far jump to calculated location of pm_entry
  o32 jmp far [cs:pm_entry_pointer - ap_asm_start]
[bits 32]
pm_entry:
  ; Load segment registers
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

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

  ; Initialize stack.
  mov esp, 0
after_stack_pointer:

  ; Far jump again to C entry point
  jmp 0x8:ap_c_entry
ap_asm_end:
