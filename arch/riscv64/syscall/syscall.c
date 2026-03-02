#include "arch_syscall.h"

#define RISCV64_SYSCALL_VECTOR 64ULL

status_t arch_syscall_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

status_t arch_syscall_get_vector(BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = RISCV64_SYSCALL_VECTOR;
  return STATUS_OK;
}

status_t arch_syscall_trigger(void) {
  __asm__ volatile("csrsi sie, 2\n\t"
                   "csrsi sip, 2"
                   :
                   :
                   : "memory");
  return STATUS_OK;
}
