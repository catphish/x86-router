#include "stdint.h"
#include "ports.h"

uint32_t pciConfigRead (uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset)
{
    uint32_t address;
    /* create configuration address as per Figure 1 */
    address = ((bus << 16) | (slot << 11) |
              (func << 8) | (offset & 0xfc) | 0x80000000);
    /* write out the address */
    outl (address, 0xCF8);
    /* read in the data */
    return inl(0xCFC);
}
