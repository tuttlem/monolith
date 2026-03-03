#ifndef KERNEL_ARCH_SYSCALL_ABI_H
#define KERNEL_ARCH_SYSCALL_ABI_H

#include "kernel.h"

typedef struct {
  BOOT_U64 op;
  BOOT_U64 args[6];
} syscall_abi_frame_t;

status_t arch_syscall_decode(const void *trap_frame, syscall_abi_frame_t *out);
status_t arch_syscall_encode_ret(void *trap_frame, BOOT_U64 value);

#endif
