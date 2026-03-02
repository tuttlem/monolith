#include "arch_syscall.h"
#include "interrupts.h"
#include "syscall.h"

#define RISCV64_SYSCALL_VECTOR 64ULL

static void riscv64_syscall_dispatch_fallback(void) {
  interrupt_frame_t frame;
  frame.arch_id = BOOT_INFO_ARCH_RISCV64;
  frame.vector = RISCV64_SYSCALL_VECTOR;
  frame.error_code = 0;
  frame.fault_addr = 0;
  frame.ip = 0;
  frame.sp = 0;
  frame.flags = 0;
  interrupts_dispatch(&frame);
}

status_t arch_syscall_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

status_t arch_syscall_get_vector(BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = RISCV64_SYSCALL_VECTOR;
  return STATUS_OK;
}

status_t arch_syscall_trigger(void) {
  BOOT_U64 old_sstatus;
  BOOT_U64 spin;

  __asm__ volatile("csrr %0, sstatus" : "=r"(old_sstatus));
  __asm__ volatile("csrsi sie, 2\n\t"
                   "csrsi sstatus, 2\n\t"
                   "csrsi sip, 2"
                   :
                   :
                   : "memory");
  for (spin = 0; spin < 2048ULL; ++spin) {
    if (!syscall_trap_mailbox_active()) {
      break;
    }
    __asm__ volatile("" : : : "memory");
  }
  __asm__ volatile("csrci sip, 2" : : : "memory");
  if (syscall_trap_mailbox_active()) {
    riscv64_syscall_dispatch_fallback();
  }
  if ((old_sstatus & 2ULL) == 0ULL) {
    __asm__ volatile("csrci sstatus, 2" : : : "memory");
  }
  return STATUS_OK;
}
