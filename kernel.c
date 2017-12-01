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

  asm volatile("int $0x3");
  asm volatile("int $0x4");
  asm volatile("sti");

  uint32_t tmp;
  uint32_t bus;
  uint32_t device;

  struct nic *nic;
  nic = malloc(sizeof(struct nic) * 4);
  int n = 0;
  for(bus=0;bus<256;bus++) {
    for(device = 0; device < 32; device++) {
      tmp = pciConfigRead(bus,device,0,0);
      if(tmp == 0x15218086) {
        nic[n].base_address = pciConfigRead(bus,device,0,0x10) & 0xfffffff0;
        nic_reset(nic+n);
        putstring("NIC detected.\n");
        n++;
      }
    }
  }
  putstring("Done.\n");
  while(1) {
    nic_rx(nic+0, nic+2);
    nic_rx(nic+3, nic+1);
  }
}
