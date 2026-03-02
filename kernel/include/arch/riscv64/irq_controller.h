#ifndef KERNEL_ARCH_RISCV64_IRQ_CONTROLLER_H
#define KERNEL_ARCH_RISCV64_IRQ_CONTROLLER_H

#include "kernel.h"

status_t riscv64_irq_controller_init(const boot_info_t *boot_info);

#endif
