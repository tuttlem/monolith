#ifndef KERNEL_CPU_CONTEXT_H
#define KERNEL_CPU_CONTEXT_H

#include "kernel.h"

typedef struct {
  u64 sp;
  u64 ip;
  u64 arg;
  u64 flags;
} cpu_context_t;

status_t cpu_context_init(cpu_context_t *ctx, void (*entry)(void *), void *arg, void *stack_top);
status_t cpu_context_switch(cpu_context_t *from, cpu_context_t *to);

#endif
