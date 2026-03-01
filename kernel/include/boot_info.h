#ifndef KERNEL_BOOT_INFO_H
#define KERNEL_BOOT_INFO_H

typedef unsigned long long BOOT_U64;
typedef unsigned int BOOT_U32;

#define BOOT_INFO_ABI_VERSION 1ULL

struct boot_info {
  /* ABI identity/versioning */
  BOOT_U64 abi_version;

  /* CPU entry snapshot at bootloader -> kernel handoff */
  BOOT_U64 entry_rip;
  BOOT_U64 entry_rsp;
  BOOT_U64 paging_enabled;
  BOOT_U64 current_page_map;

  /* Firmware pointers */
  BOOT_U64 uefi_system_table;
  BOOT_U64 uefi_configuration_table;

  /* Memory map (UEFI memory descriptor stream) */
  BOOT_U64 memory_map;
  BOOT_U64 memory_map_size;
  BOOT_U64 memory_map_descriptor_size;
  BOOT_U64 memory_map_descriptor_version;

  /* Platform tables */
  BOOT_U64 acpi_rsdp;

  /* Early console outputs */
  BOOT_U64 framebuffer_base;
  BOOT_U32 framebuffer_width;
  BOOT_U32 framebuffer_height;
  BOOT_U32 framebuffer_pixels_per_scanline;
  BOOT_U32 framebuffer_format;
  BOOT_U64 serial_port;
};

typedef struct boot_info boot_info_t;

#endif
