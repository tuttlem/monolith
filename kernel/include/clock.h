#ifndef KERNEL_CLOCK_H
#define KERNEL_CLOCK_H

#include "kernel.h"

typedef BOOT_U64 clock_id_t;

status_t clock_enable(clock_id_t clk);
status_t clock_disable(clock_id_t clk);
status_t clock_set_rate(clock_id_t clk, BOOT_U64 hz);
status_t clock_get_rate(clock_id_t clk, BOOT_U64 *out_hz);

#endif
