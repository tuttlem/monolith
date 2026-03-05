#ifndef KERNEL_ARCH_USER_MODE_H
#define KERNEL_ARCH_USER_MODE_H

#include "kernel.h"

typedef void (*arch_user_entry_t)(void *arg);

typedef struct {
  u64 user_ip;
  u64 user_sp;
  u64 arg0;
  u64 flags;
} arch_user_frame_t;

/*
 * Configure kernel return stack used after trap/syscall return from user mode.
 * Must be called before arch_user_mode_enter().
 */
status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top);

/*
 * Optional helper for backends that require explicit frame preparation.
 * Backends that do not use this path return STATUS_NOT_SUPPORTED.
 */
status_t arch_user_mode_prepare_frame(arch_user_frame_t *frame);

/*
 * Enter user mode and transfer control to entry(arg) with user_sp stack.
 * This function never returns.
 */
__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, u64 user_sp);

#endif
