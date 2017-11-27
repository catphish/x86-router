#include "stdint.h"

struct rx_descriptor {
  uint32_t address_upper;
  void *address;
  uint16_t vlan_tag;
  uint8_t errors;
  uint8_t status;
  uint16_t checksum;
  uint16_t length;
};

struct nic {
  struct rx_descriptor rx_ring[1024];
  uint32_t base_address;
};

void nic_reset(struct nic *nic);
void nic_rx(struct nic *nic);
