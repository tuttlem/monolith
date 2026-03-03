#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel.h"

typedef enum {
  TASK_NEW = 0,
  TASK_RUNNABLE = 1,
  TASK_RUNNING = 2,
  TASK_BLOCKED = 3,
  TASK_ZOMBIE = 4
} task_state_t;

typedef struct task {
  BOOT_U64 tid;
  task_state_t state;
  void *arch_ctx;
  void *kernel_stack;
  BOOT_U64 kernel_stack_size;
  struct task *next;
} task_t;

status_t sched_init(void);
task_t *sched_current(void);
status_t sched_add(task_t *task);
void sched_tick(void);
status_t arch_context_switch(task_t *from, task_t *to);

#endif
