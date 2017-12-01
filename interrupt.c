#include "stdint.h"
#include "malloc.h"
#include "debug.h"
#include "interrupt.h"

typedef struct registers
{
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

/* Defines an IDT entry */
struct idt_entry
{
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* This exists in 'start.asm', and is used to load our IDT */
extern void idt_load();

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    /* The interrupt routine's base address */
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

/* Installs the IDT */
void idt_install()
{
  /* Sets the special IDT pointer up, just like in 'gdt.c' */
  idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
  idtp.base = (uint32_t)&idt;

  /* Clear out the entire IDT, initializing it to zeros */
  memset(&idt, 0, sizeof(struct idt_entry) * 256);

  /* Add any new ISRs to the IDT here using idt_set_gate */
  idt_set_gate( 0, (uint32_t)isr0 , 0x08, 0x8E);
  idt_set_gate( 1, (uint32_t)isr1 , 0x08, 0x8E);
  idt_set_gate( 2, (uint32_t)isr2 , 0x08, 0x8E);
  idt_set_gate( 3, (uint32_t)isr3 , 0x08, 0x8E);
  idt_set_gate( 4, (uint32_t)isr4 , 0x08, 0x8E);
  idt_set_gate( 5, (uint32_t)isr5 , 0x08, 0x8E);
  idt_set_gate( 6, (uint32_t)isr6 , 0x08, 0x8E);
  idt_set_gate( 7, (uint32_t)isr7 , 0x08, 0x8E);
  idt_set_gate( 8, (uint32_t)isr8 , 0x08, 0x8E);
  idt_set_gate( 9, (uint32_t)isr9 , 0x08, 0x8E);
  idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
  idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
  idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
  idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
  idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
  idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
  idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
  idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
  idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);

  /* Points the processor's internal register to the new IDT */
  idt_load();
}

void isr_handler(registers_t regs)
{
   putstring("recieved interrupt: ");
   puthex32(regs.int_no);
   putchar(' ');
   puthex32(regs.err_code);
   putchar('\n');
}
