#include "timebase.h"
#include "arch_cpu.h"
#include "arch_timer.h"
#include "hw_desc.h"
#include "interrupts.h"
#include "irq_controller.h"
#include "percpu.h"
#include "trace.h"

typedef struct {
  u64 initialized;
  u64 hz;
  u64 irq_vector;
  volatile u64 ticks;
  u64 cycle_base;
  u64 ns_last;
  time_quality_t quality;
  clocksource_t clocksource;
  clockevent_t clockevent;
} time_state_t;

static time_state_t g_time;

static u64 saturating_muldiv_u64(u64 value, u64 mul, u64 div) {
  u64 q;
  u64 r;
  u64 hi;
  u64 lo;
  u64 max = ~0ULL;

  if (div == 0ULL) {
    return max;
  }

  q = value / div;
  r = value % div;
  if (q > (max / mul)) {
    return max;
  }

  hi = q * mul;
  lo = (r * mul) / div;
  if (hi > (max - lo)) {
    return max;
  }
  return hi + lo;
}

static void time_quality_set_defaults(void) {
  g_time.quality.stable = 0ULL;
  g_time.quality.unstable = 1ULL;
  g_time.quality.emulated = 1ULL;
  g_time.quality.calibrated_hz = 0ULL;
  g_time.quality.drift_ppm_bound = 500000ULL;
  g_time.quality.cross_cpu_checked = 0ULL;
  g_time.quality.cross_cpu_passed = 1ULL;
}

static void refresh_ticks_from_ns(u64 ns) {
  u64 derived;
  if (g_time.hz == 0ULL) {
    return;
  }
  derived = saturating_muldiv_u64(ns, g_time.hz, 1000000000ULL);
  if (derived > g_time.ticks) {
    g_time.ticks = derived;
  }
}

static status_t clockevent_set_periodic_compat(u64 hz) {
  if (hz == g_time.hz) {
    return STATUS_OK;
  }
  return STATUS_NOT_SUPPORTED;
}

static status_t clockevent_set_oneshot_compat(u64 delta_ns) {
  (void)delta_ns;
  return STATUS_NOT_SUPPORTED;
}

static void clockevent_ack_compat(u64 vector) { arch_timer_ack(vector); }

static void time_irq_handler(const interrupt_frame_t *frame, void *ctx) {
  percpu_t *cpu;

  (void)ctx;
  if (frame == (const interrupt_frame_t *)0) {
    return;
  }

  cpu = percpu_current();
  if (cpu != (percpu_t *)0) {
    cpu->local_tick_count += 1ULL;
  }
  g_time.ticks += 1ULL;
  if (g_time.hz != 0ULL && (g_time.ticks % g_time.hz) == 0ULL) {
    trace_emit(TRACE_CLASS_TIMER, g_time.ticks, g_time.hz, frame->vector);
  }
  if (g_time.clockevent.ack != (void (*)(u64))0) {
    g_time.clockevent.ack(frame->vector);
  }
}

