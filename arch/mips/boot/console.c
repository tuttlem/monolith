#include "kernel.h"

typedef unsigned char u8;

#ifndef UART_BASE
#define UART_BASE 0xBFD003F8UL
#endif

static inline void mmio_write_raw(unsigned long addr, u8 value) {
  *(volatile u8 *)addr = value;
}

static void arch_putc(char c) {
  if (c == '\n') {
    arch_putc('\r');
  }

  mmio_write_raw(UART_BASE, (u8)c);
}

void arch_puts(const char *s) {
  while (*s != '\0') {
    arch_putc(*s++);
  }
}

void arch_halt(void) { asm volatile("wait"); }
