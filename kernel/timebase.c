#include "timebase.h"
#include "arch_cpu.h"
#include "arch_timer.h"
#include "interrupts.h"
#include "irq_controller.h"

typedef struct {
  BOOT_U64 initialized;
  BOOT_U64 hz;
  BOOT_U64 irq_vector;
  volatile BOOT_U64 ticks;
  BOOT_U64 cycle_base;
  BOOT_U64 ns_last;
  clocksource_t clocksource;
  clockevent_t clockevent;
} time_state_t;

static time_state_t g_time;

static BOOT_U64 saturating_muldiv_u64(BOOT_U64 value, BOOT_U64 mul, BOOT_U64 div) {
  BOOT_U64 q;
  BOOT_U64 r;
  BOOT_U64 hi;
  BOOT_U64 lo;
  BOOT_U64 max = ~0ULL;

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

static void refresh_ticks_from_ns(BOOT_U64 ns) {
  BOOT_U64 derived;
  if (g_time.hz == 0ULL) {
    return;
  }
  derived = saturating_muldiv_u64(ns, g_time.hz, 1000000000ULL);
  if (derived > g_time.ticks) {
    g_time.ticks = derived;
  }
}

static status_t clockevent_set_periodic_compat(BOOT_U64 hz) {
  if (hz == g_time.hz) {
    return STATUS_OK;
  }
  return STATUS_NOT_SUPPORTED;
}

static status_t clockevent_set_oneshot_compat(BOOT_U64 delta_ns) {
  (void)delta_ns;
  return STATUS_NOT_SUPPORTED;
}

static void clockevent_ack_compat(BOOT_U64 vector) { arch_timer_ack(vector); }

static void time_irq_handler(const interrupt_frame_t *frame, void *ctx) {
  (void)ctx;
  if (frame == (const interrupt_frame_t *)0) {
    return;
  }

  g_time.ticks += 1ULL;
  if (g_time.clockevent.ack != (void (*)(BOOT_U64))0) {
    g_time.clockevent.ack(frame->vector);
  }
}

status_t time_init(const boot_info_t *boot_info) {
  BOOT_U64 hz = 0;
  BOOT_U64 vector = 0;
  BOOT_U64 irq = 0;
  BOOT_U64 cycle_hz = 0;
  status_t st;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  g_time.initialized = 0;
  g_time.hz = 0;
  g_time.irq_vector = 0;
  g_time.ticks = 0;
  g_time.cycle_base = 0;
  g_time.ns_last = 0;
  g_time.clocksource.name = "none";
  g_time.clocksource.freq_hz = 0;
  g_time.clocksource.read_cycles = (BOOT_U64(*)(void))0;
  g_time.clockevent.name = "none";
  g_time.clockevent.set_periodic = (status_t(*)(BOOT_U64))0;
  g_time.clockevent.set_oneshot_ns = (status_t(*)(BOOT_U64))0;
  g_time.clockevent.ack = (void (*)(BOOT_U64))0;

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
  } else {
    g_time.clocksource.name = "tick-fallback";
    g_time.clocksource.freq_hz = hz;
    g_time.clocksource.read_cycles = (BOOT_U64(*)(void))0;
    g_time.cycle_base = 0;
  }

  g_time.clockevent.name = "arch-timer";
  g_time.clockevent.set_periodic = clockevent_set_periodic_compat;
  g_time.clockevent.set_oneshot_ns = clockevent_set_oneshot_compat;
  g_time.clockevent.ack = clockevent_ack_compat;

  g_time.hz = hz;
  g_time.irq_vector = vector;
  g_time.initialized = 1ULL;
  interrupts_enable();
  return STATUS_OK;
}

BOOT_U64 time_now_ns(void) {
  BOOT_U64 ns;
  BOOT_U64 ticks;
  BOOT_U64 hz;

  if (g_time.initialized == 0) {
    return 0ULL;
  }

  if (g_time.clocksource.read_cycles != (BOOT_U64(*)(void))0 && g_time.clocksource.freq_hz != 0ULL) {
    BOOT_U64 cycles = g_time.clocksource.read_cycles();
    BOOT_U64 delta = cycles - g_time.cycle_base;
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

BOOT_U64 time_ticks(void) {
  (void)time_now_ns();
  return g_time.ticks;
}

BOOT_U64 time_hz(void) { return g_time.hz; }

const clocksource_t *time_clocksource(void) { return &g_time.clocksource; }

const clockevent_t *time_clockevent(void) { return &g_time.clockevent; }
