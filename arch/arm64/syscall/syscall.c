#include "arch_syscall.h"
#include "arch_syscall_abi.h"

#define ARM64_SYSCALL_VECTOR 64ULL

status_t arch_syscall_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

status_t arch_syscall_get_vector(BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = ARM64_SYSCALL_VECTOR;
  return STATUS_OK;
}

status_t arch_syscall_trigger(void) {
  __asm__ volatile("svc #0" : : : "memory");
  return STATUS_OK;
}

status_t arch_syscall_decode(const void *trap_frame, syscall_abi_frame_t *out) {
  const syscall_abi_frame_t *in;
  BOOT_U64 i;

  if (trap_frame == (const void *)0 || out == (syscall_abi_frame_t *)0) {
    return STATUS_INVALID_ARG;
  }
  in = (const syscall_abi_frame_t *)trap_frame;
  out->op = in->op;
  for (i = 0; i < 6ULL; ++i) {
    out->args[i] = in->args[i];
  }
  return STATUS_OK;
}

status_t arch_syscall_encode_ret(void *trap_frame, BOOT_U64 value) {
  syscall_abi_frame_t *out;
  if (trap_frame == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  out = (syscall_abi_frame_t *)trap_frame;
  out->args[0] = value;
  return STATUS_OK;
}
