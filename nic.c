#include "stdint.h"
#include "ports.h"
#include "nic.h"
#include "malloc.h"
#include "debug.h"

#define CTRL     0x0000
#define STATUS   0x0008
#define CTRL_EXT 0x0018
#define RCTL     0x0100
#define RDBAL    0xC000
#define RDBAH    0xC004
#define RDLEN    0xC008
#define RDH      0xC010
#define RDT      0xC018
#define RXDCTL   0xC028
#define RAL      0x5400
#define RAH      0x5404
#define MRQC     0x5818

inline static void write_register(struct nic *nic, uint32_t offset, uint32_t value) {
  volatile uint32_t * r;
  r = (uint32_t *) (nic->base_address + offset);
  *r = value;
}

inline static uint32_t read_register(struct nic *nic, uint32_t offset) {
  volatile uint32_t * r;
  r = (uint32_t *) (nic->base_address + offset);
  return(*r);
}

void nic_reset(struct nic *nic) {
  int n;
  // We could disable interrupts, reset the NIC, and disable them again.
  // The documentation suggests doing this.
  //write_register(nic, 0x1528, 0xffffffff);
  //write_register(nic, 0, 1<<29);
  //write_register(nic, 0x1528, 0xffffffff);

  // Pretty simple base setup, enable link, auto-negotiate everything
  write_register(nic, CTRL, 1<<6);

  // Erase Multicast Table
  for(n=0x5200;n<0x5400;n+=4) {
    write_register(nic, n, 0);
  }

  // Disable RX
  write_register(nic, RCTL, 0);

  // Populate ring 0 with RING_SIZE entries
  for(int n=0;n<RING_SIZE;n++) {
    // Allocate 2kB per frame
    nic->rx_ring[n].address_upper = 0;
    nic->rx_ring[n].address = malloc(2048);
  }

  // Configure multi-ring receive to default single ring operation
  write_register(nic, MRQC, 0);

  // Configure Base address of ring 0
  write_register(nic, RDBAL, (uint32_t)nic->rx_ring);
  write_register(nic, RDBAH, 0);
  // Configure byte size of receive ring 0
  write_register(nic, RDLEN, sizeof(struct rx_descriptor) * RING_SIZE);
  // Set head and tail offsets of ring 0 to zero
  write_register(nic, RDH, 0);
  write_register(nic, RDT, 0);
  // Configure settings for receive ring 0 and enable it
  write_register(nic, RXDCTL, 0x2010A0C);
  // Enable NIC global receive, and turn on promiscuous mode for now
  write_register(nic, RCTL, (1<<1)|(1<<3)|(1<<4));

  // Start ring 0 by incrementing the tail offset to the end
  write_register(nic, RDT, RING_SIZE - 1);
}

void nic_rx(struct nic *nic) {
  uint32_t rdh, rdt;
  rdh = read_register(nic, RDH) & 0xffff;
  rdt = read_register(nic, RDT) & 0xffff;
  write_register(nic, RDT, (rdh+RING_SIZE-1)%RING_SIZE);
  puthex32(rdh);
  putchar(' ');
  puthex32(rdt);
  putchar(' ');
  puthex32(nic->rx_ring[rdt].status);
  putchar(' ');  puthex32(read_register(nic, STATUS));
  putchar('\n');
}
