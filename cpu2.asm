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

  lgdt [cs:gdt_pointer - ap_entry]
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

  ; enable paging and protected mode in the cr0 register
  or eax, 0x80000011
  mov cr0, eax
  ;hlt
  cli
  [bits 32]
  jmp 0x8:.ret
  .ret:
  hlt

  ;mov eax, cr0
  ;or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
  ;mov cr0, eax
  jmp 0x8:cpu2
far_jmp_end:
ap_end:
