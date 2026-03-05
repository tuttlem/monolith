#include "arch_timer.h"

#define RISCV64_TIMER_TARGET_HZ 100ULL
#define RISCV64_TIMER_VECTOR (32ULL + 5ULL)

status_t arch_timer_init(const boot_info_t *boot_info, u64 *out_hz, u64 *out_irq_vector) {
  if (boot_info == (const boot_info_t *)0 || out_hz == (u64 *)0 || out_irq_vector == (u64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }

  (void)RISCV64_TIMER_TARGET_HZ;
  (void)RISCV64_TIMER_VECTOR;
  *out_hz = 0ULL;
  *out_irq_vector = 0ULL;
  return STATUS_DEFERRED;
}

void arch_timer_ack(u64 vector) {
  (void)vector;
}

u64 arch_timer_clocksource_hz(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return 0ULL;
  }
  /*
   * In S-mode boots via OpenSBI, cycle/time counter accessibility can vary
   * with platform counter delegation. Keep the timer source conservative and
   * avoid direct CSR sampling here.
   */
  return 0ULL;
}
