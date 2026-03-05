#ifndef KERNEL_CPU_CAPS_H
#define KERNEL_CPU_CAPS_H

#include "kernel.h"

typedef struct {
  u64 arch_id;
  u64 has_fp;
  u64 has_simd;
  u64 has_virtualization;
  u64 has_atomic;
  u64 has_cycle_counter;
} cpu_caps_t;

status_t cpu_caps_query(cpu_caps_t *out_caps);

#endif
