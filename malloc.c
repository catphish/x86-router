#include "stdint.h"

void memcpy(void *dest, void *src, uint32_t n)
{
   // Typecast src and dest addresses to (char *)
   char *csrc = (char *)src;
   char *cdest = (char *)dest;

   // Copy contents of src[] to dest[]
   for (uint32_t i=0; i<n; i++)
       cdest[i] = csrc[i];
}

void memset(void *b, int c, int len)
{
  unsigned char *p = b;
  while(len > 0) {
    *p = c;
    p++;
    len--;
  }
}

extern char _end;
void* mem_top = &_end;

void* malloc(uint32_t bytes) {
  bytes = (bytes + 127) & 0xFFFFFF80;
  void* location = mem_top;
  mem_top += bytes;
  return location;
}
