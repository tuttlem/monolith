#include "arch_user_mode.h"

static BOOT_U64 g_x86_64_kernel_stack_top;

status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  if (kernel_stack_top == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  g_x86_64_kernel_stack_top = (BOOT_U64)(BOOT_UPTR)kernel_stack_top;
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
  if (entry == (arch_user_entry_t)0 || user_sp == 0ULL || g_x86_64_kernel_stack_top == 0ULL) {
    for (;;) {
      arch_halt();
    }
  }

  user_sp &= ~0xFULL;
  __asm__ volatile("mov %0, %%rsp\n\t"
                   "mov %1, %%rdi\n\t"
                   "call *%2\n\t"
                   :
                   : "r"(user_sp), "r"(arg), "r"(entry)
                   : "rdi", "memory");
  for (;;) {
    arch_halt();
  }
}
