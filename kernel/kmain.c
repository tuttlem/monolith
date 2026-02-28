#include "kernel.h"

#ifndef CORE_ARCH_NAME
#define CORE_ARCH_NAME "unknown"
#endif

void kmain(void) {
  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ")\n");

  for (;;) {
    arch_halt();
  }
}
