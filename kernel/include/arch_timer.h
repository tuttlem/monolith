#ifndef KERNEL_ARCH_TIMER_H
#define KERNEL_ARCH_TIMER_H

#include "kernel.h"

#define ARCH_TIMER_API_VERSION_MAJOR 1U
#define ARCH_TIMER_API_VERSION_MINOR 0U

status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector);
void arch_timer_ack(BOOT_U64 vector);

#endif
