#include "arch_timer.h"

#define ARM64_TIMER_TARGET_HZ 100ULL
#define ARM64_TIMER_PPI_INTID 27ULL
#define ARM64_TIMER_VECTOR (32ULL + ARM64_TIMER_PPI_INTID)

static u64 g_arm64_timer_reload = 0;

static u64 arm64_read_cntfrq(void) {
  u64 v;
  __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(v));
  return v;
}

static void arm64_write_cntv_tval(u64 v) {
  __asm__ volatile("msr cntv_tval_el0, %0" : : "r"(v) : "memory");
}

static void arm64_write_cntv_ctl(u64 v) {
  __asm__ volatile("msr cntv_ctl_el0, %0\n\t"
                   "isb\n\t"
                   :
                   : "r"(v)
                   : "memory");
}

status_t arch_timer_init(const boot_info_t *boot_info, u64 *out_hz, u64 *out_irq_vector) {
  u64 freq;
  u64 reload;

  if (boot_info == (const boot_info_t *)0 || out_hz == (u64 *)0 || out_irq_vector == (u64 *)0) {
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

  arm64_write_cntv_tval(reload);
  /* ENABLE=1, IMASK=0 */
  arm64_write_cntv_ctl(1ULL);

  *out_hz = ARM64_TIMER_TARGET_HZ;
  *out_irq_vector = ARM64_TIMER_VECTOR;
  return STATUS_OK;
}

void arch_timer_ack(u64 vector) {
  if (vector != ARM64_TIMER_VECTOR || g_arm64_timer_reload == 0ULL) {
    return;
  }
  arm64_write_cntv_tval(g_arm64_timer_reload);
}

u64 arch_timer_clocksource_hz(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return 0ULL;
  }
  return arm64_read_cntfrq();
}
