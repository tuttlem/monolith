#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "kernel.h"

status_t timer_init(const boot_info_t *boot_info);
BOOT_U64 timer_ticks(void);
BOOT_U64 timer_hz(void);

#endif
