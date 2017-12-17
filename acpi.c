#include "debug.h"
#include "stdint.h"
#include "acpi.h"
#include "malloc.h"

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

void parse_acpi() {
  memset(cpu_list, 0, 256);
  cpu_count = 0;

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
        parse_sdt((void*)rsdp_descriptor->rsdt_address);
      }
    }
  }
}

void parse_sdt(struct rsdt *rsdt) {
    uint32_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    for (uint32_t i = 0; i < entries; i++) {
        struct sdt_header *entry = (rsdt->entry_address[i]);
        if(strcmp(entry->signature, "APIC", 4)) {
          parse_madt((struct madt *)entry);
        }
    }
}

void parse_madt(struct madt *madt) {
  putstring("Local APIC address: ");
  puthex32(madt->local_controller_address);
  putchar('\n');
  uint32_t offset = 0;
  uint32_t madt_data_length = madt->header.length - sizeof(struct sdt_header) - 8;
  while(offset < madt_data_length) {
    struct madt_entry *entry = (void*)madt->entries + offset;
    if(entry->type == 0) {
      uint8_t *apic_id = (uint8_t*)(madt->entries + offset + 3);
      uint32_t *lapic_id = (void*)0xFEE00020;

      putstring("Processor detected. APIC ID: ");
      puthex8(*apic_id);
      if(*apic_id == *lapic_id>>24) {
        putstring(" (PRIMARY)");
      } else {
        cpu_list[cpu_count] = *apic_id;
        cpu_count++;
      }
      putchar('\n');
    }
    offset += entry->length;
  }
}
