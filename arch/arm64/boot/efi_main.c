#include "uefi.h"
#include "kernel.h"

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *g_con_out;
static EFI_MEMORY_DESCRIPTOR g_memory_map[512];

static const EFI_GUID k_gop_guid = {
    0x9042a9deU, 0x23dcU, 0x4a38U, {0x96U, 0xfbU, 0x7aU, 0xdeU, 0xd0U, 0x80U, 0x51U, 0x6aU}};

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
  boot_info_ext_uefi_t uefi_ext;
  EFI_BOOT_SERVICES *boot_services;
  EFI_STATUS status;

  (void)image_handle;

  if (system_table == (VOID *)0 || system_table->ConOut == (VOID *)0 ||
      system_table->ConOut->OutputString == (VOID *)0) {
    return (EFI_STATUS)1;
  }

  g_con_out = system_table->ConOut;
  boot_services = (EFI_BOOT_SERVICES *)system_table->BootServices;
  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.arch_id = BOOT_INFO_ARCH_ARM64;
  boot_info.valid_mask =
      BOOT_INFO_HAS_UEFI_SYSTEM_TABLE | BOOT_INFO_HAS_UEFI_CONFIG_TABLE | BOOT_INFO_HAS_ARCH_DATA;
  boot_info.entry_pc = 0;
  boot_info.entry_sp = 0;
  boot_info.vm_enabled = 0;
  boot_info.vm_root_table = 0;
  boot_info.uefi_system_table = (BOOT_U64)(UINTN)system_table;
  boot_info.uefi_configuration_table = (BOOT_U64)(UINTN)system_table->ConfigurationTable;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  if (boot_services != (VOID *)0 && boot_services->GetMemoryMap != (VOID *)0) {
    UINTN map_size = (UINTN)sizeof(g_memory_map);
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;

    status = boot_services->GetMemoryMap(&map_size, g_memory_map, &map_key, &descriptor_size,
                                         &descriptor_version);
    if (!EFI_ERROR(status)) {
      boot_info.memory_map = (BOOT_U64)(UINTN)g_memory_map;
      boot_info.memory_map_size = (BOOT_U64)map_size;
      boot_info.memory_map_descriptor_size = (BOOT_U64)descriptor_size;
      boot_info.memory_map_descriptor_version = (BOOT_U64)descriptor_version;
      boot_info.valid_mask |= BOOT_INFO_HAS_MEMMAP;
    }
  }
  boot_info.acpi_rsdp = 0;
  boot_info.dtb_ptr = 0;
  boot_info.boot_cpu_id = 0;
  uefi_ext.image_handle = (BOOT_U64)(UINTN)image_handle;
  uefi_ext.system_table = (BOOT_U64)(UINTN)system_table;
  uefi_ext.configuration_table = (BOOT_U64)(UINTN)system_table->ConfigurationTable;
  uefi_ext.boot_services = (BOOT_U64)(UINTN)system_table->BootServices;
  uefi_ext.runtime_services = (BOOT_U64)(UINTN)system_table->RuntimeServices;
  uefi_ext.con_out = (BOOT_U64)(UINTN)system_table->ConOut;
  uefi_ext.std_err = (BOOT_U64)(UINTN)system_table->StdErr;
  uefi_ext.firmware_vendor = (BOOT_U64)(UINTN)system_table->FirmwareVendor;
  uefi_ext.firmware_revision = (BOOT_U64)system_table->FirmwareRevision;
  boot_info.arch_data_ptr = (BOOT_U64)(UINTN)&uefi_ext;
  boot_info.arch_data_size = (BOOT_U64)sizeof(uefi_ext);
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  if (boot_services != (VOID *)0 && boot_services->LocateProtocol != (VOID *)0) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = (VOID *)0;

    status = boot_services->LocateProtocol((EFI_GUID *)&k_gop_guid, (VOID *)0, (VOID **)&gop);
    if (!EFI_ERROR(status) && gop != (VOID *)0 && gop->Mode != (VOID *)0 && gop->Mode->Info != (VOID *)0) {
      boot_info.framebuffer_base = (BOOT_U64)gop->Mode->FrameBufferBase;
      boot_info.framebuffer_width = (BOOT_U32)gop->Mode->Info->HorizontalResolution;
      boot_info.framebuffer_height = (BOOT_U32)gop->Mode->Info->VerticalResolution;
      boot_info.framebuffer_pixels_per_scanline = (BOOT_U32)gop->Mode->Info->PixelsPerScanLine;
      boot_info.framebuffer_format = (BOOT_U32)gop->Mode->Info->PixelFormat;
      boot_info.valid_mask |= BOOT_INFO_HAS_FRAMEBUFFER;
    }
  }
  boot_info.serial_port = 0;
  kmain(&boot_info);
  return (EFI_STATUS)0;
}
