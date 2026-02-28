#include "uefi.h"
#include "kernel.h"

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *g_con_out;

static void uefi_putc(char c) {
  CHAR16 buf[2];

  if (g_con_out == (VOID *)0 || g_con_out->OutputString == (VOID *)0) {
    return;
  }

  if (c == '\n') {
    uefi_putc('\r');
  }

  buf[0] = (CHAR16)(unsigned char)c;
  buf[1] = 0;
  g_con_out->OutputString(g_con_out, buf);
}

void arch_puts(const char *s) {
  while (*s != '\0') {
    uefi_putc(*s++);
  }
}

void arch_halt(void) {
  for (;;) {
  }
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
  (void)image_handle;

  if (system_table == (VOID *)0 || system_table->ConOut == (VOID *)0 ||
      system_table->ConOut->OutputString == (VOID *)0) {
    return (EFI_STATUS)1;
  }

  g_con_out = system_table->ConOut;
  kmain();
  return (EFI_STATUS)0;
}
