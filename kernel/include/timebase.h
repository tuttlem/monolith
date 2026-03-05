#ifndef KERNEL_TIMEBASE_H
#define KERNEL_TIMEBASE_H

#include "kernel.h"

typedef struct {
  const char *name;
  u64 freq_hz;
  u64 (*read_cycles)(void);
} clocksource_t;

typedef struct {
  const char *name;
  status_t (*set_periodic)(u64 hz);
  status_t (*set_oneshot_ns)(u64 delta_ns);
  void (*ack)(u64 vector);
} clockevent_t;

typedef struct {
  u64 stable;
  u64 unstable;
  u64 emulated;
  u64 calibrated_hz;
  u64 drift_ppm_bound;
  u64 cross_cpu_checked;
  u64 cross_cpu_passed;
} time_quality_t;

status_t time_init(const boot_info_t *boot_info);
u64 time_now_ns(void);
u64 time_ticks(void);
u64 time_hz(void);
u64 time_cycles_to_ns(u64 cycles);
u64 time_ns_to_cycles(u64 ns);
status_t time_quality(time_quality_t *out);
const clocksource_t *time_clocksource(void);
const clockevent_t *time_clockevent(void);

#endif
