# 140 Scheduler Scaffolding

## Goal

Create minimal task/run-queue/context-switch scaffolding without committing to full scheduling policy.

## Minimal Structures

```c
typedef enum {
  TASK_NEW,
  TASK_RUNNABLE,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_ZOMBIE,
} task_state_t;

typedef struct task {
  BOOT_U64 tid;
  task_state_t state;
  void *arch_ctx;
  void *kernel_stack;
  BOOT_U64 kernel_stack_size;
  struct task *next;
} task_t;
```

## Required APIs

- `status_t sched_init(void)`
- `task_t *sched_current(void)`
- `status_t sched_add(task_t *task)`
- `void sched_tick(void)`
- `status_t arch_context_switch(task_t *from, task_t *to)`

## Phase Policy

- single runnable kernel task initially acceptable
- timer tick integration required for future preemption hooks
- architecture context-switch hook may start with conservative register set

## Acceptance Criteria

- scheduler compiles and runs with one idle task on all architectures
- context-switch path can be invoked in controlled test on at least x86_64 and arm64
