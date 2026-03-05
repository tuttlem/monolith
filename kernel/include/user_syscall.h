#ifndef KERNEL_USER_SYSCALL_H
#define KERNEL_USER_SYSCALL_H

#include "arch_user_syscall.h"

static inline status_t user_syscall0(u64 op, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, out_ret);
}

static inline status_t user_syscall1(u64 op, u64 a0, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, a0, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, out_ret);
}

static inline status_t user_syscall2(u64 op, u64 a0, u64 a1, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, a0, a1, 0ULL, 0ULL, 0ULL, 0ULL, out_ret);
}

static inline status_t user_syscall3(u64 op, u64 a0, u64 a1, u64 a2, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, a0, a1, a2, 0ULL, 0ULL, 0ULL, out_ret);
}

static inline status_t user_syscall4(u64 op, u64 a0, u64 a1, u64 a2, u64 a3, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, a0, a1, a2, a3, 0ULL, 0ULL, out_ret);
}

static inline status_t user_syscall5(u64 op, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, a0, a1, a2, a3, a4, 0ULL, out_ret);
}

static inline status_t user_syscall6(u64 op, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 *out_ret) {
  return arch_user_syscall_invoke6(op, a0, a1, a2, a3, a4, a5, out_ret);
}

#endif
