#include "arch/riscv64/irq_controller.h"
#include "interrupts.h"
#include "irq_controller.h"

static status_t riscv64_irq_enable(BOOT_U64 irq) {
  (void)irq;
  return STATUS_DEFERRED;
}

static status_t riscv64_irq_disable(BOOT_U64 irq) {
  (void)irq;
  return STATUS_DEFERRED;
}

static void riscv64_irq_ack(BOOT_U64 irq) { (void)irq; }

static void riscv64_irq_eoi(BOOT_U64 irq) { (void)irq; }

static status_t riscv64_map_irq(BOOT_U64 irq, BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0 || irq >= (INTERRUPT_MAX_VECTORS - 32ULL)) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = 32ULL + irq;
  return STATUS_OK;
}

static status_t riscv64_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq) {
  if (out_irq == (BOOT_U64 *)0 || vector < 32ULL || vector >= INTERRUPT_MAX_VECTORS) {
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
