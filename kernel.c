#include "pci.h"
#include "nic.h"
#include "stdint.h"
#include "ports.h"
#include "debug.h"
#include "multiboot.h"
#include "malloc.h"
#include "interrupt.h"

void kernel_main(multiboot_info_t* mbd) {
  idt_install();
  (void)(mbd); // Suppress warning abut mbd, we'll use this later
  putstring("Hello world. I am a router. Moo.\n");

  volatile uint32_t *lapic_task_priority = (void*)0xFEE00080;
  volatile uint32_t *lapic_timer_interrupt_vector = (void*)0xFEE00320;
  volatile uint32_t *lapic_performance_counter_interrupt = (void*)0xFEE00340;
  volatile uint32_t *lapic_local_interrupt_0 = (void*)0xFEE00350;
  volatile uint32_t *lapic_local_interrupt_1 = (void*)0xFEE00360;
  volatile uint32_t *lapic_error_interrupt = (void*)0xFEE00370;
  volatile uint32_t *lapic_spurious_interrupt = (void*)0xFEE000F0;
  volatile uint32_t *lapic_timer_divide = (void*)0xFEE003E0;
  volatile uint32_t *lapic_timer_initial_count = (void*)0xFEE00380;
  puthex32(*lapic_task_priority);
  putchar('\n');
  puthex32(*lapic_timer_interrupt_vector);
  putchar('\n');
  puthex32(*lapic_performance_counter_interrupt);
  putchar('\n');
  puthex32(*lapic_local_interrupt_0);
  putchar('\n');
  puthex32(*lapic_local_interrupt_1);
  putchar('\n');
  puthex32(*lapic_error_interrupt);
  putchar('\n');
  puthex32(*lapic_spurious_interrupt);
  putchar('\n');
  puthex32(*lapic_timer_divide);
  putchar('\n');
  puthex32(*lapic_timer_initial_count);
  putchar('\n');
  *lapic_timer_interrupt_vector = 0x20020;
  *lapic_timer_divide = 0b1011;
  //asm volatile("int $0x3");
  //asm volatile("int $0x4");
  asm volatile("mov $27, %ecx");
  asm volatile("rdmsr");
  asm volatile("bts $11, %eax");
  asm volatile("wrmsr");
  *lapic_timer_initial_count = 62500000;
  asm volatile("sti");
  //asm volatile("hlt");
  //puthex8(1 / 0);

  // uint32_t tmp;
  // uint32_t bus;
  // uint32_t device;

  // struct nic *nic[32];
  // int n = 0;
  // for(bus=0;bus<256;bus++) {
  //   for(device = 0; device < 32; device++) {
  //     tmp = pciConfigRead(bus,device,0,0);
  //     if(tmp == 0x15218086) {
  //       nic[n] = malloc(sizeof(struct nic));
  //       nic[n]->base_address = pciConfigRead(bus,device,0,0x10) & 0xfffffff0;
  //       putstring("NIC detected.");
  //       puthex8(bus);
  //       putchar(' ');
  //       puthex8(device);
  //       putchar('\n');
  //       nic_reset(nic[n]);
  //       n++;
  //     }
  //   }
  // }
  // putstring("Done.\n");

  // while(1) {
  //   nic_forward(nic[0], nic[2]);
  //   nic_forward(nic[3], nic[3]);
  //   nic_forward(nic[2], nic[1]);
  // }
}
