#include "kernel.h"

#ifndef CORE_ARCH_NAME
#define CORE_ARCH_NAME "unknown"
#endif

void kmain(const boot_info_t *boot_info) {
  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ") We good!\n");
  if (boot_info == (const boot_info_t *)0) {
    arch_puts("boot_info: null\n");
  } else {
    kprintf("boot_info.abi_version=0x%llx\n", boot_info->abi_version);
    kprintf("boot_info.entry_rip=0x%llx\n", boot_info->entry_rip);
    kprintf("boot_info.entry_rsp=0x%llx\n", boot_info->entry_rsp);
    kprintf("boot_info.paging_enabled=%u\n", (unsigned)boot_info->paging_enabled);
    kprintf("boot_info.current_page_map=0x%llx\n", boot_info->current_page_map);
    kprintf("boot_info.uefi_system_table=0x%llx\n", boot_info->uefi_system_table);
    kprintf("boot_info.uefi_configuration_table=0x%llx\n", boot_info->uefi_configuration_table);
    kprintf("boot_info.acpi_rsdp=0x%llx\n", boot_info->acpi_rsdp);
    kprintf("boot_info.memory_map=0x%llx size=0x%llx desc_size=0x%llx desc_ver=0x%llx\n",
            boot_info->memory_map, boot_info->memory_map_size,
            boot_info->memory_map_descriptor_size, boot_info->memory_map_descriptor_version);
    kprintf("boot_info.framebuffer=0x%llx %ux%u ppsl=%u fmt=%u\n",
            boot_info->framebuffer_base, boot_info->framebuffer_width,
            boot_info->framebuffer_height, boot_info->framebuffer_pixels_per_scanline,
            boot_info->framebuffer_format);
    kprintf("boot_info.serial_port=0x%llx\n", boot_info->serial_port);
  }

  for (;;) {
    arch_halt();
  }
}
