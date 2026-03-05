#include "arch_input.h"
#include "input.h"

#define RISCV64_UART_BASE 0x10000000UL
#define RISCV64_UART_LSR_OFFSET 5UL
#define RISCV64_UART_RX_OFFSET 0UL
#define RISCV64_UART_LSR_DATA_READY 0x01U

static inline u8 riscv64_uart_read(u64 offset) {
  volatile u8 *base = (volatile u8 *)(uptr)RISCV64_UART_BASE;
  return base[offset];
}

status_t arch_input_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }
  return STATUS_OK;
}

void arch_input_poll(void) {
  while ((riscv64_uart_read(RISCV64_UART_LSR_OFFSET) & RISCV64_UART_LSR_DATA_READY) != 0U) {
    char ch = (char)riscv64_uart_read(RISCV64_UART_RX_OFFSET);
    (void)input_push_char(ch);
  }
}
