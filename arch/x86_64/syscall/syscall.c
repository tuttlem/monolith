#include "arch_syscall.h"

#define X86_64_SYSCALL_VECTOR 0x80ULL

status_t arch_syscall_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

status_t arch_syscall_get_vector(BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = X86_64_SYSCALL_VECTOR;
  return STATUS_OK;
}

status_t arch_syscall_trigger(void) {
  __asm__ volatile("int $0x80" : : : "memory");
  return STATUS_OK;
}
