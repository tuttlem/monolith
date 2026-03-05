#ifndef KERNEL_ARCH_USER_SYSCALL_H
#define KERNEL_ARCH_USER_SYSCALL_H

#include "kernel.h"

status_t arch_user_syscall_invoke6(u64 op, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 *out_ret);

#endif
