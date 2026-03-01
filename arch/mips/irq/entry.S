.set noreorder
.set noat
.section .text

.align 12
.globl mips_vector_base
mips_vector_base:
.rept (0x180 / 4)
  nop
.endr
  j mips_trap_entry
   nop

.globl mips_trap_entry
mips_trap_entry:
  addiu $sp, $sp, -152
  sw $ra,   0($sp)
  sw $at,   4($sp)
  sw $v0,   8($sp)
  sw $v1,  12($sp)
  sw $a0,  16($sp)
  sw $a1,  20($sp)
  sw $a2,  24($sp)
  sw $a3,  28($sp)
  sw $t0,  32($sp)
  sw $t1,  36($sp)
  sw $t2,  40($sp)
  sw $t3,  44($sp)
  sw $t4,  48($sp)
  sw $t5,  52($sp)
  sw $t6,  56($sp)
  sw $t7,  60($sp)
  sw $s0,  64($sp)
  sw $s1,  68($sp)
  sw $s2,  72($sp)
  sw $s3,  76($sp)
  sw $s4,  80($sp)
  sw $s5,  84($sp)
  sw $s6,  88($sp)
  sw $s7,  92($sp)
  sw $t8,  96($sp)
  sw $t9, 100($sp)
  sw $gp, 104($sp)
  sw $fp, 108($sp)
  sw $k0, 112($sp)
  sw $k1, 116($sp)
  mfhi $t0
  mflo $t1
  sw $t0, 120($sp)
  sw $t1, 124($sp)
  sw $zero, 128($sp)
  sw $zero, 132($sp)
  sw $zero, 136($sp)
  sw $zero, 140($sp)
  sw $zero, 144($sp)
  sw $zero, 148($sp)

  mfc0 $a0, $13
  mfc0 $a1, $14
  mfc0 $a2, $8
  mfc0 $a3, $12
  jal mips_trap_c
   nop
  mtc0 $v0, $14

  lw $ra,   0($sp)
  lw $at,   4($sp)
  lw $v0,   8($sp)
  lw $v1,  12($sp)
  lw $a0,  16($sp)
  lw $a1,  20($sp)
  lw $a2,  24($sp)
  lw $a3,  28($sp)
  lw $t0,  32($sp)
  lw $t1,  36($sp)
  lw $t2,  40($sp)
  lw $t3,  44($sp)
  lw $t4,  48($sp)
  lw $t5,  52($sp)
  lw $t6,  56($sp)
  lw $t7,  60($sp)
  lw $s0,  64($sp)
  lw $s1,  68($sp)
  lw $s2,  72($sp)
  lw $s3,  76($sp)
  lw $s4,  80($sp)
  lw $s5,  84($sp)
  lw $s6,  88($sp)
  lw $s7,  92($sp)
  lw $t8,  96($sp)
  lw $t9, 100($sp)
  lw $gp, 104($sp)
  lw $fp, 108($sp)
  lw $k0, 112($sp)
  lw $k1, 116($sp)
  lw $t0, 120($sp)
  lw $t1, 124($sp)
  mthi $t0
  mtlo $t1

  addiu $sp, $sp, 152
  eret
