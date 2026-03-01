#include "arch_interrupts.h"

status_t arch_interrupts_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_SPARC64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_DEFERRED;
}

void arch_interrupts_enable(void) {}

void arch_interrupts_disable(void) {}

void arch_exception_selftest_trigger(void) {
  kprintf("exception self-test: sparc64 trigger deferred\n");
}
