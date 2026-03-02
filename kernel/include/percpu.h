#ifndef KERNEL_PERCPU_H
#define KERNEL_PERCPU_H

#include "kernel.h"

#define PERCPU_API_VERSION_MAJOR 1U
#define PERCPU_API_VERSION_MINOR 0U

#define PERCPU_MAX_CPUS 64U

typedef struct {
  BOOT_U64 cpu_id;
  BOOT_U64 online;
  BOOT_U64 irq_nesting;
  BOOT_U64 preempt_disable_depth;
  BOOT_U64 local_tick_count;
  void *current_task;
  void *arch_local;
} percpu_t;

status_t percpu_init_boot_cpu(const boot_info_t *boot_info);
status_t percpu_register_current_cpu(BOOT_U64 cpu_id);
percpu_t *percpu_current(void);
percpu_t *percpu_by_id(BOOT_U64 cpu_id);
BOOT_U64 percpu_online_count(void);

#endif
