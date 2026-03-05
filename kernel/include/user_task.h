#ifndef KERNEL_USER_TASK_H
#define KERNEL_USER_TASK_H

#include "arch_mm.h"
#include "boot_info.h"

typedef struct {
  u64 user_base;
  u64 user_size;
  u64 user_ip;
  u64 user_sp;
  void *kernel_stack_top;
} user_task_bootstrap_t;

status_t user_stack_alloc(u64 size, u64 *out_base);
status_t user_window_map(u64 base, u64 size, u64 prot);
status_t user_task_bootstrap_prepare(const boot_info_t *boot_info, user_task_bootstrap_t *out_ctx);

#endif
