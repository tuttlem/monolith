#include "kernel.h"

typedef unsigned long u64;
typedef unsigned char u8;

/*
 * OpenBIOS reports /ebus/su reg as 0x3f8.
 * Use physical ASI bypass store to hit UART THR directly.
 */
#define UART_PHYS_BASE 0x1fe020003f8UL

static inline void uart_store_asi(u64 addr, u8 value) {
  asm volatile("stba %0, [%1] 0x14" : : "r"(value), "r"(addr) : "memory");
}

static void arch_putc(char c) {
  if (c == '\n') {
    arch_putc('\r');
  }
  uart_store_asi(UART_PHYS_BASE, (u8)c);
}

void arch_puts(const char *s) {
  while (*s != '\0') {
    arch_putc(*s++);
  }
}

void arch_halt(void) {
  for (;;) {
  }
}
