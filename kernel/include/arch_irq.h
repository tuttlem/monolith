#ifndef KERNEL_ARCH_IRQ_H
#define KERNEL_ARCH_IRQ_H

#include "kernel.h"

#define ARCH_IRQ_API_VERSION_MAJOR 1U
#define ARCH_IRQ_API_VERSION_MINOR 0U

/*
 * Stable IRQ HAL interface.
 * Backward-compatibility is provided through arch_interrupts_* names until
 * all backend files are migrated.
 */
status_t arch_interrupts_init(const boot_info_t *boot_info);
void arch_interrupts_enable(void);
void arch_interrupts_disable(void);

static inline status_t arch_irq_init(const boot_info_t *boot_info) {
  return arch_interrupts_init(boot_info);
}

static inline void arch_irq_enable(void) { arch_interrupts_enable(); }

static inline void arch_irq_disable(void) { arch_interrupts_disable(); }

#endif
