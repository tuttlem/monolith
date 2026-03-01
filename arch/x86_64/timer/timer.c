#include "arch_timer.h"

#define X86_64_PIC1_CMD 0x20U
#define X86_64_PIC1_DATA 0x21U
#define X86_64_PIC2_CMD 0xA0U
#define X86_64_PIC2_DATA 0xA1U
#define X86_64_PIC_EOI 0x20U

#define X86_64_PIT_CH0 0x40U
#define X86_64_PIT_CMD 0x43U
#define X86_64_PIT_HZ 1193182U
#define X86_64_TIMER_HZ 100U
#define X86_64_TIMER_VECTOR 32ULL

static void outb(unsigned short port, unsigned char value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void io_wait(void) { outb(0x80U, 0U); }

static void pic_remap(void) {
  unsigned char pic1_mask = 0xFFU;
  unsigned char pic2_mask = 0xFFU;

  outb(X86_64_PIC1_CMD, 0x11U);
  io_wait();
  outb(X86_64_PIC2_CMD, 0x11U);
  io_wait();

  outb(X86_64_PIC1_DATA, 0x20U);
  io_wait();
  outb(X86_64_PIC2_DATA, 0x28U);
  io_wait();

  outb(X86_64_PIC1_DATA, 0x04U);
  io_wait();
  outb(X86_64_PIC2_DATA, 0x02U);
  io_wait();

  outb(X86_64_PIC1_DATA, 0x01U);
  io_wait();
  outb(X86_64_PIC2_DATA, 0x01U);
  io_wait();

  outb(X86_64_PIC1_DATA, pic1_mask);
  outb(X86_64_PIC2_DATA, pic2_mask);
}

static void pic_unmask_irq0_only(void) {
  outb(X86_64_PIC1_DATA, 0xFEU);
  outb(X86_64_PIC2_DATA, 0xFFU);
}

static void pit_program_periodic_100hz(void) {
  unsigned short divisor = (unsigned short)(X86_64_PIT_HZ / X86_64_TIMER_HZ);
  outb(X86_64_PIT_CMD, 0x36U);
  outb(X86_64_PIT_CH0, (unsigned char)(divisor & 0xFFU));
  outb(X86_64_PIT_CH0, (unsigned char)((divisor >> 8) & 0xFFU));
}

status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector) {
  if (boot_info == (const boot_info_t *)0 || out_hz == (BOOT_U64 *)0 || out_irq_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }

  pic_remap();
  pic_unmask_irq0_only();
  pit_program_periodic_100hz();

  *out_hz = X86_64_TIMER_HZ;
  *out_irq_vector = X86_64_TIMER_VECTOR;
  return STATUS_OK;
}

void arch_timer_ack(BOOT_U64 vector) {
  if (vector < 32ULL || vector > 47ULL) {
    return;
  }

  if (vector >= 40ULL) {
    outb(X86_64_PIC2_CMD, X86_64_PIC_EOI);
  }
  outb(X86_64_PIC1_CMD, X86_64_PIC_EOI);
}
