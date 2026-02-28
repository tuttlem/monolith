.section ".text"
.global _start
_start:
  sethi %hi(_stack_top), %g1
  or %g1, %lo(_stack_top), %g1
  sub %g1, 2047, %sp
  call arch_main
   nop
1:
  ba 1b
   nop

.section ".bss"
.align 16
_stack:
  .skip 16384
_stack_top:
