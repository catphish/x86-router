#include "debug.h"
#include "stdint.h"
#include "acpi.h"

inline static uint32_t read_register(uint32_t address) {
  volatile uint32_t * r;
  r = (uint32_t *) (address);
  return(*r);
}

uint32_t strcmp(char* s1, char* s2, uint32_t length) {
  while(length--) {
    if(*s1 != *s2)
      return(0);
    s1++;
    s2++;
  }
  return(1);
}

uint32_t find_rsdp() {
  for(uint32_t address = 0x00000000; address < 0x00100000; address += 16)
  {
    if(strcmp((char*)address, "RSD PTR ", 8)) {
      struct rsdp_descriptor *rsdp_descriptor = (void*)address;
      putstring("Found Potential RSDP: ");
      puthex32(address);
      uint8_t checksum = 0;
      char *byte = (char*)address;
      for(int n=0; n<20; n++) {
        checksum += *byte;
        byte++;
      }
      if(checksum) {
        putstring(" (invalid)\n");
      } else {
        putstring(" (valid) ");
        puthex32(rsdp_descriptor->rsdt_address);
        putchar('\n');
        browse_sdt((void*)rsdp_descriptor->rsdt_address);
        return rsdp_descriptor->rsdt_address;
      }
    }
  }
  return 0;
}

void browse_sdt(struct rsdt *rsdt) {
    uint32_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    for (uint32_t i = 0; i < entries; i++) {
        struct sdt_header *entry = (rsdt->entry_address[i]);
        if(strcmp(entry->signature, "APIC", 4)) {
          browse_madt((struct madt *)entry);
        }
    }
}

void browse_madt(struct madt *madt) {
  putstring("Local APIC address: ");
  puthex32(madt->local_controller_address);
  putchar('\n');
  uint32_t offset = 0;
  uint32_t madt_data_length = madt->header.length - sizeof(struct sdt_header) - 8;
  volatile uint32_t *io_api_address;
  //volatile uint32_t *ioregsel;
  //volatile uint32_t *iowin;
  while(offset < madt_data_length) {
    struct madt_entry *entry = (void*)madt->entries + offset;
    switch(entry->type) {
      case 0 :
        putstring("Processor detected. APIC ID: ");
        puthex8(*((uint8_t*)(madt->entries + offset + 3)));
        putchar('\n');
        break;
      case 1 :
        putstring("IO APIC detected: ");
        io_api_address = (uint32_t*)(madt->entries + offset + 4);
        //ioregsel = (uint32_t*) (*io_api_address);
        //iowin    = (uint32_t*) (*io_api_address + 0x10);
        puthex32(*io_api_address);
        putchar('\n');
        //putstring("Scanning IO APIC:\n");
        //for(int n = 0x10; n < 0x10+24*2; n+=2) {
        //  *ioregsel = n+1;
        //  puthex32(*iowin);
        //  putchar('.');
        //  *ioregsel = n;
        //  puthex32(*iowin);
        //  putchar('\n');
        //}
        //putchar('\n');
        break;
    }
    offset += entry->length;
  }
}
