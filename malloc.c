#include "stdint.h"

extern char _end;
void* mem_top = &_end;

void* malloc(uint32_t bytes) {
  bytes = (bytes + 127) & 0xFFFFFF80;
  void* location = mem_top;
  mem_top += bytes;
  return location;
}
