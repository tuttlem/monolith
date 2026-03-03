#ifndef KERNEL_CPU_CAPS_H
#define KERNEL_CPU_CAPS_H

#include "kernel.h"

typedef struct {
  BOOT_U64 arch_id;
  BOOT_U64 has_fp;
  BOOT_U64 has_simd;
  BOOT_U64 has_virtualization;
  BOOT_U64 has_atomic;
  BOOT_U64 has_cycle_counter;
} cpu_caps_t;

status_t cpu_caps_query(cpu_caps_t *out_caps);

#endif
