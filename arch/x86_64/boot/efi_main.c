#include "uefi.h"
#include "kernel.h"

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *g_con_out;

static const EFI_GUID k_gop_guid = {
    0x9042a9deU, 0x23dcU, 0x4a38U, {0x96U, 0xfbU, 0x7aU, 0xdeU, 0xd0U, 0x80U, 0x51U, 0x6aU}};

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

static BOOT_U32 uefi_memory_kind(UINT32 uefi_type) {
  switch (uefi_type) {
  case 7U:
    return BOOT_MEM_REGION_USABLE;
  case 9U:
    return BOOT_MEM_REGION_ACPI_RECLAIM;
  case 10U:
    return BOOT_MEM_REGION_ACPI_NVS;
  case 11U:
  case 12U:
    return BOOT_MEM_REGION_MMIO;
  default:
    return BOOT_MEM_REGION_RESERVED;
  }
}

static void add_memory_region(boot_info_t *boot_info, BOOT_U64 base, BOOT_U64 size, BOOT_U32 kind) {
  BOOT_U32 idx;

  if (size == 0 || boot_info->memory_region_count >= boot_info->memory_region_capacity) {
    return;
  }

  idx = boot_info->memory_region_count++;
  boot_info->memory_regions[idx].base = base;
  boot_info->memory_regions[idx].size = size;
  boot_info->memory_regions[idx].kind = kind;
  boot_info->memory_regions[idx].reserved = 0;
}

static void capture_uefi_memory_map(boot_info_t *boot_info, EFI_BOOT_SERVICES *boot_services) {
  EFI_MEMORY_DESCRIPTOR *map = (EFI_MEMORY_DESCRIPTOR *)0;
  EFI_STATUS status;
  UINTN map_size = 0;
  UINTN map_key = 0;
  UINTN descriptor_size = 0;
  UINT32 descriptor_version = 0;
  UINTN i;
  UINTN descriptor_count;
  UINT8 *cursor;

  if (boot_services == (VOID *)0 || boot_services->GetMemoryMap == (VOID *)0 ||
      boot_services->AllocatePool == (VOID *)0) {
    return;
  }

  status = boot_services->GetMemoryMap(&map_size, map, &map_key, &descriptor_size, &descriptor_version);
  if (status != EFI_BUFFER_TOO_SMALL || descriptor_size == 0) {
    return;
  }

  map_size += 2 * descriptor_size;
  status = boot_services->AllocatePool(EfiLoaderData, map_size, (VOID **)&map);
  if (EFI_ERROR(status) || map == (VOID *)0) {
    return;
  }

  status = boot_services->GetMemoryMap(&map_size, map, &map_key, &descriptor_size, &descriptor_version);
  if (EFI_ERROR(status)) {
    if (boot_services->FreePool != (VOID *)0) {
      boot_services->FreePool((VOID *)map);
    }
    return;
  }

  boot_info->memory_map = (BOOT_U64)(UINTN)map;
  boot_info->memory_map_size = (BOOT_U64)map_size;
  boot_info->memory_map_descriptor_size = (BOOT_U64)descriptor_size;
  boot_info->memory_map_descriptor_version = (BOOT_U64)descriptor_version;
  boot_info->valid_mask |= BOOT_INFO_HAS_MEMMAP;

  descriptor_count = map_size / descriptor_size;
  cursor = (UINT8 *)map;
  for (i = 0; i < descriptor_count; ++i) {
    EFI_MEMORY_DESCRIPTOR *d = (EFI_MEMORY_DESCRIPTOR *)(VOID *)cursor;
    BOOT_U64 base = (BOOT_U64)d->PhysicalStart;
    BOOT_U64 size = (BOOT_U64)d->NumberOfPages * 4096ULL;
    add_memory_region(boot_info, base, size, uefi_memory_kind(d->Type));
    cursor += descriptor_size;
  }

  if (boot_info->memory_region_count > 0) {
    boot_info->valid_mask |= BOOT_INFO_HAS_MEM_REGIONS;
  }
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
  boot_info_ext_uefi_t uefi_ext;
  EFI_BOOT_SERVICES *boot_services;
  EFI_STATUS status;
  BOOT_U64 cr0;

  (void)image_handle;

  if (system_table == (VOID *)0 || system_table->ConOut == (VOID *)0 ||
      system_table->ConOut->OutputString == (VOID *)0) {
    return (EFI_STATUS)1;
  }

  cr0 = read_cr0();
  g_con_out = system_table->ConOut;
  boot_services = (EFI_BOOT_SERVICES *)system_table->BootServices;

  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.arch_id = BOOT_INFO_ARCH_X86_64;
  boot_info.valid_mask = BOOT_INFO_HAS_ENTRY_STATE | BOOT_INFO_HAS_VM_STATE |
                         BOOT_INFO_HAS_UEFI_SYSTEM_TABLE | BOOT_INFO_HAS_UEFI_CONFIG_TABLE |
                         BOOT_INFO_HAS_ARCH_DATA;
  boot_info.entry_pc = read_rip();
  boot_info.entry_sp = read_rsp();
  boot_info.vm_enabled = (cr0 >> 31) & 1ULL;
  boot_info.vm_root_table = read_cr3();
  boot_info.uefi_system_table = (BOOT_U64)(UINTN)system_table;
  boot_info.uefi_configuration_table = (BOOT_U64)(UINTN)system_table->ConfigurationTable;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  boot_info.memory_region_count = 0;
  boot_info.memory_region_capacity = BOOT_INFO_MAX_MEM_REGIONS;
  capture_uefi_memory_map(&boot_info, boot_services);
  boot_info.acpi_rsdp = find_acpi_rsdp(system_table);
  if (boot_info.acpi_rsdp != 0) {
    boot_info.valid_mask |= BOOT_INFO_HAS_ACPI_RSDP;
  }
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
