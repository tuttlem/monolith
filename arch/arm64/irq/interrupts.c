#include "arch_interrupts.h"
#include "arch/arm64/gicv2.h"
#include "interrupts.h"
#include "irq_controller.h"
#include "panic.h"

#define ARM64_TRAP_SYNC 0ULL
#define ARM64_TRAP_IRQ 1ULL
#define ARM64_TRAP_FIQ 2ULL
#define ARM64_TRAP_SERROR 3ULL

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

  return arm64_gicv2_controller_init(boot_info);
}

void arch_interrupts_enable(void) { __asm__ volatile("msr daifclr, #2" : : : "memory"); }

void arch_interrupts_disable(void) { __asm__ volatile("msr daifset, #2" : : : "memory"); }

BOOT_U64 arm64_trap_c(BOOT_U64 trap_kind, BOOT_U64 esr, BOOT_U64 elr, BOOT_U64 spsr, BOOT_U64 far,
  BOOT_U64 sp_at_trap) {
  interrupt_frame_t frame;
  BOOT_U64 irq = 0;
  BOOT_U64 vector = 0;
  BOOT_U64 kind = trap_kind & 3ULL;
  status_t st;

  if (kind == ARM64_TRAP_IRQ) {
    st = arm64_gicv2_claim_irq(&irq);
    if (st != STATUS_OK) {
      return elr;
    }
    st = irq_controller_map(irq, &vector);
    if (st != STATUS_OK) {
      arm64_gicv2_eoi_irq(irq);
      return elr;
    }

    frame.arch_id = BOOT_INFO_ARCH_ARM64;
    frame.vector = vector;
    frame.error_code = esr;
    frame.fault_addr = far;
    frame.ip = elr;
    frame.sp = sp_at_trap;
    frame.flags = spsr;
    interrupts_dispatch(&frame);
    arm64_gicv2_eoi_irq(irq);
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
  panic("arm64_exception_selftest_did_not_trap");
}
