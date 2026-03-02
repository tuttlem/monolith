#ifndef KERNEL_ARCH_CPU_H
#define KERNEL_ARCH_CPU_H

#include "kernel.h"

#define ARCH_CPU_API_VERSION_MAJOR 1U
#define ARCH_CPU_API_VERSION_MINOR 0U

/*
 * Stable CPU HAL interface.
 * 010-core-interfaces freezes this contract; 040-arch-cpu will provide
 * architecture-specific implementations behind it.
 */
static inline status_t arch_cpu_early_init(const boot_info_t *boot_info) {
  (void)boot_info;
  return STATUS_OK;
}

static inline status_t arch_cpu_late_init(void) { return STATUS_OK; }

static inline BOOT_U64 arch_cpu_id(void) { return 0ULL; }

static inline void arch_cpu_relax(void) { __asm__ volatile("" : : : "memory"); }

static inline void arch_cpu_halt(void) { arch_halt(); }

#endif
