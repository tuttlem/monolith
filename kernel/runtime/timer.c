#include "clock.h"
#include "power_domain.h"
#include "reset.h"
#include "timer.h"
#include "timebase.h"

status_t timer_init(const boot_info_t *boot_info) { return time_init(boot_info); }

BOOT_U64 timer_ticks(void) { return time_ticks(); }

BOOT_U64 timer_hz(void) { return time_hz(); }

#define CLOCK_CTRL_MAX 32U
#define RESET_CTRL_MAX 32U
#define POWER_DOMAIN_MAX 16U

static BOOT_U64 g_clock_enabled[CLOCK_CTRL_MAX];
static BOOT_U64 g_clock_rate_hz[CLOCK_CTRL_MAX];
static BOOT_U64 g_reset_asserted[RESET_CTRL_MAX];
static BOOT_U64 g_power_on[POWER_DOMAIN_MAX];

status_t clock_enable(clock_id_t clk) {
  if (clk >= CLOCK_CTRL_MAX) {
    return STATUS_NOT_FOUND;
  }
  g_clock_enabled[clk] = 1ULL;
  return STATUS_OK;
}

status_t clock_disable(clock_id_t clk) {
  if (clk >= CLOCK_CTRL_MAX) {
    return STATUS_NOT_FOUND;
  }
  g_clock_enabled[clk] = 0ULL;
  return STATUS_OK;
}

status_t clock_set_rate(clock_id_t clk, BOOT_U64 hz) {
  if (clk >= CLOCK_CTRL_MAX || hz == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  g_clock_rate_hz[clk] = hz;
  return STATUS_OK;
}

status_t clock_get_rate(clock_id_t clk, BOOT_U64 *out_hz) {
  if (clk >= CLOCK_CTRL_MAX || out_hz == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_hz = g_clock_rate_hz[clk];
  return (g_clock_rate_hz[clk] == 0ULL) ? STATUS_DEFERRED : STATUS_OK;
}

status_t reset_assert(reset_id_t rst) {
  if (rst >= RESET_CTRL_MAX) {
    return STATUS_NOT_FOUND;
  }
  g_reset_asserted[rst] = 1ULL;
  return STATUS_OK;
}

status_t reset_deassert(reset_id_t rst) {
  if (rst >= RESET_CTRL_MAX) {
    return STATUS_NOT_FOUND;
  }
  g_reset_asserted[rst] = 0ULL;
  return STATUS_OK;
}

status_t reset_pulse(reset_id_t rst) {
  status_t st = reset_assert(rst);
  if (st != STATUS_OK) {
    return st;
  }
  return reset_deassert(rst);
}

status_t power_domain_on(power_domain_id_t pd) {
  if (pd >= POWER_DOMAIN_MAX) {
    return STATUS_NOT_FOUND;
  }
  g_power_on[pd] = 1ULL;
  return STATUS_OK;
}

status_t power_domain_off(power_domain_id_t pd) {
  if (pd >= POWER_DOMAIN_MAX) {
    return STATUS_NOT_FOUND;
  }
  g_power_on[pd] = 0ULL;
  return STATUS_OK;
}

status_t power_domain_status(power_domain_id_t pd, BOOT_U64 *out_on) {
  if (pd >= POWER_DOMAIN_MAX || out_on == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_on = g_power_on[pd];
  return STATUS_OK;
}
