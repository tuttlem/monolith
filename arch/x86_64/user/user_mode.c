#include "arch_user_mode.h"

status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  (void)kernel_stack_top;
  return STATUS_NOT_SUPPORTED;
}

status_t arch_user_mode_prepare_frame(arch_user_frame_t *frame) {
  (void)frame;
  return STATUS_NOT_SUPPORTED;
}

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, BOOT_U64 user_sp) {
  (void)entry;
  (void)arg;
  (void)user_sp;
  for (;;) {
    arch_halt();
  }
}
