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
#define EIMS     0x1524
#define TDH      0xE010
#define TDT      0xE018
#define TCTL     0x0400
#define TDBAL    0xE000
#define TDBAH    0xE004
#define TDLEN    0xE008
#define TXDCTL   0xE028

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
  write_register(nic, 0x1528, 0xffffffff);
  putstring("Disabling bus master\n");
  write_register(nic, CTRL, read_register(nic, CTRL) | (1<<2));
  while(read_register(nic, STATUS) & (1<<19)) {
    puthex32(read_register(nic, STATUS));
    putchar('\n');
  }

  putstring("Resetting\n");
  write_register(nic, CTRL, read_register(nic, CTRL) | (1<<26));
  while(!(read_register(nic, STATUS) & (1<<21))) {
    puthex32(read_register(nic, STATUS));
    putchar('\n');
  }
  write_register(nic, 0x1528, 0xffffffff);

  // Pretty simple base setup, enable link, auto-negotiate everything
  write_register(nic, CTRL, 1<<6);

  // Erase Multicast Table
  for(n=0x5200;n<0x5400;n+=4) {
    write_register(nic, n, 0);
  }

  // Disable RX
  write_register(nic, RCTL, 0);

  // Populate rx ring 0 with RING_SIZE entries
  for(int n=0;n<RING_SIZE;n++) {
    // Allocate 2kB per frame
    nic->rx_ring[n].address_upper = 0;
    nic->rx_ring[n].address = (uint32_t)malloc(2048);
  }

  // Configure multi-ring receive to default single ring operation
  write_register(nic, MRQC, 0);

  // Configure Base address of rx ring 0
  write_register(nic, RDBAL, (uint32_t)nic->rx_ring);
  write_register(nic, RDBAH, 0);
  // Configure byte size of receive rx ring 0
  write_register(nic, RDLEN, sizeof(struct rx_descriptor) * RING_SIZE);
  // Set head and tail offsets of rx ring 0 to zero
  write_register(nic, RDH, 0);
  write_register(nic, RDT, 0);
  // Set "next" offet for rx ring 0
  nic->rx_ring_next = 0;
  // Configure settings for receive rx ring 0 and enable it
  write_register(nic, RXDCTL, 0x2010A0C);
  // Enable NIC global receive, and turn on promiscuous mode for now
  write_register(nic, RCTL, (1<<1)|(1<<3)|(1<<4));
  // Start rx ring 0 by incrementing the tail offset to the end
  write_register(nic, RDT, RING_SIZE-1);

  // Populate tx ring 0 with RING_SIZE entries
  for(int n=0;n<RING_SIZE;n++) {
    // Allocate 2kB per frame
    nic->tx_ring[n].address_upper = 0;
    nic->tx_ring[n].address = (uint32_t)malloc(2048);
    nic->tx_ring[n].status = 1;
  }
  write_register(nic, TDH, 0);
  write_register(nic, TDT, 0);
  write_register(nic, TDBAL, (uint32_t)nic->tx_ring);
  write_register(nic, TDBAH, 0);
  write_register(nic, TDLEN, sizeof(struct tx_descriptor) * RING_SIZE);
  write_register(nic, TXDCTL, (1<<25));
  write_register(nic, TCTL, read_register(nic, TCTL)|2);
}

uint32_t count = 0;

void nic_forward(struct nic *rxnic, struct nic *txnic) {
  while(rxnic->rx_ring[rxnic->rx_ring_next].status & 0x1) {
    if(txnic->tx_ring[txnic->tx_ring_next].status & 0x1) {
      // Data pointer swap
      uint32_t tmp_ptr;
      tmp_ptr = txnic->tx_ring[txnic->tx_ring_next].address;
      txnic->tx_ring[txnic->tx_ring_next].address = rxnic->rx_ring[rxnic->rx_ring_next].address;
      rxnic->rx_ring[rxnic->rx_ring_next].address = tmp_ptr;

      // Copy length
      txnic->tx_ring[txnic->tx_ring_next].length = rxnic->rx_ring[rxnic->rx_ring_next].length;
      // Set tx params
      txnic->tx_ring[txnic->tx_ring_next].cmd = 11;
      txnic->tx_ring[txnic->tx_ring_next].status = 0;

      // Incremement TX pointer
      txnic->tx_ring_next = (txnic->tx_ring_next + 1) % RING_SIZE;
    } else {
      putchar('.');
    }
    // We've processed this RX descriptor so zero its status
    rxnic->rx_ring[rxnic->rx_ring_next].status = 0;

    // Incremement RX pointer
    rxnic->rx_ring_next = (rxnic->rx_ring_next + 1) % RING_SIZE;
  }
  write_register(rxnic, RDT, (rxnic->rx_ring_next - 1) % RING_SIZE);
  write_register(txnic, TDT, txnic->tx_ring_next);
}

void nic_tx_status(struct nic *txnic) {
  puthex32(read_register(txnic, TDT));
  putchar(' ');
  puthex32(read_register(txnic, TDH));
  putchar('\n');
}
