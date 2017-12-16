#include "stdint.h"

uint32_t pciConfigRead (uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);
void pciConfigWrite (uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t value);
