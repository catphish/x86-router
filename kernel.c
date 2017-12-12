#include "nic.h"
#include "stdint.h"
#include "ports.h"
#include "debug.h"
#include "multiboot.h"
#include "malloc.h"
#include "interrupt.h"
#include "smp.h"

void kernel_main(multiboot_info_t* mbd) {
  idt_install();
  install_ap_entry();
  (void)(mbd); // Suppress warning abut mbd, we'll use this later
  putstring("Hello world. I am a router. Moo.\n");

  detect_nics(nic);
  start_aps();
  // while(1) {
  //   nic_forward(nic[0], nic[2]);
  //   nic_forward(nic[3], nic[3]);
  //   nic_forward(nic[2], nic[1]);
  // }
}
