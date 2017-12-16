; This file contains the functions necessary to populate the
; IDT as well as the interrupt routines themselves.

global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    ;cli                         ; Disable interrupts firstly.
    push byte 0                 ; Push a dummy error code.
    push byte %1                ; Push the interrupt number.
    jmp isr_common_stub         ; Go to our common handler code.
%endmacro

; This macro creates a stub for an ISR which passes it's own
; error code.
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    ;cli                         ; Disable interrupts.
    push byte %1                ; Push the interrupt number
    jmp isr_common_stub
%endmacro

ISR_NOERRCODE 32
ISR_NOERRCODE 33

; In isr.c
extern isr_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    call isr_handler
    xor eax, eax
    mov [0xFEE000B0], eax
    popa                     ; Pops edi,esi,ebp...
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    ;sti
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
