#include "arch_timer.h"

status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector) {
  if (boot_info == (const boot_info_t *)0 || out_hz == (BOOT_U64 *)0 || out_irq_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_SPARC64) {
    return STATUS_INVALID_ARG;
  }

  *out_hz = 0;
  *out_irq_vector = 0;
  return STATUS_DEFERRED;
}

void arch_timer_ack(BOOT_U64 vector) { (void)vector; }
