#ifndef KERNEL_CLOCK_H
#define KERNEL_CLOCK_H

#include "kernel.h"

typedef u64 clock_id_t;

status_t clock_enable(clock_id_t clk);
status_t clock_disable(clock_id_t clk);
status_t clock_set_rate(clock_id_t clk, u64 hz);
status_t clock_get_rate(clock_id_t clk, u64 *out_hz);

#endif
