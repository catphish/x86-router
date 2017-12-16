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
  idt_install();
  install_ap_entry();
  putstring("Hello world. I am a router. Moo.\n");
  find_rsdp();
  start_aps();
  detect_nics(nic);
  // while(1) {
  //   nic_forward(nic[0], nic[2]);
  //   nic_forward(nic[3], nic[3]);
  //   nic_forward(nic[2], nic[1]);
  // }
}
