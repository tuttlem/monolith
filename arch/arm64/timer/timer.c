#include "arch_timer.h"
#include "irq_controller.h"

#define ARM64_TIMER_TARGET_HZ 100ULL
#define ARM64_TIMER_PPI_INTID 27ULL
#define ARM64_TIMER_VECTOR (32ULL + ARM64_TIMER_PPI_INTID)

static BOOT_U64 g_arm64_timer_reload = 0;

static BOOT_U64 arm64_read_cntfrq(void) {
  BOOT_U64 v;
  __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(v));
  return v;
}

static void arm64_write_cntv_tval(BOOT_U64 v) {
  __asm__ volatile("msr cntv_tval_el0, %0" : : "r"(v) : "memory");
}

static void arm64_write_cntv_ctl(BOOT_U64 v) {
  __asm__ volatile("msr cntv_ctl_el0, %0\n\t"
                   "isb\n\t"
                   :
                   : "r"(v)
                   : "memory");
}

status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector) {
  BOOT_U64 freq;
  BOOT_U64 reload;
  status_t st;

  if (boot_info == (const boot_info_t *)0 || out_hz == (BOOT_U64 *)0 || out_irq_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }

  freq = arm64_read_cntfrq();
  if (freq == 0ULL) {
    return STATUS_FAULT;
  }

  reload = freq / ARM64_TIMER_TARGET_HZ;
  if (reload == 0ULL) {
    reload = 1ULL;
  }

  g_arm64_timer_reload = reload;

  st = irq_controller_enable(ARM64_TIMER_PPI_INTID);
  if (st != STATUS_OK) {
    return st;
  }

  arm64_write_cntv_tval(reload);
  /* ENABLE=1, IMASK=0 */
  arm64_write_cntv_ctl(1ULL);

  *out_hz = ARM64_TIMER_TARGET_HZ;
  *out_irq_vector = ARM64_TIMER_VECTOR;
  return STATUS_OK;
}

void arch_timer_ack(BOOT_U64 vector) {
  if (vector != ARM64_TIMER_VECTOR || g_arm64_timer_reload == 0ULL) {
    return;
  }
  arm64_write_cntv_tval(g_arm64_timer_reload);
}

BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return 0ULL;
  }
  return arm64_read_cntfrq();
}
