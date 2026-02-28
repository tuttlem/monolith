.section .text
.global _start
_start:
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
