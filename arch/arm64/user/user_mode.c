#include "arch_user_mode.h"

static u64 g_arm64_kernel_stack_top;

status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  if (kernel_stack_top == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  g_arm64_kernel_stack_top = (u64)(uptr)kernel_stack_top;
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

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, u64 user_sp) {
  u64 elr;
  u64 spsr;

  if (entry == (arch_user_entry_t)0 || user_sp == 0ULL || g_arm64_kernel_stack_top == 0ULL) {
    for (;;) {
      arch_halt();
    }
  }

  user_sp &= ~0xFULL;
  elr = (u64)(uptr)entry;
  spsr = 0ULL; /* EL0t with interrupts unmasked by default bring-up policy */

  __asm__ volatile("mov sp, %0\n\t"
                   "msr sp_el0, %1\n\t"
                   "mov x0, %2\n\t"
                   "msr elr_el1, %3\n\t"
                   "msr spsr_el1, %4\n\t"
                   "eret\n\t"
                   :
                   : "r"(g_arm64_kernel_stack_top), "r"(user_sp), "r"(arg), "r"(elr), "r"(spsr)
                   : "x0", "memory");
  __builtin_unreachable();

  for (;;) {
    arch_halt();
  }
}
