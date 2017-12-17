#include "stdint.h"

#define RING_SIZE 2048

struct rx_descriptor {
  volatile uint32_t address;
  volatile uint32_t address_upper;
  volatile uint16_t length;
  volatile uint16_t checksum;
  volatile uint8_t status;
  volatile uint8_t errors;
  volatile uint16_t vlan_tag;
}__attribute__((packed));

struct tx_descriptor {
  volatile uint32_t address;
  volatile uint32_t address_upper;
  volatile uint16_t length;
  volatile uint8_t cso;
  volatile uint8_t cmd;
  volatile uint8_t status;
  volatile uint8_t css;
  volatile uint16_t vlan_tag;
}__attribute__((packed));

struct nic {
  struct rx_descriptor rx_ring[RING_SIZE];
  struct tx_descriptor tx_ring[RING_SIZE];
  uint32_t base_address;
  uint32_t rx_ring_next;
  uint32_t tx_ring_next;
};

struct nic *nics[32];

void nic_reset(struct nic *nic);
void nic_forward(struct nic *rxnic, struct nic *txnic);
void detect_nics();
void clear_icr(struct nic *nic);
