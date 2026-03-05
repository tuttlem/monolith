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
  u64 tid;
  task_state_t state;
  void *arch_ctx;
  void *kernel_stack;
  u64 kernel_stack_size;
  struct task *next;
} task_t;

typedef struct {
  status_t (*init)(void);
  status_t (*set_foreground)(task_t *task);
  status_t (*enqueue)(task_t *task);
  task_t *(*pick_next)(void);
  void (*on_yield)(task_t *task);
  void (*on_exit)(task_t *task, u64 code);
} scheduler_ops_t;

status_t sched_register_backend(const scheduler_ops_t *ops);
status_t sched_init(void);
task_t *sched_current(void);
status_t sched_set_foreground(task_t *task);
status_t sched_add(task_t *task);
void sched_on_exit(task_t *task, u64 code);
void sched_tick(void);
status_t arch_context_switch(task_t *from, task_t *to);

#endif
