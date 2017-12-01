#include "stdint.h"

#define RING_SIZE 32

struct rx_descriptor {
  void *address;
  uint32_t address_upper;
  volatile uint16_t length;
  uint16_t checksum;
  volatile uint8_t status;
  uint8_t errors;
  uint16_t vlan_tag;
};

struct tx_descriptor {
  void *address;
  uint32_t address_upper;
  volatile uint16_t length;
  uint8_t cso;
  volatile uint8_t cmd;
  volatile uint8_t status;
  uint8_t css;
  uint16_t vlan_tag;
};

struct nic {
  struct rx_descriptor rx_ring[RING_SIZE];
  struct tx_descriptor tx_ring[RING_SIZE];
  uint32_t base_address;
  uint32_t rx_ring_next;
  uint32_t tx_ring_next;
};

void nic_reset(struct nic *nic);
void nic_rx(struct nic *nic, struct nic *txnic);
