#include "arch_interrupts.h"
#include "interrupts.h"

#define ARM64_TRAP_SYNC 0ULL
#define ARM64_TRAP_IRQ 1ULL
#define ARM64_TRAP_FIQ 2ULL
#define ARM64_TRAP_SERROR 3ULL

#define ARM64_GICD_BASE 0x08000000ULL
#define ARM64_GICC_BASE 0x08010000ULL
#define ARM64_GIC_TIMER_PPI 27U

#define ARM64_GICD_CTLR 0x000U
#define ARM64_GICD_ISENABLER0 0x100U
#define ARM64_GICC_CTLR 0x000U
#define ARM64_GICC_PMR 0x004U
#define ARM64_GICC_IAR 0x00CU
#define ARM64_GICC_EOIR 0x010U

static BOOT_U32 arm64_mmio_read32(BOOT_U64 addr) {
  return *(volatile BOOT_U32 *)(BOOT_UPTR)addr;
}

static void arm64_mmio_write32(BOOT_U64 addr, BOOT_U32 value) {
  *(volatile BOOT_U32 *)(BOOT_UPTR)addr = value;
}

static void arm64_gic_cpuif_init(void) {
  BOOT_U32 v;

  /* Enable distributor. */
  v = arm64_mmio_read32(ARM64_GICD_BASE + ARM64_GICD_CTLR);
  v |= 1U;
  arm64_mmio_write32(ARM64_GICD_BASE + ARM64_GICD_CTLR, v);

  /* Enable timer PPI 27 (virtual timer) in banked ISENABLER0 for this CPU. */
  arm64_mmio_write32(ARM64_GICD_BASE + ARM64_GICD_ISENABLER0, (1U << ARM64_GIC_TIMER_PPI));

  /* Enable CPU interface and accept all priorities. */
  arm64_mmio_write32(ARM64_GICC_BASE + ARM64_GICC_PMR, 0xFFU);
  v = arm64_mmio_read32(ARM64_GICC_BASE + ARM64_GICC_CTLR);
  v |= 1U;
  arm64_mmio_write32(ARM64_GICC_BASE + ARM64_GICC_CTLR, v);
}

static BOOT_U32 arm64_gic_read_iar(void) { return arm64_mmio_read32(ARM64_GICC_BASE + ARM64_GICC_IAR); }

static void arm64_gic_write_eoir(BOOT_U32 iar) { arm64_mmio_write32(ARM64_GICC_BASE + ARM64_GICC_EOIR, iar); }

status_t arch_interrupts_init(const boot_info_t *boot_info) {
  extern char arm64_vector_table[];
  BOOT_U64 vbar;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }

  vbar = (BOOT_U64)(BOOT_UPTR)arm64_vector_table;
  __asm__ volatile("msr vbar_el1, %0\n\t"
                   "isb\n\t"
                   :
                   : "r"(vbar)
                   : "memory");

  arm64_gic_cpuif_init();
  return STATUS_OK;
}

void arch_interrupts_enable(void) { __asm__ volatile("msr daifclr, #2" : : : "memory"); }

void arch_interrupts_disable(void) { __asm__ volatile("msr daifset, #2" : : : "memory"); }

BOOT_U64 arm64_trap_c(BOOT_U64 trap_kind, BOOT_U64 esr, BOOT_U64 elr, BOOT_U64 spsr, BOOT_U64 far,
  BOOT_U64 sp_at_trap) {
  interrupt_frame_t frame;
  BOOT_U32 iar = 0;
  BOOT_U64 intid = 0;
  BOOT_U64 vector = 0;
  BOOT_U64 kind = trap_kind & 3ULL;

  if (kind == ARM64_TRAP_IRQ) {
    iar = arm64_gic_read_iar();
    intid = (BOOT_U64)(iar & 0x3FFU);

    if (intid < 1020ULL) {
      if (intid < (INTERRUPT_MAX_VECTORS - 32ULL)) {
        vector = 32ULL + intid;
      } else {
        vector = INTERRUPT_MAX_VECTORS - 1ULL;
      }

      frame.arch_id = BOOT_INFO_ARCH_ARM64;
      frame.vector = vector;
      frame.error_code = esr;
      frame.fault_addr = far;
      frame.ip = elr;
      frame.sp = sp_at_trap;
      frame.flags = spsr;
      interrupts_dispatch(&frame);
    }

    arm64_gic_write_eoir(iar);
    return elr;
  } else if (kind == ARM64_TRAP_FIQ) {
    vector = 33;
  } else if (kind == ARM64_TRAP_SERROR) {
    vector = 2;
  } else {
    vector = 0;
  }

  frame.arch_id = BOOT_INFO_ARCH_ARM64;
  frame.vector = vector;
  frame.error_code = esr;
  frame.fault_addr = far;
  frame.ip = elr;
  frame.sp = sp_at_trap;
  frame.flags = spsr;
  interrupts_dispatch(&frame);
  return elr;
}

void arch_exception_selftest_trigger(void) {
  __asm__ volatile("brk #0");
  for (;;) {
    arch_halt();
  }
}
