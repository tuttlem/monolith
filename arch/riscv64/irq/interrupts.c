#include "arch_interrupts.h"
#include "arch/riscv64/irq_controller.h"
#include "interrupts.h"
#include "panic.h"
#include "syscall.h"

status_t arch_interrupts_init(const boot_info_t *boot_info) {
  extern void riscv64_trap_entry(void);
  BOOT_U64 stvec_base;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }

  stvec_base = (BOOT_U64)(BOOT_UPTR)&riscv64_trap_entry;
  __asm__ volatile("csrw stvec, %0" : : "r"(stvec_base) : "memory");
  return riscv64_irq_controller_init(boot_info);
}

void arch_interrupts_enable(void) { __asm__ volatile("csrsi sstatus, 2" : : : "memory"); }

void arch_interrupts_disable(void) { __asm__ volatile("csrci sstatus, 2" : : : "memory"); }

BOOT_U64 riscv64_trap_c(BOOT_U64 scause, BOOT_U64 sepc, BOOT_U64 stval, BOOT_U64 sstatus,
                        BOOT_U64 sp_at_trap) {
  interrupt_frame_t frame;
  BOOT_U64 is_interrupt = (scause >> 63) & 1ULL;
  BOOT_U64 cause_code = scause & ((1ULL << 63) - 1ULL);
  BOOT_U64 vector;

  if (is_interrupt != 0ULL) {
    if (cause_code == 1ULL && syscall_trap_mailbox_active()) {
      __asm__ volatile("csrci sip, 2" : : : "memory");
      vector = 64ULL;
    } else {
      vector = 32ULL + cause_code;
    }
  } else if (cause_code == 3ULL && syscall_trap_mailbox_active()) {
    vector = 64ULL;
  } else if (cause_code == 8ULL || cause_code == 9ULL || cause_code == 11ULL) {
    vector = 64ULL;
  } else {
    vector = cause_code;
  }

  frame.arch_id = BOOT_INFO_ARCH_RISCV64;
  frame.vector = vector;
  frame.error_code = scause;
  frame.fault_addr = stval;
  frame.ip = sepc;
  frame.sp = sp_at_trap;
  frame.flags = sstatus;
  interrupts_dispatch(&frame);

  /*
   * ECALL is synchronous and should resume at the next instruction.
   * If sepc is not advanced, the trap instruction is re-executed forever.
   */
  if (is_interrupt == 0ULL &&
      (cause_code == 3ULL || cause_code == 8ULL || cause_code == 9ULL || cause_code == 11ULL)) {
    return sepc + 4ULL;
  }

  return sepc;
}

void arch_exception_selftest_trigger(void) {
  __asm__ volatile("ebreak");
  panic("riscv64_exception_selftest_did_not_trap");
}
