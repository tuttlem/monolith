#include "arch_interrupts.h"
#include "arch_cpu.h"
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
                        BOOT_U64 sp_at_trap, BOOT_U64 regs_base) {
  interrupt_frame_t frame;
  BOOT_U64 is_interrupt = (scause >> 63) & 1ULL;
  BOOT_U64 cause_code = scause & ((1ULL << 63) - 1ULL);
  BOOT_U64 vector;
  BOOT_U64 next_sepc = sepc;
  int syscall_trap = 0;

  if (is_interrupt != 0ULL) {
    if (cause_code == 1ULL && syscall_trap_mailbox_active()) {
      __asm__ volatile("csrci sip, 2" : : : "memory");
      (void)syscall_trap_mailbox_consume();
      vector = 64ULL;
      syscall_trap = 1;
    } else {
      vector = 32ULL + cause_code;
    }
  } else if (cause_code == 2ULL && syscall_trap_mailbox_active()) {
    (void)syscall_trap_mailbox_consume();
    next_sepc = sepc + 4ULL;
    syscall_trap = 1;
    vector = 64ULL;
  } else if (cause_code == 8ULL && ((sstatus & 0x100ULL) == 0ULL)) {
    BOOT_U64 *regs = (BOOT_U64 *)(BOOT_UPTR)regs_base;
    BOOT_U64 ret = 0ULL;
    status_t st;
    if (regs != (BOOT_U64 *)0) {
      st = syscall_handle_user_trap(regs[15], regs[8], regs[9], regs[10], regs[11], regs[12], regs[13], &ret);
      if (st != STATUS_OK && ret == 0ULL) {
        ret = (BOOT_U64)st;
      }
      regs[8] = ret;
    }
    next_sepc = sepc + 4ULL;
    syscall_trap = 1;
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
  if (!syscall_trap) {
    interrupts_dispatch(&frame);
  }

  if (syscall_trap) {
    return next_sepc;
  }

  return sepc;
}

void arch_exception_selftest_trigger(void) {
  __asm__ volatile("ebreak");
  panic("riscv64_exception_selftest_did_not_trap");
}
