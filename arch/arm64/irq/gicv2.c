#include "arch/arm64/gicv2.h"
#include "interrupts.h"
#include "irq_controller.h"

#define ARM64_GICD_BASE 0x08000000ULL
#define ARM64_GICC_BASE 0x08010000ULL

#define ARM64_GICD_CTLR 0x000U
#define ARM64_GICD_ISENABLER0 0x100U
#define ARM64_GICD_ICENABLER0 0x180U
#define ARM64_GICC_CTLR 0x000U
#define ARM64_GICC_PMR 0x004U
#define ARM64_GICC_IAR 0x00CU
#define ARM64_GICC_EOIR 0x010U

static BOOT_U32 g_last_iar = 1023U;

static BOOT_U32 arm64_mmio_read32(BOOT_U64 addr) {
  return *(volatile BOOT_U32 *)(BOOT_UPTR)addr;
}

static void arm64_mmio_write32(BOOT_U64 addr, BOOT_U32 value) {
  *(volatile BOOT_U32 *)(BOOT_UPTR)addr = value;
}

static status_t gic_enable_irq(BOOT_U64 irq) {
  BOOT_U64 reg = irq / 32ULL;
  BOOT_U32 bit = (BOOT_U32)(1U << (irq % 32ULL));
  arm64_mmio_write32(ARM64_GICD_BASE + ARM64_GICD_ISENABLER0 + (BOOT_U32)(reg * 4ULL), bit);
  return STATUS_OK;
}

static status_t gic_disable_irq(BOOT_U64 irq) {
  BOOT_U64 reg = irq / 32ULL;
  BOOT_U32 bit = (BOOT_U32)(1U << (irq % 32ULL));
  arm64_mmio_write32(ARM64_GICD_BASE + ARM64_GICD_ICENABLER0 + (BOOT_U32)(reg * 4ULL), bit);
  return STATUS_OK;
}

static void gic_ack_irq(BOOT_U64 irq) { (void)irq; }

static void gic_eoi_irq(BOOT_U64 irq) {
  (void)irq;
  arm64_mmio_write32(ARM64_GICC_BASE + ARM64_GICC_EOIR, g_last_iar);
}

static status_t gic_map_irq(BOOT_U64 irq, BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0 || irq >= (INTERRUPT_MAX_VECTORS - 32ULL)) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = 32ULL + irq;
  return STATUS_OK;
}

static status_t gic_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq) {
  if (out_irq == (BOOT_U64 *)0 || vector < 32ULL || vector >= INTERRUPT_MAX_VECTORS) {
    return STATUS_INVALID_ARG;
  }
  *out_irq = vector - 32ULL;
  return STATUS_OK;
}

status_t arm64_gicv2_controller_init(const boot_info_t *boot_info) {
  BOOT_U32 v;
  irq_controller_ops_t ops;
  status_t st;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }

  v = arm64_mmio_read32(ARM64_GICD_BASE + ARM64_GICD_CTLR);
  v |= 1U;
  arm64_mmio_write32(ARM64_GICD_BASE + ARM64_GICD_CTLR, v);

  arm64_mmio_write32(ARM64_GICC_BASE + ARM64_GICC_PMR, 0xFFU);
  v = arm64_mmio_read32(ARM64_GICC_BASE + ARM64_GICC_CTLR);
  v |= 1U;
  arm64_mmio_write32(ARM64_GICC_BASE + ARM64_GICC_CTLR, v);

  ops.enable_irq = gic_enable_irq;
  ops.disable_irq = gic_disable_irq;
  ops.ack_irq = gic_ack_irq;
  ops.eoi_irq = gic_eoi_irq;
  ops.map_irq = gic_map_irq;
  ops.vector_to_irq = gic_vector_to_irq;
  st = irq_controller_register("arm64-gicv2", &ops);
  return st;
}

status_t arm64_gicv2_claim_irq(BOOT_U64 *out_irq) {
  BOOT_U32 iar;
  BOOT_U32 intid;

  if (out_irq == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }

  iar = arm64_mmio_read32(ARM64_GICC_BASE + ARM64_GICC_IAR);
  intid = iar & 0x3FFU;
  g_last_iar = iar;
  if (intid >= 1020U) {
    return STATUS_NOT_FOUND;
  }
  *out_irq = (BOOT_U64)intid;
  return STATUS_OK;
}

void arm64_gicv2_eoi_irq(BOOT_U64 irq) { gic_eoi_irq(irq); }
