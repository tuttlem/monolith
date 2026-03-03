#ifndef KERNEL_CPU_CONTEXT_H
#define KERNEL_CPU_CONTEXT_H

#include "kernel.h"

typedef struct {
  BOOT_U64 sp;
  BOOT_U64 ip;
  BOOT_U64 arg;
  BOOT_U64 flags;
} cpu_context_t;

status_t cpu_context_init(cpu_context_t *ctx, void (*entry)(void *), void *arg, void *stack_top);
status_t cpu_context_switch(cpu_context_t *from, cpu_context_t *to);

#endif
