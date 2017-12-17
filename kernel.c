#include "nic.h"
#include "stdint.h"
#include "ports.h"
#include "debug.h"
#include "multiboot.h"
#include "malloc.h"
#include "interrupt.h"
#include "smp.h"
#include "acpi.h"

void kernel_main(multiboot_info_t* mbd) {
  (void)(mbd); // Suppress warning abut mbd, we'll use this later
  putstring("Hello world. I am a router. Moo.\n");
  idt_install();
  parse_acpi();
  detect_nics();
  configure_lapic();
  start_ap(1);
  start_ap(2);
  start_ap(3);
  arm_timer();
  while(1) asm("hlt");
}
