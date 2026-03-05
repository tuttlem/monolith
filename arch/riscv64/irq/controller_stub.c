#include "arch/riscv64/irq_controller.h"
#include "interrupts.h"
#include "irq_controller.h"

#define RISCV64_IRQ_SUPERVISOR_TIMER 5ULL
#define RISCV64_IRQ_SUPERVISOR_EXTERNAL 9ULL
#define RISCV64_SIE_STIE (1ULL << 5)
#define RISCV64_SIE_SEIE (1ULL << 9)

static status_t riscv64_irq_enable(u64 irq) {
  u64 mask = 0;
  if (irq == RISCV64_IRQ_SUPERVISOR_TIMER) {
    mask = RISCV64_SIE_STIE;
  } else if (irq == RISCV64_IRQ_SUPERVISOR_EXTERNAL) {
    mask = RISCV64_SIE_SEIE;
  } else {
    return STATUS_NOT_SUPPORTED;
  }

  __asm__ volatile("csrs sie, %0" : : "r"(mask) : "memory");
  return STATUS_OK;
}

static status_t riscv64_irq_disable(u64 irq) {
  u64 mask = 0;
  if (irq == RISCV64_IRQ_SUPERVISOR_TIMER) {
    mask = RISCV64_SIE_STIE;
  } else if (irq == RISCV64_IRQ_SUPERVISOR_EXTERNAL) {
    mask = RISCV64_SIE_SEIE;
  } else {
    return STATUS_NOT_SUPPORTED;
  }

  __asm__ volatile("csrc sie, %0" : : "r"(mask) : "memory");
  return STATUS_OK;
}

static void riscv64_irq_ack(u64 irq) { (void)irq; }

static void riscv64_irq_eoi(u64 irq) { (void)irq; }

static status_t riscv64_map_irq(u64 irq, u64 *out_vector) {
  if (out_vector == (u64 *)0 || irq >= (INTERRUPT_MAX_VECTORS - 32ULL)) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = 32ULL + irq;
  return STATUS_OK;
}

static status_t riscv64_vector_to_irq(u64 vector, u64 *out_irq) {
  if (out_irq == (u64 *)0 || vector < 32ULL || vector >= INTERRUPT_MAX_VECTORS) {
    return STATUS_INVALID_ARG;
  }
  *out_irq = vector - 32ULL;
  return STATUS_OK;
}

status_t riscv64_irq_controller_init(const boot_info_t *boot_info) {
  irq_controller_ops_t ops;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }

  ops.enable_irq = riscv64_irq_enable;
  ops.disable_irq = riscv64_irq_disable;
  ops.ack_irq = riscv64_irq_ack;
  ops.eoi_irq = riscv64_irq_eoi;
  ops.map_irq = riscv64_map_irq;
  ops.vector_to_irq = riscv64_vector_to_irq;
  return irq_controller_register("riscv64-stub", &ops);
}
