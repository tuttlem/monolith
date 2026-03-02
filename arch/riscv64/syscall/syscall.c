#include "arch_syscall.h"

status_t arch_syscall_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_DEFERRED;
}
