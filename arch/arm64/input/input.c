#include "arch_input.h"

status_t arch_input_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

void arch_input_poll(void) {}
