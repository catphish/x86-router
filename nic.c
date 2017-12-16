#include "stdint.h"
#include "ports.h"
#include "nic.h"
#include "malloc.h"
#include "debug.h"
#include "pci.h"

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

void detect_nics(struct nic **nic) {
  uint32_t bus;
  uint32_t device;
  uint32_t n;
  uint32_t msi_config;

  putstring("Scannig PCIe bus.\n");
  for(bus=0;bus<256;bus++) {
    for(device = 0; device < 32; device++) {
      if(pciConfigRead(bus,device,0,0) == 0x15218086) {
        *nic = malloc(sizeof(struct nic));
        (*nic)->base_address = pciConfigRead(bus,device,0,0x10) & 0xfffffff0;
        putstring("NIC detected: ");
        puthex8(bus);
        putchar(':');
        puthex8(device);
        putchar('\n');

        msi_config = pciConfigRead(bus,device,0,0x50);
        msi_config |= 1<<16;
        pciConfigWrite(bus,device,0,0x50, msi_config);
        pciConfigWrite(bus,device,0,0x54, 0xFEE00000);
        pciConfigWrite(bus,device,0,0x58, 0x00000000);
        pciConfigWrite(bus,device,0,0x5C, 0x21);

        putstring("PCIe Config:\n");
        for(n=0x50; n<0x68; n+=0x4) {
          puthex8(n);
          putstring(": ");
          puthex32(pciConfigRead(bus,device,0,n));
          putchar('\n');
        }
        putchar('\n');
        nic_reset(*nic);
        nic++;
      }
    }
  }
  putstring("Done scanning.\n");
}

void nic_reset(struct nic *nic) {
  int n;
  // The documentation suggests this reset procedure...

  // Mask all interrupts
  write_register(nic, 0x1528, 0xffffffff);
  // Disable bus mastering
  putstring("Disabling bus master\n");
  write_register(nic, CTRL, read_register(nic, CTRL) | (1<<2));
  // Wait for bus mastering to be disabled
  while(read_register(nic, STATUS) & (1<<19));

  putstring("Resetting\n");
  write_register(nic, CTRL, read_register(nic, CTRL) | (1<<26));
  // Wait for reset to complete
  while(!(read_register(nic, STATUS) & (1<<21)));
  // Mask all interrupts again
  write_register(nic, 0x1528, 0xffffffff);

  // Pretty simple base setup, enable link, auto-negotiate everything
  // Also re-enables bus mastering
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
  // A bunch more tx config I haven't documented yet
  write_register(nic, TDH, 0);
  write_register(nic, TDT, 0);
  write_register(nic, TDBAL, (uint32_t)nic->tx_ring);
  write_register(nic, TDBAH, 0);
  write_register(nic, TDLEN, sizeof(struct tx_descriptor) * RING_SIZE);
  write_register(nic, TXDCTL, (1<<25));
  write_register(nic, TCTL, read_register(nic, TCTL)|2);

  // Enable interrupts
  write_register(nic, 0x1508, 1<<7);
  putstring("IMS value: ");
  puthex32(read_register(nic, 0x1508));
  putchar('\n');
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
      //putchar('');
    }
    // We've processed this RX descriptor so zero its status
    rxnic->rx_ring[rxnic->rx_ring_next].status = 0;

    // Incremement RX pointer
    rxnic->rx_ring_next = (rxnic->rx_ring_next + 1) % RING_SIZE;
  }
  write_register(rxnic, RDT, (rxnic->rx_ring_next - 1) % RING_SIZE);
  write_register(txnic, TDT, txnic->tx_ring_next);
}

void check_icr(struct nic *nic)
{
  putstring("ICR value: ");
  puthex32(read_register(nic, 0x1500));
  putchar('\n');
}
