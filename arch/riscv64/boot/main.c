#include "kernel.h"

void arch_main(void) {
  boot_info_t boot_info;
  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.entry_rip = 0;
  boot_info.entry_rsp = 0;
  boot_info.paging_enabled = 0;
  boot_info.current_page_map = 0;
  boot_info.uefi_system_table = 0;
  boot_info.uefi_configuration_table = 0;
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
}
