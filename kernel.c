#include "pci.h"
#include "nic.h"
#include "stdint.h"
#include "ports.h"
#include "debug.h"
#include "multiboot.h"
#include "malloc.h"

void kernel_main(multiboot_info_t* mbd) {
  putstring("Hello world. I am a router. Moo.\n");

  uint32_t tmp;
  uint32_t bus;
  uint32_t device;

  struct nic *nic = malloc(sizeof(struct nic));
  for(bus=0;bus<256;bus++) {
    for(device = 0; device < 32; device++) {
      tmp = pciConfigRead(bus,device,0,0);
      if(tmp == 0x15218086) {
        nic->base_address = pciConfigRead(bus,device,0,0x10) & 0xfffffff0;
        nic_reset(nic);
      }
    }
  }
  putstring("Done.\n");
  while(1) {
    nic_rx(nic);
  }
}
