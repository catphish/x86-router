uint8_t cpu_list[256];
uint8_t cpu_count;

struct rsdp_descriptor {
 char signature[8];
 uint8_t checksum;
 char oemid[6];
 uint8_t revision;
 uint32_t rsdt_address;
} __attribute__ ((packed));

struct sdt_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__ ((packed));

struct rsdt {
  struct sdt_header header;
  struct sdt_header *entry_address[];
}__attribute__ ((packed));

struct madt {
  struct sdt_header header;
  uint32_t local_controller_address;
  uint32_t flags;
  char entries[];
}__attribute__ ((packed));

struct madt_entry {
  uint8_t type;
  uint8_t length;
  char data[];
}__attribute__ ((packed));

void parse_acpi();
void parse_sdt(struct rsdt *rsdt);
void parse_madt(struct madt *madt);
