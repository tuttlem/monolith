#include "uefi.h"
#include "kernel.h"

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *g_con_out;

static const EFI_GUID k_acpi_20_table_guid = {
    0x8868e871U, 0xe4f1U, 0x11d3U, {0xbcU, 0x22U, 0x00U, 0x80U, 0xc7U, 0x3cU, 0x88U, 0x81U}};
static const EFI_GUID k_acpi_10_table_guid = {
    0xeb9d2d30U, 0x2d88U, 0x11d3U, {0x9aU, 0x16U, 0x00U, 0x90U, 0x27U, 0x3fU, 0xc1U, 0x4dU}};

static int guid_equal(const EFI_GUID *a, const EFI_GUID *b) {
  UINTN i;

  if (a->Data1 != b->Data1 || a->Data2 != b->Data2 || a->Data3 != b->Data3) {
    return 0;
  }

  for (i = 0; i < 8; ++i) {
    if (a->Data4[i] != b->Data4[i]) {
      return 0;
    }
  }

  return 1;
}

static BOOT_U64 find_acpi_rsdp(EFI_SYSTEM_TABLE *system_table) {
  EFI_CONFIGURATION_TABLE *tables;
  UINTN i;

  if (system_table == (VOID *)0 || system_table->ConfigurationTable == (VOID *)0) {
    return 0;
  }

  tables = (EFI_CONFIGURATION_TABLE *)system_table->ConfigurationTable;
  for (i = 0; i < system_table->NumberOfTableEntries; ++i) {
    if (guid_equal(&tables[i].VendorGuid, &k_acpi_20_table_guid) ||
        guid_equal(&tables[i].VendorGuid, &k_acpi_10_table_guid)) {
      return (BOOT_U64)(UINTN)tables[i].VendorTable;
    }
  }

  return 0;
}

static BOOT_U64 read_rip(void) {
  BOOT_U64 rip;
  __asm__ volatile("leaq 0(%%rip), %0" : "=r"(rip));
  return rip;
}

static BOOT_U64 read_rsp(void) {
  BOOT_U64 rsp;
  __asm__ volatile("movq %%rsp, %0" : "=r"(rsp));
  return rsp;
}

static BOOT_U64 read_cr0(void) {
  BOOT_U64 cr0;
  __asm__ volatile("movq %%cr0, %0" : "=r"(cr0));
  return cr0;
}

static BOOT_U64 read_cr3(void) {
  BOOT_U64 cr3;
  __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
  return cr3;
}

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
  BOOT_U64 cr0;

  (void)image_handle;

  if (system_table == (VOID *)0 || system_table->ConOut == (VOID *)0 ||
      system_table->ConOut->OutputString == (VOID *)0) {
    return (EFI_STATUS)1;
  }

  cr0 = read_cr0();
  g_con_out = system_table->ConOut;

  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.entry_rip = read_rip();
  boot_info.entry_rsp = read_rsp();
  boot_info.paging_enabled = (cr0 >> 31) & 1ULL;
  boot_info.current_page_map = read_cr3();
  boot_info.uefi_system_table = (BOOT_U64)(UINTN)system_table;
  boot_info.uefi_configuration_table = (BOOT_U64)(UINTN)system_table->ConfigurationTable;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  boot_info.acpi_rsdp = find_acpi_rsdp(system_table);
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  boot_info.serial_port = 0;

  kmain(&boot_info);
  return (EFI_STATUS)0;
}
