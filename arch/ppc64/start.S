.section ".text.start","ax",@progbits
.globl _start
_start:
  lis 10, __opal_base@ha
  std 8, __opal_base@l(10)
  lis 10, __opal_entry@ha
  std 9, __opal_entry@l(10)

  lis 1, _stack_top@ha
  addi 1, 1, _stack_top@l
  bl 0f
0:
  mflr 12
  addis 2, 12, (.TOC. - 0b)@ha
  addi 2, 2, (.TOC. - 0b)@l
  addis 12, 12, (arch_main + 8 - 0b)@ha
  addi 12, 12, (arch_main + 8 - 0b)@l
  mtctr 12
  bctrl
1:
  b 1b

.globl opal_console_write_call
opal_console_write_call:
  mr 10, 2
  lis 11, __opal_base@ha
  ld 11, __opal_base@l(11)
  lis 12, __opal_entry@ha
  ld 12, __opal_entry@l(12)
  mr 2, 11
  li 0, 1
  mtctr 12
  bctrl
  mr 2, 10
  blr

.section ".data","aw",@progbits
.balign 8
.globl __opal_base
__opal_base:
  .quad 0
.globl __opal_entry
__opal_entry:
  .quad 0

.section ".bss","aw",@nobits
.balign 16
_stack:
  .space 16384
_stack_top:
