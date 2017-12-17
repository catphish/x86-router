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
#define ICR      0x1500
#define EICR     0x1580
#define EIMC     0x1528
#define EIMS     0x1524
#define IMS      0x1508
#define MULTICAST_TABLE_START 0x5200
#define MULTICAST_TABLE_END   0x5400

#define CTRL_MASTER_DISABLE   (1<<2)
#define CTRL_RST              (1<<26)
#define STATUS_MASTER_ENABLED (1<<19)
#define STATUS_PF_RST_DONE    (1<<21)
#define CTRL_EXT_DRV_LOAD     (1<<28)
#define RCTL_RXEN             (1<<1)
#define RCTL_UPE              (1<<3)
#define RCTL_MPE              (1<<4)
#define RXDCTL_ENABLE         (1<<25)
#define TXDCTL_ENABLE         (1<<25)
#define TCTL_EN               (1<<1)
#define IMS_RXDW              (1<<7)
#define EIMS_OTHER            (1<<31)

#define I350_ID                      0x15218086
#define I350_MSI_OFFSET              0x50
#define I350_MSI_LOW_ADDRESS_OFFSET  0x54
#define I350_MSI_HIGH_ADDRESS_OFFSET 0x58
#define I350_MSI_DATA_OFFSET         0x5C
#define PCI_MSI_ENABLE               0x10000
#define PCI_BAR0                     0x10

// Allocate some memory to test MSI memory write
volatile uint32_t interrupt_fired = 0;

// Inline helper method to write NIC registers
inline static void write_register(struct nic *nic, uint32_t offset, uint32_t value) {
  volatile uint32_t * r;
  r = (uint32_t *) (nic->base_address + offset);
  *r = value;
}

// Inline helper method to read NIC registers
inline static uint32_t read_register(struct nic *nic, uint32_t offset) {
  volatile uint32_t * r;
  r = (uint32_t *) (nic->base_address + offset);
  return(*r);
}

// Loop over PCI bus to find NICs
// No support for multiple functions yet
void detect_nics(struct nic **nic) {
  uint32_t bus;
  uint32_t device;
  uint32_t msi_config;
  uint32_t n;

  putstring("Scannig PCIe bus.\n");
  // Loop over all devices
  for(bus=0;bus<256;bus++) {
    for(device = 0; device < 32; device++) {
      // Read the vendor and device ID, look for i350 devices
      if(pciConfigRead(bus,device,0,0) == I350_ID) {
        // Allocate some memory for this NIC
        *nic = malloc(sizeof(struct nic));
        // Store the base memory address of this NIC
        (*nic)->base_address = pciConfigRead(bus,device,0,PCI_BAR0) & 0xfffffff0;
        // Inform the user
        putstring("NIC detected: ");
        puthex8(bus); putchar(':'); puthex8(device); putchar('\n');

        // The i350 has MSI config starting at 0x50 in the PCI configuration space.
        msi_config = pciConfigRead(bus,device,0,I350_MSI_OFFSET);
        msi_config |= PCI_MSI_ENABLE;
        // Write 0x21 to trigger vector 0x21
        pciConfigWrite(bus,device,0,I350_MSI_DATA_OFFSET, 0x21);
        //pciConfigWrite(bus,device,0,I350_MSI_LOW_ADDRESS_OFFSET, 0xFEE00000);
        // Temporary variable address used for testing
        pciConfigWrite(bus,device,0,I350_MSI_LOW_ADDRESS_OFFSET, (uint32_t)&interrupt_fired);
        pciConfigWrite(bus,device,0,I350_MSI_HIGH_ADDRESS_OFFSET, 0x00000000);
        pciConfigWrite(bus,device,0,I350_MSI_OFFSET, msi_config);

        // Reset and initialize the NIC
        nic_reset(*nic);

        // Write the PCI address space for debugging
        putstring("PCIe Config:\n");
        for(n=0x00; n<0x100; n+=0x4) {
          puthex8(n);
          putstring(": ");
          puthex32(pciConfigRead(bus,device,0,n));
          putchar('\n');
        }

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
  write_register(nic, EIMC, 0xffffffff);
  // Disable bus mastering
  putstring("Disabling bus master\n");
  write_register(nic, CTRL, read_register(nic, CTRL) | CTRL_MASTER_DISABLE);
  // Wait for bus mastering to be disabled
  while(read_register(nic, STATUS) & STATUS_MASTER_ENABLED);

  putstring("Resetting\n");
  write_register(nic, CTRL, read_register(nic, CTRL) | CTRL_RST);
  // Wait for reset to complete
  while(!(read_register(nic, STATUS) & STATUS_PF_RST_DONE));
  // Mask all interrupts again
  write_register(nic, EIMC, 0xffffffff);

  // Tell the NIC a driver is loaded
  write_register(nic, CTRL_EXT, read_register(nic, CTRL_EXT)|CTRL_EXT_DRV_LOAD);

  // Erase Multicast Table
  for(n=MULTICAST_TABLE_START;n<MULTICAST_TABLE_END;n+=4) {
    write_register(nic, n, 0);
  }

  // Disable RX
  write_register(nic, RCTL, read_register(nic, RCTL) & (~RCTL_RXEN));

  // Populate rx ring 0 with RING_SIZE entries
  for(int n=0;n<RING_SIZE;n++) {
    // Allocate 2kB per frame
    nic->rx_ring[n].address_upper = 0;
    nic->rx_ring[n].address = (uint32_t)malloc(2048);
  }

  // Configure multi-ring receive to default single ring operation
  // write_register(nic, MRQC, 0); // This is the default

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
  // Configure enable ring 0
  write_register(nic, RXDCTL, read_register(nic, RXDCTL) | RXDCTL_ENABLE);
  // Enable NIC global receive, and turn on promiscuous mode for now
  write_register(nic, RCTL, read_register(nic, RCTL) | RCTL_RXEN | RCTL_UPE | RCTL_MPE);
  // Start rx ring 0 by incrementing the tail offset to the end
  write_register(nic, RDT, RING_SIZE-1);

  // Populate tx ring 0 with RING_SIZE entries
  for(int n=0;n<RING_SIZE;n++) {
    // Allocate 2kB per frame
    nic->tx_ring[n].address_upper = 0;
    nic->tx_ring[n].address = (uint32_t)malloc(2048);
    nic->tx_ring[n].status = 1;
  }
  // Zero the TX head and tail pointers
  write_register(nic, TDH, 0);
  write_register(nic, TDT, 0);
  // Point the TX ring to the allocatd memory
  write_register(nic, TDBAL, (uint32_t)nic->tx_ring);
  write_register(nic, TDBAH, 0);
  write_register(nic, TDLEN, sizeof(struct tx_descriptor) * RING_SIZE);
  // Enable TX ring
  write_register(nic, TXDCTL, read_register(nic, TXDCTL) | TXDCTL_ENABLE);
  // Globaly enable TX
  write_register(nic, TCTL, read_register(nic, TCTL)|TCTL_EN);

  // Enable specific interrupts
  write_register(nic, IMS, IMS_RXDW);
  write_register(nic, EIMS, EIMS_OTHER);

  putstring("IMS value: ");
  puthex32(read_register(nic, IMS));
  putchar('\n');

  putstring("EIMS value: ");
  puthex32(read_register(nic, EIMS));
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

// This gets run at 1-second intervals by a timer
void check_icr(struct nic *nic)
{
  putstring("ICR value: ");
  puthex32(read_register(nic, ICR));
  putchar('\n');

  putstring("EICR value: ");
  puthex32(read_register(nic, EICR));
  putchar('\n');

  putstring("interrupt_fired value: ");
  puthex32(interrupt_fired);
  putchar('\n');

  // Clear interrupt causes
  write_register(nic, ICR, 0xffffffff);
  write_register(nic, EICR, 0xffffffff);
  interrupt_fired = 0;

  putchar('\n');
}
