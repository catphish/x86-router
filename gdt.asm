gdt_data:
    dq 0

.code:   ;cs should point to this descriptor
    dw 0xffff ;segment limit first 0-15 bits
    dw 0      ;base first 0-15 bits
    db 0      ;base 16-23 bits
    db 0x9a  ;access byte
    db 11001111b  ;high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
    db 0     ;base 24-31 bits

.data: ;ds ,ss ,es ,fs ,gs should point to this descriptor
    dw 0xffff ;segment limit first 0-15 bits
    dw 0      ;base first 0-15 bits
    db 0      ;base 16-23 bits
    db 0x92  ;access byte
    db 11001111b  ;high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
    db 0     ;base 24-31 bits

end_of_gdt:

gdt_pointer:
    dw end_of_gdt - gdt_data - 1 ; limit (Size of GDT)
    dd gdt_data                  ; base of GDT

global set_up_gdt
set_up_gdt:
    lgdt [gdt_pointer]
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x08:.ret
.ret:
    ret
