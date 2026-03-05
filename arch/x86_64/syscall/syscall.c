#include "arch_syscall.h"
#include "arch_syscall_abi.h"
#include "arch_user_syscall.h"

#define X86_64_SYSCALL_VECTOR 0x80ULL

status_t arch_syscall_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

status_t arch_syscall_get_vector(u64 *out_vector) {
  if (out_vector == (u64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = X86_64_SYSCALL_VECTOR;
  return STATUS_OK;
}

status_t arch_syscall_trigger(void) {
  __asm__ volatile("int $0x80" : : : "memory");
  return STATUS_OK;
}

status_t arch_syscall_decode(const void *trap_frame, syscall_abi_frame_t *out) {
  const syscall_abi_frame_t *in;
  u64 i;

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

status_t arch_syscall_encode_ret(void *trap_frame, u64 value) {
  syscall_abi_frame_t *out;

  if (trap_frame == (void *)0) {
    return STATUS_INVALID_ARG;
  }

  out = (syscall_abi_frame_t *)trap_frame;
  out->args[0] = value;
  return STATUS_OK;
}

status_t arch_user_syscall_invoke6(u64 op, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 *out_ret) {
  register u64 rax __asm__("rax") = op;
  register u64 rdi __asm__("rdi") = a0;
  register u64 rsi __asm__("rsi") = a1;
  register u64 rdx __asm__("rdx") = a2;
  register u64 r10 __asm__("r10") = a3;
  register u64 r8 __asm__("r8") = a4;
  register u64 r9 __asm__("r9") = a5;

  if (out_ret == (u64 *)0) {
    return STATUS_INVALID_ARG;
  }

  __asm__ volatile("int $0x80"
                   : "+a"(rax)
                   : "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8), "r"(r9)
                   : "rcx", "r11", "memory");

  *out_ret = rax;
  return STATUS_OK;
}
