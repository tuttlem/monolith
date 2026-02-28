.section ".text.start"
.global _start
_start:
  sethi %hi(__prom_o0), %g2
  stx %o0, [%g2 + %lo(__prom_o0)]
  sethi %hi(__prom_o1), %g2
  stx %o1, [%g2 + %lo(__prom_o1)]
  sethi %hi(__prom_o2), %g2
  stx %o2, [%g2 + %lo(__prom_o2)]
  sethi %hi(__prom_o3), %g2
  stx %o3, [%g2 + %lo(__prom_o3)]
  sethi %hi(__prom_o4), %g2
  stx %o4, [%g2 + %lo(__prom_o4)]
  sethi %hi(__prom_o5), %g2
  stx %o5, [%g2 + %lo(__prom_o5)]
  sethi %hi(__prom_g7), %g2
  stx %g7, [%g2 + %lo(__prom_g7)]

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

.section ".data"
.align 8
.global __prom_o0
__prom_o0:
  .xword 0
.global __prom_o1
__prom_o1:
  .xword 0
.global __prom_o2
__prom_o2:
  .xword 0
.global __prom_o3
__prom_o3:
  .xword 0
.global __prom_o4
__prom_o4:
  .xword 0
.global __prom_o5
__prom_o5:
  .xword 0
.global __prom_g7
__prom_g7:
  .xword 0
