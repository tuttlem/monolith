.section .text
.align 11
.global arm64_vector_table

arm64_vector_table:
  b arm64_vec_curr_sp0_sync
  .space 124
  b arm64_vec_curr_sp0_irq
  .space 124
  b arm64_vec_curr_sp0_fiq
  .space 124
  b arm64_vec_curr_sp0_serror
  .space 124

  b arm64_vec_curr_spx_sync
  .space 124
  b arm64_vec_curr_spx_irq
  .space 124
  b arm64_vec_curr_spx_fiq
  .space 124
  b arm64_vec_curr_spx_serror
  .space 124

  b arm64_vec_lower_a64_sync
  .space 124
  b arm64_vec_lower_a64_irq
  .space 124
  b arm64_vec_lower_a64_fiq
  .space 124
  b arm64_vec_lower_a64_serror
  .space 124

  b arm64_vec_lower_a32_sync
  .space 124
  b arm64_vec_lower_a32_irq
  .space 124
  b arm64_vec_lower_a32_fiq
  .space 124
  b arm64_vec_lower_a32_serror
  .space 124

arm64_vec_curr_sp0_sync:
  b arm64_trap_common_sync
arm64_vec_curr_sp0_irq:
  b arm64_trap_common_irq
arm64_vec_curr_sp0_fiq:
  b arm64_trap_common_fiq
arm64_vec_curr_sp0_serror:
  b arm64_trap_common_serror

arm64_vec_curr_spx_sync:
  b arm64_trap_common_sync
arm64_vec_curr_spx_irq:
  b arm64_trap_common_irq
arm64_vec_curr_spx_fiq:
  b arm64_trap_common_fiq
arm64_vec_curr_spx_serror:
  b arm64_trap_common_serror

arm64_vec_lower_a64_sync:
  b arm64_trap_common_sync
arm64_vec_lower_a64_irq:
  b arm64_trap_common_irq
arm64_vec_lower_a64_fiq:
  b arm64_trap_common_fiq
arm64_vec_lower_a64_serror:
  b arm64_trap_common_serror

arm64_vec_lower_a32_sync:
  b arm64_trap_common_sync
arm64_vec_lower_a32_irq:
  b arm64_trap_common_irq
arm64_vec_lower_a32_fiq:
  b arm64_trap_common_fiq
arm64_vec_lower_a32_serror:
  b arm64_trap_common_serror

.macro ARM64_TRAP_SAVE_AND_CALL kind
  sub sp, sp, #256

  stp x0, x1, [sp, #0]
  stp x2, x3, [sp, #16]
  stp x4, x5, [sp, #32]
  stp x6, x7, [sp, #48]
  stp x8, x9, [sp, #64]
  stp x10, x11, [sp, #80]
  stp x12, x13, [sp, #96]
  stp x14, x15, [sp, #112]
  stp x16, x17, [sp, #128]
  stp x18, x19, [sp, #144]
  stp x20, x21, [sp, #160]
  stp x22, x23, [sp, #176]
  stp x24, x25, [sp, #192]
  stp x26, x27, [sp, #208]
  stp x28, x29, [sp, #224]
  str x30, [sp, #240]

  add x9, sp, #256
  str x9, [sp, #248]

  mov x0, #\kind
  mrs x1, esr_el1
  mrs x2, elr_el1
  mrs x3, spsr_el1
  mrs x4, far_el1
  ldr x5, [sp, #248]
  mov x6, sp
  bl arm64_trap_c
  msr elr_el1, x0

  ldp x0, x1, [sp, #0]
  ldp x2, x3, [sp, #16]
  ldp x4, x5, [sp, #32]
  ldp x6, x7, [sp, #48]
  ldp x8, x9, [sp, #64]
  ldp x10, x11, [sp, #80]
  ldp x12, x13, [sp, #96]
  ldp x14, x15, [sp, #112]
  ldp x16, x17, [sp, #128]
  ldp x18, x19, [sp, #144]
  ldp x20, x21, [sp, #160]
  ldp x22, x23, [sp, #176]
  ldp x24, x25, [sp, #192]
  ldp x26, x27, [sp, #208]
  ldp x28, x29, [sp, #224]
  ldr x30, [sp, #240]

  add sp, sp, #256
  eret
.endm

arm64_trap_common_sync:
  ARM64_TRAP_SAVE_AND_CALL 0
arm64_trap_common_irq:
  ARM64_TRAP_SAVE_AND_CALL 1
arm64_trap_common_fiq:
  ARM64_TRAP_SAVE_AND_CALL 2
arm64_trap_common_serror:
  ARM64_TRAP_SAVE_AND_CALL 3
