#ifndef KERNEL_USER_TASK_H
#define KERNEL_USER_TASK_H

#include "arch_mm.h"
#include "boot_info.h"

typedef struct {
  BOOT_U64 user_base;
  BOOT_U64 user_size;
  BOOT_U64 user_ip;
  BOOT_U64 user_sp;
  void *kernel_stack_top;
} user_task_bootstrap_t;

status_t user_stack_alloc(BOOT_U64 size, BOOT_U64 *out_base);
status_t user_window_map(BOOT_U64 base, BOOT_U64 size, BOOT_U64 prot);
status_t user_task_bootstrap_prepare(const boot_info_t *boot_info, user_task_bootstrap_t *out_ctx);

#endif
