#include "arch_timer.h"
#include "irq_controller.h"

#define X86_64_PIT_CH0 0x40U
#define X86_64_PIT_CMD 0x43U
#define X86_64_PIT_HZ 1193182U
#define X86_64_TIMER_HZ 100U
#define X86_64_TIMER_IRQ 0ULL

static void outb(unsigned short port, unsigned char value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pit_program_periodic_100hz(void) {
  unsigned short divisor = (unsigned short)(X86_64_PIT_HZ / X86_64_TIMER_HZ);
  outb(X86_64_PIT_CMD, 0x36U);
  outb(X86_64_PIT_CH0, (unsigned char)(divisor & 0xFFU));
  outb(X86_64_PIT_CH0, (unsigned char)((divisor >> 8) & 0xFFU));
}

status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector) {
  BOOT_U64 vector;
  status_t st;

  if (boot_info == (const boot_info_t *)0 || out_hz == (BOOT_U64 *)0 || out_irq_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }

  st = irq_controller_map(X86_64_TIMER_IRQ, &vector);
  if (st != STATUS_OK) {
    return st;
  }
  st = irq_controller_enable(X86_64_TIMER_IRQ);
  if (st != STATUS_OK) {
    return st;
  }
  pit_program_periodic_100hz();

  *out_hz = X86_64_TIMER_HZ;
  *out_irq_vector = vector;
  return STATUS_OK;
}

void arch_timer_ack(BOOT_U64 vector) {
  BOOT_U64 irq = 0;
  if (irq_controller_vector_to_irq(vector, &irq) != STATUS_OK) {
    return;
  }
  irq_controller_eoi(irq);
}

BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return 0ULL;
  }
  return 0ULL;
}
