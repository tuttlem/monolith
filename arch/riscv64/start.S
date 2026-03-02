.section .text
.global _start
_start:
  bnez a0, 1f
  la sp, _stack_top
  call arch_main
1:
  wfi
  j 1b

.section .bss
.align 12
_stack:
  .space 16384
_stack_top:
