#include "arch/x86_64/pic.h"
#include "irq_controller.h"

#define X86_64_PIC1_CMD 0x20U
#define X86_64_PIC1_DATA 0x21U
#define X86_64_PIC2_CMD 0xA0U
#define X86_64_PIC2_DATA 0xA1U
#define X86_64_PIC_EOI 0x20U

static void outb(unsigned short port, unsigned char value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
  unsigned char value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

static void io_wait(void) { outb(0x80U, 0U); }

static void pic_remap(void) {
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
}

static status_t pic_enable_irq(BOOT_U64 irq) {
  unsigned short data_port;
  unsigned char mask;
  unsigned char bit;

  if (irq >= 16ULL) {
    return STATUS_INVALID_ARG;
  }

  data_port = (irq < 8ULL) ? X86_64_PIC1_DATA : X86_64_PIC2_DATA;
  bit = (unsigned char)(1U << (irq & 7ULL));
  mask = inb(data_port);
  mask = (unsigned char)(mask & (unsigned char)(~bit));
  outb(data_port, mask);
  return STATUS_OK;
}

static status_t pic_disable_irq(BOOT_U64 irq) {
  unsigned short data_port;
  unsigned char mask;
  unsigned char bit;

  if (irq >= 16ULL) {
    return STATUS_INVALID_ARG;
  }

  data_port = (irq < 8ULL) ? X86_64_PIC1_DATA : X86_64_PIC2_DATA;
  bit = (unsigned char)(1U << (irq & 7ULL));
  mask = inb(data_port);
  mask = (unsigned char)(mask | bit);
  outb(data_port, mask);
  return STATUS_OK;
}

static void pic_ack_irq(BOOT_U64 irq) { (void)irq; }

static void pic_eoi_irq(BOOT_U64 irq) {
  if (irq >= 8ULL) {
    outb(X86_64_PIC2_CMD, X86_64_PIC_EOI);
  }
  outb(X86_64_PIC1_CMD, X86_64_PIC_EOI);
}

static status_t pic_map_irq(BOOT_U64 irq, BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0 || irq >= 16ULL) {
    return STATUS_INVALID_ARG;
  }
  *out_vector = 32ULL + irq;
  return STATUS_OK;
}

static status_t pic_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq) {
  if (out_irq == (BOOT_U64 *)0 || vector < 32ULL || vector > 47ULL) {
    return STATUS_INVALID_ARG;
  }
  *out_irq = vector - 32ULL;
  return STATUS_OK;
}

status_t x86_64_pic_controller_init(const boot_info_t *boot_info) {
  irq_controller_ops_t ops;
  status_t st;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }

  pic_remap();
  outb(X86_64_PIC1_DATA, 0xFFU);
  outb(X86_64_PIC2_DATA, 0xFFU);

  ops.enable_irq = pic_enable_irq;
  ops.disable_irq = pic_disable_irq;
  ops.ack_irq = pic_ack_irq;
  ops.eoi_irq = pic_eoi_irq;
  ops.map_irq = pic_map_irq;
  ops.vector_to_irq = pic_vector_to_irq;

  st = irq_controller_register("x86_64-pic", &ops);
  return st;
}
