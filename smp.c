#include "stdint.h"
#include "debug.h"
#include "malloc.h"
#include "nic.h"

void ap_c_entry() {
  volatile uint32_t *lapic_id = (void*)0xFEE00020;
  putstring("Hello from additional CPU ");
  puthex8(*lapic_id>>24);
  putchar('\n');
  while(1)
    nic_forward(nic[0], nic[1]);
  while(1) asm("hlt");
}

void install_ap_entry() {
  extern char ap_asm_start[];
  extern uint32_t ap_asm_length;
  putstring("Copying AP boot code.\n");
  memcpy((char*)0x80000, ap_asm_start, ap_asm_length);
}

void start_aps() {
  volatile uint32_t *lapic_timer_interrupt_vector = (void*)0xFEE00320;
  volatile uint32_t *lapic_timer_divide = (void*)0xFEE003E0;
  volatile uint32_t *lapic_timer_initial_count = (void*)0xFEE00380;
  *lapic_timer_interrupt_vector = 0x20020;
  *lapic_timer_divide = 0b1011;
  asm volatile(
    "mov $27, %%ecx\n\t"
    "rdmsr\n\t"
    "bts $11, %%eax\n\t"
    "wrmsr\n\t" : : : "eax", "ebx", "ecx", "edx"
  );
  *lapic_timer_initial_count = 1000000000;
  asm volatile("sti");

  // Set up pointers to IPI registers in the LAPIC
  volatile uint32_t *lapic_offset_300 = (void*)0xFEE00300;
  volatile uint32_t *lapic_offset_310 = (void*)0xFEE00310;

  // We're not addressing a specific CPU
  *lapic_offset_310 = 0x00000000;

  // Send the INIT (reset) to all AP
  putstring("Sending INIT\n");
  *lapic_offset_300 = 0x000C4500;

  // Sleep a little
  int i = 0x1fffffff; while(--i) asm("");

  // Send the SIPI to commence execution
  // Send to all AP, begin execution at page 8 (0x8000)
  putstring("Sending SIPI\n");
  *lapic_offset_310 = 0x00000000;
  *lapic_offset_300 = 0x000C4600+0x80;
}
