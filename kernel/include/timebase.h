#ifndef KERNEL_TIMEBASE_H
#define KERNEL_TIMEBASE_H

#include "kernel.h"

typedef struct {
  const char *name;
  BOOT_U64 freq_hz;
  BOOT_U64 (*read_cycles)(void);
} clocksource_t;

typedef struct {
  const char *name;
  status_t (*set_periodic)(BOOT_U64 hz);
  status_t (*set_oneshot_ns)(BOOT_U64 delta_ns);
  void (*ack)(BOOT_U64 vector);
} clockevent_t;

typedef struct {
  BOOT_U64 stable;
  BOOT_U64 unstable;
  BOOT_U64 emulated;
  BOOT_U64 calibrated_hz;
  BOOT_U64 drift_ppm_bound;
  BOOT_U64 cross_cpu_checked;
  BOOT_U64 cross_cpu_passed;
} time_quality_t;

status_t time_init(const boot_info_t *boot_info);
BOOT_U64 time_now_ns(void);
BOOT_U64 time_ticks(void);
BOOT_U64 time_hz(void);
BOOT_U64 time_cycles_to_ns(BOOT_U64 cycles);
BOOT_U64 time_ns_to_cycles(BOOT_U64 ns);
status_t time_quality(time_quality_t *out);
const clocksource_t *time_clocksource(void);
const clockevent_t *time_clockevent(void);

#endif