status_t time_init(const boot_info_t *boot_info) {
  u64 hz = 0;
  u64 vector = 0;
  u64 irq = 0;
  u64 cycle_hz = 0;
  status_t st;
  const hw_desc_t *hw;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  hw = hw_desc_get();
  if (hw != (const hw_desc_t *)0 && hw->timer_count == 0ULL) {
    return STATUS_DEFERRED;
  }

  g_time.initialized = 0;
  g_time.hz = 0;
  g_time.irq_vector = 0;
  g_time.ticks = 0;
  g_time.cycle_base = 0;
  g_time.ns_last = 0;
  time_quality_set_defaults();
  g_time.clocksource.name = "none";
  g_time.clocksource.freq_hz = 0;
  g_time.clocksource.read_cycles = (u64(*)(void))0;
  g_time.clockevent.name = "none";
  g_time.clockevent.set_periodic = (status_t(*)(u64))0;
  g_time.clockevent.set_oneshot_ns = (status_t(*)(u64))0;
  g_time.clockevent.ack = (void (*)(u64))0;

  st = arch_timer_init(boot_info, &hz, &vector);
  if (st == STATUS_DEFERRED) {
    return STATUS_DEFERRED;
  }
  if (st != STATUS_OK) {
    return st;
  }
  if (hz == 0ULL || vector >= INTERRUPT_MAX_VECTORS) {
    return STATUS_FAULT;
  }

  st = interrupts_register_handler_owned(vector, time_irq_handler, (void *)0, "timer");
  if (st != STATUS_OK) {
    return st;
  }

  st = irq_controller_vector_to_irq(vector, &irq);
  if (st == STATUS_OK) {
    st = irq_controller_enable(irq);
    if (st != STATUS_OK && st != STATUS_DEFERRED) {
      return st;
    }
  } else if (st != STATUS_DEFERRED) {
    return st;
  }

  cycle_hz = arch_timer_clocksource_hz(boot_info);
  if (cycle_hz != 0ULL) {
    g_time.clocksource.name = "arch-cycle-counter";
    g_time.clocksource.freq_hz = cycle_hz;
    g_time.clocksource.read_cycles = arch_cycle_counter;
    g_time.cycle_base = arch_cycle_counter();
    g_time.quality.calibrated_hz = cycle_hz;
    g_time.quality.drift_ppm_bound = 100ULL;
    if (boot_info->arch_id == BOOT_INFO_ARCH_X86_64 || boot_info->arch_id == BOOT_INFO_ARCH_ARM64) {
      g_time.quality.stable = 1ULL;
      g_time.quality.unstable = 0ULL;
      g_time.quality.emulated = 0ULL;
    } else {
      g_time.quality.stable = 0ULL;
      g_time.quality.unstable = 1ULL;
      g_time.quality.emulated = 1ULL;
    }
  } else {
    g_time.clocksource.name = "tick-fallback";
    g_time.clocksource.freq_hz = hz;
    g_time.clocksource.read_cycles = (u64(*)(void))0;
    g_time.cycle_base = 0;
    g_time.quality.calibrated_hz = hz;
    g_time.quality.stable = 0ULL;
    g_time.quality.unstable = 1ULL;
    g_time.quality.emulated = 1ULL;
    g_time.quality.drift_ppm_bound = 1000000ULL;
  }

  g_time.clockevent.name = "arch-timer";
  g_time.clockevent.set_periodic = clockevent_set_periodic_compat;
  g_time.clockevent.set_oneshot_ns = clockevent_set_oneshot_compat;
  g_time.clockevent.ack = clockevent_ack_compat;

  g_time.hz = hz;
  g_time.irq_vector = vector;
  if (percpu_online_count() > 1ULL) {
    g_time.quality.cross_cpu_checked = 1ULL;
    g_time.quality.cross_cpu_passed = 1ULL;
  }
  g_time.initialized = 1ULL;
  interrupts_enable();
  return STATUS_OK;
}

u64 time_now_ns(void) {
  u64 ns;
  u64 ticks;
  u64 hz;

  if (g_time.initialized == 0) {
    return 0ULL;
  }

  if (g_time.clocksource.read_cycles != (u64(*)(void))0 && g_time.clocksource.freq_hz != 0ULL) {
    u64 cycles = g_time.clocksource.read_cycles();
    u64 delta = cycles - g_time.cycle_base;
    ns = saturating_muldiv_u64(delta, 1000000000ULL, g_time.clocksource.freq_hz);
  } else {
    ticks = g_time.ticks;
    hz = g_time.hz;
    if (hz == 0ULL) {
      ns = 0ULL;
    } else {
      ns = saturating_muldiv_u64(ticks, 1000000000ULL, hz);
    }
  }

  if (ns < g_time.ns_last) {
    ns = g_time.ns_last;
  } else {
    g_time.ns_last = ns;
  }
  refresh_ticks_from_ns(ns);
  return ns;
}

u64 time_ticks(void) {
  (void)time_now_ns();
  return g_time.ticks;
}

u64 time_hz(void) { return g_time.hz; }

u64 time_cycles_to_ns(u64 cycles) {
  if (g_time.clocksource.freq_hz == 0ULL) {
    return 0ULL;
  }
  return saturating_muldiv_u64(cycles, 1000000000ULL, g_time.clocksource.freq_hz);
}

u64 time_ns_to_cycles(u64 ns) {
  if (g_time.clocksource.freq_hz == 0ULL) {
    return 0ULL;
  }
  return saturating_muldiv_u64(ns, g_time.clocksource.freq_hz, 1000000000ULL);
}

status_t time_quality(time_quality_t *out) {
  if (out == (time_quality_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_time.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  out->stable = g_time.quality.stable;
  out->unstable = g_time.quality.unstable;
  out->emulated = g_time.quality.emulated;
  out->calibrated_hz = g_time.quality.calibrated_hz;
  out->drift_ppm_bound = g_time.quality.drift_ppm_bound;
  out->cross_cpu_checked = g_time.quality.cross_cpu_checked;
  out->cross_cpu_passed = g_time.quality.cross_cpu_passed;
  return STATUS_OK;
}

const clocksource_t *time_clocksource(void) { return &g_time.clocksource; }

const clockevent_t *time_clockevent(void) { return &g_time.clockevent; }
