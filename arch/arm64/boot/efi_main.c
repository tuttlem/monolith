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
  boot_info_t boot_info;

  (void)image_handle;

  if (system_table == (VOID *)0 || system_table->ConOut == (VOID *)0 ||
      system_table->ConOut->OutputString == (VOID *)0) {
    return (EFI_STATUS)1;
  }

  g_con_out = system_table->ConOut;
  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.entry_rip = 0;
  boot_info.entry_rsp = 0;
  boot_info.paging_enabled = 0;
  boot_info.current_page_map = 0;
  boot_info.uefi_system_table = (BOOT_U64)(UINTN)system_table;
  boot_info.uefi_configuration_table = (BOOT_U64)(UINTN)system_table->ConfigurationTable;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  boot_info.acpi_rsdp = 0;
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  boot_info.serial_port = 0;
  kmain(&boot_info);
  return (EFI_STATUS)0;
}
