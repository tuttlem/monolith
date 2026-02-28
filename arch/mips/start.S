.set noreorder
.section .text
.globl _start
_start:
  lui $gp, %hi(__global_pointer$)
  addiu $gp, $gp, %lo(__global_pointer$)
  lui $sp, %hi(_stack_top)
  addiu $sp, $sp, %lo(_stack_top)
  jal arch_main
   nop
1:
  b 1b
   nop

.section .bss
.align 12
_stack:
  .space 16384
_stack_top:
