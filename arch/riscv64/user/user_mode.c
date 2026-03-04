#include "arch_user_mode.h"

static BOOT_U64 g_riscv64_kernel_stack_top;

status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  if (kernel_stack_top == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  g_riscv64_kernel_stack_top = (BOOT_U64)(BOOT_UPTR)kernel_stack_top;
  return STATUS_OK;
}

status_t arch_user_mode_prepare_frame(arch_user_frame_t *frame) {
  if (frame == (arch_user_frame_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (frame->user_ip == 0ULL || frame->user_sp == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  frame->user_sp &= ~0xFULL;
  return STATUS_OK;
}

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, BOOT_U64 user_sp) {
  BOOT_U64 sepc;
  BOOT_U64 sstatus;

  if (entry == (arch_user_entry_t)0 || user_sp == 0ULL || g_riscv64_kernel_stack_top == 0ULL) {
    for (;;) {
      arch_halt();
    }
  }

  user_sp &= ~0xFULL;
  sepc = (BOOT_U64)(BOOT_UPTR)entry;
  __asm__ volatile("csrr %0, sstatus" : "=r"(sstatus));
  sstatus &= ~(1ULL << 8); /* clear SPP => return to U-mode */

  __asm__ volatile("mv sp, %0\n\t"
                   "csrw sscratch, %1\n\t"
                   "mv a0, %2\n\t"
                   "csrw sepc, %3\n\t"
                   "csrw sstatus, %4\n\t"
                   "sret\n\t"
                   :
                   : "r"(user_sp), "r"(g_riscv64_kernel_stack_top), "r"(arg), "r"(sepc), "r"(sstatus)
                   : "a0", "memory");
  __builtin_unreachable();

  for (;;) {
    arch_halt();
  }
}
