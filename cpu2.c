#include "stdint.h"
#include "debug.h"

void cpu2() {
  while(1) asm("hlt");
}
