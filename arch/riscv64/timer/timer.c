#include "arch_timer.h"

#define RISCV64_TIMER_TARGET_HZ 100ULL
#define RISCV64_TIMER_VECTOR (32ULL + 5ULL)
#define RISCV64_TIMEBASE_HZ 10000000ULL

status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector) {
  if (boot_info == (const boot_info_t *)0 || out_hz == (BOOT_U64 *)0 || out_irq_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }

  /*
   * Keep timer API live with clocksource-backed time until a verified
   * platform-specific interrupt event path is selected.
   */
  *out_hz = RISCV64_TIMER_TARGET_HZ;
  *out_irq_vector = RISCV64_TIMER_VECTOR;
  return STATUS_OK;
}

void arch_timer_ack(BOOT_U64 vector) {
  (void)vector;
}

BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return 0ULL;
  }
  return RISCV64_TIMEBASE_HZ;
}
