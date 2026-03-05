#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "kernel.h"

status_t timer_init(const boot_info_t *boot_info);
u64 timer_ticks(void);
u64 timer_hz(void);

#endif
