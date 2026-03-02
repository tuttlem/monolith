#ifndef KERNEL_ARCH_MM_H
#define KERNEL_ARCH_MM_H

#include "memory_init.h"

#define ARCH_MM_API_VERSION_MAJOR 1U
#define ARCH_MM_API_VERSION_MINOR 0U

/*
 * Stable MM HAL interface.
 * The existing arch_memory_init() path is retained as the backend-compatible
 * implementation for early MMU takeover.
 */
static inline status_t arch_mm_early_init(boot_info_t *boot_info) {
  return arch_memory_init(boot_info);
}

#endif
