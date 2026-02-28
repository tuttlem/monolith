#include "kernel.h"

typedef unsigned char u8;

#define UART_BASE_ADDR 0x10000000UL

static inline void mmio_write(u8 value, unsigned long offset) {
  volatile u8 *base = (volatile u8 *)UART_BASE_ADDR;
  base[offset] = value;
}

static inline u8 mmio_read(unsigned long offset) {
  volatile u8 *base = (volatile u8 *)UART_BASE_ADDR;
  return base[offset];
}

static void arch_putc(char c) {
  if (c == '\n') {
    arch_putc('\r');
  }

  while ((mmio_read(5) & 0x20u) == 0u) {
  }

  mmio_write((u8)c, 0);
}

void arch_puts(const char *s) {
  while (*s != '\0') {
    arch_putc(*s++);
  }
}

void arch_halt(void) { asm volatile("wfi"); }
