#include "stdint.h"
#include "debug.h"
#include "malloc.h"
#include "nic.h"
#include "acpi.h"

void ap_c_entry() {
  // General purpose counter
  uint32_t n;
  // The local APIC ID of this AP
  uint32_t *lapic_id = (void*)0xFEE00020;
  uint8_t cpu_id = *lapic_id >> 24;
  // Find the index of this AP in the cpu_list array
  uint8_t cpu_index = 0xFF;
  for(n=0;n<cpu_count;n++) {
    if(cpu_id == cpu_list[n]) cpu_index = n;
  }
  putstring("Hello from additional CPU ");
  puthex8(cpu_id);
  putstring(" index ");
  puthex8(cpu_index);
  putchar('\n');
  // Choose some NICs and forward frames
  while(1) {
    for(n=cpu_index;n<nic_count;n+=cpu_count) {
      nic_forward(nic_list[n], nic_list[1]);
    }
  }
}

void install_ap_entry(uint32_t location) {
  extern char ap_asm_start[];
  extern uint32_t ap_asm_length;
  extern uint32_t ap_stack_pointer_offset;
  putstring("Copying AP boot code.\n");
  memcpy((char*)location, ap_asm_start, ap_asm_length);
  void* stack_pointer_location = (void*)location + ap_stack_pointer_offset;
  void* stack_pointer = malloc(1024);
  memcpy(stack_pointer_location, &stack_pointer, 4);
}

void configure_lapic() {
  // Set up the local APIC timer
  // Enable the local APIC
  asm volatile(
    "mov $27, %%ecx\n\t"
    "rdmsr\n\t"
    "bts $11, %%eax\n\t"
    "wrmsr\n\t" : : : "eax", "ebx", "ecx", "edx"
  );
  // Enable interrupts
  asm volatile("sti");
}

void arm_timer() {
  volatile uint32_t *lapic_timer_interrupt_vector = (void*)0xFEE00320;
  volatile uint32_t *lapic_timer_divide = (void*)0xFEE003E0;
  volatile uint32_t *lapic_timer_initial_count = (void*)0xFEE00380;
  *lapic_timer_interrupt_vector = 0x20020;
  *lapic_timer_divide = 0b1011;
  *lapic_timer_initial_count = 1000000000; // 1 second
}

void start_ap(uint8_t cpu_id) {
  install_ap_entry(0x8000 + cpu_id * 0x1000);

  // Set up pointers to IPI registers in the LAPIC
  volatile uint32_t *lapic_offset_300 = (void*)0xFEE00300;
  volatile uint32_t *lapic_offset_310 = (void*)0xFEE00310;

  // We're addressing a specific CPU
  *lapic_offset_310 = cpu_id << 24;
  // Send the INIT (reset) to selected AP
  putstring("Sending INIT\n");
  *lapic_offset_300 = 0x00004500;

  // Sleep a little
  int i = 0x1fffffff; while(--i) asm("");

  // Send the SIPI to commence execution
  putstring("Sending SIPI\n");
  *lapic_offset_310 = cpu_id << 24;
  *lapic_offset_300 = 0x00004600+0x8+cpu_id;
}
