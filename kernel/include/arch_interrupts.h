#ifndef KERNEL_ARCH_INTERRUPTS_H
#define KERNEL_ARCH_INTERRUPTS_H

#include "kernel.h"

status_t arch_interrupts_init(const boot_info_t *boot_info);
void arch_interrupts_enable(void);
void arch_interrupts_disable(void);

#endif
