#include "scheduler.h"
#include "arch_cpu.h"
#include "cpu_context.h"

typedef struct {
  BOOT_U64 initialized;
  BOOT_U64 next_tid;
  task_t *current;
  task_t *foreground;
  task_t *runq_head;
  task_t *runq_tail;
  task_t idle_task;
  cpu_context_t idle_ctx;
  const scheduler_ops_t *ops;
} scheduler_state_t;

static scheduler_state_t g_sched;

static void sched_idle_entry(void *arg) {
  (void)arg;
  for (;;) {
    arch_cpu_halt();
  }
}

static status_t sched_enqueue(task_t *task) {
  if (task == (task_t *)0) {
    return STATUS_INVALID_ARG;
  }
  task->next = (task_t *)0;
  if (g_sched.runq_tail == (task_t *)0) {
    g_sched.runq_head = task;
    g_sched.runq_tail = task;
  } else {
    g_sched.runq_tail->next = task;
    g_sched.runq_tail = task;
  }
  return STATUS_OK;
}

static task_t *sched_dequeue(void) {
  task_t *task = g_sched.runq_head;
  if (task == (task_t *)0) {
    return (task_t *)0;
  }
  g_sched.runq_head = task->next;
  if (g_sched.runq_head == (task_t *)0) {
    g_sched.runq_tail = (task_t *)0;
  }
  task->next = (task_t *)0;
  return task;
}

static status_t sched_default_set_foreground(task_t *task) {
  if (task == (task_t *)0) {
    return STATUS_INVALID_ARG;
  }
  g_sched.foreground = task;
  return STATUS_OK;
}

static status_t sched_default_init(void) {
  status_t st;

  g_sched.next_tid = 1ULL;
  g_sched.current = (task_t *)0;
  g_sched.foreground = (task_t *)0;
  g_sched.runq_head = (task_t *)0;
  g_sched.runq_tail = (task_t *)0;
  g_sched.idle_task.tid = g_sched.next_tid++;
  g_sched.idle_task.state = TASK_RUNNABLE;
  g_sched.idle_task.arch_ctx = &g_sched.idle_ctx;
  g_sched.idle_task.kernel_stack = (void *)0;
  g_sched.idle_task.kernel_stack_size = 0ULL;
  g_sched.idle_task.next = (task_t *)0;

  st = cpu_context_init(&g_sched.idle_ctx, sched_idle_entry, (void *)0, (void *)(BOOT_UPTR)0x1000ULL);
  if (st != STATUS_OK) {
    return st;
  }
  st = sched_enqueue(&g_sched.idle_task);
  if (st != STATUS_OK) {
    return st;
  }

  g_sched.current = sched_dequeue();
  if (g_sched.current == (task_t *)0) {
    return STATUS_FAULT;
  }
  g_sched.current->state = TASK_RUNNING;
  g_sched.foreground = g_sched.current;
  return STATUS_OK;
}

static void sched_default_on_yield(task_t *task) {
  if (task == (task_t *)0) {
    return;
  }
  if (task->state == TASK_RUNNING) {
    task->state = TASK_RUNNABLE;
    (void)sched_enqueue(task);
  }
}

static void sched_default_on_exit(task_t *task, BOOT_U64 code) {
  (void)code;
  if (task == (task_t *)0) {
    return;
  }
  task->state = TASK_ZOMBIE;
}

static const scheduler_ops_t g_default_ops = {
    .init = sched_default_init,
    .set_foreground = sched_default_set_foreground,
    .enqueue = sched_enqueue,
    .pick_next = sched_dequeue,
    .on_yield = sched_default_on_yield,
    .on_exit = sched_default_on_exit,
};

status_t sched_register_backend(const scheduler_ops_t *ops) {
  if (ops == (const scheduler_ops_t *)0 || ops->init == (status_t (*)(void))0 ||
      ops->set_foreground == (status_t (*)(task_t *))0 || ops->enqueue == (status_t (*)(task_t *))0 ||
      ops->pick_next == (task_t *(*)(void))0 || ops->on_yield == (void (*)(task_t *))0 ||
      ops->on_exit == (void (*)(task_t *, BOOT_U64))0) {
    return STATUS_INVALID_ARG;
  }
  if (g_sched.initialized != 0ULL) {
    return STATUS_BUSY;
  }
  g_sched.ops = ops;
  return STATUS_OK;
}

status_t sched_init(void) {
  status_t st;
  if (g_sched.initialized != 0ULL) {
    return STATUS_OK;
  }
  if (g_sched.ops == (const scheduler_ops_t *)0) {
    g_sched.ops = &g_default_ops;
  }
  st = g_sched.ops->init();
  if (st != STATUS_OK) {
    return st;
  }
  g_sched.initialized = 1ULL;
  return STATUS_OK;
}

task_t *sched_current(void) { return g_sched.current; }

status_t sched_set_foreground(task_t *task) {
  if (g_sched.ops == (const scheduler_ops_t *)0) {
    return STATUS_DEFERRED;
  }
  return g_sched.ops->set_foreground(task);
}

status_t sched_add(task_t *task) {
  if (task == (task_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_sched.initialized == 0ULL) {
    return STATUS_TRY_AGAIN;
  }
  if (task->tid == 0ULL) {
    task->tid = g_sched.next_tid++;
  }
  task->state = TASK_RUNNABLE;
  return g_sched.ops->enqueue(task);
}

void sched_on_exit(task_t *task, BOOT_U64 code) {
  if (g_sched.ops == (const scheduler_ops_t *)0) {
    return;
  }
  g_sched.ops->on_exit(task, code);
}

void sched_tick(void) {
  task_t *next;
  task_t *prev;

  if (g_sched.initialized == 0ULL || g_sched.current == (task_t *)0) {
    return;
  }

  prev = g_sched.current;
  g_sched.ops->on_yield(prev);

  next = g_sched.ops->pick_next();
  if (next == (task_t *)0) {
    next = &g_sched.idle_task;
  }
  next->state = TASK_RUNNING;
  g_sched.current = next;

  if (next != prev) {
    (void)arch_context_switch(prev, next);
  }
}
